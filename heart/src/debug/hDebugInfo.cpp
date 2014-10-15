/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#if 0 // todo !!JM
#include "debug/hDebugInfo.h"
#include "base/hStringUtil.h"
#include "core/hHeart.h"
#include "render/hDebugDraw.h"
#include "render/hRenderSubmissionContext.h"
#include "render/hRenderer.h"

namespace Heart
{

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hDebugInfo::hDebugInfo(hHeartEngine* engine) 
        : engine_(engine)
    {

    }
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hDebugInfo::~hDebugInfo() {

    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hDebugInfo::Render(hRenderSubmissionCtx* ctx, hRenderBuffer* instanceCB, const hDebugRenderParams& params) {
        static const hUint strbufsize=1024;
        static const hFloat debugFntSize=12;
        hDebugDraw* dd=hDebugDraw::it();
        hRenderer* renderer=engine_->GetRenderer();
        hRenderFrameStats stats=*renderer->getRenderStats();
        hChar strbuf[strbufsize];
        hVec3 screenpos(params.rtWidth_/-2.f, (params.rtHeight_/-2.f)+debugFntSize, 0.f);
        hVec3 screenposinc(0.f, debugFntSize, 0.f);
        hColour textcolour(1.f, 1.f, 1.f, 1.f);

        dd->begin();
        hStrPrintf(strbuf, strbufsize, "Device Calls: %u Draw Calls %u Prims %u", stats.nDeviceCalls_, stats.nDrawCalls_, stats.nPrims_);
        dd->drawText(screenpos, strbuf, textcolour);
        screenpos+=screenposinc;
        hStrPrintf(strbuf, strbufsize, "Frame Time: %.1fms", stats.frametime_);
        dd->drawText(screenpos, strbuf, textcolour);
        screenpos+=screenposinc;
        dd->end();
    }

}
#endif