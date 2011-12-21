/********************************************************************

	filename: 	DataTexture.h	
	
	Copyright (c) 10:5:2011 James Moran
	
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
#ifndef DATATEXTURE_H__
#define DATATEXTURE_H__

#include <string>
#include "Serialiser.h"

namespace Data
{
	enum TextureFormat
	{
		TextureFormat_RGBA8,
		TextureFormat_RGB8,
		TextureFormat_DXT5,

		TextureFormat_MAX
	};

	DECLARE_ENUM_NAMES( TextureFormat );

	enum TextureInstanceXML
	{
		TextureInstanceXML_filename,
		TextureInstanceXML_format,
		
		TextureInstanceXML_MAX
	};

	DECLARE_ENUM_NAMES( TextureInstanceXML );

	class TextureInstance : public Serialiser
	{
	public:
		TextureInstance()
		{

		}
		~TextureInstance()
		{

		}

		std::string				filename_;
		TextureFormat			format_;

		bool			Serialise( TiXmlElement* toelement );
		bool			Deserialise( TiXmlElement* fromelement );
	};

}

#endif // DATATEXTURE_H__