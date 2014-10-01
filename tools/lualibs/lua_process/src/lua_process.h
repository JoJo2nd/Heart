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

#ifdef _WINDLL
#   define luaPROCESS_SHARED_LIB
#endif

#ifdef luaPROCESS_SHARED_LIB
#   ifdef lua_process_EXPORTS //generated by cmake
#       define lua_process_dll_export __declspec(dllexport)
#   else
#       define lua_process_dll_export __declspec(dllimport)
#   endif
#   define lua_process_api          __cdecl
#else
#   define lua_process_dll_export extern
#   define lua_process_api 
#endif 

#ifdef __cplusplus
extern "C" {
#endif
lua_process_dll_export int lua_process_api luaopen_lua_process(lua_State* L);
#ifdef __cplusplus
}//extern "C" {
#endif