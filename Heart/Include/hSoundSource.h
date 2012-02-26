/********************************************************************

	filename: 	hSoundSource.h	
	
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
#ifndef HSOUNDSOURCE_H__
#define HSOUNDSOURCE_H__

#include "hTypes.h"
#include "hSoundResource.h"
#include "hSoundSourceBuffer.h"

namespace Heart
{

    class hSoundSource : public hLinkedListElement< hSoundSource >
    {
    public:
        hSoundSource()
            : soundBuffer_(NULL)
            , playbackHandle_(~0U)
            , looping_(hFalse)
        {

        }
        ~hSoundSource()
        {

        }

        void SetSoundResource( hISoundSourceBuffer* resource ) { soundBuffer_ = resource; }
        void Start();
        void Stop();
        void Update();
        void SetPause( hBool pause );
        void GetPause() const;
        void SetLooping( hBool looping ) { looping_ = looping; }
        void GetLooping() const;
        void SetVolume( hFloat val ) { deviceVoice_.SetVolume(val); }

    private:

        void                 DeviceCallback( hdSoundVoice* voice, hdSoundCallbackReason reason );

        hdSoundVoice         deviceVoice_;
        hISoundSourceBuffer* soundBuffer_;
        hSoundPlaybackHandle playbackHandle_;
        void*                nextPCMData_;
        hUint32              nextPCMSize_;
        hBool                pcmDataWaiting_;
        hBool                looping_;
    };
}

#endif // HSOUNDSOURCE_H__