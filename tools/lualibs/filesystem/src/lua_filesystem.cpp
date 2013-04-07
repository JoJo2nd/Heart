/********************************************************************

    filename:   lua_filesystem.cpp  
    
    Copyright (c) 1:4:2013 James Moran
    
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

extern "C" {
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}
#include "lua_filesystem.h"
#include <boost/filesystem.hpp>

#define nullptr (std::nullptr_t())


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
static int fs_modifiedDate(lua_State* L) {
    luaL_checkstring(L, -1);
    boost::system::error_code ec;
    boost::filesystem::path filepath(lua_tostring(L, -1));
    time_t t=boost::filesystem::last_write_time(filepath, ec);
    if (!ec) {
        lua_pushnumber(L, (lua_Number)t);
    } else {
        lua_pushnil(L);
    }
    return 1;
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
static int fs_exists(lua_State* L) {
    luaL_checkstring(L, -1);
    boost::system::error_code ec;
    boost::filesystem::path filepath(lua_tostring(L, -1));
    bool r=boost::filesystem::exists(filepath, ec);
    if (!ec) {
        lua_pushboolean(L, r);
    } else {
        lua_pushboolean(L, false);
    }
    return 1;
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
static int fs_isFile(lua_State* L) {
    luaL_checkstring(L, -1);
    boost::system::error_code ec;
    boost::filesystem::path filepath(lua_tostring(L, -1));
    bool r=boost::filesystem::is_regular_file(filepath, ec);
    if (!ec) {
        lua_pushboolean(L, r);
    } else {
        lua_pushboolean(L, false);
    }
    return 1;
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
static int fs_isDirectory(lua_State* L) {
    luaL_checkstring(L, -1);
    boost::system::error_code ec;
    boost::filesystem::path filepath(lua_tostring(L, -1));
    bool r=boost::filesystem::is_directory(filepath, ec);
    if (!ec) {
        lua_pushboolean(L, r);
    } else {
        lua_pushboolean(L, false);
    }
    return 1;
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
static int fs_makeDirectories(lua_State* L) {
    luaL_checkstring(L, -1);
    std::string p=lua_tostring(L, -1);
    boost::system::error_code ec;
    boost::filesystem::path filepath(p);
    bool r=boost::filesystem::create_directories(filepath, ec);
    if (!ec) {
        lua_pushboolean(L, r);
    } else {
        lua_pushboolean(L, false);
    }
    return 1;
}

extern "C" {
//Lua entry point calls
luaFILESYSTEM_EXPORT int luaFILESYSTEM_API luaopen_filesystem(lua_State *L) {
    static const luaL_Reg filesystemlib[] = {
        {"modifieddate",fs_modifiedDate},
        {"exists",fs_exists},
        {"isfile",fs_isFile},
        {"isdirectory",fs_isDirectory},
        {"makedirectories",fs_makeDirectories},
        {NULL, NULL}
    };
    luaL_newlib(L, filesystemlib);
    //lua_setglobal(L, "filesystem");
    return 1;
}
};