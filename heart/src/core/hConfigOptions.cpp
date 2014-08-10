/********************************************************************

    filename: 	hConfigOptions.cpp
    
    Copyright (c) 2011/06/17 James Moran
    
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

#include "core/hConfigOptions.h"
#include "core/hIFile.h"
#include "core/hIFileSystem.h"

namespace Heart {
namespace hConfigurationVariables {
namespace {
    static lua_State* lua_;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

void loadCVars(lua_State* L, hIFileSystem* file_system)
{
    lua_ = L;
    hChar* script=nullptr;
    auto* file = file_system->OpenFile("/script/config.lua", FILEMODE_READ);
    if (file->getIsMemMapped()) {
        script=(hChar*)file->getMemoryMappedBase();
    } else {
        script = new hChar[file->Length()+1];
        file->Read(script, (hUint)file->Length());
    }
    if (int error = luaL_loadbuffer(lua_, script, file->Length(), "/script/config.lua") != LUA_OK) {
        hcAssertFailMsg("config.lua Failed to compile, Error: %d\nScript:%s", error, script);
    } else if (lua_pcall(lua_, 0, LUA_MULTRET, 0) != 0) {
        hcAssertFailMsg("config.lua Failed to run, Error: %s", lua_tostring(lua_, -1));
        lua_pop(lua_, 1);
    }
    if (!file->getIsMemMapped()) {
        delete script;
    }
    file_system->CloseFile(file);
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
