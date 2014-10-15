/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#ifdef __cplusplus
extern "C" {
#endif
#   include "lua.h"
#   include "lualib.h"
#   include "lauxlib.h"
#ifdef __cplusplus
}
#endif

/* NOTE: These includes are generated files*/
#include "debug_server_common.pb.lua.h"
#include "package.pb.lua.h"
#include "resource_common.pb.lua.h"
#include "resource_material_fx.pb.lua.h"
#include "resource_mesh.pb.lua.h"
#include "resource_shader.pb.lua.h"
#include "resource_texture.pb.lua.h"

#include "proto_lua.h" 

#ifdef __cplusplus
extern "C" {
#endif

//Lua entry point calls
proto_lua_dll_export int proto_lua_api luaopen_proto_lua(lua_State *L) {
    lua_newtable(L);
    lua_protobuf_coded_streams_open(L);

    lua_protobuf_debug_server_common_open(L);
    lua_protobuf_package_open(L);
    lua_protobuf_resource_common_open(L);
    lua_protobuf_resource_material_fx_open(L);
    lua_protobuf_resource_mesh_open(L);
    lua_protobuf_resource_shader_open(L);
    lua_protobuf_resource_shader_open(L);
    lua_protobuf_resource_texture_open(L);

    return 1;
}

#ifdef __cplusplus
}
#endif