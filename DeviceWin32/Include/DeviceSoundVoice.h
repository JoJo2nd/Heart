/********************************************************************

	filename: 	DeviceSoundVoice.h	
	
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
#ifndef DEVICESOUNDVOICE_H__
#define DEVICESOUNDVOICE_H__

namespace Heart
{
    class hdW32SoundVoiceDevice
    {
    public:
        hdW32SoundVoiceDevice()
            : voice_(0)
        {
            alGenBuffers( BUFFER_COUNT, buffers_ );
        }
        ~hdW32SoundVoiceDevice()
        {
            ReleaseVoice();
            if ( buffers_ )
            {
                alDeleteBuffers( BUFFER_COUNT, buffers_ );
                hZeroMem( buffers_, sizeof(BUFFER_COUNT) );
            }
        }

    private:

        static const hUint32 BUFFER_COUNT = 2;

        void    AquireVoice()
        {
            if ( !voice_ )
            {
                alGenSources( 1, &voice_ );
            }
        }
        void    ReleaseVoice()
        {
            if ( voice_ )
            {
                alSourceUnqueueBuffers( voice_, BUFFER_COUNT, buffers_ );
                alDeleteSources( 1, &voice_ );
                voice_ = 0;
            }
        }

        ALuint  buffers_[BUFFER_COUNT];
        ALuint  voice_;
    };
}

#endif // DEVICESOUNDVOICE_H__