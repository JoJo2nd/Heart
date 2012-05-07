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
#ifndef DEVICECONFIG_H__
#define DEVICECONFIG_H__

namespace Heart
{
	class hdDeviceConfig
	{
	public:
		static const hUint32			HOME_DIRECTORY_MAX_LEN = 2048;
		static const hUint32			WNDCLASSNAMELEN = 256;

		HINSTANCE		Instance_;
		hChar			pWorkingDir_[ HOME_DIRECTORY_MAX_LEN ];
		hChar			classname_[ WNDCLASSNAMELEN ];
		hUint32			Width_;
		hUint32			Height_;
	};

    int WINAPI HeartMain( HINSTANCE hinstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow );
}

#endif // DEVICECONFIG_H__