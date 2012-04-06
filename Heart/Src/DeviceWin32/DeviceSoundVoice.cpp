/********************************************************************

	filename: 	DeviceSoundVoice.cpp	
	
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

    hdW32SoundVoiceDevice::hdW32SoundVoiceDevice() : voice_(0)
        , nextRead_(0)
        , paused_(hFalse)
        , sourceComplete_(hFalse)
        , volume_(1.f)
        , pitch_(1.f)
    {
        alGenBuffers( BUFFER_COUNT, buffers_ );
        HEART_CHECK_OPENAL_ERRORS();
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hdW32SoundVoiceDevice::~hdW32SoundVoiceDevice()
    {
        ReleaseVoice();
        if ( buffers_ )
        {
            alDeleteBuffers( BUFFER_COUNT, buffers_ );
            HEART_CHECK_OPENAL_ERRORS();
            hZeroMem( buffers_, sizeof(BUFFER_COUNT) );
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdW32SoundVoiceDevice::SetInfoAndInitialReads( const hdW32SoundVoiceInfo& info )
    {
        sourceComplete_ = hFalse;
        nextRead_ = 0;
        info_ = info;
        if ( info_.firstBufferSize_ )
        {
            alBufferData( buffers_[0], info_.audioFormat_, info_.firstBuffer_, info_.firstBufferSize_, (ALsizei)info_.pitch_ );
            alSourceQueueBuffers( buffers_[0], 1, &buffers_[0] );
            HEART_CHECK_OPENAL_ERRORS();
            nextRead_ = (nextRead_+1) % BUFFER_COUNT;
        }
        if ( info_.secondBufferSize_ )
        {
            alBufferData( buffers_[1], info_.audioFormat_, info_.secondBuffer_, info_.secondBufferSize_, (ALsizei)info_.pitch_ );
            alSourceQueueBuffers( buffers_[1], 1, &buffers_[1] );
            HEART_CHECK_OPENAL_ERRORS();
            nextRead_ = (nextRead_+1) % BUFFER_COUNT;
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdW32SoundVoiceDevice::SetNextRead( void* buffer, hUint32 sizeBytes )
    {
        if ( buffer )
        {
            alBufferData( buffers_[nextRead_], info_.audioFormat_, buffer, sizeBytes, (ALsizei)info_.pitch_ );
            HEART_CHECK_OPENAL_ERRORS();
            alSourceQueueBuffers( voice_, 1, buffers_+nextRead_ );
            HEART_CHECK_OPENAL_ERRORS();
            nextRead_ = (nextRead_+1) % BUFFER_COUNT;
        }
        else
        {
            // no more buffers to play
            sourceComplete_ = hTrue;
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdW32SoundVoiceDevice::Start()
    {
        AquireVoice();
        alSourcePlay( voice_ );
        HEART_CHECK_OPENAL_ERRORS();
        hcAssert( nextRead_ == 0 );
        SetVolume( volume_ );
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdW32SoundVoiceDevice::TogglePause()
    {
        paused_ = !paused_;
        if ( voice_ )
        {
            if ( paused_ )
            {
                alSourcePause( voice_ );
                HEART_CHECK_OPENAL_ERRORS();
            }
            else
            {
                alSourcePlay( voice_ );
                HEART_CHECK_OPENAL_ERRORS();
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdW32SoundVoiceDevice::Stop()
    {
        ReleaseVoice();
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdW32SoundVoiceDevice::SetPitch( hFloat pitch )
    {
        pitch_ = pitch;
        if ( voice_ )
        {
            alSourcef( voice_, AL_PITCH, pitch_ );
            HEART_CHECK_OPENAL_ERRORS();
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdW32SoundVoiceDevice::SetVolume( hFloat vol )
    {
        volume_ = vol;
        if ( voice_ )
        {
            alSourcef( voice_, AL_GAIN, volume_ );
            HEART_CHECK_OPENAL_ERRORS();
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdW32SoundVoiceDevice::UpdateVoice()
    {
        if ( voice_ )
        {
            ALint processed;
            ALint queued;
            alGetSourceiv( voice_, AL_BUFFERS_PROCESSED, &processed );
            HEART_CHECK_OPENAL_ERRORS();
            alGetSourceiv( voice_, AL_BUFFERS_QUEUED, &queued );
            HEART_CHECK_OPENAL_ERRORS();
            if ( processed > 0 )
            {
                ALuint proBufs[BUFFER_COUNT];
                alSourceUnqueueBuffers( voice_, processed, proBufs );
                HEART_CHECK_OPENAL_ERRORS();
            }

            ALint playing;
            alGetSourcei( voice_, AL_SOURCE_STATE, &playing );
            HEART_CHECK_OPENAL_ERRORS();

            if ( queued && playing != AL_PLAYING && playing != AL_PAUSED )
            {
                alSourcePlay( voice_ );
            }

            if ( queued == 0 && sourceComplete_ == hTrue )
            {
                info_.callback_( this, VOICE_STOPPED );
            }

            if ( queued < BUFFER_COUNT && sourceComplete_ != hTrue )
            {
                info_.callback_( this, NEED_MORE_PCM_DATA );
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdW32SoundVoiceDevice::AquireVoice()
    {
        if ( !voice_ )
        {
            alGenSources( 1, &voice_ );
            HEART_CHECK_OPENAL_ERRORS();

            SetVolume(volume_);
            SetPitch(pitch_);
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdW32SoundVoiceDevice::ReleaseVoice()
    {
        if ( voice_ )
        {
            alSourceUnqueueBuffers( voice_, BUFFER_COUNT, buffers_ );
            HEART_CHECK_OPENAL_ERRORS();
            alDeleteSources( 1, &voice_ );
            HEART_CHECK_OPENAL_ERRORS();
            voice_ = 0;
        }
    }

}