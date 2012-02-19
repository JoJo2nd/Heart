/********************************************************************

	filename: 	hSoundResource.h	
	
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
#ifndef HSOUNDRESOURCE_H__
#define HSOUNDRESOURCE_H__


#include "hTypes.h"
#include "hResource.h"
#include "vorbis/codec.h"

namespace Heart
{

    typedef hUint32 hSoundPlaybackHandle;

    enum hOGGDecodeState
    {
        OGGDecode_OK,
        OGGDecode_BUSY,
        OGGDecode_NEED_MORE_DATA,
        OGGDecode_NEED_MORE_DATA_WAIT,
        OGGDecode_END,
    };

    class hSoundResource : public hStreamingResourceBase
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
        hSoundPlaybackHandle           CreatePlaybackHandle();
        void                           Rewind( hSoundPlaybackHandle handle );
        hOGGDecodeState                DecodeAudioBlock( hSoundPlaybackHandle handle, void** dstPtr, hUint32* outSize );
        void                           ReleasePlaybackHandle( hSoundPlaybackHandle /*handle*/ ) {}
        static hResourceClassBase*     OnSoundLoad( const hChar* ext, hUint32 resID, hSerialiserFileStream* dataStream, hResourceManager* resManager );
        static hUint32                 OnSoundUnload( const hChar* ext, hResourceClassBase* resource, hResourceManager* resManager );

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

        hBool                   DecodeVorbisHeader( hSerialiserFileStream* dataStream );
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