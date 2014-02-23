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

    hResourceClassBase* hSoundBankResource::OnSoundBankLoad( const hChar* ext, hUint32 resID, void* dataStream, hResourceManager* resManager )
    {
        hSoundBankResource* resource = hNEW(hSoundBankResource);
        hSerialiser ser;

        return resource;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hUint32 hSoundBankResource::OnSoundBankUnload( const hChar* ext, hResourceClassBase* resource, hResourceManager* resManager )
    {
        hDELETE(resource);

        return 0;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hSize_t hStaticSoundSource::OGGReadFunc(void *ptr, size_t size, size_t nmemb, void *datasource)
    {
        hPlaybackInfo* ctx = (hPlaybackInfo*)datasource;
        hSize_t s = hMin( size, ctx->tell_ - ctx->source_->oggDataSize_ );
        hMemCpy( ptr, ctx->source_->oggData_+ctx->tell_, s );
        ctx->tell_ += s;
        return s;
    }

    int hStaticSoundSource::OGGSeekFunc(void *datasource, ogg_int64_t offset, int whence)
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

        return (int)ret;
    }

    long hStaticSoundSource::OGGTellFunc(void *datasource)
    {
        hPlaybackInfo* ctx = (hPlaybackInfo*)datasource;
        return (long)ctx->tell_;
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
        playbackInfos_[ret].source_ = this;
        playbackInfos_[ret].eof_ = hFalse;
        playbackInfos_[ret].tell_ = 0;

        ov_open_callbacks( &playbackInfos_[ret], &playbackInfos_[ret].oggctx_, NULL, 0, ovcb );

        channels_ = playbackInfos_[ret].oggctx_.vi->channels;
        pitch_ = (hFloat)playbackInfos_[ret].oggctx_.vi->rate;
        format_ = channels_ >= 2 ? HEART_SOUND_FMT_STEREO16 : HEART_SOUND_FMT_MONO16;

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
        src->eof_ = hFalse;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hOGGDecodeState hStaticSoundSource::DecodeAudioBlock( hSoundPlaybackHandle handle, void** dstPtr, hUint32* outSize )
    {
        hPlaybackInfo* src = &playbackInfos_[handle];
        hUint32 read = 0;
        hUint32 left = hPlaybackInfo::PCM_BUFFER_SIZE;

        while ( !src->eof_ && read < hPlaybackInfo::PCM_BUFFER_SIZE*sizeof(ogg_uint16_t) )
        {
            hUint32 got = ov_read( &src->oggctx_, ((char*)src->outBuffers_[src->buf_])+read, (hPlaybackInfo::PCM_BUFFER_SIZE*sizeof(ogg_int16_t))-read, 0, 2, 1, &src->currentSelection_ );
            if ( got == 0 )
                src->eof_ = hTrue;
            read += got;
        }

        *dstPtr = src->outBuffers_[src->buf_];
        *outSize = read;
        ++src->buf_;
        src->buf_ %= hPlaybackInfo::NUM_BUFFERS;

        return src->eof_ ? OGGDecode_END : OGGDecode_OK; 
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hStaticSoundSource::ReleasePlaybackHandle( hSoundPlaybackHandle handle )
    {

    }

}