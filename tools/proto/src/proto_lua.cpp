/********************************************************************

    filename:   proto_lua.c  
    
    Copyright (c) 15:2:2014 James Moran
    
    This software is provided 'as-is', without any express or implied
    warranty. In no event will the authors be held liable for any damages
    arising from the use of this software.
    
    Permission is granted to anyone to use this software for any purpose,
    including commercial applications, and to alter it and redistribute it
    freely, subject to the following restrictions:
    
    1. The origin of this software must not be misrepresented; you must not
    claim that you wrote the original software. If you use this software
    in a product, an acknowledgment in the product documentation would be
    appreciated but is not required.
    
    2. Altered source versions must be plainly marked as such, and must not be
    misrepresented as being the original software.
    
    3. This notice may not be removed or altered from any source
    distribution.

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

#include "debug_server_common.pb.lua.h"
#include "resource_common.pb.lua.h"
#include "resource_material_fx.pb.lua.h"
#include "resource_mesh.pb.lua.h"
#include "resource_shader.pb.lua.h"
#include "resource_texture.pb.lua.h"

#ifdef __cplusplus
extern "C" {
#endif

//Lua entry point calls
__declspec(dllexport) int __cdecl luaopen_proto_lua(lua_State *L) {
    lua_newtable(L);
    lua_protobuf_coded_streams_open(L);

    lua_protobuf_debug_server_common_open(L);
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