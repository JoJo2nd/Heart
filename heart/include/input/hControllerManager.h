/********************************************************************

	filename: 	hControllerManager.h	
	
	Copyright (c) 1:4:2012 James Moran
	
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

#ifndef HOCONTROLLERMANAGER_H__
#define HOCONTROLLERMANAGER_H__

namespace Heart
{
namespace Events
{
	class EventManager;
}

	class hControllerManager : public hdInputDeviceManager
	{
	public:

        HEART_BASECLASS(hdInputDeviceManager);

		hControllerManager();
		~hControllerManager();

		void						InitPad( hUint32 port, const ControllerConfig* config )
		{
			controllers_[ port ].SetPadPort( port );
		}
		hController*					pController( hUint32 i )
		{
			return &controllers_[ i ];
		}

	private:
		
		hUint32						nControllers_;
		hController					controllers_[ 4 ];

	};
}

#endif // HOCONTROLLERMANAGER_H__