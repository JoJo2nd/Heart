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

    void hDebugDraw::drawLines(hDebugLine* lines, hUint32 linecount, hDebugSet set) {
        hDebugPrimsSet* prims=debugPrims_+set;
        prims->lines_.Reserve(prims->lines_.GetSize()+linecount);
        for (hUint32 i=0, n=linecount; i<n; ++i) {
            prims->lines_.PushBack(lines[i]);
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hDebugDraw::drawText(const hVec3& screenpos, const hChar* buffer, const hColour& colour, hDebugSet set) {
        hDebugPrimsSet* prims=debugPrims_+set;
        hDebugTextString txt;
        txt.colour=colour;
        txt.position=screenpos;
        txt.txtBufOffset=prims->txtBuffer_.GetSize();
        while(*buffer) {
            prims->txtBuffer_.PushBack(*buffer);
            ++buffer;
        }
        prims->txtBuffer_.PushBack(0);
        prims->strings_.PushBack(txt);
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
        strings_.Resize(0);
        txtBuffer_.Resize(0);
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
        hInputLayoutDesc poscoldesc[] = {
            {eIS_POSITION, 0, eIF_FLOAT4, 0, 0},
            {eIS_COLOUR, 0, eIF_FLOAT4, 0, 0},
        };
        hInputLayoutDesc poscoluvdesc[] = {
            {eIS_POSITION, 0, eIF_FLOAT4, 0, 0},
            {eIS_COLOUR, 0, eIF_FLOAT4, 0, 0},
            {eIS_TEXCOORD, 0, eIF_FLOAT4, 0, 0},
        };
        hRenderMaterialManager* matmgr=renderer->GetMaterialManager();
        debugPosColMat_=matmgr->getDebugPosColMat();
        debugPosColUVMat_=matmgr->getDebugPosColUVMat();
        debugFont_=hNEW(GetGlobalHeap(), hFont)(GetGlobalHeap());
        hRenderUtility::createDebugFont(renderer, debugFont_, &debugFontTex_, GetGlobalHeap());
        renderer->createVertexBuffer(hNullptr, s_maxDebugPrims, poscoldesc, (hUint)hArraySize(poscoldesc), RESOURCEFLAG_DYNAMIC, GetGlobalHeap(), &posColBuffer_);
        renderer->createVertexBuffer(hNullptr, s_maxDebugPrims, poscoluvdesc, (hUint)hArraySize(poscoluvdesc), RESOURCEFLAG_DYNAMIC, GetGlobalHeap(), &posColUVBuffer_);
        hShaderResourceViewDesc srvdesc;
        hZeroMem(&srvdesc, sizeof(srvdesc));
        srvdesc.format_=debugFontTex_->getTextureFormat();
        srvdesc.resourceType_=debugFontTex_->getRenderType();
        srvdesc.tex2D_.mipLevels_=debugFontTex_->getMipCount();
        srvdesc.tex2D_.topMip_=0;
        renderer->createShaderResourceView(debugFontTex_, srvdesc, &debugFontSRV_);

        hRenderCommandGenerator rcGen(&posColRdrCmds_);

        inputlayout_=debugPosColMat_->getGroup(0)->getTech(0)->GetPass(0)->GetVertexShader()->createVertexLayout(poscoldesc, (hUint)hArraySize(poscoldesc));
        rcGen.resetCommands();
        rcGen.setJump(debugPosColMat_->getRenderCommandsBegin(0, 0, 0));
        rcGen.setStreamInputs(PRIMITIVETYPE_LINELIST, NULL, hIndexBufferType_Index16, inputlayout_, &posColBuffer_, 0, 1);
        rcGen.setReturn();

        rcGen.setRenderCommands(&posColUVRdrCmds_);

        inputlayout_=debugPosColUVMat_->getGroup(0)->getTech(0)->GetPass(0)->GetVertexShader()->createVertexLayout(poscoluvdesc, (hUint)hArraySize(poscoluvdesc));
        rcGen.resetCommands();
        rcGen.setJump(debugPosColUVMat_->getRenderCommandsBegin(0, 0, 0));
        rcGen.setStreamInputs(PRIMITIVETYPE_TRILIST, NULL, hIndexBufferType_Index16, inputlayout_, &posColUVBuffer_, 0, 1);
        rcGen.setReturn();

        hRenderMaterialManager* matMgr=renderer->GetMaterialManager();
        hTexture* backbuffer=matMgr->getGlobalTexture("back_buffer");
        hTexture* depth=matMgr->getGlobalTexture("depth_buffer");
        hRenderTargetViewDesc rtvd;
        hDepthStencilViewDesc dsvd;

        hZeroMem(&rtvd, sizeof(rtvd));
        rtvd.format_=backbuffer->getTextureFormat();
        rtvd.resourceType_=backbuffer->getRenderType();
        hcAssert(backbuffer->getRenderType()==eRenderResourceType_Tex2D);
        rtvd.tex2D_.topMip_=0;
        rtvd.tex2D_.mipLevels_=~0;
        renderer->createRenderTargetView(backbuffer, rtvd, &colourView_);
        hZeroMem(&dsvd, sizeof(dsvd));
        dsvd.format_=eTextureFormat_D32_float;
        dsvd.resourceType_=depth->getRenderType();
        hcAssert(depth->getRenderType()==eRenderResourceType_Tex2D);
        dsvd.tex2D_.topMip_=0;
        dsvd.tex2D_.mipLevels_=~0;
        renderer->createDepthStencilView(depth, dsvd, &depthView_);

        viewParameters_=renderer->GetMaterialManager()->GetGlobalConstantBlockByAlias("ViewportConstants");
        perDrawParameters_=renderer->GetMaterialManager()->GetGlobalConstantBlockByAlias("InstanceConstants");

        resourcesCreated_=true;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hDebugDrawRenderer::render(hRenderer* renderer, hRenderSubmissionCtx* ctx) {
        if (!colourView_ || !depthView_ || !resourcesCreated_) {
            return;
        }
        struct DebugDrawCall {
            struct {
                hUint prims;
                hUint start;
            } lines;
            struct {
                hUint prims;
                hUint start;
            } text;
        } debugcalls[eDebugSet_Max];
        struct PosColVtx {
            hVec3 vtx;
            hColour colour;
        };
        struct PosColUVVtx {
            hVec3 vtx;
            hColour colour;
            hFloat uv[4];
        };
        hMutexAutoScope mutscope(&critSection_);
        hVertexBufferMapInfo vbmap;
        hUint colourW = colourView_->getTarget()->getWidth();
        hUint colourH = colourView_->getTarget()->getHeight();
        hViewport vp(0, 0, colourW, colourH);
        hScissorRect scissor={0, 0, colourW, colourH};
        hRendererCamera* camera = renderer->GetRenderCamera(0);
        hInstanceConstants* inst;
        hViewportShaderConstants* viewconst;
        hRenderBufferMapInfo map;;

        ctx->Map(posColBuffer_, &vbmap); {
            PosColVtx* mapptr=(PosColVtx*)vbmap.ptr_;
            PosColVtx* mapend=mapptr+(s_maxDebugPrims);
            hUint linescount=0;
            for (hUint dps=0,ndps=eDebugSet_Max; dps<ndps && mapptr<mapend; ++dps) {
                const hVector< hDebugLine >& lines = debugPrims_[dps].lines_;
                debugcalls[dps].lines.prims=0;
                debugcalls[dps].lines.start=linescount;
                for (hUint i=0,n=lines.GetSize(); i<n && mapptr<mapend; ++i) {
                    mapptr->vtx=lines[i].p1;
                    mapptr->colour=lines[i].colour;
                    ++mapptr;
                    mapptr->vtx=lines[i].p2;
                    mapptr->colour=lines[i].colour;
                    ++mapptr;

                    ++debugcalls[dps].lines.prims;
                    ++linescount;
                }
            }
            ctx->Unmap(&vbmap);
        }

        ctx->Map(posColUVBuffer_, &vbmap); {
            PosColUVVtx* mapptr=(PosColUVVtx*)vbmap.ptr_;
            PosColUVVtx* mapend=mapptr+(s_maxDebugPrims);
            hUint vtxcount=0;
            for (hUint dps=0,ndps=eDebugSet_Max; dps<ndps && mapptr<mapend; ++dps) {
                const hVector< hDebugTextString >& txt = debugPrims_[dps].strings_;
                const hVector< hChar >& txtbuf=debugPrims_[dps].txtBuffer_;
                debugcalls[dps].text.prims=0;
                debugcalls[dps].text.start=vtxcount;
                for (hUint i=0,n=txt.GetSize(); i<n && mapptr<mapend; ++i) {
                    hUint c=txt[i].txtBufOffset;
                    hVec3 pos=txt[i].position;
                    hColour colour= txt[i].colour;
                    while (txtbuf[c]) {
                        hUTF8::Unicode uc;
                        c+=hUTF8::DecodeToUnicode(&txtbuf[c], uc);
                        const hFontCharacter& fc=*debugFont_->GetFontCharacter(uc);
                        // 
                        hVec3 bl(fc.xOffset_,           fc.yOffset_,            0.f);
                        hVec3 tl(fc.xOffset_,           fc.yOffset_+fc.height_, 0.f);
                        hVec3 br(fc.xOffset_+fc.width_, fc.yOffset_,            0.f);
                        hVec3 tr(fc.xOffset_+fc.width_, fc.yOffset_+fc.height_, 0.f);
                        //
                        mapptr->vtx=pos+tl;
                        mapptr->colour=colour;
                        mapptr->uv[0]=fc.UV1_.x;
                        mapptr->uv[1]=fc.UV2_.y;
                        ++mapptr;
                        //
                        mapptr->vtx=pos+tr;
                        mapptr->colour=colour;
                        mapptr->uv[0]=fc.UV2_.x;
                        mapptr->uv[1]=fc.UV2_.y;
                        ++mapptr;
                        //
                        mapptr->vtx=pos+bl;
                        mapptr->colour=colour;
                        mapptr->uv[0]=fc.UV1_.x;
                        mapptr->uv[1]=fc.UV1_.y;
                        ++mapptr;
                        //
                        mapptr->vtx=pos+tr;
                        mapptr->colour=colour;
                        mapptr->uv[0]=fc.UV2_.x;
                        mapptr->uv[1]=fc.UV2_.y;
                        ++mapptr;
                        //
                        mapptr->vtx=pos+br;
                        mapptr->colour=colour;
                        mapptr->uv[0]=fc.UV2_.x;
                        mapptr->uv[1]=fc.UV1_.y;
                        ++mapptr;
                        //
                        mapptr->vtx=pos+bl;
                        mapptr->colour=colour;
                        mapptr->uv[0]=fc.UV1_.x;
                        mapptr->uv[1]=fc.UV1_.y;
                        ++mapptr;

                        pos+=hVec3(fc.xAdvan_, 0.f, 0.f);
                        debugcalls[dps].text.prims+=2;
                        vtxcount+=6;
                    }
                }
            }
            ctx->Unmap(&vbmap);
        }

        //3D pass
        ctx->setTargets(1, &colourView_, depthView_);
        ctx->SetViewport(vp);
        ctx->SetScissorRect(scissor);
        ctx->Map(perDrawParameters_, &map);
        inst = (hInstanceConstants*)map.ptr;
        inst->world_ = hMatrixFunc::identity();
        ctx->Unmap(&map);

        ctx->Map(viewParameters_, &map);
        viewconst=(hViewportShaderConstants*)map.ptr;
        *viewconst=*camera->GetViewportConstants();
        ctx->Unmap(&map);
        //lines
        if (debugcalls[eDebugSet_3DDepth].lines.prims) {
            ctx->runRenderCommands(posColRdrCmds_.getFirst());
            ctx->DrawPrimitive(debugcalls[eDebugSet_3DDepth].lines.prims, debugcalls[eDebugSet_3DDepth].lines.start);
        }

        //2D pass
        ctx->Map(viewParameters_, &map); {
            hMatrix view=hMatrixFunc::identity();
            hMatrix proj=hMatrixFunc::orthoProj((hFloat)colourW, (hFloat)colourH, 0.f, 1000.f);
            viewconst=(hViewportShaderConstants*)map.ptr;
            viewconst->view_=view;
            viewconst->viewInverse_=view;
            viewconst->viewInverseTranspose_=view;
            viewconst->projection_=proj;
            viewconst->projectionInverse_=hMatrixFunc::inverse(proj);
            viewconst->viewProjection_=proj;
            viewconst->viewProjectionInverse_=viewconst->projectionInverse_;
            viewconst->viewportSize_[0]=(hFloat)colourW;
            viewconst->viewportSize_[1]=(hFloat)colourH;
            viewconst->viewportSize_[2]=0.f;
            viewconst->viewportSize_[3]=0.f;
            ctx->Unmap(&map);
        }
        //
        if (debugcalls[eDebugSet_2DNoDepth].lines.prims) {
            ctx->runRenderCommands(posColRdrCmds_.getFirst());
            ctx->DrawPrimitive(debugcalls[eDebugSet_2DNoDepth].lines.prims, debugcalls[eDebugSet_2DNoDepth].lines.start);
        }
        if (debugcalls[eDebugSet_2DNoDepth].text.prims) {
            ctx->runRenderCommands(posColUVRdrCmds_.getFirst());
            ctx->setViewPixel(0, debugFontSRV_);
            ctx->DrawPrimitive(debugcalls[eDebugSet_2DNoDepth].text.prims, debugcalls[eDebugSet_2DNoDepth].text.start);
        }

        //Reset everything
        for (hUint32 i=0; i<eDebugSet_Max; ++i) {
            debugPrims_[i].reset();
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hDebugDrawRenderer::append(hDebugDraw* debugdraw) {
        hcAssert(debugdraw);
        hMutexAutoScope mutscope(&critSection_);
        hDebugPrimsSet* ddprims=debugdraw->debugPrims_;
        for (hUint32 i=0; i<eDebugSet_Max; ++i) {
            debugPrims_[i].lines_.reserveGrow(ddprims[i].lines_.GetSize());
            for (hUint32 p=0, n=ddprims[i].lines_.GetSize(); p<n; ++p) {
                debugPrims_[i].lines_.PushBack(ddprims[i].lines_[p]);
            }
            hUint newtxtstart=debugPrims_[i].txtBuffer_.GetSize();
            hUint stringcount=debugPrims_[i].strings_.GetSize();
            debugPrims_[i].txtBuffer_.reserveGrow(ddprims[i].txtBuffer_.GetSize());
            for (hUint c=0, n=ddprims[i].txtBuffer_.GetSize(); c<n; ++c) {
                debugPrims_[i].txtBuffer_.PushBack(ddprims[i].txtBuffer_[c]);
            }
            for (hUint t=0, n=ddprims[i].strings_.GetSize(); t<n; ++t) {
                debugPrims_[i].strings_.PushBack(ddprims[i].strings_[t]);
                debugPrims_[i].strings_[stringcount++].txtBufOffset+=newtxtstart;
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