/********************************************************************

	filename: 	SceneNodeLight.cpp	
	
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

#include "stdafx.h"
#include "SceneNodeLight.h"
#include "SceneDatabase.h"
#include "DataLight.h"

namespace HScene
{
	BEGIN_ENUM_NAMES( LightType )
		ENUM_NAME( Direction )
		ENUM_NAME( Point )
		ENUM_NAME( Spot )
		ENUM_NAME( MAX )
	END_ENUM_NAMES();

	ACCEPT_VISITOR( Light );

	const char* Light::StaticNodeTypeName = "Light";

	//////////////////////////////////////////////////////////////////////////
	// 22:37:11 ////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	bool Light::Serialise( TiXmlElement* toelement )
	{
		Node::Serialise( toelement );

		XML_LINK_ENUM( lightType_, LightType, toelement );
		XML_LINK_VEC3( direction_, toelement );
		XML_LINK_FLOAT( attenuation_, toelement );
		XML_LINK_FLOAT( attenuationLinear_, toelement );
		XML_LINK_FLOAT( attenuationQuadratic_, toelement );
		XML_LINK_FLOAT( innerCone_, toelement );
		XML_LINK_FLOAT( outerCone_, toelement );
		XML_LINK_COLOUR( diffuse_, toelement );
		XML_LINK_COLOUR( specular_, toelement );
		XML_LINK_COLOUR( ambient_, toelement );
		XML_LINK_FLOAT( brightness_, toelement );
		XML_LINK_FLOAT( minRadius_, toelement );
		XML_LINK_FLOAT( maxRadius_, toelement );
		XML_LINK_FLOAT( falloff_, toelement );

		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	// 22:37:14 ////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	bool Light::Deserialise( TiXmlElement* fromelement )
	{
		if ( !Node::Deserialise( fromelement ) )
			return false;

		XML_UNLINK_ENUM( lightType_, LightType, fromelement );
		XML_UNLINK_VEC3( direction_, fromelement );
		XML_UNLINK_FLOAT( attenuation_, fromelement );
		XML_UNLINK_FLOAT( attenuationLinear_, fromelement );
		XML_UNLINK_FLOAT( attenuationQuadratic_, fromelement );
		XML_UNLINK_FLOAT( innerCone_, fromelement );
		XML_UNLINK_FLOAT( outerCone_, fromelement );
		XML_UNLINK_COLOUR( diffuse_, fromelement );
		XML_UNLINK_COLOUR( specular_, fromelement );
		XML_UNLINK_COLOUR( ambient_, fromelement );
		XML_UNLINK_FLOAT( brightness_, fromelement );
		XML_UNLINK_FLOAT( minRadius_, fromelement );
		XML_UNLINK_FLOAT( maxRadius_, fromelement );
		XML_UNLINK_FLOAT( falloff_, fromelement );

		return true;
	}

}