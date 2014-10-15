/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/
#ifndef DEVICESOUNDUTIL_H__
#define DEVICESOUNDUTIL_H__

#include "base/hTypes.h"
#include <al.h>         //openAL
#include <alc.h>

namespace Heart
{
#ifdef HEART_DEBUG
    #define HEART_CHECK_OPENAL_ERRORS() { ALenum er = alGetError(); hcAssertMsg( er == AL_NO_ERROR, "OpenAL Error: %s(%u)", GetOpenALErrorString(er), er ); }
#else
    #define HEART_CHECK_OPENAL_ERRORS()
#endif

    extern const hChar* GetOpenALErrorString( ALenum ercode );

    enum hdSoundCallbackReason
    {
        NEED_MORE_PCM_DATA,
        VOICE_STOPPED,
    };

    enum hdSoundFormat
    {
        HEART_SOUND_FMT_MONO8    = AL_FORMAT_MONO8,
        HEART_SOUND_FMT_MONO16   = AL_FORMAT_MONO16,
        HEART_SOUND_FMT_STEREO8  = AL_FORMAT_STEREO8,
        HEART_SOUND_FMT_STEREO16 = AL_FORMAT_STEREO16,
    };
}

#endif // DEVICESOUNDUTIL_H__