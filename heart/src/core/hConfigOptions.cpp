/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#include "core/hConfigOptions.h"
#include "core/hIFileSystem.h"
#include <memory>

namespace Heart {
namespace hConfigurationVariables {
namespace {
    static lua_State* lua_;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

void loadCVars(lua_State* L, const hChar* script_text, hSize_t text_len)
{
    lua_ = L;

    if (int error = luaL_loadbuffer(lua_, script_text, text_len, "/startup.lua") != LUA_OK) {
        hcAssertFailMsg("config.lua Failed to compile, Error: %d", error);
    } else if (lua_pcall(lua_, 0, LUA_MULTRET, 0) != 0) {
        hcAssertFailMsg("config.lua Failed to run, Error: %s", lua_tostring(lua_, -1));
        lua_pop(lua_, 1);
    }
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

hUint getCVarUint(const hChar* key, hUint defval)  {
    hcAssert(lua_);
    hInt t=lua_gettop(lua_);
    lua_getfield(lua_, LUA_REGISTRYINDEX, "_hCVARS");
    if (!lua_istable(lua_, -1)) {
        lua_settop(lua_, t);
        return defval;
    }
    lua_getfield(lua_, -1, key);
    hUint retval = defval;
    if (lua_isnumber(lua_, -1)) {
        retval = (hUint)lua_tointeger(lua_, -1);
    }
    lua_settop(lua_, t);
    return retval;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

hInt getCVarInt(const hChar* key, hInt defval) {
    hcAssert(lua_);
    hInt t=lua_gettop(lua_);
    lua_getfield(lua_, LUA_REGISTRYINDEX, "_hCVARS");
    if (!lua_istable(lua_, -1)) {
        lua_settop(lua_, t);
        return defval;
    }
    lua_getfield(lua_, -1, key);
    hInt retval = defval;
    if (lua_isnumber(lua_, -1)) {
        retval = (hInt)lua_tointeger(lua_, -1);
    }
    lua_settop(lua_, t);
    return retval;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

hFloat getCVarFloat(const hChar* key, hFloat defval) {
    hcAssert(lua_);
    hInt t=lua_gettop(lua_);
    lua_getfield(lua_, LUA_REGISTRYINDEX, "_hCVARS");
    if (!lua_istable(lua_, -1)) {
        lua_settop(lua_, t);
        return defval;
    }
    lua_getfield(lua_, -1, key);
    hFloat retval = defval;
    if (lua_isnumber(lua_, -1)) {
        retval = (hFloat)lua_tonumber(lua_, -1);
    }
    lua_settop(lua_, t);
    return retval;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

hBool getCVarBool(const hChar* key, hBool defval) {
    hcAssert(lua_);
    hInt t=lua_gettop(lua_);
    lua_getfield(lua_, LUA_REGISTRYINDEX, "_hCVARS");
    if (!lua_istable(lua_, -1)) {
        lua_settop(lua_, t);
        return defval;
    }
    lua_getfield(lua_, -1, key);
    hBool retval = defval;
    if (lua_isboolean(lua_, -1)) {
        retval = !!lua_toboolean(lua_, -1);
    }
    lua_settop(lua_, t);
    return retval;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

const hChar* getCVarStr(const hChar* key, const hChar* defval) {
    hcAssert(lua_);
    hInt t=lua_gettop(lua_);
    lua_getfield(lua_, LUA_REGISTRYINDEX, "_hCVARS");
    if (!lua_istable(lua_, -1)) {
        lua_settop(lua_, t);
        return defval;
    }
    lua_getfield(lua_, -1, key);
    const hChar* retval = defval;
    if (lua_isstring(lua_, -1)) {
        retval = lua_tostring(lua_, -1);
    }
    lua_settop(lua_, t);
    return retval;
}
}
}
