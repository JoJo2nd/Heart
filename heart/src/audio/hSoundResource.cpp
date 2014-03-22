/********************************************************************

	filename: 	hSoundResource.cpp	
	
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

#if 0
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hResourceClassBase* hSoundResource::OnSoundLoad( const hChar* ext, hUint32 resID, void* dataStream, hResourceManager* resManager )
    {
#if 1
        hcPrintf("Stub :"__FUNCTION__);
        return hNullptr;
#else
        hSoundResource* sound = hNEW(hSoundResource);
        if ( !sound->DecodeVorbisHeader( dataStream ) )
        {
            hDELETE(sound);
            sound = NULL;
        }
        return sound;
#endif
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hUint32 hSoundResource::OnSoundUnload( const hChar* ext, hResourceClassBase* resource, hResourceManager* resManager )
    {
#if 1
        hcPrintf("Stub :"__FUNCTION__);
#else
        hSoundResource* sound = static_cast<hSoundResource*>(resource);

        hDELETE(sound);
#endif
        return 0;
    }
#endif

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hBool hSoundResource::DecodeVorbisHeader( void* dataStream )
    {
#if 1
        hcPrintf("Stub :"__FUNCTION__);
        return hTrue;
#else
        static const hUint32 bufSize = 4096;
        hChar* buffer;
        hUint32 bytes;
        //Open the ogg file
        ogg_sync_init( &oggState_ );

        /* grab some data at the head of the stream. We want the first page
        (which is guaranteed to be small and only contain the Vorbis
        stream initial header) We need the first page to get the stream
        serialno. */

        /* submit a 4k block to libvorbis' Ogg layer */
        buffer = ogg_sync_buffer( &oggState_, bufSize );
        bytes = dataStream->Read( buffer, bufSize );
        ogg_sync_wrote( &oggState_, bytes );

        /* Get the first page. */
        if ( ogg_sync_pageout( &oggState_, &oggPage_ )!=1 )
        {
            /* have we simply run out of data?  If so, we're done. */
            /* error case.  Must not be Vorbis data */
            hcPrintf( "Input does not appear to be an Ogg bitstream." );
            return hFalse;
        }

        /* Get the serial number and set up the rest of decode. */
        /* serialno first; use it to set up a logical stream */
        ogg_stream_init( &oggStream_, ogg_page_serialno( &oggPage_ ) );

        /* extract the initial header from the first page and verify that the
        Ogg bitstream is in fact Vorbis data */

        /* I handle the initial header first instead of just having the code
        read all three Vorbis headers at once because reading the initial
        header is an easy way to identify a Vorbis bitstream and it's
        useful to see that functionality seperated out. */

        vorbis_info_init( &vobInfo_ );
        vorbis_comment_init( &vobComment_ );
        if ( ogg_stream_pagein( &oggStream_, &oggPage_ ) < 0 )
        { 
            /* error; stream version mismatch perhaps */
            hcPrintf( "Error reading first page of Ogg bitstream data." );
            return hFalse;
        }

        if ( ogg_stream_packetout( &oggStream_, &oggPacket_ ) !=1 )
        { 
            /* no page? must not be vorbis */
            hcPrintf( "Error reading initial header packet." );
            return hFalse;
        }

        if ( vorbis_synthesis_headerin( &vobInfo_, &vobComment_, &oggPacket_ ) < 0 )
        { 
            /* error case; not a vorbis header */
            hcPrintf( "This Ogg bitstream does not contain Vorbis audio data.");
            return hFalse;
        }

        /* At this point, we're sure we're Vorbis. We've set up the logical
        (Ogg) bitstream decoder. Get the comment and codebook headers and
        set up the Vorbis decoder */

        /* The next two packets in order are the comment and codebook headers.
        They're likely large and may span multiple pages. Thus we read
        and submit data until we get our two packets, watching that no
        pages are missing. If a page is missing, error out; losing a
        header page is the only place where missing data is fatal. */

        hUint32 i = 0;
        while(i<2)
        {
            while(i<2)
            {
                int result = ogg_sync_pageout( &oggState_, &oggPage_ );
                if (result == 0 )
                {
                    break; /* Need more data */
                }
                /* Don't complain about missing or corrupt data yet. We'll
                catch it at the packet output phase */
                if(result==1)
                {
                    /* we can ignore any errors here as they'll also become 
                     * apparent at packetout */
                    ogg_stream_pagein( &oggStream_, &oggPage_ ); 
                    while(i<2)
                    {
                        result = ogg_stream_packetout( &oggStream_, &oggPacket_ );
                        if(result==0)break;
                        
                        if(result<0)
                        {
                            /* Uh oh; data at some point was corrupted or missing!
                            We can't tolerate that in a header.  Die. */
                            hcPrintf( "Corrupt secondary header." );
                            return hFalse;
                        }
                        result=vorbis_synthesis_headerin( &vobInfo_, &vobComment_, &oggPacket_ );
                        if(result<0)
                        {
                            hcPrintf( "Corrupt secondary header." );
                            return hFalse;
                        }
                        i++;
                    }
                }
            }

            /* no harm in not checking before adding more */
            buffer = ogg_sync_buffer( &oggState_, bufSize );
            bytes = dataStream->Read( buffer, bufSize );
            if ( bytes == 0 && i < 2)
            {
                hcPrintf( "End of file before finding all Vorbis headers!" );
                return hFalse;
            }
            ogg_sync_wrote( &oggState_, bytes );
        }

        /* Throw the comments plus a few lines about the bitstream we're decoding */
        hcPrintf( "Bitstream is %d channel, %ldHz", vobInfo_.channels, vobInfo_.rate );
        hcPrintf( "Encoded by: %s", vobComment_.vendor);

        initTell_ = (hUint32)dataStream->Tell() - 4096;

        return hTrue;
#endif
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hSoundPlaybackHandle hSoundResource::CreatePlaybackHandle()
    {
        hSoundPlaybackHandle ret = ~0U;

        for ( hUint32 i = 0; i < playbackInfos_.size(); ++i )
        {
            if ( !playbackInfos_[i].inUse_ )
            {
                ret = i;
                break;
            }
        }

        if ( ret == ~0U )
        {
            ret = playbackInfos_.size();
            playbackInfos_.resize( ret+1 );
        }

        playbackInfos_[ret].Init( &vobInfo_, initTell_ );

        return ret;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hBool hSoundResource::RequestNextAudioBlock( hSoundPlaybackHandle handle )
    {
#if 1 
        hcPrintf("Stub :"__FUNCTION__);
        return hFalse;
#else
        hPlaybackInfo* info = &playbackInfos_[handle];
        hUint ret = QueueStreamRead( info->inBuffer_, hPlaybackInfo::OGG_BUFFER_SIZE, info->tell_, &info->readOpID_ );
        return ret == 0;
#endif
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hBool hSoundResource::NextAudioBlockReady( hSoundPlaybackHandle handle )
    {
#if 1
        hcPrintf("Stub :"__FUNCTION__);
        return hFalse;
#else
        hUint32 read;
        hPlaybackInfo* info = &playbackInfos_[handle];

        if ( info->eoSource_ )
            return hTrue;

        if ( PollSteamRead( info->readOpID_, &read ) == 0 )
        {
            info->inSizes = read;
            info->readOpID_ = ~0U;
            info->tell_ += read;

            hChar* buffer = ogg_sync_buffer( &oggState_, hPlaybackInfo::OGG_BUFFER_SIZE );
            hMemCpy( buffer, info->inBuffer_, read );
            int er = ogg_sync_wrote( &oggState_, read );
            hcAssert( er == 0 );

            if ( read == 0 ) 
            {
                info->eoSource_ = hTrue;
            }

            return hTrue;
        }

        return hFalse;
#endif
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hOGGDecodeState hSoundResource::DecodeAudioBlock( hSoundPlaybackHandle handle, void** dstPtr, hUint32* outSize )
    {
        hPlaybackInfo* info = &playbackInfos_[handle];

        if ( info->state_ == OGGDecode_END )
            return info->state_;

        if ( info->state_ == OGGDecode_NEED_MORE_DATA )
        {
            if ( !info->eoSource_ )
            {
                RequestNextAudioBlock( handle );
                info->state_ = OGGDecode_NEED_MORE_DATA_WAIT;
            }
            else
            {
                info->state_ = OGGDecode_END;
            }
        }
        else if ( info->state_ == OGGDecode_NEED_MORE_DATA_WAIT )
        {
            if ( NextAudioBlockReady( handle ) )
            {     
                info->state_ = OGGDecode_BUSY;
            }
        }
        else if ( info->state_ == OGGDecode_BUSY )
        {
            //Decode this block.
            int result = ogg_sync_pageout( &oggState_, &oggPage_ );
            if( result == 0 )
            {
                info->decodingBlock = hFalse;
                info->state_ = OGGDecode_NEED_MORE_DATA;
                return info->state_; /* need more data */
            }
            if( result < 0 )
            { 
                /* missing or corrupt data at this page position, ogg deals with this very well so
                 * just plod on...*/
            }
            else
            {
                ogg_stream_pagein( &oggStream_, &oggPage_ ); /* can safely ignore errors at this point */
                while(1)
                {
                    result = ogg_stream_packetout( &oggStream_, &oggPacket_ );

                    if( result == 0 )
                    {
                        //info->state_ = OGGDecode_NEED_MORE_DATA;
                        //return info->state_; /* need more data */
                        break;
                    }
                    if(result<0)
                    { /* missing or corrupt data at this page position */
                        /* no reason to complain; */
                    }
                    else
                    {
                        /* we have a packet.  Decode it */
                        float** pcm;
                        hUint32 samples;

                        if( vorbis_synthesis( &info->vobBlock_, &oggPacket_ ) == 0 ) /* test for success! */
                            vorbis_synthesis_blockin( &info->vobDSP_, &info->vobBlock_ );
                        /* 
                         * pcm is a multichannel float vector.  In stereo, for
                        example, pcm[0] is left, and pcm[1] is right.  samples is
                        the size of each channel.  Convert the float values
                        (-1.<=range<=1.) to whatever PCM format and write it out 
                        */
                        while( ( samples = vorbis_synthesis_pcmout( &info->vobDSP_, &pcm ) ) > 0 )
                        {
                            ogg_int16_t* convbuffer = info->outBuffers_[info->currentOutBuffer_]+(info->currentDecodeSamples_*vobInfo_.channels);
                            int clipflag = 0;
                            hUint32 bout = samples;
                            if ( info->currentDecodeSamples_ + bout >= info->maxDecodeSamples_ )
                            {
                                hUint32 over = (info->currentDecodeSamples_ + bout) - info->maxDecodeSamples_;
                                bout -= over;
                            }

                            info->currentDecodeSamples_ += bout;

                            /* convert floats to 16 bit signed ints (host order) and interleave */
                            for( hUint32 i = 0 ; i < (hUint32)vobInfo_.channels; ++i )
                            {
                                ogg_int16_t* ptr = convbuffer+i;
                                float* mono = pcm[i];
                                for( hUint32 j=0; j < bout; ++j )
                                {
                                    int val = (int)hFloor( mono[j]*32767.f+.5f );

                                    /* might as well guard against clipping */
                                    if(val>32767)
                                    {
                                        val=32767;
                                        clipflag=1;
                                    }
                                    if(val<-32768)
                                    {
                                        val=-32768;
                                        clipflag=1;
                                    }
                                    *ptr = val;
                                    ptr += vobInfo_.channels;
                                }
                            }

                            vorbis_synthesis_read( &info->vobDSP_, bout ); /* tell libvorbis how many samples we actually consumed */
                            /* If we've consumed enough to fill a buffer, return it and swap */
                            if ( info->currentDecodeSamples_ == hPlaybackInfo::PCM_BUFFER_SIZE )
                            {
                                *outSize = info->currentDecodeSamples_*sizeof(hInt16)*vobInfo_.channels;
                                *dstPtr = info->outBuffers_[info->currentOutBuffer_];
                                ++info->currentOutBuffer_;
                                info->currentOutBuffer_ %= hPlaybackInfo::NUM_BUFFERS;
                                info->decodingBlock = hFalse;
                                info->currentDecodeSamples_ = 0;
                                return OGGDecode_OK;
                            }
                        }            
                    }
                }
                if ( ogg_page_eos( &oggPage_ ) ) 
                {
                    *outSize = info->currentDecodeSamples_*sizeof(ogg_uint16_t);
                    *dstPtr = info->outBuffers_[info->currentOutBuffer_];
                    info->eos_ = hTrue;
                    info->state_ = OGGDecode_END;
                    return OGGDecode_OK;
                }
            }
        }

        return info->state_;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hSoundResource::Rewind( hSoundPlaybackHandle handle )
    {
        hPlaybackInfo* info = &playbackInfos_[handle];

        info->tell_ = initTell_;
        info->readOpID_ = ~0U;
        info->inUse_ = hTrue;
        info->eos_ = hFalse;
        info->eoSource_ = hFalse;
        info->state_ = OGGDecode_NEED_MORE_DATA;
        info->decodingBlock = hFalse;

        hcPrintf( "Stream Rewind" );
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hSoundResource::hPlaybackInfo::Init( vorbis_info* vobinfo, hUint32 initTell )
    {
        hUint32 er;
        er = vorbis_synthesis_init( &vobDSP_, vobinfo );
        hcAssert( er == 0 );
        vorbis_block_init( &vobDSP_, &vobBlock_ );  
        tell_ = initTell;
        readOpID_ = ~0U;
        maxDecodeSamples_ = PCM_BUFFER_SIZE;
        currentOutBuffer_ = 0;
        currentDecodeSamples_ = 0;
        inSizes = 0;
        hZeroMem( outSizes, sizeof(outSizes) );
        hZeroMem( inBuffer_, sizeof(inBuffer_) );
        hZeroMem( outBuffers_, sizeof(outBuffers_) );
        inUse_ = hTrue;
        eos_ = hFalse;
        eoSource_ = hFalse;
        state_ = OGGDecode_NEED_MORE_DATA;
        decodingBlock = hFalse;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hSoundResource::GetBufferInfo( hSoundSourceBufferInfo* outInfo )
    {
        outInfo->channels_  = GetChannels();
        outInfo->format_    = GetFormat();
        outInfo->pitch_     = (hFloat)GetPitch();//Plays too fast -- not sure why
    }

}