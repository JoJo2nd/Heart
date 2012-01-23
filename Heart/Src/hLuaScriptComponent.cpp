/********************************************************************

	filename: 	hLuaScriptComponent.cpp	
	
	Copyright (c) 23:1:2012 James Moran
	
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

#include "Common.h"
#include "hTypes.h"
#include "hLuaScriptComponent.h"
#include "hCRC32.h"

namespace Heart
{
    HEART_DEFINE_COMPONENT_TYPE( hLuaScriptComponent, "ScriptComponent", "A Component to link a script to an entity." );
    HEART_COMPONET_PROPERTIES_BEGIN( hLuaScriptComponent )
        HEART_COMPONENT_PROPERTY( hLuaScriptComponent, "Auto Run",      autoLoadScript_,    Bool,           "Automatically run script when object is created." )
        HEART_COMPONENT_PROPERTY( hLuaScriptComponent, "Script Asset",  resourceCRC_,       ResourceAsset,  "Script asset to use." )
        HEART_COMPONENT_PROPERTY( hLuaScriptComponent, "Auto Run",      scriptName_,        String,         "Name of script in debugger." )
    HEART_COMPONET_PROPERTIES_END( hLuaScriptComponent );
}