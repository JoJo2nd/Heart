/********************************************************************

	filename: 	DeviceConfig.h	
	
	Copyright (c) 23:7:2011 James Moran
	
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
#pragma once

#include "base/hTypes.h"
#include "core/hConfigOptions.h"

namespace Heart
{
	class hdDeviceConfig
	{
	public:
		virtual ~hdDeviceConfig() {}
        virtual hUint32 		getWidth() const { return hConfigurationVariables::getCVarUint("renderer.width", 640); }
        virtual void			setWidth(hUint32) {}
        virtual hUint32 		getHeight() const { return hConfigurationVariables::getCVarUint("renderer.height", 640); }
        virtual void 			setHeight(hUint32) {}
        virtual void 			getDataMember(const hChar* member_name, hUintptr_t* out) { *out = hConfigurationVariables::getCVarUint(member_name, 0); }
        virtual void 			setDataMember(const hChar* member_name, hUintptr_t) {}
        virtual void 			getDataMember(const hChar* member_name, hInt32* out) { *out = hConfigurationVariables::getCVarInt(member_name, 0); }
        virtual void 			setDataMember(const hChar* member_name, hInt32) {}
        virtual void 			getDataMember(const hChar* member_name, hFloat* out) { *out = hConfigurationVariables::getCVarFloat(member_name, 0.f); }
        virtual void 			setDataMember(const hChar* member_name, hFloat) {}
	};
}
