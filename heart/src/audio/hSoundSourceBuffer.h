/********************************************************************

	filename: 	hSoundSourceBuffer.h	
	
	Copyright (c) 21:2:2012 James Moran
	
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
#ifndef HSOUNDSOURCEBUFFER_H__
#define HSOUNDSOURCEBUFFER_H__

namespace Heart
{
    enum hOGGDecodeState
    {
        OGGDecode_OK,
        OGGDecode_BUSY,
        OGGDecode_NEED_MORE_DATA,
        OGGDecode_NEED_MORE_DATA_WAIT,
        OGGDecode_END,
    };

    typedef hUint32 hSoundPlaybackHandle;

    struct hSoundSourceBufferInfo
    {
        hUint32         channels_;
        hdSoundFormat   format_;
        hFloat          pitch_;
    };

    class hISoundSourceBuffer
    {
    public:
        virtual hSoundPlaybackHandle CreatePlaybackHandle() = 0;
        virtual void                 GetBufferInfo( hSoundSourceBufferInfo* outInfo ) = 0;
        virtual void                 Rewind( hSoundPlaybackHandle handle ) = 0;
        virtual hOGGDecodeState      DecodeAudioBlock( hSoundPlaybackHandle handle, void** dstPtr, hUint32* outSize ) = 0;
        virtual void                 ReleasePlaybackHandle( hSoundPlaybackHandle handle ) = 0;
    };
}

#endif // HSOUNDSOURCEBUFFER_H__