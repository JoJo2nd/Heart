/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#pragma once

#include "base/hTypes.h"
#include "base/hConfigOptionsInterface.h"
extern "C" {
#   include "lua.h"
#   include "lualib.h"
#   include "lauxlib.h"
}

namespace Heart {
class hIFileSystem;

namespace hConfigurationVariables {

void         loadCVars(lua_State* L, const hChar* script_text, hSize_t text_len);
hUint        getCVarUint(const hChar* key, hUint defval);
hInt         getCVarInt(const hChar* key, hInt defval);
hFloat       getCVarFloat(const hChar* key, hFloat defval);
hBool        getCVarBool(const hChar* key, hBool defval);
const hChar* getCVarStr(const hChar* key, const hChar* defval);

}

// Interface for passing to plug-in libs.
class hConfigurationVariablesImpl : public hIConfigurationVariables {
public:
    hUint getCVarUint(const hChar* key, hUint defval) override {
        return hConfigurationVariables::getCVarUint(key, defval);
    }
    hInt getCVarInt(const hChar* key, hInt defval) override {
        return hConfigurationVariables::getCVarInt(key, defval);
    }
    hFloat getCVarFloat(const hChar* key, hFloat defval) override {
        return hConfigurationVariables::getCVarFloat(key, defval);
    }
    hBool getCVarBool(const hChar* key, hBool defval) override {
        return hConfigurationVariables::getCVarBool(key, defval);
    }
    const hChar* getCVarStr(const hChar* key, const hChar* defval) override {
        return hConfigurationVariables::getCVarStr(key, defval);
    }
};
}

