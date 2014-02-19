/********************************************************************

    filename:   lwrapper.cpp  
    
    Copyright (c) 22:12:2013 James Moran
    
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

#include "types.h"
#include "dbserver.h"
#include "dbclient.h"
extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
};

#include <future>
#include <chrono>
#include <iostream>
#include <string>
#include <sstream>


#if defined (luadb_EXPORTS)
#   define ldb_dll_export __declspec(dllexport)
#else
#   define ldb_dll_export
#endif

static const char *const luaT_typenames_[] = {
    "no value",
    "nil", "boolean", "userdata", "number",
    "string", "table", "function", "userdata", "thread",
    "proto", "upval"  /* these last two cases are used for tests only */
};

#define ldb_checkclient(L, si) (ldbClient*)luaL_checkudata(L, si, "luadb.client")

struct ldbClientLuaState {
    ldb_int debugMsgRef_;
};

static int ldb_api ldb_attach(lua_State* L) {
    int port = luaL_checkint(L, -1);
    ldb_bool result = ldbDebugServer::get()->attach(L, port);
    lua_pushboolean(L, result);
    return 1;
}

static int ldb_api ldb_dettach(lua_State* L) {
    ldb_bool result = ldbDebugServer::get()->dettach();
    lua_pushboolean(L, result);
    return 1;
}

static int ldb_api ldb_new_client(lua_State* L) {
    ldbClient* client = new (lua_newuserdata(L, sizeof(ldbClient)+sizeof(ldbClientLuaState))) ldbClient;
    ldbClientLuaState* lstate = (ldbClientLuaState*)(client+1);
    lstate->debugMsgRef_ = LUA_REFNIL;
    luaL_getmetatable(L, "luadb.client");
    lua_setmetatable(L, -2);
    return 1;//return the userdata
}

static int ldb_api ldb_client_tick(lua_State* L) {
    ldbClient* client = ldb_checkclient(L, 1);
    ldb_int timelimit=luaL_checkint(L, 2);
    ldbClient::ldbTickResults results;
    client->tickMultipleEvents(timelimit, &results);
    return 0;
}

static int ldb_api ldb_client_connect(lua_State* L) {
    ldbClient* client=ldb_checkclient(L, 1);
    const char* address=luaL_checkstring(L, 2);
    int port=luaL_checkint(L, 3);
    lua_pushboolean(L, client->connect(ldb_string(address), port));
    return 1;
}

static int ldb_api ldb_client_isrunning(lua_State* L) {
    ldbClient* client = ldb_checkclient(L, -1);
    lua_pushboolean(L, !!client->isRunning());
    return 1;
}

static int ldb_api ldb_client_isconnected(lua_State* L) {
    ldbClient* client = ldb_checkclient(L, -1);
    lua_pushboolean(L, !!client->isConnected());
    return 1;
}

static int ldb_api ldb_client_debuggerexit(lua_State* L) {
    ldbClient* client = ldb_checkclient(L, -1);
    lua_pushboolean(L, !!client->debuggerExit());
    return 1;
}

static int ldb_api ldb_client_getcurrentfileline(lua_State* L) {
    ldbClient* client = ldb_checkclient(L, -1);
    lua_pushstring(L, client->getCurrentFile().c_str());
    lua_pushinteger(L, client->getCurrentLine());
    return 2;
}

static int ldb_api ldb_client_run(lua_State* L) {
    ldbClient* client = ldb_checkclient(L, -1);
    lua_pushboolean(L, client->run());
    return 1;
}

static int ldb_api ldb_client_trap(lua_State* L) {
    ldbClient* client = ldb_checkclient(L, -1);
    lua_pushboolean(L, client->trap());
    return 1;
}

static int ldb_api ldb_client_step(lua_State* L) {
    ldbClient* client=ldb_checkclient(L, -1);
    lua_pushboolean(L, client->step());
    return 1;
}

static int ldb_api ldb_client_stepin(lua_State* L) {
    ldbClient* client=ldb_checkclient(L, -1);
    lua_pushboolean(L, client->stepIn());
    return 1;
}

static int ldb_api ldb_client_stepout(lua_State* L) {
    ldbClient* client=ldb_checkclient(L, -1);
    lua_pushboolean(L, client->stepOut());
    return 1;
}

static int ldb_api ldb_client_setbreakpoint(lua_State* L) {
    ldbClient* client=ldb_checkclient(L, 1);
    const char* filename=luaL_checkstring(L, 2);
    int line=luaL_checkint(L, 3);
    lua_pushboolean(L, client->setBreakpoint(ldb_string(filename), line, true));
    return 1;
}

static int ldb_api ldb_client_removebreakpoint(lua_State* L) {
    ldbClient* client=ldb_checkclient(L, 1);
    const char* filename=luaL_checkstring(L, 2);
    int line=luaL_checkint(L, 3);
    lua_pushboolean(L, client->setBreakpoint(ldb_string(filename), line, false));
    return 1;
}

static int ldb_api ldb_client_backtrace(lua_State* L) {
    ldbClient* client=ldb_checkclient(L, 1);

    if (!client->getBacktrace()) {
        lua_pushnil(L);
        return 1;
    }

    lua_newtable(L);
    for (ldb_uint i=0, n=client->getBacktraceSize(); i<n; ++i) {
        const ldbStackLevel* level=client->getBacktraceLevel(i);
        lua_newtable(L);
        //stack[i].name = 
        lua_pushstring(L, level->name_.c_str());
        lua_setfield(L, -2, "name");
        //stack[i].namewhat = 
        switch(level->nameType_) {
        case eNameType_Global: lua_pushstring(L, "global"); break;
        case eNameType_Local: lua_pushstring(L, "local"); break;
        case eNameType_Method: lua_pushstring(L, "method"); break;
        case eNameType_Field: lua_pushstring(L, "field"); break;
        case eNameType_UpValue: lua_pushstring(L, "upvalue"); break;
        case eNameType_None:
        default: lua_pushstring(L, ""); break;
        }
        lua_setfield(L, -2, "namewhat");
        //stack[i].what = 
        switch(level->funcType_) {
        case eFunctionType_C: lua_pushstring(L, "C"); break;
        case eFunctionType_Lua: lua_pushstring(L, "Lua"); break;
        case eFunctionType_main: lua_pushstring(L, "main"); break;
        default: lua_pushnil(L);
        }
        lua_setfield(L, -2, "what");
        //stack[i].source = 
        lua_pushstring(L, level->source_.c_str());
        lua_setfield(L, -2, "source");
        //stack[i].shortsrc = 
        lua_pushstring(L, level->shortSrc_.c_str());
        lua_setfield(L, -2, "shortsrc");
        //stack[i].currentline
        lua_pushinteger(L, level->currentLine_);
        lua_setfield(L, -2, "currentline");
        //stack[i] = newtable
        lua_pushinteger(L, i+1); // key index
        lua_pushvalue(L, -2); // push the table we just filled back to top
        lua_settable(L, -4);
        lua_pop(L, 1);// pop the orginial ref to the table
    }
    //return the table
    return 1;
}

static void pushNewVarDescTable(lua_State* L, const ldbVarDesc& i) {
    lua_newtable(L);
    lua_pushstring(L, i.name_.c_str());
    lua_setfield(L, -2, "name");
    if (!i.printableValue_.empty()) {
        lua_pushstring(L, i.printableValue_.c_str());
        lua_setfield(L, -2, "printablevalue");
    }
    lua_pushstring(L, luaT_typenames_[i.type_+1]);
    lua_setfield(L, -2, "typename");

    lua_newtable(L);
    ldb_int fieldidx=1;
    for (auto ifield=i.fields_.begin(), nfield=i.fields_.end(); ifield!=nfield; ++ifield, ++fieldidx) {
        lua_newtable(L);
        lua_pushstring(L, ifield->printableKey_.c_str());
        lua_setfield(L, -2, "printablekey");
        lua_pushstring(L, ifield->printableValue_.c_str());
        lua_setfield(L, -2, "printablevalue");
        lua_pushstring(L, luaT_typenames_[ifield->type_+1]);
        lua_setfield(L, -2, "typename");
        lua_pushinteger(L, ifield->entryIndex_);
        lua_setfield(L, -2, "entryindex");

        lua_rawseti(L, -2, fieldidx);
    }
    lua_setfield(L, -2, "fields");
}

static int ldb_api ldb_client_getlocals(lua_State* L) {
    ldbClient* client=ldb_checkclient(L, 1);
    ldb_int stacklevel=luaL_checkint(L, 2);

    if (stacklevel < 0) {
        luaL_error(L, "Stack index should be greater or equal to 0");
        return 0;
    }

    std::vector<ldbVarDesc> locals;
    client->getLocals(stacklevel, &locals);

    // push results
    lua_newtable(L);
    ldb_int tableidx=1;
    for (auto i=locals.begin(), n=locals.end(); i!=n; ++i, ++tableidx) {
        pushNewVarDescTable(L, *i);
        lua_rawseti(L, -2, tableidx);
    }

    return 1;
}

static int ldb_api ldb_client_getmoreinfo(lua_State* L) {
    ldbClient* client=ldb_checkclient(L, 1);
    ldb_int varEntryID=luaL_checkint(L, 2);
    //TODO: !! ERROR CHECKK THIS
    ldbVarDesc var;
    client->getMoreInfo(varEntryID, &var);

    // push results
    pushNewVarDescTable(L, var);

    return 1;
}

static int ldb_api ldb_client_getwatch(lua_State* L) {
    ldbClient* client=ldb_checkclient(L, 1);
    const ldb_char* varname=luaL_checkstring(L, 2);
    ldb_int stacklevel=luaL_checkint(L, 3);
    //TODO: !! ERROR CHECKK THIS
    ldbVarDesc var;
    var.type_ = eLuaType_None;
    client->getWatch(varname, stacklevel, &var);

    // push results
    if (var.type_ == eLuaType_None) {
        return 0;
    }

    pushNewVarDescTable(L, var);
    return 1;
}

static int ldb_api ldb_client_disconnect(lua_State* L) {
    ldbClient* client=ldb_checkclient(L, -1);
    client->disconnect();
    return 0;
}

static int ldb_api ldb_client_setdebugmsg(lua_State* L) {
    ldbClient* client=ldb_checkclient(L, 1);
    ldbClientLuaState* lstate=(ldbClientLuaState*)(client+1);
    if (lua_isfunction(L, 2)) {
        if (lstate->debugMsgRef_ == LUA_REFNIL) {
            lua_pushvalue(L, 2);
            lstate->debugMsgRef_ = luaL_ref(L, LUA_REGISTRYINDEX);
        } else {
            lua_pushvalue(L, 2);
            lua_rawseti(L, LUA_REGISTRYINDEX, lstate->debugMsgRef_);
        }
        auto msgfunc = [=] (const ldb_char* str) { 
            lua_rawgeti(L, LUA_REGISTRYINDEX, lstate->debugMsgRef_);
            lua_pushstring(L, str);
            lua_pcall(L, 1, 0, 0); //ignore any errors from this
        };
        client->setMessageDelegate(msgfunc);
    } else if (lua_isnil(L, 2)) {
        lua_rawseti(L, LUA_REGISTRYINDEX, lstate->debugMsgRef_);
        client->setMessageDelegate(ldb_debugMsgDelegate());
    } else {
        luaL_error(L, "bad arg at param 2, expected function or nil got %s", lua_typename(L, 2));
        return 0;
    }

    return 0;
}

static int ldb_client_addsearchpath(lua_State* L) {
    ldbClient* client=ldb_checkclient(L, 1);
    const ldb_char* path=luaL_checkstring(L, 2);
    client->addSearchPath(path);
    return 0;
}

static int ldb_api ldb_client_gc(lua_State* L) {
    ldbClient* client = ldb_checkclient(L, -1);
    ldbClientLuaState* lstate=(ldbClientLuaState*)(client+1);
    luaL_unref(L, LUA_REGISTRYINDEX, lstate->debugMsgRef_);
    client->~ldbClient();
    return 0;
}

static const luaL_Reg ldb_client_lib[] = {
    {"__gc", ldb_client_gc},
    {"tick", ldb_client_tick},
    {"connect", ldb_client_connect},
    {"isrunning", ldb_client_isrunning},
    {"isconnected", ldb_client_isconnected},
    {"debuggerexit", ldb_client_debuggerexit},
    {"getcurrentfileline", ldb_client_getcurrentfileline},
    {"run", ldb_client_run},
    {"step", ldb_client_step},
    {"stepin", ldb_client_stepin},
    {"stepout", ldb_client_stepout},
    {"trap", ldb_client_trap},
    {"setbreakpoint", ldb_client_setbreakpoint},
    {"removebreakpoint", ldb_client_removebreakpoint},
    {"backtrace", ldb_client_backtrace},
    {"getlocals", ldb_client_getlocals},
    {"getmoreinfo", ldb_client_getmoreinfo},
    {"getwatch", ldb_client_getwatch},
    {"disconnect", ldb_client_disconnect},
    {"setdebugmsg", ldb_client_setdebugmsg},
    {"addsearchpath", ldb_client_addsearchpath},
    {nullptr, nullptr}
};

static const luaL_Reg ldb_lib[] = {
    {"attach", ldb_attach},
    {"dettach", ldb_dettach},
    {"newclient", ldb_new_client},
    {nullptr, nullptr}
};

extern "C" {

ldb_dll_export int ldb_api luaopen_luadb(lua_State* L) {
    luaL_newmetatable(L, "luadb.client");
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    luaL_setfuncs(L, ldb_client_lib, 0);

    luaL_newlib(L, ldb_lib);

    enet_initialize();
    return 1;
}

}