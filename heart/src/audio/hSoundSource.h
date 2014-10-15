/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/
#ifndef HSOUNDSOURCE_H__
#define HSOUNDSOURCE_H__

#if 0
namespace Heart
{

    class  hSoundSource : public hLinkedListElement< hSoundSource >
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
#endif