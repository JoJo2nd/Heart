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
        case 'z': fprintf(stdout, "heart entity definition builder v0.8.0"); exit(0);
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

    lua_State* L = luaL_newstate();
    luaL_openlibs(L);
    luaopen_proto_lua(L);
    lua_setglobal(L, "protobuf");

    const char* script_path=input_pb.resourceinputpath().c_str();
    if (luaL_dofile(L, script_path)) {
        std::string errstr = lua_tostring(L, -1);
        fatal_error("Error in Entity Def script \"%s\": \"%s\"", script_path, errstr.c_str());
    }

    //dofile should have returned a object to store
    fatal_error_check(lua_istable(L, -1), "Entity Definition script did not return an object definition table");

    //copy-paste nasty until I fix protobuf-lua
    struct msg_udata {
        ::google::protobuf::MessageLite * msg;
        bool lua_owns;
        lua_protobuf_gc_callback gc_callback;
        void * callback_data;
    };

    Heart::proto::EntityDef entity_def;
    //get name, 
    lua_getfield(L, -1, "objectname");
    fatal_error_check(lua_isstring(L, -1), "object definition table field \"objectname\" is not a string");

    entity_def.set_entryname(lua_tostring(L, -1));
    lua_pop(L, 1);

    lua_getfield(L, -1, "canserialise");
    fatal_error_check(lua_isboolean(L, -1) || lua_isnil(L, -1), "object definition table field \"canserialise\" is not a boolean");
    if (lua_isnil(L, -1)) {
        entity_def.set_canserialise(false);
    } else {
        entity_def.set_canserialise(!!lua_toboolean(L, -1));
    }
    lua_pop(L, 1);

    lua_getfield(L, -1, "components");
    fatal_error_check(lua_istable(L, -1), "object definition table field \"components\" is not a table");

    int i = 0;
    lua_pushnil(L);
    while (lua_next(L, -2) != 0) {
        /* uses 'key' (at index -2) and 'value' (at index -1) */
        ///if (lua_getmetatable(L, -1)) {  /* does it have a metatable? */
        ///    if (!lua_rawequal(L, -1, -2))  /* not the same? */
        // This is !!REALLY BAD!! I need to fix this by storing all protobuf objects metatables into
        // a table within the REGISTRY and grabbing the object metatable to check if exists as a key in that table.
        // ONLY then would the following lines be safe. Currently, we need to script to be sane...
        msg_udata* p = (msg_udata*)lua_touserdata(L, -1);
        auto* msgContainer = entity_def.add_components();
        msgContainer->set_type_name(p->msg->GetTypeName());
        msgContainer->set_messagedata(p->msg->SerializeAsString());

        /* removes 'value'; keeps 'key' for next iteration */
        lua_pop(L, 1);
        ++i;
    }
    lua_pop(L, 1);
    lua_close(L);

    //write the resource
    Heart::builder::Output output;

    //write the resource header
    output.mutable_pkgdata()->set_type_name(entity_def.GetTypeName());
    output.mutable_pkgdata()->set_messagedata(entity_def.SerializeAsString());

    google::protobuf::io::OstreamOutputStream filestream(&std::cout);
    return output.SerializeToZeroCopyStream(&filestream) ? 0 : -2;
}

