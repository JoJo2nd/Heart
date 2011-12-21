/********************************************************************

	filename: 	DataTexture.cpp
	
	Copyright (c) 2011/05/11 James Moran
	
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
#include "DataTexture.h"
#include "tinyxml.h"

namespace Data
{

	BEGIN_ENUM_NAMES( TextureFormat )
		ENUM_NAME( RGBA8 )
		ENUM_NAME( RGB8 )
		ENUM_NAME( DXT5 )
		ENUM_NAME( MAX )
	END_ENUM_NAMES();

	BEGIN_ENUM_NAMES( TextureInstanceXML )
		ENUM_NAME( filename )
		ENUM_NAME( format )
		ENUM_NAME( MAX )
	END_ENUM_NAMES();

	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////

	bool TextureInstance::Serialise( TiXmlElement* toelement )
	{
		XML_LINK_STRING( filename_, toelement );
		XML_LINK_ENUM( format_, TextureFormat, toelement );

		return true;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////

	bool TextureInstance::Deserialise( TiXmlElement* fromelement )
	{
		XML_UNLINK_STRING( filename_, fromelement );
		XML_UNLINK_ENUM( format_, TextureFormat, fromelement );

		return true;
	}

}