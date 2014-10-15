/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/
#ifndef DEVICESOUNDCORE_H__
#define DEVICESOUNDCORE_H__

#include "base/hTypes.h"
#include <al.h>         //openAL
#include <alc.h>


namespace Heart
{
    class hdW32SoundVoiceDevice;

    class  hdW32SoundCoreDevice
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