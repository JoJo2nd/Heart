/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/
#if 0
namespace Heart
{

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hSoundSource::Start()
    {
       hdSoundVoiceInfo info;
       hSoundSourceBufferInfo bufInfo;
       playbackHandle_ = soundBuffer_->CreatePlaybackHandle();
       soundBuffer_->GetBufferInfo( &bufInfo );
       nextPCMSize_ = 0;

       hZeroMem( &info, sizeof(info) );
       info.audioFormat_ = bufInfo.format_;
       info.pitch_ = (hFloat)bufInfo.pitch_;
       info.callback_ = hdSoundVoiceCallback::bind< hSoundSource, &hSoundSource::DeviceCallback >( this );
       
       deviceVoice_.SetInfoAndInitialReads( info );
       pcmDataWaiting_ = hFalse;
       deviceVoice_.Start();
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hSoundSource::Stop()
    {

    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hSoundSource::Update()
    {
        if ( playbackHandle_ != ~0U && soundBuffer_ && !nextPCMSize_ )
        {
            nextPCMSize_ = 0;
            hUint32 res = soundBuffer_->DecodeAudioBlock( playbackHandle_, &nextPCMData_, &nextPCMSize_ );
            if ( res == OGGDecode_END && looping_ )
            {
                soundBuffer_->Rewind( playbackHandle_ );
            }
            if ( nextPCMSize_ )
            {
                pcmDataWaiting_ = hTrue;
            }
        }
        deviceVoice_.UpdateVoice();
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hSoundSource::DeviceCallback( hdSoundVoice* voice, hdSoundCallbackReason reason )
    {
        if ( reason == NEED_MORE_PCM_DATA && pcmDataWaiting_ )
        {
/*
            the nth sample is given by A*sin(2*pi*n*f/R), where A is the amplitude (volume), f is the frequency in Hertz, and R is the sample rate in samples per second
*/
#if 0
            static hUint32 n = 0;
            hInt16 tone[4096];

            for ( hUint32 i = 0; i < 4096; ++i, ++n )
            {
                tone[i] = (hInt16)hFloor( sin( 2.f*3.14f*n*261.626f/44100.f)*32767.f+.5f );
            }

            voice->SetNextRead( tone, sizeof(tone) );
            pcmDataWaiting_ = hTrue;
#else
            voice->SetNextRead( nextPCMData_, nextPCMSize_ );
            nextPCMData_ = NULL;
            nextPCMSize_ = 0;
            pcmDataWaiting_ = hFalse;
#endif
        }
        else if ( reason == VOICE_STOPPED )
        {

        }
    }

}
#endif