/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/
#ifndef DEVICESOUNDVOICE_H__
#define DEVICESOUNDVOICE_H__

#include "base/hTypes.h"
#include "base/hFunctor.h"
#include "pal/hDeviceSoundUtil.h"
#include <al.h>         //openAL
#include <alc.h>

namespace Heart
{
    class hdW32SoundVoiceDevice;

    hFUNCTOR_TYPEDEF(void (*)(hdW32SoundVoiceDevice* , hdSoundCallbackReason), hdSoundVoiceCallback);

    struct hdW32SoundVoiceInfo
    {
        void*                   firstBuffer_;
        hUint32                 firstBufferSize_;
        void*                   secondBuffer_;
        hUint32                 secondBufferSize_;
        hdSoundVoiceCallback    callback_;
        hdSoundFormat           audioFormat_;
        hFloat                  pitch_;
    };

    class  hdW32SoundVoiceDevice
    {
    public:
        hdW32SoundVoiceDevice();
        ~hdW32SoundVoiceDevice();

        void    SetInfoAndInitialReads( const hdW32SoundVoiceInfo& info );
        void    SetNextRead( void* buffer, hUint32 sizeBytes );
        void    Start();
        void    TogglePause();
        hBool   GetPausedState() const { return paused_; }
        void    UpdateVoice();
        void    Stop();
        hBool   GetIsPlaying() const { return voice_ > 0;}
        void    SetPitch( hFloat pitch );
        void    SetVolume( hFloat vol );

    private:

        static const hUint32 BUFFER_COUNT = 2;

        void    AquireVoice();
        void    ReleaseVoice();

        ALuint              buffers_[BUFFER_COUNT];
        ALuint              voice_;
        hUint32             nextRead_;
        hBool               paused_;
        hBool               sourceComplete_;
        hFloat              volume_;
        hFloat              pitch_;
        hdW32SoundVoiceInfo info_;
    };
}

#endif // DEVICESOUNDVOICE_H__
