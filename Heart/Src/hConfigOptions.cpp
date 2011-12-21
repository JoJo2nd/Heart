/********************************************************************

	filename: 	hConfigOptions.cpp
	
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

#include "Common.h"
#include "hConfigOptions.h"
#include "hIFileSystem.h"
#include "hIFile.h"
#include "tinyxml\tinyxml.h"
#include "DeviceKernel.h"

namespace Heart
{


	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////

	void hConfigOptions::ReadConfig( const hChar* filename, hIFileSystem* filesystem )
	{
		hBool readDefaults = hTrue;

		hIFile* file = filesystem->OpenFile( filename, FILEMODE_READ );
		if ( file )
		{
			hUint32 read;
			char* data = hNEW ( hGeneralHeap ) char[(hUint32)file->Length()];
			read = file->Read( data, (hUint32)file->Length() ) ;

			if ( read == file->Length() )
			{
				
				doc_.Parse( data );
			
				TiXmlHandle docHandle( &doc_ );
				TiXmlElement* fullscreen = docHandle.FirstChild( "renderer" ).FirstChild( "fullscreen" ).ToElement();
				TiXmlElement* vsync = docHandle.FirstChild( "renderer" ).FirstChild( "vsync" ).ToElement();
				TiXmlElement* width = docHandle.FirstChild( "renderer" ).FirstChild( "width" ).ToElement();
				TiXmlElement* height = docHandle.FirstChild( "renderer" ).FirstChild( "height" ).ToElement();
				
				fullscreen_ = fullscreen && fullscreen->GetText() ? strcmp( fullscreen->GetText(), "true" ) == 0 : Device::DefaultFullscreenSetting();
				width_ = width && width->GetText() ? atoi( width->GetText() ) : Device::DefaultScreenWidth();
				height_ = height && height->GetText() ? atoi( height->GetText() ) : Device::DefaultScreenHeight();
				vsync_ = vsync && vsync->GetText() ? strcmp( vsync->GetText(), "true" ) == 0 : Device::DefaultVsyncSetting();

				readDefaults = hFalse;
			}

			delete[] data;
			filesystem->CloseFile( file );
		}

		//
		if ( readDefaults )
		{
			fullscreen_ = Device::DefaultFullscreenSetting();
			width_		= Device::DefaultScreenWidth();
			height_		= Device::DefaultScreenHeight();
			vsync_		= Device::DefaultVsyncSetting();
		}
	}

}