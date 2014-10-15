/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/
#ifndef _CONFIGOPTIONS_H__
#define _CONFIGOPTIONS_H__

#include "base/hTypes.h"
extern "C" {
#   include "lua.h"
#   include "lualib.h"
#   include "lauxlib.h"
}

namespace Heart {
class hIFileSystem;

namespace hConfigurationVariables {

void         loadCVars(lua_State* L, hIFileSystem* file_system);
hUint        getCVarUint(const hChar* key, hUint defval);
hInt         getCVarInt(const hChar* key, hInt defval);
hFloat       getCVarFloat(const hChar* key, hFloat defval);
hBool        getCVarBool(const hChar* key, hBool defval);
const hChar* getCVarStr(const hChar* key, const hChar* defval);

}
}

#endif // _CONFIGOPTIONS_H__