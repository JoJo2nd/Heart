/********************************************************************

	filename: 	SceneNodeCamera.cpp	
	
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
#include "SceneNodeCamera.h"

namespace HScene
{
	ACCEPT_VISITOR( Camera );

	const char* Camera::StaticNodeTypeName = "Camera";

	//////////////////////////////////////////////////////////////////////////
	// 22:37:28 ////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	bool Camera::Serialise( TiXmlElement* toelement )
	{
		Node::Serialise( toelement );

		XML_LINK_FLOAT( aspect_, toelement );
		XML_LINK_FLOAT( nearPlane_, toelement );
		XML_LINK_FLOAT( farPlane_, toelement );
		XML_LINK_FLOAT( fov_, toelement );

		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	// 22:37:32 ////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	bool Camera::Deserialise( TiXmlElement* fromelement )
	{
		Node::Deserialise( fromelement );

		XML_UNLINK_FLOAT( aspect_, fromelement );
		XML_UNLINK_FLOAT( nearPlane_, fromelement );
		XML_UNLINK_FLOAT( farPlane_, fromelement );
		XML_UNLINK_FLOAT( fov_, fromelement );

		return true;
	}

}