/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#include "pal/hDeviceSoundCore.h"
#include "pal/hDeviceSoundUtil.h"
#include "base/hStringUtil.h"

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
            HEART_CHECK_OPENAL_ERRORS();
            CreateOpenALContext();

        }
        else
        {
            const char* deviceNames = alGetString( ALC_DEVICE_SPECIFIER );
            const char* defaultDeviceName = alGetString( ALC_DEFAULT_DEVICE_SPECIFIER );

            alDevice_ = alcOpenDevice( defaultDeviceName );
            HEART_CHECK_OPENAL_ERRORS();
            if ( alDevice_ )
            {
                CreateOpenALContext();
                return;
            }

            do 
            {
                alDevice_ = alcOpenDevice( deviceNames );
                HEART_CHECK_OPENAL_ERRORS();
                if ( alDevice_ )
                {
                    CreateOpenALContext();
                    return;
                }
                //Get next possible device name
                if ( deviceNames )
                {
                    deviceNames += hStrLen( deviceNames ) + 1;
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
        HEART_CHECK_OPENAL_ERRORS();
        alcMakeContextCurrent( alContext_ );
        HEART_CHECK_OPENAL_ERRORS();

        hcPrintf( "OpenAL Device: 0x%08X Context 0x%08X", alDevice_, alContext_ );
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdW32SoundCoreDevice::Destory()
    {
        alcDestroyContext( alContext_ );
        HEART_CHECK_OPENAL_ERRORS();
        alcCloseDevice( alDevice_ );
        HEART_CHECK_OPENAL_ERRORS();

        alContext_ = NULL;
        alDevice_ = NULL;
    }

}