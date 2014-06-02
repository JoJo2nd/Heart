/********************************************************************

	filename: 	hStaticSoundResource.h	
	
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
#ifndef HSTATICSOUNDRESOURCE_H__
#define HSTATICSOUNDRESOURCE_H__

class OGGSoundBankBuilder;

namespace Heart
{
#if 0
    struct hStaticSoundSource : public hISoundSourceBuffer
    {
    public:
        
        hSoundPlaybackHandle CreatePlaybackHandle();
        void                 GetBufferInfo( hSoundSourceBufferInfo* outInfo );
        void                 Rewind( hSoundPlaybackHandle handle );
        hOGGDecodeState      DecodeAudioBlock( hSoundPlaybackHandle handle, void** dstPtr, hUint32* outSize );
        void                 ReleasePlaybackHandle( hSoundPlaybackHandle handle );
        static hSize_t       OGGReadFunc(void *ptr, size_t size, size_t nmemb, void *datasource);
        static int           OGGSeekFunc(void *datasource, ogg_int64_t offset, int whence);
        static long          OGGTellFunc(void *datasource);

        friend class hSoundBankResource;

        struct hPlaybackInfo
        {
            static const hUint32 NUM_BUFFERS = 2;
            static const hUint32 MAX_CHANNELS = 2;
            static const hUint32 PCM_BUFFER_SIZE = 12*1024;//May reduce size...12k(48K overall) is a bit much?
        
            hPlaybackInfo() 
                : source_(NULL)
                , tell_(0)
                , inUse_(0)
                , buf_(0)
            {}

            OggVorbis_File       oggctx_;
            hStaticSoundSource*  source_;
            int                  currentSelection_;
            hSize_t              tell_;
            hBool                inUse_;
            hSize_t              buf_;
            hBool                eof_;
            ogg_int16_t          outBuffers_[NUM_BUFFERS][PCM_BUFFER_SIZE*MAX_CHANNELS];
        };

        hVector< hPlaybackInfo > playbackInfos_; 
        hUint32       nameLen_;
        const hChar*  name_;
        hSize_t       oggDataSize_;
        hByte*        oggData_;  
        hdSoundFormat format_;
        hUint32       channels_;
        hFloat        pitch_;       
    };

    class hSoundBankResource
    {
    public:
        hSoundBankResource()
            : sourceCount_(0)
            , sources_(NULL)
            , sourceDataSize_(0)
            , sourcesData_(NULL)
            , namePoolSize_(0)
            , namePool_(NULL)
        {
        }
        ~hSoundBankResource()
        {
            delete[] sources_;
            sources_ = nullptr;
            delete[] sourcesData_;
            sourcesData_ = nullptr;
            delete[] namePool_;
            namePool_ = nullptr;
        }

        hUint32                      GetSourceCount() { return sourceCount_; }
        hUint32                      GetSourceIndex( const hChar* sourceName );
        const hChar*                 GetSourceName( hUint32 idx ) { return sources_[idx].name_; }
        hStaticSoundSource*          GetSoundSource( hUint32 idx ) { return sources_+idx; }
#if 0
        static hResourceClassBase*   OnSoundBankLoad( const hChar* ext, hUint32 resID, void* dataStream, hResourceManager* resManager );
        static hUint32               OnSoundBankUnload( const hChar* ext, hResourceClassBase* resource, hResourceManager* resManager );
#endif

    private:

        friend class ::OGGSoundBankBuilder;

        hUint32              sourceCount_;
        hStaticSoundSource*  sources_;
        hUint32              sourceDataSize_;
        hByte*               sourcesData_;
        hUint32              namePoolSize_;
        hChar*               namePool_;
    };
#endif

}

#endif // HSTATICSOUNDRESOURCE_H__