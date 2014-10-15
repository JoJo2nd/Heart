/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/
#pragma once

#define HDEBUGINFO_H__ //todo !!JM 
#ifndef HDEBUGINFO_H__

#include "base/hTypes.h"
#include "debug/hDebugMenuManager.h"

namespace Heart
{
    class hHeartEngine;
    class hRenderSubmissionCtx;
    class hRenderBuffer;

    class hDebugInfo : public hDebugMenuBase
    {
    public:
        hDebugInfo(hHeartEngine* enigne);
        ~hDebugInfo();

        void PreRenderUpdate() {};
        void Render(hRenderSubmissionCtx* ctx, hRenderBuffer* instanceCB, const hDebugRenderParams& params);
        void EndFrameUpdate() {};

    private:
        hHeartEngine* engine_;
    };
}

#endif // HDEBUGINFO_H__