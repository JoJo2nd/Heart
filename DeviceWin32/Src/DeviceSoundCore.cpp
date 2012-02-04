/********************************************************************

	filename: 	DeviceSoundCore.cpp	
	
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

namespace Heart
{
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdW32SoundCoreDevice::Initialise()
    {
        if ( !alcIsExtensionPresent( NULL, "ALC_ENUMERATION_EXT" ) )
        {
            //Attempt to open the default device
            alDevice_ = alcOpenDevice( NULL );
            CreateOpenALContext();

        }
        else
        {
            const char* deviceNames = alGetString( ALC_DEVICE_SPECIFIER );
            const char* defaultDeviceName = alGetString( ALC_DEFAULT_DEVICE_SPECIFIER );

            alDevice_ = alcOpenDevice( defaultDeviceName );
            if ( alDevice_ )
            {
                CreateOpenALContext();
                return;
            }

            do 
            {
                alDevice_ = alcOpenDevice( deviceNames );
                if ( alDevice_ )
                {
                    CreateOpenALContext();
                    return;
                }
                //Get next possible device name
                if ( deviceNames )
                {
                    deviceNames += strlen( deviceNames ) + 1;
                }
            }
            while ( deviceNames && *deviceNames != '\0' );
        }
    }


    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdW32SoundCoreDevice::CreateOpenALContext()
    {
        alContext_ = alcCreateContext( alDevice_, NULL );
        alcMakeContextCurrent( alContext_ );
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdW32SoundCoreDevice::Destory()
    {

    }

}