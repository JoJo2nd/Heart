/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
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
#   if defined (proto_lua_EXPORTS)
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