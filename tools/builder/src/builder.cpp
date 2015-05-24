/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#include "base/hStringUtil.h"
#include <functional>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory>
#include <queue>
#include <unordered_map>
#include <stack>

#ifdef _WIN32
#   include <io.h>
#   include <fcntl.h>
#   define popen _popen
#   define write _write
#   define close _close
#   define read _read
#endif

#include "getopt.h"
#include "lua/builder_script.inl"
#include "minfs.h"
#include "builder.pb.h"

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

static int exec2(char* cmdline, HANDLE* pstdin, HANDLE* pstdout) {
    HANDLE childStdin_Rd = NULL;
    HANDLE childStdin_Wr = NULL;
    HANDLE childStdout_Rd = NULL;
    HANDLE childStdout_Wr = NULL;
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

    // Create a pipe for the child process's STDIN. 
    if (!CreatePipe(&childStdin_Rd, &childStdin_Wr, &saAttr, 0))
        return -1;

    // Ensure the write handle to the pipe for STDIN is not inherited. 
    if (!SetHandleInformation(childStdin_Wr, HANDLE_FLAG_INHERIT, 0))
        return -1;


    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    si.hStdError = childStdout_Wr;
    si.hStdOutput = childStdout_Wr;
    si.hStdInput = childStdin_Rd;
    si.dwFlags |= STARTF_USESTDHANDLES;
    ZeroMemory(&pi, sizeof(pi));

    // Start the child process. 
    if (!CreateProcess(NULL,   // No module name (use command line)
        cmdline,            // Command line
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

    *pstdin = childStdin_Wr;
    *pstdout = childStdout_Rd;

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
        printf("Syntax error in builder config script:\n%s", lua_tostring(L, -1));
        return EXIT_FAILURE;
    }
    if (errorCode != LUA_OK) {
        printf("Error loading builder const script:\n%s", lua_tostring(L, -1));
        return EXIT_FAILURE;
    }
    if (errorCode == LUA_ERRRUN) {
        printf("Builder config script runtime Error:\n%s", lua_tostring(L, -1));
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

struct Builder {
    typedef std::unordered_multimap< std::string, std::shared_ptr<Heart::builder::InputParameter> > ResourceParameterTable;
    typedef std::unordered_map< std::string, std::shared_ptr<Heart::builder::Input> > ResourceTable;

    std::string rootBuilderScript;
    std::string srcDataPath;
    std::string dstDataPath;
    std::string cacheDataPath;
    int parallelJobCount;
    std::unordered_map<std::string, std::string> jobCommands;
    std::queue< std::shared_ptr<Heart::builder::Input> > jobQueue;
    ResourceTable pkgLookup;
    ResourceTable resourceLookup;
    std::stack<std::string> currentPackage;
    std::stack<ResourceParameterTable> resourceParameters;
    std::stack<std::string> currentDirectory;

    void push(const char* path) {
        currentDirectory.push(path);
        currentPackage.push(currentPackage.size() ? currentPackage.top() : std::string());
        resourceParameters.push(resourceParameters.size() ? resourceParameters.top() : ResourceParameterTable());
    }
    void pop() {
        currentDirectory.pop();
        currentPackage.pop();
        resourceParameters.pop();
    }
};

static std::shared_ptr<Heart::builder::InputParameter> getInputParameter(lua_State* L, const char* parameter_name, int idx) {
    std::shared_ptr<Heart::builder::InputParameter> input_param(new Heart::builder::InputParameter);
    input_param->set_name(parameter_name);
    switch (lua_type(L, idx)) {
    case LUA_TBOOLEAN: {
        input_param->add_values()->set_boolvalue(!!lua_toboolean(L, 3));
    } break;
    case LUA_TNUMBER: {
        float value = (float)lua_tonumber(L, 3);
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
        input_param->add_values()->set_strvalue(lua_tostring(L, 3));
    } break;
    case LUA_TTABLE: {
        int i = 0;
        lua_pushnil(L);
        while (lua_next(L, -2) != 0) {
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
        lua_pop(L, 1);
    } break;
    default:
        luaL_errorthrow(L, "Cannot set parameter with type %s", lua_typename(L, lua_type(L, 3)));
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
    b->push(canonical_path);
    int result = luaL_dofile(L, canonical_buildscript);
    if (handleLuaFileReadError(L, result)) {
        return luaL_error(L, "error parsing build script %s", canonical_buildscript);
    }
    b->pop();
    return 0;
}
int set_current_package(lua_State* L) {
    Builder* b = (Builder*)lua_topointer(L, lua_upvalueindex(1));
    b->currentPackage.top() = luaL_checkstring(L, 1);
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
    minfs_path_join(b->currentDirectory.top().c_str(), wildcard_parent, canoncal_path, BUILDER_MAX_PATH);
    auto lambda = std::function<void(const char* origpath, const char* file, void* opaque)>([&](const char* origpath, const char* file, void* opaque) {
        if (Heart::hStrWildcardMatch(wildcard_leaf, file)) {
            fprintf(stderr, "will add resource %s/%s\n", origpath, file);
            char resource_path[BUILDER_MAX_PATH];
            minfs_path_join(origpath, file, resource_path, BUILDER_MAX_PATH);
            std::shared_ptr<Heart::builder::Input> new_input(new Heart::builder::Input());
            new_input->set_resourceinputpath(resource_path);
            new_input->set_resourcetype(resource_name);
            new_input->set_package(b->currentPackage.top());
            
            auto parameter_range = b->resourceParameters.top().equal_range(new_input->resourcetype());
            for (auto p = parameter_range.first; p != parameter_range.second; ++p) {
                auto* bp = new_input->add_buildparameters();
                bp->CopyFrom(*p->second);
            }

            b->jobQueue.push(new_input);
            b->resourceLookup.insert(Builder::ResourceTable::value_type(new_input->resourceinputpath(), new_input));
            b->pkgLookup.insert(Builder::ResourceTable::value_type(new_input->package(), new_input));
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
        {"set_current_package", set_current_package},
        {"set_type_parameter", set_type_parameter},
        {"add_files", add_files},
        {NULL,NULL}
    };

    lua_newtable(L);
    lua_pushlightuserdata(L, builder.get());
    luaL_setfuncs(L, builder_funcs, 1);
    lua_setglobal(L, "buildsystem");

    return builder;
}

static int do_build(Builder* ) {
}

int main (int argc, char **argv) {
    int result = EXIT_SUCCESS;
    int verbose = 0, corecount = 2;
    int c;
    char* srcpath = NULL, *cachepath = NULL, *destpath = NULL; // we leak these
    const char argopts[] = "vj:s:d:c:abxn";
    bool version_2 = false;
    bool test_write = false;
    bool test_read = false;
    bool dry_run = false;
    lua_State *L = luaL_newstate();  /* create state */
    luaL_openlibs(L);  /* open libraries */

    while ((c = gop_getopt(argc, argv, argopts)) != -1) {
        switch(c) {
        case 'v': verbose = 1; break;
        case 's':
            srcpath = (char*)malloc(strlen(optarg)+1);
            if (!srcpath) return EXIT_FAILURE;
            strcpy(srcpath, optarg);
            break;
        case 'd':
            destpath = (char*)malloc(strlen(optarg)+1);
            if (!destpath) return EXIT_FAILURE;
            strcpy(destpath, optarg);
            break;
		case 'c':
			cachepath = (char*)malloc(strlen(optarg) + 1);
			if (!cachepath) return EXIT_FAILURE;
			strcpy(cachepath, optarg);
			break;
        case 'j':
            corecount = atoi(optarg);
            if (corecount == 0) {
                fprintf (stderr, "Option -%c requires an valid numeric argument, '%s' is not valid.\n", optopt, optarg);
                return EXIT_FAILURE;
            }
            break;
        case 'a': test_write = true; break;
        case 'b': test_read = true; break;
        case 'x': version_2 = true; break;
        case 'n': dry_run = true; break;
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

    if (test_write) {
        Heart::builder::Input test_input;

        test_input.set_resourceinputpath("/system/test/resource/path");
        auto* bp = test_input.add_buildparameters();
        bp->set_name("test_param");
        auto* bpv = bp->add_values();
        bpv->set_intvalue(4);

        HANDLE pstdin, pstdout;
        if (!exec2("builder -b", &pstdin, &pstdout)) {
            auto str = test_input.SerializeAsString();
            DWORD written;
            WriteFile(pstdin, str.c_str(), (int)str.size(), &written, nullptr);
            CloseHandle(pstdin);
        }

        BOOL success;
        DWORD bytesread;
        std::string finaloutput;
        do{
            char buf[4096];
            // TODO: handle pipes i.e. ERROR_MORE_DATA
            success = ReadFile(pstdout, buf, sizeof(buf), &bytesread, nullptr);
            finaloutput.append(buf, bytesread);
        } while (success && bytesread == 4096);

        Heart::builder::Output test_output;
        test_output.ParseFromString(finaloutput);
        fprintf(stderr, "test output: %s & %s", test_output.pkgdata().type_name().c_str(), test_output.pkgdata().messagedata().c_str());

        return 0;
    } else if (test_read) {
        char buf[1024];
        int success;
        std::string input;
        do {
            if (success = read(_fileno(stdin), buf, sizeof(buf))) {
                input += buf;
            }
        } while(success > 0);

        Heart::builder::Input test_input;
        test_input.ParseFromString(input);

        Heart::builder::Output test_output;
        test_output.mutable_pkgdata()->set_type_name("test_type_name");
        test_output.mutable_pkgdata()->set_messagedata("test_message_data");
        auto out = test_output.SerializeAsString();

        fprintf(stderr, "some test output");

        write(_fileno(stdout), out.c_str(), (int)out.size());

        return 0;
    }

    if (L == NULL) {
        fprintf(stderr, "cannot create state: not enough memory");
        return EXIT_FAILURE;
    }

    if (version_2) {
        result = luaL_dofile(L, ".build_config");
        if (handleLuaFileReadError(L, result)) {
            return EXIT_FAILURE;
        }

        auto builder_ctx = open_builder_lib(L);
        builder_ctx->push(builder_ctx->srcDataPath.c_str());
        result = luaL_dofile(L, builder_ctx->rootBuilderScript.c_str());
        if (handleLuaFileReadError(L, result)) {
            return EXIT_FAILURE;
        }
        builder_ctx->pop();

        //TODO: build resources

        lua_close(L);
    } else {
        lua_pushstring(L, srcpath);
        lua_setglobal(L, "in_data_path");

        lua_pushstring(L, destpath);
        lua_setglobal(L, "in_output_data_path");

	    lua_pushstring(L, cachepath); 
	    lua_setglobal(L, "in_cache_data_path");

        lua_pushboolean(L, verbose);
        lua_setglobal(L, "in_verbose");

        lua_pushinteger(L, corecount);
        lua_setglobal(L, "in_cores");

        result = luaL_loadbuffer(L, builder_script_data, builder_script_data_len, "data builder script");
        if (result == LUA_ERRSYNTAX) {
            printf("syntax error in builder script:\n%s", lua_tostring(L, -1));
            return EXIT_FAILURE;
        }
        if (result != LUA_OK) {
            printf("Memory error loading builder script");
            return EXIT_FAILURE;
        }
        result = lua_pcall(L, 0, LUA_MULTRET, 0);
        if (result == LUA_ERRRUN) {
            printf("Runtime Error: %s", lua_tostring(L,-1));
        }

        // Close the lua state or some files/etc wont' get flushed
        lua_gc(L, LUA_GCCOLLECT, 0);
        lua_close(L);
    }

    return (result == LUA_OK) ? EXIT_SUCCESS : EXIT_FAILURE;
}

