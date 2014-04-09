#include "lauxlib.h"
/********************************************************************

    filename: 	LuaHeartLib.cpp	
    
    Copyright (c) 8:5:2011 James Moran
    
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

namespace Heart
{
/*
typedef int (*lua_CFunction) (lua_State *L);
Type for C functions.

In order to communicate properly with Lua, a C function must use the following protocol, 
which defines the way parameters and results are passed: a C function receives its arguments 
from Lua in its stack in direct order (the first argument is pushed first). So, when the 
function starts, lua_gettop(L) returns the number of arguments received by the function. The 
first argument (if any) is at index 1 and its last argument is at index lua_gettop(L). To return 
values to Lua, a C function just pushes them onto the stack, in direct order (the first result 
is pushed first), and returns the number of results. Any other value in the stack 
below the results will be properly discarded by Lua. Like a Lua function, a C function called 
by Lua can also return many results.

*/

    int hLuaElasped( lua_State* L )
    {
        lua_pushnumber( L, hClock::elapsed() );
        return 1;
    }

    int hLuaElaspedHoursMinSecs( lua_State* L )
    {
        lua_pushinteger( L, hClock::hours() );
        lua_pushinteger( L, hClock::mins() );
        lua_pushinteger( L, hClock::secs() );
        return 3;
    }

    int hLuaSetDebugMenuVisiable(lua_State* L)
    {
        HEART_LUA_GET_ENGINE(L);
        const hChar* str = luaL_checkstring(L, -2);
        hDebugMenuManager::GetInstance()->SetMenuVisiablity(str, luaL_checkinteger(L, -1) != 0);
        return 0;
    }

    int hLuaExit(lua_State* L)
    {
        HEART_LUA_GET_ENGINE(L);
        engine->GetSystem()->signalExit();
        return 0;
    }

    int hLuaResourceInfo(lua_State* L)
    {
        HEART_LUA_GET_ENGINE(L);
        hResourceManager::printResourceInfo();
        return 0;
    }

    int hLuaConsoleSize(lua_State* L)
    {
        hSystemConsole::setFontSize((hFloat)lua_tonumber(L,-1));
        return 0;
    }

    //functions that don't need upvalues
    static const luaL_Reg libcore[] = {
        {"elasped",     hLuaElasped},
        {"elaspedHMS",  hLuaElaspedHoursMinSecs},
        {"fntsize",     hLuaConsoleSize},
        {NULL, NULL}
    };

    //functions that need up values
    static const luaL_Reg libcoreuv[] = {
        {"exit",    hLuaExit},
        {"resinfo", hLuaResourceInfo},
        {NULL, NULL}
    };

    /*
    * New 'base' functions - These need to go into seperate file 
    */
    static int heart_luaB_print (lua_State *L) {
        int n = lua_gettop(L);  /* number of arguments */
        int i;
        lua_getglobal(L, "tostring");
        for (i=1; i<=n; i++) {
            const char *s;
            size_t l;
            lua_pushvalue(L, -1);  /* function to be called */
            lua_pushvalue(L, i);   /* value to print */
            lua_call(L, 1, 1);
            s = lua_tolstring(L, -1, &l);  /* get result */
            if (s == NULL)
                return luaL_error(L,
                LUA_QL("tostring") " must return a string to " LUA_QL("print"));
            if (i>1) hSystemConsole::printConsoleMessage("\t");
            hSystemConsole::printConsoleMessage(s);
            lua_pop(L, 1);  /* pop result */
        }
        hSystemConsole::printConsoleMessage("\n");
        return 0;
    }

    static const luaL_Reg heartBaseLib[] = {
        {"print", heart_luaB_print},
        {NULL, NULL}
    };

    int luaopen_heartbase(lua_State *L) 
    {
        // override the normal print with our version
        lua_pushglobaltable(L);
        luaL_setfuncs(L, heartBaseLib, 0);

        //pust base functions
        luaL_newlibtable(L,libcore);
        luaL_setfuncs(L,libcore,0);

        return 1;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

#ifndef HEART_LUA_SEARCHPATH
#   define HEART_LUA_SEARCHPATH "script:/?;script:/?.lua"
#endif
#ifndef HEART_LUA_READ_SIZE
#   define HEART_LUA_READ_SIZE (4*1024)
#endif
#if !defined (LUA_PATH_SEP)
#   define LUA_PATH_SEP		";"
#endif
#if !defined (LUA_PATH_MARK)
#   define LUA_PATH_MARK		"?"
#endif

    static const hChar* pushNextSearchPath(lua_State* L, const hChar* path) {
        while(*path == *LUA_PATH_SEP) path++;
        if (*path == 0) return NULL;
        const hChar* n = hStrChr(path, *LUA_PATH_SEP);/* find next separator */
        if (!n) n = path + hStrLen(path);
        lua_pushlstring(L, path, n-path);  /* template */
        return n;
    }

    static const char* luaReader(lua_State* L, void* data, size_t* size) {
        hIFile* file = (hIFile*)((void**)data)[0];
        char* buf = (char*)((void**)data)[1];
        *size = file->Read(buf, HEART_LUA_READ_SIZE);
        return size != 0 ? buf : NULL;
    }

    int heart_lua_require(lua_State* L) {
        HEART_LUA_GET_ENGINE(L);
        hIFileSystem* fsys = engine->GetFileManager();
        hIFile* file = NULL;
        void* lrdata[2];
        const hChar* name = luaL_checkstring(L,-1);
        lua_getfield(L, LUA_REGISTRYINDEX, "_hLOADED");
        lua_getfield(L, -1, name); //look for _hLOADED[name]
        if (lua_toboolean(L, -1)) return 1; //Loaded already
        // Attempt to load the module
        lua_pop(L, 1);// remove get _HEARTLOADED[name] result
        lua_getfield(L, LUA_REGISTRYINDEX, "_hSEARCHPATH");//get search string, default is HEART_LUA_SEARCHPATH
        if (!lua_isstring(L,-1)) return luaL_error(L, "require search path must be a string");
        const hChar* path = lua_tostring(L, -1);
        while (path = pushNextSearchPath(L, path)) {
            const hChar* filename = luaL_gsub(L, lua_tostring(L,-1), LUA_PATH_MARK, name);
            lua_remove(L, -2);//remove path template left by pushNextSearchPath
            file = fsys->OpenFile(filename, FILEMODE_READ);
            if (file) {
                //Opened file, read, parse and run
                lrdata[0] = (void*)file;
                lrdata[1] = hAlloca(HEART_LUA_READ_SIZE);
                int loadret = lua_load(L, luaReader, (void*)lrdata, filename, NULL);
                fsys->CloseFile(file);
                if (loadret != 0) {
                    return luaL_error(L, 
                        "error loading module %s from file %s: %s", 
                        name, filename, lua_tostring(L, -1));
                }
                if (lua_pcall(L, 0, LUA_MULTRET, 0) != 0) {
                    return luaL_error(L, 
                        "error loading module %s from file %s: %s", 
                        name, filename, lua_tostring(L, -1));
                }
                //module loaded ok, set _hLOADED[name] to true
                lua_getfield(L, LUA_REGISTRYINDEX, "_hLOADED");
                lua_pushboolean(L, 1);
                lua_setfield(L, -2, name);
                return 1;
            }
        }
        //if we get this far we failed to load the package
        return luaL_error(L, "error loading module \"%s\": file not found", name);
    }

    int heart_lua_require_flush(lua_State* L) {
        HEART_LUA_GET_ENGINE(L);
        hIFileSystem* fsys = engine->GetFileManager();
        hIFile* file = NULL;
        void* lrdata[2];
        const hChar* name = luaL_checkstring(L,-1);
        lua_getfield(L, LUA_REGISTRYINDEX, "_hSEARCHPATH");//get search string, default is HEART_LUA_SEARCHPATH
        if (!lua_isstring(L,-1)) return luaL_error(L, "require search path must be a string");
        const hChar* path = lua_tostring(L, -1);
        while (path = pushNextSearchPath(L, path)) {
            const hChar* filename = luaL_gsub(L, lua_tostring(L,-1), LUA_PATH_MARK, name);
            lua_remove(L, -2);//remove path template left by pushNextSearchPath
            file = fsys->OpenFile(filename, FILEMODE_READ);
            if (file) {
                //Opened file, read, parse and run
                lrdata[0] = (void*)file;
                lrdata[1] = hAlloca(HEART_LUA_READ_SIZE);
                int loadret = lua_load(L, luaReader, (void*)lrdata, filename, NULL);
                fsys->CloseFile(file);
                if (loadret != 0) {
                    return luaL_error(L, 
                        "error loading module %s from file %s: %s", 
                        name, filename, lua_tostring(L, -1));
                }
                if (lua_pcall(L, 0, LUA_MULTRET, 0) != 0) {
                    return luaL_error(L, 
                        "error loading module %s from file %s: %s", 
                        name, filename, lua_tostring(L, -1));
                }
                //module loaded ok, set _hLOADED[name] to true
                lua_getfield(L, LUA_REGISTRYINDEX, "_hLOADED");
                lua_pushboolean(L, 1);
                lua_setfield(L, -2, name);
                return 1;
            }
        }
        //if we get this far we failed to load the package
        return luaL_error(L, "error loading module \"%s\": file not found", name);
    }

    static const luaL_Reg heartBasePackageLib[] = {
        {"require", heart_lua_require},
        {"requireflush", heart_lua_require_flush},
        {NULL, NULL}
    };

    void heart_lpackage_open(lua_State* L, hHeartEngine* engine) {
        lua_newtable(L);
        lua_setfield(L, LUA_REGISTRYINDEX, "_hLOADED");
        
        lua_pushstring(L, HEART_LUA_SEARCHPATH);
        lua_setfield(L, LUA_REGISTRYINDEX, "_hSEARCHPATH");

        // push our version of require and other package releated things
        lua_pushglobaltable(L);
        lua_pushlightuserdata(L, engine);
        luaL_setfuncs(L, heartBasePackageLib, 1);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void OpenHeartLuaLib(lua_State* L, hHeartEngine* engine)
    {
        luaL_requiref(L, "heart", luaopen_heartbase, true);
        lua_pop(L, 1);//remove module from stack return by requiref

        lua_getglobal(L, "heart");
        hcAssertMsg(lua_istable(L, -1),"Type is not table, is %s",lua_typename(L, lua_type(L, -1)));
        lua_pushlightuserdata(L, engine);
        luaL_setfuncs(L,libcoreuv,1);
        lua_pop(L, 1);// pop heart module table

        heart_lpackage_open(L, engine);
    }
}
