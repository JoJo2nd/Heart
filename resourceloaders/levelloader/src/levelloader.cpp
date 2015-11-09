/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#include "minfs.h"
#include "entity_def.pb.h"
#include "builder.pb.h"
extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
};
#include "proto_lua.h"
#include "lua-protobuf.h" // remove when protobuf-lua is fixed.
#include "getopt.h"
#include <fstream>
#include <memory>
#include <iostream>
#include <unordered_set>

#if defined (_MSC_VER)
#   pragma warning(push)
#   pragma warning(disable:4244)
#   pragma warning(disable:4267)
#else
#   pragma error ("Unknown platform")
#endif
#include "google/protobuf/io/zero_copy_stream_impl.h"
#include "google/protobuf/io/coded_stream.h"

#if defined (_MSC_VER)
#   pragma warning(pop)
#endif

#ifdef _WIN32
#   include <io.h>
#   include <fcntl.h>
#endif

static const char argopts[] = "vi:";
static struct option long_options[] = {
    { "version", no_argument, 0, 'z' },
    { 0, 0, 0, 0 }
};

#define fatal_error_check(x, msg, ...) if (!(x)) {fprintf(stderr, msg, __VA_ARGS__); exit(-1);}
#define fatal_error(msg, ...) fatal_error_check(false, msg, __VA_ARGS__)

std::unordered_set<std::string> resource_imports;

int lua_import_resource(lua_State* L) {
    const char* resource_path = luaL_checkstring(L, 1);
    resource_imports.insert(resource_path);
    return 0;
}

int main(int argc, char* argv[]) {
#ifdef _WIN32
    _setmode(_fileno(stdin), _O_BINARY);
    _setmode(_fileno(stdout), _O_BINARY);
    _setmode(_fileno(stderr), _O_BINARY);
#endif
    using namespace Heart;
    google::protobuf::io::IstreamInputStream input_stream(&std::cin);
    Heart::builder::Input input_pb;

    int c;
    int option_index = 0;
    bool verbose = false, use_stdin = true;

    while ((c = gop_getopt_long(argc, argv, argopts, long_options, &option_index)) != -1) {
        switch (c) {
        case 'z': fprintf(stdout, "heart level script builder v0.8.0"); exit(0);
        case 'v': verbose = 1; break;
        case 'i': {
            std::ifstream input_file_stream;
            input_file_stream.open(optarg, std::ios_base::binary | std::ios_base::in);
            if (input_file_stream.is_open()) {
                google::protobuf::io::IstreamInputStream file_stream(&input_file_stream);
                input_pb.ParseFromZeroCopyStream(&file_stream);
                use_stdin = false;
            }
        } break;
        default: return 2;
        }
    }

    if (use_stdin) {
        input_pb.ParseFromZeroCopyStream(&input_stream);
    }

    const char* script_path = input_pb.resourceinputpath().c_str();
    char path_root[260];
    minfs_path_parent(script_path, path_root, 260);

    lua_State* L = luaL_newstate();
    luaL_openlibs(L);
    luaopen_proto_lua(L);
    lua_setglobal(L, "protobuf");
    lua_pushcclosure(L, lua_import_resource, 0);
    lua_setglobal(L, "importResource");

    lua_getglobal(L, "package");
    lua_getfield(L, -1, "path");
    std::string path = lua_tostring(L, -1);
    path += ";";
    path += path_root;
    path += "\\?.lua";
    path += ";";
    path += input_pb.resourcedatarootpath();
    path += "\\lua\\?";
    path += ";";
    path += input_pb.resourcedatarootpath();
    path += "\\lua\\?.lua";
    std::replace(path.begin(), path.end(), '/', '\\');
    lua_pop(L, 1);
    lua_pushstring(L, path.c_str());
    lua_setfield(L, -2, "path");
    lua_pop(L, 1);




    if (luaL_dofile(L, script_path)) {
        std::string errstr = lua_tostring(L, -1);
        fatal_error("Error in Level Definition script \"%s\": \"%s\"", script_path, errstr.c_str());
    }

    //copy-paste nasty until I fix protobuf-lua
    struct msg_udata {
        ::google::protobuf::MessageLite * msg;
        bool lua_owns;
        lua_protobuf_gc_callback gc_callback;
        void * callback_data;
    };

    Heart::proto::LevelDefinition level_script;
    //get name, 
    lua_getglobal(L, "levelName");
    fatal_error_check(lua_isstring(L, -1), "object definition table field \"objectname\" is not a string");

    level_script.set_levelname(lua_tostring(L, -1));
    lua_pop(L, 1);

    lua_getglobal(L, "entities");
    fatal_error_check(lua_istable(L, -1), "object definition table field \"components\" is not a table");

    int i = 0;
    lua_pushnil(L);
    while (lua_next(L, -2) != 0) {
        /* uses 'key' (at index -2) and 'value' (at index -1) */
        Heart::proto::EntityDefinition* entity = level_script.add_entities();
        fatal_error_check(lua_isstring(L, -2), "entities table keys must be GUID strings.");
        entity->set_objectguid(lua_tostring(L, -2));
        // transient flag
        lua_getfield(L, -1, "transient");
        entity->set_transient(!lua_isnil(L, -1) ? !!lua_toboolean(L, -1) : false);
        lua_pop(L, 1);
        // grab the object components
        lua_getfield(L, -1, "components");
        fatal_error_check(lua_istable(L, -1), "entities table entry is missing components table.");
        int j = 0;
        lua_pushnil(L);
        while (lua_next(L, -2) != 0) {
            ///if (lua_getmetatable(L, -1)) {  /* does it have a metatable? */
            ///    if (!lua_rawequal(L, -1, -2))  /* not the same? */
            // This is !!REALLY BAD!! I need to fix this by storing all protobuf objects metatables into
            // a table within the REGISTRY and grabbing the object metatable to check if exists as a key in that table.
            // ONLY then would the following lines be safe. Currently, we need to script to be sane...
            msg_udata* p = (msg_udata*)lua_touserdata(L, -1);
            auto* msgContainer = entity->add_components();
            msgContainer->set_type_name(p->msg->GetTypeName());
            msgContainer->set_messagedata(p->msg->SerializeAsString());

            /* removes 'value'; keeps 'key' for next iteration */
            lua_pop(L, 1);
            ++j;
        }
        lua_pop(L, 1);
        // Grab the debug object components
        // grab the object components
        lua_getfield(L, -1, "debugcomponents");
        if (lua_istable(L, -1)) {
            int j = 0;
            lua_pushnil(L);
            while (lua_next(L, -2) != 0) {
                ///if (lua_getmetatable(L, -1)) {  /* does it have a metatable? */
                ///    if (!lua_rawequal(L, -1, -2))  /* not the same? */
                // This is !!REALLY BAD!! I need to fix this by storing all protobuf objects metatables into
                // a table within the REGISTRY and grabbing the object metatable to check if exists as a key in that table.
                // ONLY then would the following lines be safe. Currently, we need to script to be sane...
                msg_udata* p = (msg_udata*)lua_touserdata(L, -1);
                auto* msgContainer = entity->add_debugcomponents();
                msgContainer->set_type_name(p->msg->GetTypeName());
                msgContainer->set_messagedata(p->msg->SerializeAsString());

                /* removes 'value'; keeps 'key' for next iteration */
                lua_pop(L, 1);
                ++j;
            }
        }
        lua_pop(L, 1);

        /* removes 'value'; keeps 'key' for next iteration */
        lua_pop(L, 1);
        ++i;
    }
    lua_pop(L, 1);
    lua_close(L);

    //write the resource
    Heart::builder::Output output;
    output.add_filedependency(script_path);
    for (const auto& res_dep : resource_imports) {
        output.add_resourcedependency(res_dep);
    }
    //write the resource header
    output.mutable_pkgdata()->set_type_name(level_script.GetTypeName());
    output.mutable_pkgdata()->set_messagedata(level_script.SerializeAsString());

    google::protobuf::io::OstreamOutputStream filestream(&std::cout);
    return output.SerializeToZeroCopyStream(&filestream) ? 0 : -2;
}

