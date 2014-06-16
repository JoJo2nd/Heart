/********************************************************************

    filename: 	hConfigOptions.h
    
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