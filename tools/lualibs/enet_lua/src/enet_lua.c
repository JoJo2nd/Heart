/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#include "enet/enet.h"
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

#if defined PLATFORM_WINDOWS
#   define enet_lua_api __cdecl
#elif PLATFORM_LINUX
#   if BUILD_64_BIT
#       define enet_lua_api
#   else
#       define enet_lua_api __attribute__((cdecl))
#   endif
#else
#   error
#endif

#if defined (PLATFORM_WINDOWS)
#   if defined (enet_lua_EXPORTS)
#       define enet_lua_dll_export __declspec(dllexport)
#   else
#       define enet_lua_dll_export __declspec(dllimport)
#   endif
#else
#   define enet_lua_dll_export
#endif

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

typedef struct enet_lua_Address {
    ENetAddress address_;
} enet_lua_Address_t;

int enet_lua_api enet_lua_create_address(lua_State* L) {
    const char* name = NULL;
    enet_uint32 host = ENET_HOST_ANY;
    enet_uint32 port = 0;
    enet_lua_Address_t* udata = NULL;
    if (lua_isstring(L, 1)) {
        name = luaL_checkstring(L, 1);
    }
    if (lua_isnumber(L, 1)) {
        host = (enet_uint32)luaL_checkinteger(L, 1);
    }
    if (lua_isnumber(L, 2)) {
        port = (enet_uint32)luaL_checkinteger(L, 2);
    }
    udata = lua_newuserdata(L, sizeof(enet_lua_Address_t));
    luaL_getmetatable(L, "_enet.address");
    lua_setmetatable(L, -2);
    if (name) {
        enet_address_set_host(&udata->address_, name);
    } else {
        udata->address_.host = host;
        udata->address_.port = port;
    }
    return 1;
}

int enet_lua_api enet_lua_address_get_host_ip(lua_State* L) {
    int err;
    char namebuffer[256];
    enet_lua_Address_t* udata = luaL_checkudata(L, 1, "_enet.address");
    err = enet_address_get_host_ip(&udata->address_, namebuffer, sizeof(namebuffer));
    if (err < 0) {
        luaL_error(L, "enet_address_get_host_ip failed with error code %d", err);
    }
    lua_pushstring(L, namebuffer);
    return 1;
}

int enet_lua_api enet_lua_address_get_host(lua_State* L) {
    int err;
    char namebuffer[256];
    enet_lua_Address_t* udata = luaL_checkudata(L, 1, "_enet.address");
    err = enet_address_get_host(&udata->address_, namebuffer, sizeof(namebuffer));
    if (err < 0) {
        luaL_error(L, "enet_address_get_host failed with error code %d", err);
    }
    lua_pushstring(L, namebuffer);
    lua_pushinteger(L, udata->address_.host);
    return 2;
}

int enet_lua_api enet_lua_address_set_host(lua_State* L) {
    enet_lua_Address_t* udata = luaL_checkudata(L, 1, "_enet.address");
    int host = (int)luaL_checkinteger(L, 2);
    udata->address_.host = host;
    return 0;
}

int enet_lua_api enet_lua_address_set_port(lua_State* L) {
    enet_lua_Address_t* udata = luaL_checkudata(L, 1, "_enet.address");
    int port = (int)luaL_checkinteger(L, 2);
    udata->address_.port = port;
    return 0;
}

int enet_lua_api enet_lua_address_get_port(lua_State* L) {
    enet_lua_Address_t* udata = luaL_checkudata(L, 1, "_enet.address");
    lua_pushinteger(L, udata->address_.port);
    return 0;
}

static const luaL_Reg enet_lua_address_lib[] = {
    {"get_host", enet_lua_address_get_host},
    {"get_host_ip", enet_lua_address_get_host_ip},
    {"set_host", enet_lua_address_set_host},
    {"set_port", enet_lua_address_set_port},
    {"get_port", enet_lua_address_get_port},
    {NULL, NULL}
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

int enet_lua_api enet_lua_peer_push(lua_State* L, ENetPeer* peer) {
    ENetPeer** udata = lua_newuserdata(L, sizeof(ENetPeer*));
    luaL_getmetatable(L, "_enet.peer");
    lua_setmetatable(L, -2);
    return 1;
}

int enet_lua_api enet_lua_peer_gc(lua_State* L) {
    //TODO: release ref to host?
    return 0;
}

int enet_lua_api enet_lua_peer_send(lua_State* L) {
    ENetPeer** udata = luaL_checkudata(L, 1, "_enet.peer");
    enet_uint32 channel = (enet_uint32)luaL_checkinteger(L, 2);
    size_t strsize = 0;
    const char* packetdata = lua_tolstring(L, 3, &strsize);
    enet_uint32 flags = luaL_optint(L, 4, 0);
    ENetPacket* packet = enet_packet_create(packetdata, strsize, flags);
    int err;
    if (!packet) {
        luaL_error(L, "Couldn't create packet");
        return 0;
    }
    err = enet_peer_send(*udata, channel, packet);
    if (err < 0) {
        luaL_error(L, "enet_peer_send Error %d", err);
        return 0;
    }
    return 0;
}

int enet_lua_api enet_lua_peer_reset(lua_State* L) {
    ENetPeer** udata = luaL_checkudata(L, 1, "_enet.peer");
    enet_peer_reset(*udata);
    return 0;
}

int enet_lua_api enet_lua_peer_ping(lua_State* L) {
    ENetPeer** udata = luaL_checkudata(L, 1, "_enet.peer");
    enet_peer_reset(*udata);
    return 0;
}

int enet_lua_api enet_lua_peer_disconnect_now(lua_State* L) {
    ENetPeer** udata = luaL_checkudata(L, 1, "_enet.peer");
    enet_peer_disconnect_now(*udata, 0);
    return 0;
}

int enet_lua_api enet_lua_peer_disconnect(lua_State* L) {
    ENetPeer** udata = luaL_checkudata(L, 1, "_enet.peer");
    enet_peer_disconnect(*udata, 0);
    return 0;
}

int enet_lua_api enet_lua_peer_disconnect_later(lua_State* L) {
    ENetPeer** udata = luaL_checkudata(L, 1, "_enet.peer");
    enet_peer_disconnect_later(*udata, 0);
    return 0;
}

static const luaL_Reg enet_lua_peer_lib[] = {
    {"__gc", enet_lua_peer_gc},
    {"send", enet_lua_peer_send},
    {"ping", enet_lua_peer_ping},
    {"disconnect_now", enet_lua_peer_disconnect_now},
    {"disconnect", enet_lua_peer_disconnect},
    {"disconnect_later", enet_lua_peer_disconnect_later},
    {NULL, NULL}
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

int enet_lua_api enet_lua_host_create(lua_State* L) {
    enet_lua_Address_t* address = luaL_checkudata(L, 1, "_enet.address");
    enet_uint32 peers = luaL_checkint(L, 2);
    enet_uint32 channels = luaL_checkint(L, 3);
    enet_uint32 inBW = luaL_checkint(L, 4);
    enet_uint32 outBW = luaL_checkint(L, 5);
    ENetHost** udata = lua_newuserdata(L, sizeof(ENetHost*));
    luaL_getmetatable(L, "_enet.host");
    lua_setmetatable(L, -2);
    *udata = enet_host_create(&address->address_, peers, channels, inBW, outBW);
    if (!*udata) {
        luaL_error(L, "Couldn't create host");
        return 0;
    }
    return 1;
}

int enet_lua_api enet_lua_host_gc(lua_State* L) {
    ENetHost** udata = luaL_checkudata(L, 1, "_enet.host");
    enet_host_destroy(*udata);
    return 1;
}

int enet_lua_api enet_lua_host_connect(lua_State* L) {
    ENetHost** udata = luaL_checkudata(L, 1, "_enet.host");
    enet_lua_Address_t* address = luaL_checkudata(L, 2, "_enet.address");
    enet_uint32 channels = luaL_checkint(L, 3);
    enet_host_connect(*udata, &address->address_, channels, 0);
    return 0;
}

int enet_lua_api enet_lua_host_broadcast(lua_State* L) {
    ENetHost** udata = luaL_checkudata(L, 1, "_enet.host");
    enet_uint32 channel = luaL_checkint(L, 2);
    size_t strsize;
    const char* packetdata = luaL_checklstring(L, 3, &strsize);
    enet_uint32 flags = luaL_optint(L, 2, 0);
    ENetPacket* packet = enet_packet_create(packetdata, strsize, flags);
    if (!packet) {
        luaL_error(L, "Failed to create packet");
        return 0;
    }
    enet_host_broadcast(*udata, channel, packet);
    return 0;
}

int enet_lua_api enet_lua_host_flush(lua_State* L) {
    ENetHost** udata = luaL_checkudata(L, 1, "_enet.host");
    enet_host_flush(*udata);
    return 0;
}

int enet_lua_api enet_lua_host_service(lua_State* L) {
    ENetHost** udata = luaL_checkudata(L, 1, "_enet.host");
    enet_uint32 timeout = luaL_optint(L, 2, 0);
    ENetEvent enet_event;

    // TODO: change the push of peers so that in Lua peer == peer works as expected
    // currently peer == peer is alway false, even they are the same internally
    if (enet_host_service(*udata, &enet_event, timeout) > 0) {
        lua_pushinteger(L, enet_event.type);
        switch (enet_event.type) {
        case ENET_EVENT_TYPE_CONNECT: {
            enet_lua_peer_push(L, enet_event.peer);
        } return 2;
        case ENET_EVENT_TYPE_DISCONNECT: {
            enet_lua_peer_push(L, enet_event.peer);
        } return 2;
        case ENET_EVENT_TYPE_RECEIVE: {
            enet_lua_peer_push(L, enet_event.peer);
            lua_pushlstring(L, enet_event.packet->data, enet_event.packet->dataLength);
        } return 3;
        }
    }

    return 0;
}

static const luaL_Reg enet_lua_host_lib[] = {
    {"__gc", enet_lua_host_gc},
    {"connect", enet_lua_host_connect},
    {"broadcast", enet_lua_host_broadcast},
    {"flush", enet_lua_host_flush},
    {"service", enet_lua_host_service},
    {NULL, NULL}
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

static const luaL_Reg enet_lua_lib[] = {
    {NULL, NULL}
};

static const luaL_Reg enet_address_statics[] = {
    {"new", enet_lua_create_address},
    {NULL, NULL}
};

static const luaL_Reg enet_host_statics[] = {
    {"new", enet_lua_host_create},
    {NULL, NULL}
};


int enet_lua_api enet_lua_read_only(lua_State* L) {
    return luaL_error(L, "Attempt to write to read only table");
}

int enet_lua_api enet_lua_read_only_pairs(lua_State* L) {
    int first = lua_isnil(L, 2);
    lua_getglobal(L, "next");
    if (!luaL_getmetafield(L, 1, "__index")) {
        return luaL_error(L, "__index metafield is invalid");
    }
    luaL_checktype(L, -1, LUA_TTABLE);  /* argument must be a table */
    lua_pushnil(L);
    //function "next", table, nil
    return 3;
}

enet_lua_dll_export int enet_lua_api luaopen_enet_lua(lua_State* L) {
    luaL_newmetatable(L, "_enet.host");
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    luaL_setfuncs(L, enet_lua_host_lib, 0);
    luaL_newmetatable(L, "_enet.address");
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    luaL_setfuncs(L, enet_lua_address_lib, 0);
    luaL_newmetatable(L, "_enet.peer");
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    luaL_setfuncs(L, enet_lua_peer_lib, 0);

    luaL_newlib(L, enet_lua_lib);
    // create and add to enum table
    lua_newtable(L);
    lua_pushinteger(L, ENET_EVENT_TYPE_NONE);
    lua_setfield(L, -2, "ENET_EVENT_TYPE_NONE");
    lua_pushinteger(L, ENET_EVENT_TYPE_CONNECT);
    lua_setfield(L, -2, "ENET_EVENT_TYPE_CONNECT");
    lua_pushinteger(L, ENET_EVENT_TYPE_DISCONNECT);
    lua_setfield(L, -2, "ENET_EVENT_TYPE_DISCONNECT");
    lua_pushinteger(L, ENET_EVENT_TYPE_RECEIVE);
    lua_setfield(L, -2, "ENET_EVENT_TYPE_RECEIVE");
    // create proxy with metatable
    lua_newtable(L);
    // make read only
    lua_newtable(L);
    lua_pushvalue(L, -3); // push the created table
    lua_setfield(L, -2, "__index");
    lua_pushcfunction(L, enet_lua_read_only);
    lua_setfield(L, -2, "__newindex");
    lua_pushcfunction(L, enet_lua_read_only_pairs);
    lua_setfield(L, -2, "__pairs");
    lua_setmetatable(L, -2);
    // remove enum table
    lua_remove(L, -2);
    // set the proxy
    lua_setfield(L, -2, "ENetEventType");
    // create and add to enum table
    lua_newtable(L);
    lua_pushinteger(L, ENET_PACKET_FLAG_RELIABLE);
    lua_setfield(L, -2, "ENET_PACKET_FLAG_RELIABLE");
    lua_pushinteger(L, ENET_PACKET_FLAG_UNSEQUENCED);
    lua_setfield(L, -2, "ENET_PACKET_FLAG_UNSEQUENCED");
    // create proxy with metatable
    lua_newtable(L);
    // make read only
    lua_newtable(L);
    lua_pushvalue(L, -3); // push the created table
    lua_setfield(L, -2, "__index");
    lua_pushcfunction(L, enet_lua_read_only);
    lua_setfield(L, -2, "__newindex");
    lua_pushcfunction(L, enet_lua_read_only_pairs);
    lua_setfield(L, -2, "__pairs");
    lua_setmetatable(L, -2);
    // remove enum table
    lua_remove(L, -2);
    // set enum table
    lua_setfield(L, -2, "ENetPacketFlag");
    // add create funcs and tables
    luaL_newlib(L, enet_host_statics);
    lua_setfield(L, -2, "ENetHost");
    luaL_newlib(L, enet_address_statics);
    lua_setfield(L, -2, "ENetAddress");

    enet_initialize();
    return 1;
}
