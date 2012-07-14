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

namespace Heart
{

	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////

	void hConfigOptions::ReadConfig( const hChar* filename, hIFileSystem* filesystem )
	{
		hBool readDefaults = hTrue;

		hIFile* file = filesystem->OpenFileRoot( filename, FILEMODE_READ );
		if ( file )
		{
			hUint32 read;
			char* data = (hChar*)hHeapMalloc(GetGlobalHeap(), (hUint32)file->Length());
			read = file->Read( data, (hUint32)file->Length() ) ;

			if ( read == file->Length() )
			{
				
                if ( doc_.ParseSafe< rapidxml::parse_default >(data, GetGlobalHeap()) )
                {
                    hXMLGetter getter( &doc_ );
                    rapidxml::xml_node<>* fullscreen = getter.FirstChild("renderer").FirstChild("fullscreen").ToNode();
                    rapidxml::xml_node<>* vsync = getter.FirstChild("renderer").FirstChild( "vsync" ).ToNode();
                    rapidxml::xml_node<>* width = getter.FirstChild("renderer").FirstChild( "width" ).ToNode();
                    rapidxml::xml_node<>* height = getter.FirstChild("renderer").FirstChild( "height" ).ToNode();

                    fullscreen_ = fullscreen && fullscreen->value() ? strcmp( fullscreen->value(), "true" ) == 0 : Device::DefaultFullscreenSetting();
                    width_ = width && width->value() ? atoi( width->value() ) : Device::DefaultScreenWidth();
                    height_ = height && height->value() ? atoi( height->value() ) : Device::DefaultScreenHeight();
                    vsync_ = vsync && vsync->value() ? strcmp( vsync->value(), "true" ) == 0 : Device::DefaultVsyncSetting();

                    readDefaults = hFalse;
                }
			}

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