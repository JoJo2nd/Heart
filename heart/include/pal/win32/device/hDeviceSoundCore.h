/********************************************************************

	filename: 	DeviceSoundCore.h	
	
	Copyright (c) 4:2:2012 James Moran
	
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
#ifndef DEVICESOUNDCORE_H__
#define DEVICESOUNDCORE_H__

namespace Heart
{
    class hdW32SoundVoiceDevice;

    class HEART_DLLEXPORT hdW32SoundCoreDevice
    {
    public:
        hdW32SoundCoreDevice()
            : alDevice_(NULL)
            , alContext_(NULL)
        {

        }
        ~hdW32SoundCoreDevice()
        {

        }

        void Initialise();
        void Destory();

    private:

        void                    CreateOpenALContext();

        ALCdevice*				alDevice_;
        ALCcontext*				alContext_;
    };

}

#endif // DEVICESOUNDCORE_H__