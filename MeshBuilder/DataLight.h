/********************************************************************

	filename: 	DataLight.h	
	
	Copyright (c) 5:3:2011 James Moran
	
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

#ifndef DATALIGHT_H__
#define DATALIGHT_H__

#include <string>

namespace Data
{
	enum LightSourceType
	{
		LIGHTSOURCETYPE_POINT,
		LIGHTSOURCETYPE_DIRECTION,
		LIGHTSOURCETYPE_SPOT,

		LIGHTSOURCETYPE_MAX
	};

	class Light
	{
	public:

		std::string name_;
		LightSourceType type_;
		D3DXVECTOR3 position_;
		D3DXVECTOR3 direction_;
		float attenuationConstant_;
		float attenuationLinear_;
		float attenuationQuadratic_;
		Colour colorDiffuse_;
		Colour colorSpecular_;
		Colour colorAmbient_;
		float angleInnerCone_;
		float angleOuterCone_;
	};
}

#endif // DATALIGHT_H__