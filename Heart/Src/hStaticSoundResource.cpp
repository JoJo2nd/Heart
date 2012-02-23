/********************************************************************

	filename: 	hStaticSoundResource.cpp	
	
	Copyright (c) 22:2:2012 James Moran
	
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

#include "Common.h"
#include "hStaticSoundResource.h"
#include "hMathUtil.h"

namespace Heart
{

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hUint32 hSoundBankResource::GetSourceIndex( const hChar* sourceName )
    {
        for ( hUint32 i = 0; i < sourceCount_; ++i )
        {
            if ( hStrCmp( sources_[i].name_, sourceName ) == 0 )
            {
                return i;
            }
        }

        return ~0U;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hResourceClassBase* hSoundBankResource::OnSoundBankLoad( const hChar* ext, hUint32 resID, hSerialiserFileStream* dataStream, hResourceManager* resManager )
    {
        hSoundBankResource* resource = hNEW( hGeneralHeap ) hSoundBankResource;
        hSerialiser ser;
        ser.Deserialise( dataStream, *resource );

        return resource;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hUint32 hSoundBankResource::OnSoundBankUnload( const hChar* ext, hResourceClassBase* resource, hResourceManager* resManager )
    {
        hDELETE resource;

        return 0;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    size_t hStaticSoundSource::OGGReadFunc(void *ptr, size_t size, size_t nmemb, void *datasource)
    {
        hPlaybackInfo* ctx = (hPlaybackInfo*)datasource;
        hUint32 s = hMin( size, ctx->tell_ - ctx->source_->oggDataSize_ );
        hMemCpy( ptr, ctx->source_->oggData_+ctx->tell_, s );
        ctx->tell_ += s;
        return s;
    }

    int    hStaticSoundSource::OGGSeekFunc(void *datasource, ogg_int64_t offset, int whence)
    {
        hPlaybackInfo* ctx = (hPlaybackInfo*)datasource;
        int ret = 0;

        switch (whence)
        {
        case SEEK_SET:
            ctx->tell_ = (hUint32)offset;
            break;
        case SEEK_CUR:
            ctx->tell_ += (hUint32)offset;
            break;
        case SEEK_END:
            ctx->tell_ = ctx->source_->oggDataSize_ + (hUint32)offset;
            break;
        }

        if ( ctx->tell_ > ctx->source_->oggDataSize_ )
        {
            ret = -1;
        }

        ctx->tell_ = hMin( ctx->tell_, ctx->source_->oggDataSize_ );

        return ret;
    }

    long   hStaticSoundSource::OGGTellFunc(void *datasource)
    {
        hPlaybackInfo* ctx = (hPlaybackInfo*)datasource;
        return ctx->tell_;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hSoundPlaybackHandle hStaticSoundSource::CreatePlaybackHandle()
    {
        hSoundPlaybackHandle ret = ~0U;

        for ( hUint32 i = 0; i < playbackInfos_.GetSize(); ++i )
        {
            if ( !playbackInfos_[i].inUse_ )
            {
                ret = i;
                break;
            }
        }

        if ( ret == ~0U )
        {
            ret = playbackInfos_.GetSize();
            playbackInfos_.Resize( ret+1 );
        }

        static ov_callbacks ovcb;
        ovcb.read_func = hStaticSoundSource::OGGReadFunc;
        ovcb.seek_func = hStaticSoundSource::OGGSeekFunc;
        ovcb.tell_func = hStaticSoundSource::OGGTellFunc;
        ovcb.close_func = NULL;

        ov_open_callbacks( &playbackInfos_[ret], &playbackInfos_[ret].oggctx_, NULL, 0, ovcb );

        return ret;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hStaticSoundSource::GetBufferInfo( hSoundSourceBufferInfo* outInfo )
    {
        outInfo->channels_ = channels_;
        outInfo->format_   = format_;
        outInfo->pitch_    = pitch_;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hStaticSoundSource::Rewind( hSoundPlaybackHandle handle )
    {
        hPlaybackInfo* src = &playbackInfos_[handle];
        ov_raw_seek( &src->oggctx_, 0 );
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hOGGDecodeState hStaticSoundSource::DecodeAudioBlock( hSoundPlaybackHandle handle, void** dstPtr, hUint32* outSize )
    {
        hPlaybackInfo* src = &playbackInfos_[handle];
        hUint32 read = 0;
        hUint32 left = hPlaybackInfo::PCM_BUFFER_SIZE;

        while ( !src->eof_ && read < hPlaybackInfo::PCM_BUFFER_SIZE )
        {
            read = ov_read( &src->oggctx_, (char*)src->outBuffers_[src->buf_], (hPlaybackInfo::PCM_BUFFER_SIZE*sizeof(ogg_int16_t))-read, 0, 2, 1, &src->currentSelection_ );
            if ( read == 0 )
                src->eof_ = hTrue;
        }

        *dstPtr = src->outBuffers_[src->buf_];
        *outSize = read;

        return src->eof_ ? OGGDecode_END : OGGDecode_OK; 
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hStaticSoundSource::ReleasePlaybackHandle( hSoundPlaybackHandle handle )
    {

    }

}