/********************************************************************

	filename: 	SceneNodeLight.h	
	
	Copyright (c) 16:1:2011 James Moran
	
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

#ifndef SCENENODELIGHT_H__
#define SCENENODELIGHT_H__

#include "stdafx.h"
#include "SceneNode.h"
#include "SceneVisitor.h"
#include "SceneDatabase.h"

namespace Data
{
	class Light;
}

namespace HScene
{
	enum LightType
	{
		LightType_Direction,
		LightType_Point,
		LightType_Spot,
		LightType_MAX
	};

	DECLARE_ENUM_NAMES( LightType );

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	class Light : public Node
	{
	public:
		Light() : 
			Node( "Light" )
		{

		}
		virtual ~Light()
		{

		}

		DEFINE_VISITABLE( Light );

		virtual bool		Serialise( TiXmlElement* toelement );
		virtual bool		Deserialise( TiXmlElement* fromelement );

		LightType			lightType_;
		Vec3				direction_;
		float				brightness_;
		//Atten = 1/( att0 + att1 * d + att2 * d*d)
		float				attenuation_;//att0
		float				attenuationLinear_;//arr1
		float				attenuationQuadratic_;//arr2
		float				innerCone_;//in rads
		float				outerCone_;//in rads
		float				falloff_;
		float				minRadius_;
		float				maxRadius_;
		Data::Colour		diffuse_;
		Data::Colour		specular_;
		Data::Colour		ambient_;

		static const char*  StaticNodeTypeName;

	};
}

#endif // SCENENODELIGHT_H__