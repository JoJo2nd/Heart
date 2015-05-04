/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#include "minfs.h"
#include "entity_def.pb.h"
extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
};
#include "proto_lua.h"
#include "lua-protobuf.h" // remove when protobuf-lua is fixed.
#include <fstream>
#include <memory>

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

#if defined PLATFORM_WINDOWS
#   define EDL_API __cdecl
#elif PLATFORM_LINUX
#   if BUILD_64_BIT
#       define EDL_API
#   else
#       define EDL_API __attribute__((cdecl))
#   endif
#else
#   error
#endif

#if defined (PLATFORM_WINDOWS)
#   if defined (entitydef_builder_EXPORTS)
#       define DLL_EXPORT __declspec(dllexport)
#   else
#       define DLL_EXPORT __declspec(dllimport)
#   endif
#else
#   define DLL_EXPORT
#endif

#define luaL_errorthrow(L, fmt, ...) \
    luaL_where(L, 1); \
    lua_pushfstring(L, fmt, ##__VA_ARGS__ ); \
    lua_concat(L, 2); \
    throw std::exception();

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

int entityDefBuild(lua_State* L) {
    using namespace Heart;
    
    /* Args from Lua (1: input files table, 2: dep files table, 3: parameter table, 4: outputpath)*/
    luaL_checktype(L, 1, LUA_TSTRING);
    luaL_checktype(L, 2, LUA_TTABLE);
    luaL_checktype(L, 3, LUA_TTABLE);
    luaL_checktype(L, 4, LUA_TSTRING);

try {
    luaopen_proto_lua(L);
    lua_setglobal(L, "protobuf");

    const char* script_path=lua_tostring(L, 1);
    if (luaL_dofile(L, script_path)) {
        std::string errstr = lua_tostring(L, -1);
        luaL_errorthrow(L, "Error in Entity Def script \"%s\": \"%s\"", script_path, errstr.c_str());
    }

    //dofile should have returned a object to store
    if (!lua_istable(L, -1)) {
        luaL_errorthrow(L, "Entity Definition script did not return an object definition table");
    }

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
    if (!lua_isstring(L, -1)) {
        luaL_errorthrow(L, "object definition table field \"objectname\" is not a string");
    }
    entity_def.set_entryname(lua_tostring(L, -1));
    lua_pop(L, 1);

    lua_getfield(L, -1, "canserialise");
    if (!lua_isboolean(L, -1) && !lua_isnil(L, -1)) {
        luaL_errorthrow(L, "object definition table field \"canserialise\" is not a boolean");
    }
    if (lua_isnil(L, -1)) {
        entity_def.set_canserialise(false);
    } else {
        entity_def.set_canserialise(!!lua_toboolean(L, -1));
    }
    lua_pop(L, 1);

    lua_getfield(L, -1, "components");
    if (!lua_istable(L, -1)) {
        luaL_errorthrow(L, "object definition table field \"components\" is not a table");
    }

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

    //write the resource
    const char* outputpath = lua_tostring(L, 4);
    std::ofstream output;
    output.open(outputpath, std::ios_base::out | std::ios_base::binary);
    if (!output.is_open()) {
        luaL_errorthrow(L, "Unable to open output file %s", outputpath);
    }

    google::protobuf::io::OstreamOutputStream filestream(&output);
    google::protobuf::io::CodedOutputStream outputstream(&filestream);
    {
        google::protobuf::io::OstreamOutputStream filestream(&output);
        google::protobuf::io::CodedOutputStream outputstream(&filestream);
        Heart::proto::MessageContainer msgContainer;
        msgContainer.set_type_name(entity_def.GetTypeName());
        msgContainer.set_messagedata(entity_def.SerializeAsString());
        msgContainer.SerializePartialToCodedStream(&outputstream);
    }
    output.close();

    // push table of files files that where included (always empty for ttf fonts)
    lua_newtable(L);

    return 1;
} catch (std::exception e) {
    return lua_error(L);
}
}

extern "C" {

    int EDL_API version(lua_State* L) {
        lua_pushstring(L, "1.0.0");
        return 1;
    }

//Lua entry point calls
DLL_EXPORT int EDL_API luaopen_entitydef(lua_State *L) {
    static const luaL_Reg fontlib[] = {
        {"build"  , entityDefBuild},
        {"version", version},
        {NULL, NULL}
    };
    luaL_newlib(L, fontlib);
    return 1;
}
}

