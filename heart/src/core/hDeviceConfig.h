/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
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
