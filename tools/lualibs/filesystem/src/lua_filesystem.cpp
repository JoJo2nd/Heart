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
static int fs_currentDate(lua_State* L) {
    time_t t;
    time(&t);
    lua_pushnumber(L, (lua_Number)t);
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

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
static int fs_getCurrentPath(lua_State* L) {
    boost::system::error_code ec;
    boost::filesystem::path cd;
    cd=boost::filesystem::current_path(ec);
    if (ec) {
        lua_pushnil(L);
    } else {
        std::string cdstr=cd.generic_string();
        lua_pushstring(L, cdstr.c_str());
    }
    return 1;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
static int fs_absolute(lua_State* L) {
    std::string ret;
    boost::filesystem::path path = luaL_checkstring(L, 1);
    if (lua_isstring(L, 2)) {
        boost::filesystem::path base = luaL_checkstring(L, 2);
        ret = boost::filesystem::absolute(path, base).generic_string();
        lua_pushstring(L, ret.c_str());
    } else {
        ret = boost::filesystem::absolute(path).generic_string();
        lua_pushstring(L, ret.c_str());
    }
    return 1;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
static int fs_canonical(lua_State* L) {
    std::string ret;
    boost::system::error_code ec;
    boost::filesystem::path path = luaL_checkstring(L, 1);
    if (lua_isstring(L, 2)) {
        boost::filesystem::path base = luaL_checkstring(L, 2);
        ret = boost::filesystem::canonical(path, base, ec).generic_string();
        if (ec) {
            lua_pushnil(L);
        } else {
            lua_pushstring(L, ret.c_str());
        }
    } else {
        ret = boost::filesystem::canonical(path, ec).generic_string();
        if (ec) {
            lua_pushnil(L);
        } else {
            lua_pushstring(L, ret.c_str());
        }
    }
    return 1;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
static int fs_genericPath(lua_State* L) {
    boost::filesystem::path path = luaL_checkstring(L, 1);
    std::string ret = path.generic_string();
    lua_pushstring(L, ret.c_str());
    return 1;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
static int fs_nativePath(lua_State* L) {
    boost::filesystem::path path = luaL_checkstring(L, 1);
    std::string ret = path.string();
    lua_pushstring(L, ret.c_str());
    return 1;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
static int fs_fileWithExt(lua_State* L) {
    boost::filesystem::path path = luaL_checkstring(L, 1);
    std::string ret = path.filename().generic_string();
    lua_pushstring(L, ret.c_str());
    return 1;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
static int fs_pathRoot(lua_State* L) {
    boost::filesystem::path path = luaL_checkstring(L, 1);
    std::string ret = path.parent_path().generic_string();
    lua_pushstring(L, ret.c_str());
    return 1;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
static int fs_readDir(lua_State* L) {
    boost::filesystem::path path = luaL_checkstring(L, 1);
    
    boost::system::error_code ec;
    boost::filesystem::directory_iterator itr(path, ec);

    if (ec) {
        lua_pushnil(L);
        return 1;
    }

    lua_newtable(L);
    int tableindex = 1;
    for (boost::filesystem::directory_iterator n; itr != n; itr.increment(ec)) {
        if (ec) {
            break; //just return what we got
        }
        std::string entry = itr->path().generic_string();
        lua_pushstring(L, entry.c_str());
        lua_rawseti(L, -2, tableindex++);
    }

    return 1; // return table 
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
static int fs_readDirRecursive(lua_State* L) {
    boost::filesystem::path path = luaL_checkstring(L, 1);

    boost::system::error_code ec;
    boost::filesystem::recursive_directory_iterator itr(path, ec);

    if (ec) {
        lua_pushnil(L);
        return 1;
    }

    lua_newtable(L);
    int tableindex = 1;
    for (boost::filesystem::recursive_directory_iterator n; itr != n; itr.increment(ec)) {
        if (ec) {
            break; //just return what we got
        }
        std::string entry = itr->path().generic_string();
        lua_pushstring(L, entry.c_str());
        lua_rawseti(L, -2, tableindex++);
    }

    return 1; // return table 
}

extern "C" {
//Lua entry point calls
luaFILESYSTEM_EXPORT int luaFILESYSTEM_API luaopen_filesystem(lua_State *L) {
    static const luaL_Reg filesystemlib[] = {
        {"modifieddate",fs_modifiedDate},
        {"currentfiledate", fs_currentDate},
        {"exists",fs_exists},
        {"isfile",fs_isFile},
        {"isdirectory",fs_isDirectory},
        {"makedirectories",fs_makeDirectories},
        {"getcurrentpath",fs_getCurrentPath},
        {"absolute",fs_absolute},
        {"canonical",fs_canonical},
        {"genericpath",fs_genericPath},
        {"nativepath",fs_nativePath},
        {"filewithext", fs_fileWithExt},
        {"parentpath", fs_pathRoot},
        {"readdir", fs_readDir},
        {"readdirrecursive", fs_readDirRecursive},
        {NULL, NULL}
    };

    luaL_newlib(L, filesystemlib);
    //lua_setglobal(L, "filesystem");
    return 1;
}
};
