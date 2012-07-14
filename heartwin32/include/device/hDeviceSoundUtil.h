/********************************************************************

	filename: 	DeviceSoundUtil.h	
	
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
#ifndef DEVICESOUNDUTIL_H__
#define DEVICESOUNDUTIL_H__

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