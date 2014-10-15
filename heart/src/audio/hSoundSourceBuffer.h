/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/
#pragma once

#include "pal/hDeviceSoundUtil.h"

namespace Heart {
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