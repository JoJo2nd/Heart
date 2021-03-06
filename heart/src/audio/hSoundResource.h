/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/
#ifndef HSOUNDRESOURCE_H__
#define HSOUNDRESOURCE_H__

namespace Heart
{

    class hSoundResource : public hISoundSourceBuffer
    {
    public:
        hSoundResource()
        {

        }
        ~hSoundResource()
        {

        }

        hUint32                        GetPitch() const { return vobInfo_.rate; }
        hUint32                        GetChannels() const { return vobInfo_.channels; }
        hdSoundFormat                  GetFormat() const { return GetChannels() == 2 ? HEART_SOUND_FMT_STEREO16 : HEART_SOUND_FMT_MONO16; }
        void                           GetBufferInfo( hSoundSourceBufferInfo* outInfo );
        hUint32                        GetSourceCount() { return 0; }
        hUint32                        GetSourceIndex( const hChar* ) { return ~0U; }
        const hChar*                   GetSourceName( hUint32 ) { return NULL; }
        hSoundPlaybackHandle           CreatePlaybackHandle();
        void                           Rewind( hSoundPlaybackHandle handle );
        hOGGDecodeState                DecodeAudioBlock( hSoundPlaybackHandle handle, void** dstPtr, hUint32* outSize );
        void                           ReleasePlaybackHandle( hSoundPlaybackHandle /*handle*/ ) {}
#if 0
        static hResourceClassBase*     OnSoundLoad( const hChar* ext, hUint32 resID, void* dataStream, hResourceManager* resManager );
        static hUint32                 OnSoundUnload( const hChar* ext, hResourceClassBase* resource, hResourceManager* resManager );
#endif

    private:

        struct hPlaybackInfo
        {
            hPlaybackInfo() {}
            void                 Init( vorbis_info* vobinfo, hUint32 initTell );

            static const hUint32 NUM_BUFFERS = 2;
            static const hUint32 MAX_CHANNELS = 2;
            static const hUint32 PCM_BUFFER_SIZE = 12*1024;
            static const hUint32 OGG_BUFFER_SIZE = 2*1024;

            vorbis_dsp_state     vobDSP_;       /* central working state for the packet->PCM decoder */
            vorbis_block         vobBlock_;     /* local working space for packet->PCM decode */
            hOGGDecodeState      state_;
            hUint32              tell_;
            hUint32              readOpID_; 
            hUint32              maxDecodeSamples_;
            hUint32              currentDecodeSamples_;
            hUint32              currentOutBuffer_; //Buffer we decode ogg data into. other is being read from.
            hUint32              inSizes;
            hUint32              outSizes[NUM_BUFFERS];
            hChar                inBuffer_[OGG_BUFFER_SIZE];//Buffer we read ogg data into.
            ogg_int16_t          outBuffers_[NUM_BUFFERS][PCM_BUFFER_SIZE*MAX_CHANNELS];
            hBool                inUse_;
            hBool                eos_;
            hBool                eoSource_;
            hBool                decodingBlock;

        };

        hBool                   DecodeVorbisHeader( void* dataStream );
        hBool                   RequestNextAudioBlock( hSoundPlaybackHandle handle );
        hBool                   NextAudioBlockReady( hSoundPlaybackHandle handle );

        ogg_sync_state   oggState_;     /* sync and verify incoming physical bitstream */
        ogg_stream_state oggStream_;    /* take physical pages, weld into a logical stream of packets */
        ogg_page         oggPage_;      /* one Ogg bitstream page. Vorbis packets are inside */
        ogg_packet       oggPacket_;    /* one raw packet of data for decode */

        vorbis_info      vobInfo_;      /* struct that stores all the static vorbis bitstream settings */
        vorbis_comment   vobComment_;   /* struct that stores all the bitstream user comments */
        
        hUint32                     initTell_;
        hVector< hPlaybackInfo >    playbackInfos_;
    };
}

#endif // HSOUNDRESOURCE_H__