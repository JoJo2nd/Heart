/********************************************************************

    filename:   hDebugDraw.cpp  
    
    Copyright (c) 27:6:2013 James Moran
    
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

    hDebugDraw::hDebugDraw() 
        : renderer_(hNullptr)
    {
        renderer_=hDebugDrawRenderer::it();
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hDebugDraw::~hDebugDraw() {

    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hDebugDraw::begin() {
        for (hUint32 i=0; i<eDebugSet_Max; ++i) {
            debugPrims_[i].reset();
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hDebugDraw::drawLines(hVec3* points, hUint32 linecount, hDebugSet set) {
        hDebugPrimsSet* prims=debugPrims_+set;
        prims->lines_.Reserve(prims->lines_.GetSize()+linecount);
        for (hUint32 i=0, n=linecount; i<n; i+=2) {
            hDebugLine line;
            line.p1 = points[i];
            line.p2 = points[i+1];
            prims->lines_.PushBack(line);
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hDebugDraw::end() {
        renderer_->append(this);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hDebugDraw* hDebugDraw::it() {
        hThreadLocal static hDebugDraw* debugDraw = hNullptr;
        if (!debugDraw) {
            debugDraw=hNEW(GetGlobalHeap(), hDebugDraw)();
        }
        return debugDraw;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hDebugPrimsSet::reset() {
        lines_.Resize(0);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hDebugDrawRenderer::hDebugDrawRenderer() {

    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hDebugDrawRenderer::~hDebugDrawRenderer() {

    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hDebugDrawRenderer::initialiseResources(hRenderer* renderer) {
        hInputLayoutDesc vbdesc[] = {
            {eIS_POSITION, 0, eIF_FLOAT4, 0, 0},
        };
        hRenderMaterialManager* matmgr=renderer->GetMaterialManager();
        debugMat_=matmgr->getWireframeDebug()->createMaterialInstance(0);
        renderer->createVertexBuffer(hNullptr, s_maxDebugPrims, vbdesc, (hUint)hArraySize(vbdesc), RESOURCEFLAG_DYNAMIC, GetGlobalHeap(), &linesBuffer_);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hDebugDrawRenderer::debugRenderSetup(hRenderTargetView* colour, hDepthStencilView* depth) {

    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hDebugDrawRenderer::render(hRenderSubmissionCtx* ctx) {
        if (!colourView_ || !depthView_ || !resourcesCreated_) {
            return;
        }
//         struct DebugDrawCall {
//             
//         } debugcalls[eDebugSet_Max];
        hMutexAutoScope mutscope(&critSection_);
        hVertexBufferMapInfo vbmap;
        hUint colourW = colourView_->getTarget()->getWidth();
        hUint colourH = colourView_->getTarget()->getHeight();
        hViewport vp(0, 0, colourW, colourH);
        hScissorRect scissor={0, 0, colourW, colourH};

        ctx->Map(linesBuffer_, &vbmap);
        hVec3* mapptr=(hVec3*)vbmap.ptr_;
        hVec3* mapend=mapptr+(s_maxDebugPrims);
        for (hUint dps=0,ndps=eDebugSet_Max; dps<ndps && mapptr<mapend; ++dps) {
            const hVector< hDebugLine >& lines = debugPrims_[dps].lines_;
            for (hUint i=0,n=s_maxDebugPrims; i<n && mapptr<mapend; ++i) {
                *mapptr = lines[i].p1;
                ++mapptr;
                *mapptr = lines[i].p2;
                ++mapptr;
            }
        }
        //ctx->Unmap();

        ctx->setTargets(1, &colourView_, depthView_);
        ctx->SetViewport(vp);
        ctx->SetScissorRect(scissor);
        //ctx->
        //ctx->SetInputStreams()
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hDebugDrawRenderer::append(hDebugDraw* debugdraw) {
        hcAssert(debugdraw);
        hMutexAutoScope mutscope(&critSection_);
        hDebugPrimsSet* ddprims=debugPrims_;
        for (hUint32 i=0; i<eDebugSet_Max; ++i) {
            debugPrims_[i].lines_.reserveGrow(ddprims[i].lines_.GetSize());
            for (hUint32 p=0, n=ddprims[i].lines_.GetSize(); p<n; ++p) {
                debugPrims_[i].lines_.PushBack(ddprims[i].lines_[p]);
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hDebugDrawRenderer* hDebugDrawRenderer::it() {
        static hDebugDrawRenderer debugDrawRenderer;
        return &debugDrawRenderer;
    }

}