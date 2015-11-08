/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#include "base/hStringUtil.h"
#include "base/hUUID.h"
#include <functional>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory>
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <stack>
#include <thread>
#include <mutex>
#include <fstream>

#ifdef _WIN32
#   include <io.h>
#   include <fcntl.h>
//#   define popen _popen
//#   define write _write
//#   define close _close
//#   define read _read
#endif

#include "getopt.h"
#include "minfs.h"
#include "cryptoMD5.h"
#include "builder.pb.h"
#include "package.pb.h"
#if defined (_MSC_VER)
#   pragma warning(push)
#   pragma warning(disable:4244)
#   pragma warning(disable:4267)
#endif
#include "google/protobuf/io/zero_copy_stream_impl.h"
#include "google/protobuf/io/coded_stream.h"
#if defined (_MSC_VER)
#   pragma warning(pop)
#endif

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus
#include "lua.h"

#include "lauxlib.h"
#include "lualib.h"
#include <ctype.h>
#ifdef __cplusplus
} //extern "C" {
#endif //__cplusplus

#ifdef _WIN32

#include <windows.h>

static int exec2(const char* cmdline, HANDLE* pstdin, HANDLE* pstdout, HANDLE* pstderr, HANDLE* pid_out, HANDLE* tid_out) {
    HANDLE childStdin_Rd = NULL;
    HANDLE childStdin_Wr = NULL;
    HANDLE childStdout_Rd = NULL;
    HANDLE childStdout_Wr = NULL;
    HANDLE childStdErr_Rd = NULL;
    HANDLE childStdErr_Wr = NULL;
    STARTUPINFO         si;
    PROCESS_INFORMATION pi;
    SECURITY_ATTRIBUTES saAttr;

    // Set the bInheritHandle flag so pipe handles are inherited. 
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;
    saAttr.lpSecurityDescriptor = NULL;

    // Create a pipe for the child process's STDOUT. 
    if (!CreatePipe(&childStdout_Rd, &childStdout_Wr, &saAttr, 0))
        return -1;

    // Ensure the read handle to the pipe for STDOUT is not inherited.
    if (!SetHandleInformation(childStdout_Rd, HANDLE_FLAG_INHERIT, 0))
        return -1;

    // Create a pipe for the child process's STDERR. 
    if (!CreatePipe(&childStdErr_Rd, &childStdErr_Wr, &saAttr, 0))
        return -1;

    // Ensure the read handle to the pipe for STDERR is not inherited.
    if (!SetHandleInformation(childStdErr_Rd, HANDLE_FLAG_INHERIT, 0))
        return -1;

    // Create a pipe for the child process's STDIN. 
    if (!CreatePipe(&childStdin_Rd, &childStdin_Wr, &saAttr, 0))
        return -1;

    // Ensure the write handle to the pipe for STDIN is not inherited. 
    if (!SetHandleInformation(childStdin_Wr, HANDLE_FLAG_INHERIT, 0))
        return -1;

    std::unique_ptr<char[]> cmdline_m(new char[strlen(cmdline)+1]);
    strcpy(cmdline_m.get(), cmdline);

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    si.hStdOutput = childStdout_Wr;
    si.hStdInput = childStdin_Rd;
    si.hStdError = childStdErr_Wr;
    si.dwFlags |= STARTF_USESTDHANDLES;
    ZeroMemory(&pi, sizeof(pi));

    // Start the child process. 
    if (!CreateProcess(NULL,   // No module name (use command line)
        cmdline_m.get(),            // Command line
        NULL,               // Process handle not inheritable
        NULL,               // Thread handle not inheritable
        TRUE,              // Set handle inheritance to FALSE
        0,   // No creation flags
        NULL,               // Use parent's environment block
        NULL,               // Use parent's starting directory 
        &si,                // Pointer to STARTUPINFO structure
        &pi)               // Pointer to PROCESS_INFORMATION structure
        )
    {
        return -1;
    }

    CloseHandle(childStdin_Rd);
    CloseHandle(childStdout_Wr);
    CloseHandle(childStdErr_Wr);
    *pstdin = childStdin_Wr;
    *pstdout = childStdout_Rd;
    *pstderr = childStdErr_Rd;

    *pid_out = pi.hProcess;
    *tid_out = pi.hThread;

    return 0;
}

#endif

#define luaL_errorthrow(L, fmt, ...) \
    luaL_where(L, 1); \
    lua_pushfstring(L, fmt, ##__VA_ARGS__ ); \
    lua_concat(L, 2); \
    lua_getglobal(L, "print"); \
    lua_pushvalue(L, -2); \
    lua_pcall(L, 1, 0, 0); \
    throw std::exception();

#define BUILDER_MAX_PATH (MAX_PATH)

static void print_usage() {
    static const char* progname = "builder";
    printf("%s: ", progname);
    printf(
        "usage: %s [src data path] [dst data path]\n"
        "Available options are:\n"
        "none...yet",
        progname);
}

int handleLuaFileReadError(lua_State* L, int errorCode) {
    if (errorCode == LUA_ERRSYNTAX) {
        printf("Syntax error in builder config script:\n%s\n", lua_tostring(L, -1));
        return EXIT_FAILURE;
    }
    if (errorCode != LUA_OK) {
        printf("Error loading builder const script:\n%s\n", lua_tostring(L, -1));
        return EXIT_FAILURE;
    }
    if (errorCode == LUA_ERRRUN) {
        printf("Builder config script runtime Error:\n%s\n", lua_tostring(L, -1));
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

static std::vector<std::pair<std::string, std::string>> findFilesMatchingWildcard(const char* current_dir, const char* remapped_current_dir, const char* wildcard_path) {
	std::vector<std::pair<std::string, std::string>> found_paths;
	char scratch[BUILDER_MAX_PATH];
	char canoncal_path[BUILDER_MAX_PATH];
	char wildcard_leaf[BUILDER_MAX_PATH];
	char wildcard_parent[BUILDER_MAX_PATH];
	minfs_path_leaf(wildcard_path, wildcard_leaf, BUILDER_MAX_PATH);
	minfs_path_parent(wildcard_path, wildcard_parent, BUILDER_MAX_PATH);
	if (strcmp(wildcard_parent, wildcard_leaf) == 0) {
		wildcard_parent[0] = 0;
	}
	minfs_path_join(current_dir, wildcard_parent, canoncal_path, BUILDER_MAX_PATH);
	auto lambda = std::function<void(const char* origpath, const char* file, void* opaque)>([&](const char* origpath, const char* file, void* opaque) {
		if (Heart::hStrWildcardMatch(wildcard_leaf, file)) {
			char full_path[BUILDER_MAX_PATH];
            char tmp_path[BUILDER_MAX_PATH];
            char resource_id[BUILDER_MAX_PATH];
			minfs_path_join(origpath, file, full_path, BUILDER_MAX_PATH);
            minfs_path_join(remapped_current_dir, wildcard_parent, resource_id, BUILDER_MAX_PATH);
            minfs_path_join(resource_id, file, tmp_path, BUILDER_MAX_PATH);
            minfs_path_without_ext(tmp_path, resource_id, BUILDER_MAX_PATH);

			found_paths.push_back(std::pair<std::string, std::string>(full_path, resource_id));
		}
	});
	minfs_read_directory(canoncal_path, scratch, BUILDER_MAX_PATH, [](const char* origpath, const char* file, void* opaque) {
		(*((std::function<void(const char* origpath, const char* file, void* opaque)>*)opaque))(origpath, file, opaque);
	}, &lambda);
	return found_paths;
}

struct Builder {
    typedef std::unordered_multimap< std::string, std::shared_ptr<Heart::builder::InputParameter> > ResourceParameterTable;
    typedef std::unordered_map< std::string, std::shared_ptr<Heart::builder::Input> > ResourceTable;
    typedef std::unordered_map< std::string, std::vector<std::string> > ResourcePkgTable;
    typedef std::unordered_multimap< std::string, std::shared_ptr<Heart::builder::Input> > ResourceMutliTable;

    std::string rootBuilderScript;
    std::string srcDataPath;
    std::string dstDataPath;
    std::string cacheDataPath;
    int parallelJobCount;
    std::unordered_map<std::string, std::string> jobCommands;
    std::queue< std::shared_ptr<Heart::builder::Input> > jobQueue;
    std::unordered_set<std::string> packages;
	std::unordered_map<std::string, std::vector<std::string> > packageContents;
    //ResourceMutliTable pkgLookup;
    ResourceTable resourceLookup;
    ResourcePkgTable resourcePackageLookup;
    std::stack<ResourceParameterTable> resourceParameters;
    std::stack<std::string> currentDirectory;
    std::stack<std::string> currentResourceDirectory;
    std::atomic_bool fatalError;
    std::atomic_int builtResources;
    bool saveStdIn;

    void push(const char* path, const char* folder) {
        char resource_dir[BUILDER_MAX_PATH];
        currentDirectory.push(path);
        minfs_path_join(currentResourceDirectory.size() ? currentResourceDirectory.top().c_str() : "/", folder, resource_dir, BUILDER_MAX_PATH);
        currentResourceDirectory.push(resource_dir);
        resourceParameters.push(resourceParameters.size() ? resourceParameters.top() : ResourceParameterTable());
    }
    void pop() {
        currentDirectory.pop();
        currentResourceDirectory.pop();
        resourceParameters.pop();
    }
};

static std::vector<std::string> findResourcesMatchingWildcard(const Builder* builder_ctx, const char* current_dir, const char* wildcard_path) {
    std::vector<std::string> found_paths;
    char full_wildcard[BUILDER_MAX_PATH];
    minfs_path_join(current_dir, wildcard_path, full_wildcard, BUILDER_MAX_PATH);
    for (const auto& i : builder_ctx->resourceLookup) {
        if (Heart::hStrWildcardMatch(full_wildcard, i.first.c_str())) {
            found_paths.push_back(i.first);
        }
    }
    return found_paths;
}

static std::shared_ptr<Heart::builder::InputParameter> getInputParameter(lua_State* L, const char* parameter_name, int idx) {
    std::shared_ptr<Heart::builder::InputParameter> input_param(new Heart::builder::InputParameter);
    input_param->set_name(parameter_name);
    switch (lua_type(L, idx)) {
    case LUA_TBOOLEAN: {
        input_param->add_values()->set_boolvalue(!!lua_toboolean(L, idx));
    } break;
    case LUA_TNUMBER: {
        float value = (float)lua_tonumber(L, idx);
        float intpart;
        modff(value, &intpart);
        if (intpart == value) {
            input_param->add_values()->set_intvalue(((int)intpart));
        }
        else {
            input_param->add_values()->set_floatvalue(value);
        }
    } break;
    case LUA_TSTRING: {
        input_param->add_values()->set_strvalue(lua_tostring(L, idx));
    } break;
    case LUA_TTABLE: {
        int i = 0;
        lua_pushnil(L);
        while (lua_next(L, idx < 0 ? idx-1 : idx) != 0) {
            /* uses 'key' (at index -2) and 'value' (at index -1) */
            auto type = lua_type(L, -1);
            if (type == LUA_TBOOLEAN) {
                input_param->add_values()->set_boolvalue(!!lua_toboolean(L,-1));
            } else if (type == LUA_TNUMBER) {
                float value = (float)lua_tonumber(L, -1);
                float intpart;
                modff(value, &intpart);
                if (intpart == value) {
                    input_param->add_values()->set_intvalue(((int)intpart));
                }
                else {
                    input_param->add_values()->set_floatvalue(value);
                }
            } else if (type == LUA_TSTRING) {
                input_param->add_values()->set_strvalue(lua_tostring(L, -1));
            } else {
                luaL_errorthrow(L, "Cannot set parameter with type %s", lua_typename(L, type));
            }

            /* removes 'value'; keeps 'key' for next iteration */
            lua_pop(L, 1);
            ++i;
        }
    } break;
    default:
        luaL_errorthrow(L, "Cannot set parameter with type %s", lua_typename(L, lua_type(L, idx)));
    }

    return input_param;
}

int add_build_folder(lua_State* L) {
    Builder* b = (Builder*)lua_topointer(L, lua_upvalueindex(1));
    char tmp_path[BUILDER_MAX_PATH];
    char canonical_path[BUILDER_MAX_PATH];
    char canonical_buildscript[BUILDER_MAX_PATH];
    const char* folderpath = luaL_checkstring(L, 1);
    minfs_path_join(b->currentDirectory.top().c_str(), folderpath, tmp_path, BUILDER_MAX_PATH);
    minfs_canonical_path(tmp_path, canonical_path, BUILDER_MAX_PATH);
    minfs_path_join(canonical_path, "/.build_script", canonical_buildscript, BUILDER_MAX_PATH);
    if (!minfs_path_exist(canonical_buildscript)) {
        return luaL_error(L, "%s is missing a build folder. %s doesn't exist", canonical_path, canonical_buildscript);
    }
    b->push(canonical_path, folderpath);
    int result = luaL_dofile(L, canonical_buildscript);
    if (handleLuaFileReadError(L, result)) {
        return luaL_error(L, "error parsing build script %s", canonical_buildscript);
    }
    b->pop();
    return 0;
}

int add_resources_to_package(lua_State* L) {
	Builder* b = (Builder*)lua_topointer(L, lua_upvalueindex(1));
	const char* package_name = luaL_checkstring(L, 1);
try {
	int i=2;
	for (; i <= lua_gettop(L); ++i) {
		if (!lua_isstring(L,i)) {
			luaL_errorthrow(L, "bad argument %d: string expected, got %s", i, luaL_typename(L, i));
		}
		const char* resource_wildcard = lua_tostring(L, i);
		auto foundFiles = findResourcesMatchingWildcard(b, b->currentResourceDirectory.top().c_str(), resource_wildcard);
        auto& pkg_list =  b->packageContents[package_name];
        for (const auto& rid : foundFiles){
            // Constanst lookup Perf issue?
            pkg_list.push_back(rid);
            b->resourcePackageLookup[rid].push_back(package_name);
        }
	}
	lua_pop(L, 1);
} catch (std::exception e) {
	return lua_error(L);
}
	return 0;
}
int set_type_parameter(lua_State* L) {
    Builder* b = (Builder*)lua_topointer(L, lua_upvalueindex(1));
    const char* resource_name = luaL_checkstring(L, 1);
    const char* parameter_name = luaL_checkstring(L, 2);
try {
    std::shared_ptr<Heart::builder::InputParameter> input_param = getInputParameter(L, parameter_name, 3);
    b->resourceParameters.top().insert(Builder::ResourceParameterTable::value_type(resource_name, input_param));
} catch (std::exception e) {
    return lua_error(L);
}
    return 0;
}
int add_files(lua_State* L) {
    Builder* b = (Builder*)lua_topointer(L, lua_upvalueindex(1));
    const char* wildcard_path = luaL_checkstring(L, 1);
    const char* resource_name = luaL_checkstring(L, 2);
    const char* resource_postfix = nullptr;
    if (lua_isstring(L, 4)) {
        resource_postfix = lua_tostring(L, 4);
    }
try {
    char scratch[BUILDER_MAX_PATH];
    char canoncal_path[BUILDER_MAX_PATH];
    char wildcard_leaf[BUILDER_MAX_PATH];
    char wildcard_parent[BUILDER_MAX_PATH];
    minfs_path_leaf(wildcard_path, wildcard_leaf, BUILDER_MAX_PATH);
    minfs_path_parent(wildcard_path, wildcard_parent, BUILDER_MAX_PATH);
    if (strcmp(wildcard_parent, wildcard_leaf) == 0) {
        wildcard_parent[0] = 0;
    }
	//TODO: Replace this call with findFilesMatchingWildcard(); -> D.R.Y.
    minfs_path_join(b->currentDirectory.top().c_str(), wildcard_parent, canoncal_path, BUILDER_MAX_PATH);
    auto lambda = std::function<void(const char* origpath, const char* file, void* opaque)>([&](const char* origpath, const char* file, void* opaque) {
        if (Heart::hStrWildcardMatch(wildcard_leaf, file)) {
            char resource_path[BUILDER_MAX_PATH];
            char tmp_path[BUILDER_MAX_PATH];
            char resource_id[BUILDER_MAX_PATH];
            minfs_path_join(origpath, file, resource_path, BUILDER_MAX_PATH);
            minfs_path_join(b->currentResourceDirectory.top().c_str(), wildcard_parent, resource_id, BUILDER_MAX_PATH);
            minfs_path_join(resource_id, file, tmp_path, BUILDER_MAX_PATH);
            minfs_path_without_ext(tmp_path, resource_id, BUILDER_MAX_PATH);
            if (resource_postfix) {
                strncat(resource_id, resource_postfix, BUILDER_MAX_PATH);
            }
            std::shared_ptr<Heart::builder::Input> new_input(new Heart::builder::Input());
            new_input->set_resourceinputpath(resource_path);
            new_input->set_resourcetype(resource_name);
            new_input->set_resourceid(resource_id);
            new_input->set_resourcedatarootpath(b->srcDataPath);
            
            auto parameter_range = b->resourceParameters.top().equal_range(new_input->resourcetype());
            for (auto p = parameter_range.first; p != parameter_range.second; ++p) {
                auto* bp = new_input->add_buildparameters();
                bp->CopyFrom(*p->second);
            }

            if (lua_istable(L, 3)) {
                int i = 0;
                lua_pushnil(L);
                while (lua_next(L, 3) != 0) {
                    /* uses 'key' (at index -2) and 'value' (at index -1) */
                    if (!lua_isstring(L, -2)) {
                        luaL_errorthrow(L, "parameter table keys must be strings");
                    }
                    auto* bp = new_input->add_buildparameters();
                    bp->CopyFrom(*getInputParameter(L, lua_tostring(L, -2), -1));
                    /* removes 'value'; keeps 'key' for next iteration */
                    lua_pop(L, 1);
                    ++i;
                }
            }

            b->jobQueue.push(new_input);
            b->resourceLookup.insert(Builder::ResourceTable::value_type(new_input->resourceid(), new_input));
            //b->pkgLookup.insert(Builder::ResourceTable::value_type(new_input->package(), new_input));
            //if (b->packages.find(new_input->package()) == b->packages.end()) {
            //    b->packages.insert(new_input->package());
            //}
        }
    });
    minfs_read_directory(canoncal_path, scratch, BUILDER_MAX_PATH, [](const char* origpath, const char* file, void* opaque) {
        (*((std::function<void(const char* origpath, const char* file, void* opaque)>*)opaque))(origpath, file, opaque);
    }, &lambda);
} catch (std::exception e) {
    return lua_error(L);
}
    return 0;
}

struct LuaStackReset {
    LuaStackReset(lua_State* inL) : L(inL) {
        top = lua_gettop(L);
    }
    ~LuaStackReset() {
        lua_settop(L, top);
    }
    lua_State* L;
    int top;
};

static std::shared_ptr<Builder> open_builder_lib(lua_State* L) {
    LuaStackReset sentry(L);
    auto builder = std::shared_ptr<Builder>(new Builder());
    builder->fatalError.store(true);
    builder->builtResources.store(0);
    lua_getglobal(L, "srcDataPath");
    if (!lua_isstring(L, -1)) {
        fprintf(stderr, "srcDataPath is not a string");
        return nullptr;
    }
    builder->srcDataPath = lua_tostring(L, -1);
    lua_getglobal(L, "dstDataPath");
    builder->dstDataPath = lua_tostring(L, -1);
    lua_getglobal(L, "cacheDataPath");
    builder->cacheDataPath = lua_tostring(L, -1);
    lua_getglobal(L, "parallelJobCount");
    builder->parallelJobCount = (int)lua_tointeger(L, -1);
    builder->rootBuilderScript = builder->srcDataPath + "/.build_script";
    lua_getglobal(L, "saveStdIn");
    builder->saveStdIn = lua_isboolean(L, -1) && lua_toboolean(L, -1);

    lua_getglobal(L, "dataBuilders");
    if (!lua_istable(L, -1)) {
        fprintf(stderr, "\"dataBuilders\" is not a table");
        return nullptr;
    }

    int i = 0;
    lua_pushnil(L);
    while (lua_next(L, -2) != 0) {
        /* uses 'key' (at index -2) and 'value' (at index -1) */
        if (!lua_isstring(L, -2) || !lua_isstring(L, -1)) {
            fprintf(stderr, "both key and value in dataBuilders must be strings");
            return nullptr;
        }

        builder->jobCommands[lua_tostring(L, -2)] = lua_tostring(L, -1);

        /* removes 'value'; keeps 'key' for next iteration */
        lua_pop(L, 1);
        ++i;
    }
    lua_pop(L, 1);

    //expose the builder library to Lua
    luaL_Reg builder_funcs[] = {
        {"add_build_folder", add_build_folder},
        //{"set_current_package", set_current_package},
        {"add_resources_to_package", add_resources_to_package},
        {"set_type_parameter", set_type_parameter},
        {"add_files", add_files},
        {NULL,NULL}
    };

    lua_newtable(L);
    lua_pushlightuserdata(L, builder.get());
    luaL_setfuncs(L, builder_funcs, 1);
    lua_setglobal(L, "buildsystem");

    if (!minfs_path_exist(builder->cacheDataPath.c_str())) {
        minfs_create_directories(builder->cacheDataPath.c_str());
    }
    if (!minfs_path_exist(builder->dstDataPath.c_str())) {
        minfs_create_directories(builder->dstDataPath.c_str());
    }

    builder->fatalError.store(false);
    return builder;
}

std::unordered_map<std::string, std::string> fileMD5Map;
typedef std::unordered_map<std::string, std::string>::value_type MD5Pair;

void md5_filereader(const char* origpath, const char* file, void* user) {
	char fullpath[BUILDER_MAX_PATH];
	minfs_path_join(origpath, file, fullpath, BUILDER_MAX_PATH);
	if (minfs_is_directory(fullpath)) {
		char inner_scratch[4096];
		minfs_read_directory(fullpath, inner_scratch, sizeof(inner_scratch), md5_filereader, user);
	} else {
		cyMD5_CTX md5;
		cyMD5Init(&md5);
		auto fsize = minfs_get_file_size(fullpath);
		FILE* f = fopen(fullpath, "rb");
		if (f) {
			cyByte md5digest[CY_MD5_LEN];
			char md5str[CY_MD5_STR_LEN]={0};
			std::unique_ptr<char[]> fdata(new char[fsize]);
			fread(fdata.get(), 1, fsize, f);
			fclose(f);
			cyMD5Update(&md5, fdata.get(), (cyUint)fsize);
			cyMD5Final(&md5, md5digest);
			cyMD5DigestToString(md5digest, md5str);
			fileMD5Map.insert(MD5Pair(fullpath, md5str));
		} else {
			fprintf(stderr, "Unable to generate MD5 for %s\n", fullpath);
		}
	}
}

int main (int argc, char **argv) {
    int result = EXIT_SUCCESS;
    int verbose = 0;
    int c;
    int option_index;
    int generate_guid = false;
    bool clean_build = false;
    std::string config_script = ".build_config";
    const char argopts[] = "vg:h";
    static struct option long_options[] = {
        { "version", no_argument, 0, (int)'v' },
        { "clean", no_argument, 0, (int)'c' },
        { "force", no_argument, 0, (int)'c' },
        { "guid", no_argument, 0, (int)'i' },
        { 0, 0, 0, 0 }
    };
    lua_State *L = luaL_newstate();  /* create state */
    luaL_openlibs(L);  /* open libraries */

    while ((c = gop_getopt_long(argc, argv, argopts, long_options, &option_index)) != -1) {
        switch(c) {
        case 'v': verbose = 1; break;
        case 'g': config_script = optarg; break;
        case 'c': clean_build = true; break;
        case 'i': generate_guid = true; break;
        case '?':
            if (strchr(argopts, optopt))
                fprintf (stderr, "Option -%c requires an argument.\n", optopt);
            else if (isalpha(optopt))
                fprintf (stderr, "Unknown option `-%c'.\n", optopt);
            else
                fprintf (stderr, "Unknown option character `\\x%x'.\n", optopt);
            result = EXIT_FAILURE;
        case 'h':
            print_usage();
            return result;
        }
    }

#ifdef _WIN32
    _setmode(_fileno(stdin), _O_BINARY);
    _setmode(_fileno(stdout), _O_BINARY);
    _setmode(_fileno(stderr), _O_BINARY);
#endif

    if (generate_guid) {
        hUuid_t guid = Heart::hUUID::generateUUID();
        char guid_str[256];
        Heart::hUUID::toString(guid, guid_str, sizeof(guid_str));
        fprintf(stdout, "New GUID: %s\n", guid_str);
        return 0;
    }

    if (L == NULL) {
        fprintf(stderr, "cannot create state: not enough memory");
        return EXIT_FAILURE;
    }

    result = luaL_dofile(L, config_script.c_str());
    if (handleLuaFileReadError(L, result)) {
        return EXIT_FAILURE;
    }

	auto builder_ctx = open_builder_lib(L);
	auto file_md5_worker = std::thread([&](const std::string& src_dir){
		char scratch[4096];
		minfs_read_directory(src_dir.c_str(), scratch, sizeof(scratch), md5_filereader, nullptr);
	}, builder_ctx->srcDataPath);

    builder_ctx->push(builder_ctx->srcDataPath.c_str(), "");
    result = luaL_dofile(L, builder_ctx->rootBuilderScript.c_str());
    if (handleLuaFileReadError(L, result)) {
        return EXIT_FAILURE;
    }
    builder_ctx->pop();

	file_md5_worker.join();

    //Spawn worker threads
    std::mutex mtx;
    std::vector<std::thread> workers;
    for (auto i=0; i < builder_ctx->parallelJobCount; ++i) {
        workers.push_back(std::thread([&](){
            std::shared_ptr<Heart::builder::Input> job;
            for (;;) { {
                    std::unique_lock<std::mutex> lck(mtx);
                    if (builder_ctx->jobQueue.size() == 0) {
                        return;
                    }
                    job = builder_ctx->jobQueue.front();
                    builder_ctx->jobQueue.pop();
                }
                bool build_required = true;
                char cached_res_path[BUILDER_MAX_PATH];
                char tmp_path[BUILDER_MAX_PATH];
                std::string resource_path = job->resourceid();
                std::replace(resource_path.begin(), resource_path.end(), '/', '_');
                minfs_path_join(builder_ctx->cacheDataPath.c_str(), job->package().c_str(), tmp_path, BUILDER_MAX_PATH);
                minfs_create_directories(tmp_path);
                minfs_path_join(tmp_path, resource_path.c_str(), cached_res_path, BUILDER_MAX_PATH);
                std::string res_stdin_path = cached_res_path;
                res_stdin_path += ".stdin";
                std::string res_stdout_path = cached_res_path;
                res_stdout_path += ".stdout";
                std::string res_stderr_path = cached_res_path;
                res_stderr_path += ".stderr";
                if (minfs_path_exist(res_stdout_path.c_str()) && !clean_build) {
                    std::ifstream in_data;
                    in_data.open(res_stdout_path, std::ios_base::in | std::ios_base::binary);
                    if (in_data.is_open()) {
                        Heart::builder::Output test_output;
                        test_output.ParseFromIstream(&in_data);
                        build_required = test_output.filetimestamps_size() == 0;
                        for (int i = 0, n = test_output.filetimestamps_size(); i < n; ++i) {
                            auto it = fileMD5Map.find(test_output.filetimestamps(i).filepath());
                            if (it == fileMD5Map.end() || !test_output.filetimestamps(i).has_hash() || it->second != test_output.filetimestamps(i).hash()) {
                                //fprintf(stderr, "Building %s because it doesn't match cache (%s, %s)\n", resource_path.c_str(), test_output.filetimestamps(i).filepath().c_str(), test_output.filetimestamps(i).hash().c_str());
                                build_required = true;
                                break;
                            }
                        }
                    }
                }
                if (build_required || clean_build) {
                    if (builder_ctx->saveStdIn) {
                        std::ofstream output;
                        output.open(res_stdin_path.c_str(), std::ios_base::out | std::ios_base::binary);
                        if (!output.is_open()) {
                            fprintf(stderr, "Couldn't open %s for writing\n", res_stdin_path.c_str());
                        } else {
                            job->SerializeToOstream(&output);
                        }
                    }
                    auto cmd = builder_ctx->jobCommands[job->resourcetype()];
                    HANDLE pstdin, pstdout, pstderr, pid, tid;
                    if (exec2(cmd.c_str(), &pstdin, &pstdout, &pstderr, &pid, &tid) < 0) {
                        fprintf(stderr, "Error beginning command '%s' for type '%s'\n", cmd.c_str(), job->resourcetype().c_str());
                        continue;
                    }

                    auto str = job->SerializeAsString();
                    DWORD written;
                    WriteFile(pstdin, str.c_str(), (int)str.size(), &written, nullptr);
                    CloseHandle(pstdin);
                    BOOL success;
                    DWORD bytesread;
                    std::string finaloutput;
                    do{
                        char buf[4096];
                        success = ReadFile(pstdout, buf, sizeof(buf), &bytesread, nullptr);
                        if (success == FALSE && GetLastError() == ERROR_MORE_DATA) {
                            success = TRUE;
                        }
                        finaloutput.append(buf, bytesread);
                    } while (success);
                    CloseHandle(pstdout);

                    std::string stderroutput;
                    do{
                        char buf[4096];
                        success = ReadFile(pstderr, buf, sizeof(buf), &bytesread, nullptr);
                        if (success == FALSE && GetLastError() == ERROR_MORE_DATA) {
                            success = TRUE;
                        }
                        stderroutput.append(buf, bytesread);
                    } while (success);
                    CloseHandle(pstderr);

                    DWORD exit_code;
                    do {
                        GetExitCodeProcess(pid, &exit_code);
                        Sleep(10);
                    } while (exit_code == STILL_ACTIVE);
                    CloseHandle(tid);
                    CloseHandle(pid);

                    std::ofstream err_output;
                    err_output.open(res_stderr_path.c_str(), std::ios_base::out | std::ios_base::binary);
                    if (err_output.is_open()) {
                        err_output << stderroutput;
                    }

                    fprintf(stderr, "%s\n", stderroutput.c_str());
                    if (exit_code != 0) {
                        fprintf(stderr, "Error running build command '%s' for resource '%s'. Returned error code %d\n", cmd.c_str(), job->resourceinputpath().c_str(), exit_code);
                        continue;
                    }

                    Heart::builder::Output test_output;
                    test_output.ParseFromString(finaloutput);

                    for (int i=0, n=test_output.filedependency_size(); i < n; ++i) {
                        auto* stamp = test_output.add_filetimestamps();
                        auto dep_filepath = test_output.filedependency(i);
                        char canon_path[BUILDER_MAX_PATH];
                        minfs_canonical_path(dep_filepath.c_str(), canon_path, BUILDER_MAX_PATH);
                        stamp->set_filepath(canon_path);
                        auto it = fileMD5Map.find(canon_path);
                        if (it != fileMD5Map.end()) {
                            stamp->set_hash(it->second);
                        }
                    }

                    std::ofstream res_output;
                    res_output.open(res_stdout_path.c_str(), std::ios_base::out | std::ios_base::binary);
                    if (!res_output.is_open()) {
                        fprintf(stderr, "Couldn't open %s for writing\n", res_stdout_path.c_str());
                    } else {
                        test_output.SerializeToOstream(&res_output);
                        job->set_runtimetype(test_output.pkgdata().type_name());
                    }
                }
                job->set_builtresourcepath(res_stdout_path);
                ++builder_ctx->builtResources;
            }
        }));
    }
        
    for (auto& i : workers) {
        i.join();
    }

    builder_ctx->fatalError.store(builder_ctx->builtResources.load() != builder_ctx->resourceLookup.size());

    if (!builder_ctx->fatalError.load()) {
        for (const auto& pkg : builder_ctx->packageContents) {
            std::vector<std::shared_ptr<Heart::builder::Input>> pkg_resources_to_write;
            for (const auto& resource : pkg.second) {
                const auto i = builder_ctx->resourceLookup.find(resource);
                if (i != builder_ctx->resourceLookup.end()) {
                    pkg_resources_to_write.push_back(i->second);
                }
            }
            if (pkg_resources_to_write.size() == 0) {
                continue;
            }
            std::sort(pkg_resources_to_write.begin(), pkg_resources_to_write.end(), [](const std::shared_ptr<Heart::builder::Input>& lhs, const std::shared_ptr<Heart::builder::Input>& rhs){
                return lhs->resourceid() < rhs->resourceid();
            });
            Heart::proto::PackageHeader header;
            std::unordered_set<std::string> dep_pkgs;
            uint64_t offset = 0;
            for (const auto& i : pkg_resources_to_write) {
                //TODO: add dep array
                std::ifstream res_file;
                res_file.open(i->builtresourcepath(), std::ios_base::in | std::ios_base::binary);
                if (!res_file.is_open()) {
                    fprintf(stderr, "Couldn't open %s for reading", i->builtresourcepath().c_str());
                    builder_ctx->fatalError.store(true);
                    break;  
                }
                Heart::builder::Output resource_data;
                resource_data.ParseFromIstream(&res_file);
                auto filesize = resource_data.pkgdata().ByteSize();
                auto* entry = header.add_entries();
                entry->set_entryname(i->resourceid());
                entry->set_entryoffset(offset);
                entry->set_entrysize(filesize);
                entry->set_entrytype(i->runtimetype());
                offset += filesize;
                for (int di = 0, n = resource_data.resourcedependency_size(); di < n; ++di) {
                    auto dr = builder_ctx->resourcePackageLookup.find(resource_data.resourcedependency(di));
                    if (dr != builder_ctx->resourcePackageLookup.end()) {
                        if (dr->second.size() > 1) {
                            fprintf(stderr, "WARNING: Found resource dependent resource '%s' for resource '%s' in multiple packages. Defaulting to loading first found package at runtime (which is %s).\n", 
                                resource_data.resourcedependency(di).c_str(), i->resourceid().c_str(), dr->second[0].c_str());
                        }
                        dep_pkgs.insert(dr->second[0]);
                    } else {
                        fprintf(stderr, "WARNING: Unable to find dependent resource %s for resource %s\n", resource_data.resourcedependency(di).c_str(), i->resourceid().c_str());
                    }
                }
            }

            for (const auto& i : dep_pkgs) {
                header.add_packagedependencies(i);
            }

            char output_path[BUILDER_MAX_PATH];
            std::string package_filename = pkg.first;
            package_filename += ".pkg";
            minfs_path_join(builder_ctx->dstDataPath.c_str(), package_filename.c_str(), output_path, BUILDER_MAX_PATH);
            std::ofstream output;
            output.open(output_path, std::ios_base::out | std::ios_base::binary);
            if (!output.is_open()) {
                fprintf(stderr, "Couldn't open %s for writing", output_path);
                builder_ctx->fatalError.store(true);
                break;
            }
            std::vector<char> data;
            google::protobuf::io::OstreamOutputStream filestream(&output);
            google::protobuf::io::CodedOutputStream outputstream(&filestream);
            auto header_out = header.SerializeAsString();
            outputstream.WriteVarint32((uint32_t)header_out.size());
            outputstream.WriteRaw(header_out.c_str(), (uint32_t)header_out.size());
            for (const auto& i : pkg_resources_to_write) {
                    std::ifstream res_file;
                    res_file.open(i->builtresourcepath(), std::ios_base::in | std::ios_base::binary);
                    if (!res_file.is_open()) {
                        fprintf(stderr, "Couldn't open %s for reading", i->builtresourcepath().c_str());
                        builder_ctx->fatalError.store(true);
                        break;
                    }
                    Heart::builder::Output resource_data;
                    resource_data.ParseFromIstream(&res_file);
                    auto res_filesize = resource_data.pkgdata().ByteSize();
                    auto data_to_load = resource_data.pkgdata().SerializeAsString();
                    outputstream.WriteRaw(data_to_load.data(), (uint32_t)data_to_load.size());
            }
        }
    }

    lua_close(L);

    result = builder_ctx->fatalError.load() ? EXIT_FAILURE : EXIT_SUCCESS;

    return (result == LUA_OK) ? EXIT_SUCCESS : EXIT_FAILURE;
}

