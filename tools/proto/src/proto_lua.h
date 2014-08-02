/********************************************************************
    
    Copyright (c) 9:4:2014 James Moran
    
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

#pragma once

#if defined PLATFORM_WINDOWS
#   define proto_lua_api __cdecl
#elif PLATFORM_LINUX
#   if BUILD_64_BIT
#       define proto_lua_api
#   else
#       define proto_lua_api __attribute__((cdecl))
#   endif
#else
#   error
#endif

#if defined (PLATFORM_WINDOWS)
#   if defined (texture_builder_EXPORTS)
#       define proto_lua_dll_export __declspec(dllexport)
#   else
#       define proto_lua_dll_export __declspec(dllimport)
#   endif
#else
#   define proto_lua_dll_export
#endif

#ifdef __cplusplus
extern "C" {
#endif

//Lua entry point calls
proto_lua_dll_export int proto_lua_api luaopen_proto_lua(lua_State *L);

#ifdef __cplusplus
}
#endif