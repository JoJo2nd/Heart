/********************************************************************

    filename:   hDebugInfo.cpp  
    
    Copyright (c) 28:7:2013 James Moran
    
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