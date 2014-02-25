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
        prims->lines_.reserve(prims->lines_.size()+linecount);
        for (hUint32 i=0, n=linecount; i<n; ++i) {
            prims->lines_.push_back(lines[i]);
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hDebugDraw::drawTris(const hVec3* tris, hUint tricount, const hColour& colour, hDebugSet set) {
        hDebugPrimsSet* prims=debugPrims_+set;
        prims->tris_.reserve(prims->tris_.size()+tricount);
        for (hUint32 i=0, n=tricount; i<n; ++i) {
            hDebugTriPosCol t={tris[i], colour};
            prims->tris_.push_back(t);
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hDebugDraw::drawText(const hVec3& screenpos, const hChar* buffer, const hColour& colour, hUint textLimit/*=0*/) {
        hDebugPrimsSet* prims=debugPrims_+eDebugSet_2DNoDepth;
        hDebugTextString txt;
        hUint c=0;
        txt.colour=colour;
        txt.position=screenpos;
        txt.txtBufOffset=(hUint)prims->txtBuffer_.size();
        while(buffer[c] && (c < textLimit || textLimit==0)) {
            prims->txtBuffer_.push_back(buffer[c]);
            ++c;
        }
        prims->txtBuffer_.push_back(0);
        prims->strings_.push_back(txt);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hDebugDraw::drawTexturedQuad(const hVec3& screenpos, hFloat width, hFloat height, hShaderResourceView* texturesrv) {
        hcAssert(texturesrv);
        hDebugPrimsSet* prims=debugPrims_+eDebugSet_2DNoDepth;
        hDebugTexQuad quad;
        quad.position=screenpos;
        quad.width=hVec3(width, 0.f, 0.f);
        quad.height=hVec3(0.f, height, 0.f);
        quad.srv=texturesrv;
        quad.startvtx=0;
        prims->texQuads_.push_back(quad);
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
    hThreadLocal static hDebugDraw* g_debugDraw = hNullptr;
    static hGlobalFinaliser< hDebugDraw > g_debugDrawFinaliser; 

    hDebugDraw* hDebugDraw::it() {
        if (!g_debugDraw) {
            g_debugDraw=hNEW(hDebugDraw)();
            g_debugDrawFinaliser.addObject(g_debugDraw);
        }
        return g_debugDraw;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hDebugPrimsSet::reset() {
        lines_.resize(0);
        tris_.resize(0);
        strings_.resize(0);
        texQuads_.resize(0);
        txtBuffer_.resize(0);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hDebugPrimsSet::clear() {
        lines_.clear();
        tris_.clear();
        strings_.clear();
        texQuads_.clear();
        txtBuffer_.clear();
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

    void hDebugDrawRenderer::initialiseResources(hRenderer* renderer, hResourceManager* resourceManager) {
        hInputLayoutDesc poscoldesc[] = {
            hInputLayoutDesc("POSITION", 0, eIF_FLOAT4, 0, 0),
            hInputLayoutDesc("COLOR", 0, eIF_FLOAT4, 0, 0),
        };
        hInputLayoutDesc poscoluvdesc[] = {
            hInputLayoutDesc("POSITION", 0, eIF_FLOAT4, 0, 0),
            hInputLayoutDesc("COLOR", 0, eIF_FLOAT4, 0, 0),
            hInputLayoutDesc("TEXCOORD", 0, eIF_FLOAT4, 0, 0),
        };
        hRenderMaterialManager* matmgr=renderer->GetMaterialManager();
        debugFontMat_=matmgr->getDebugFontMat();
        debugPosColMat_=matmgr->getDebugPosColMat();
        debugPosColUVMat_=matmgr->getDebugPosColUVMat();
        debugPosColAlphaMat_=matmgr->getDebugPosColAlphaMat();
        debugPosColUVAlphaMat_=matmgr->getDebugPosColUVAlphaMat();
        debugFont_=hNEW(hFont)();
        hRenderUtility::createDebugFont(renderer, resourceManager, debugFont_, &debugFontTex_);
        renderer->createVertexBuffer(hNullptr, s_maxDebugPrims, poscoldesc, (hUint)hArraySize(poscoldesc), RESOURCEFLAG_DYNAMIC, &posColBuffer_);
        renderer->createVertexBuffer(hNullptr, s_maxDebugPrims, poscoluvdesc, (hUint)hArraySize(poscoluvdesc), RESOURCEFLAG_DYNAMIC, &posColUVBuffer_);
        hShaderResourceViewDesc srvdesc;
        hZeroMem(&srvdesc, sizeof(srvdesc));
        srvdesc.format_=debugFontTex_->getTextureFormat();
        srvdesc.resourceType_=debugFontTex_->getRenderType();
        srvdesc.tex2D_.mipLevels_=debugFontTex_->getMipCount();
        srvdesc.tex2D_.topMip_=0;
        renderer->createShaderResourceView(debugFontTex_, srvdesc, &debugFontSRV_);

        hdInputLayout* inputlayout;
        hRenderCommandGenerator rcGen(&posColRdrLineCmds_);

        inputlayout=debugPosColMat_->getGroup(0)->getTech(0)->GetPass(0)->GetVertexShader()->createVertexLayout(poscoldesc, (hUint)hArraySize(poscoldesc));
        inputlayout_.push_back(inputlayout);
        rcGen.resetCommands();
        rcGen.setJump(debugPosColMat_->getRenderCommandsBegin(0, 0, 0));
        rcGen.setStreamInputs(PRIMITIVETYPE_LINELIST, NULL, hIndexBufferType_Index16, inputlayout, &posColBuffer_, 0, 1);
        rcGen.setReturn();
        
        rcGen.setRenderCommands(&posColRdrCmds_);
        rcGen.resetCommands();
        rcGen.setJump(debugPosColMat_->getRenderCommandsBegin(0, 0, 0));
        rcGen.setStreamInputs(PRIMITIVETYPE_TRILIST, NULL, hIndexBufferType_Index16, inputlayout, &posColBuffer_, 0, 1);
        rcGen.setReturn();

        rcGen.setRenderCommands(&posColUVRdrCmds_);
        inputlayout=debugPosColUVMat_->getGroup(0)->getTech(0)->GetPass(0)->GetVertexShader()->createVertexLayout(poscoluvdesc, (hUint)hArraySize(poscoluvdesc));
        inputlayout_.push_back(inputlayout);
        rcGen.resetCommands();
        rcGen.setJump(debugPosColUVMat_->getRenderCommandsBegin(0, 0, 0));
        rcGen.setStreamInputs(PRIMITIVETYPE_TRILIST, NULL, hIndexBufferType_Index16, inputlayout, &posColUVBuffer_, 0, 1);
        rcGen.setReturn();

        rcGen.setRenderCommands(&posColAlphaRdrCmds_);
        inputlayout=debugPosColAlphaMat_->getGroup(0)->getTech(0)->GetPass(0)->GetVertexShader()->createVertexLayout(poscoldesc, (hUint)hArraySize(poscoldesc));
        inputlayout_.push_back(inputlayout);
        rcGen.resetCommands();
        rcGen.setJump(debugPosColAlphaMat_->getRenderCommandsBegin(0, 0, 0));
        rcGen.setStreamInputs(PRIMITIVETYPE_TRILIST, NULL, hIndexBufferType_Index16, inputlayout, &posColBuffer_, 0, 1);
        rcGen.setReturn();

        rcGen.setRenderCommands(&posColAlphaRdrLineCmds_);
        inputlayout=debugPosColAlphaMat_->getGroup(0)->getTech(0)->GetPass(0)->GetVertexShader()->createVertexLayout(poscoldesc, (hUint)hArraySize(poscoldesc));
        inputlayout_.push_back(inputlayout);
        rcGen.resetCommands();
        rcGen.setJump(debugPosColAlphaMat_->getRenderCommandsBegin(0, 0, 0));
        rcGen.setStreamInputs(PRIMITIVETYPE_LINELIST, NULL, hIndexBufferType_Index16, inputlayout, &posColBuffer_, 0, 1);
        rcGen.setReturn();

        rcGen.setRenderCommands(&posColUVAlphaRdrCmds_);
        inputlayout=debugPosColUVAlphaMat_->getGroup(0)->getTech(0)->GetPass(0)->GetVertexShader()->createVertexLayout(poscoluvdesc, (hUint)hArraySize(poscoluvdesc));
        inputlayout_.push_back(inputlayout);
        rcGen.resetCommands();
        rcGen.setJump(debugPosColUVAlphaMat_->getRenderCommandsBegin(0, 0, 0));
        rcGen.setStreamInputs(PRIMITIVETYPE_TRILIST, NULL, hIndexBufferType_Index16, inputlayout, &posColUVBuffer_, 0, 1);
        rcGen.setReturn();

        rcGen.setRenderCommands(&textRdrCmds_);
        inputlayout=debugFontMat_->getGroup(0)->getTech(0)->GetPass(0)->GetVertexShader()->createVertexLayout(poscoluvdesc, (hUint)hArraySize(poscoluvdesc));
        inputlayout_.push_back(inputlayout);
        rcGen.resetCommands();
        rcGen.setJump(debugFontMat_->getRenderCommandsBegin(0, 0, 0));
        rcGen.setStreamInputs(PRIMITIVETYPE_TRILIST, NULL, hIndexBufferType_Index16, inputlayout, &posColUVBuffer_, 0, 1);
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

    void hDebugDrawRenderer::destroyResources() {
        if (resourcesCreated_) {
            if (debugFontTex_) {
                debugFontTex_->DecRef();
                debugFontTex_=hNullptr;
            }
            hDELETE_SAFE(debugFont_);

            if (posColBuffer_) {
                posColBuffer_->DecRef();
                posColBuffer_=hNullptr;
            }
            if (posColUVBuffer_) {
                posColUVBuffer_->DecRef();
                posColBuffer_=hNullptr;
            }
            if (colourView_) {
                colourView_->DecRef();
                colourView_=hNullptr;
            }
            if (depthView_) {
                depthView_->DecRef();
                depthView_=hNullptr;
            }
            if (debugFontSRV_) {
                debugFontSRV_->DecRef();
                debugFontSRV_=hNullptr;
            }
            posColRdrLineCmds_.release();
            posColRdrCmds_.release();
            posColUVRdrCmds_.release();
            textRdrCmds_.release();
            posColAlphaRdrCmds_.release();
            posColAlphaRdrLineCmds_.release();
            posColUVAlphaRdrCmds_.release();

            //Reset everything
            for (hUint32 i=0; i<eDebugSet_Max; ++i) {
                debugPrims_[i].clear();
            }

            resourcesCreated_ = false;
        }
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
            struct {
                hUint prims;
                hUint start;
            } tris;
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
            hUint vtxcount=0;
            for (hUint dps=0,ndps=eDebugSet_Max; dps<ndps && mapptr<mapend; ++dps) {
                const std::vector< hDebugLine >& lines = debugPrims_[dps].lines_;
                debugcalls[dps].lines.prims=0;
                debugcalls[dps].lines.start=vtxcount;
                for (hUint i=0,n=(hUint)lines.size(); i<n && mapptr<mapend; ++i) {
                    mapptr->vtx=lines[i].p1;
                    mapptr->colour=lines[i].colour;
                    ++mapptr;
                    mapptr->vtx=lines[i].p2;
                    mapptr->colour=lines[i].colour;
                    ++mapptr;

                    ++debugcalls[dps].lines.prims;
                    vtxcount+=2;
                }
            }
            for (hUint dps=0,ndps=eDebugSet_Max; dps<ndps && mapptr<mapend; ++dps) {
                const std::vector< hDebugTriPosCol >& tris = debugPrims_[dps].tris_;
                debugcalls[dps].tris.prims=0;
                debugcalls[dps].tris.start=vtxcount;
                for (hUint i=0,n=(hUint)tris.size(); i<n && mapptr<mapend; ++i) {
                    mapptr->vtx=tris[i].pos;
                    mapptr->colour=tris[i].colour;
                    ++mapptr;

                    debugcalls[dps].tris.prims += ((i+1)%3)==0 ? 1 : 0;
                    ++vtxcount;
                }
            }
            ctx->Unmap(&vbmap);
        }

        ctx->Map(posColUVBuffer_, &vbmap); {
            PosColUVVtx* mapptr=(PosColUVVtx*)vbmap.ptr_;
            PosColUVVtx* mapend=mapptr+(s_maxDebugPrims);
            hUint vtxcount=0;
            for (hUint dps=0,ndps=eDebugSet_Max; dps<ndps && mapptr<mapend; ++dps) {
                const std::vector< hDebugTextString >& txt = debugPrims_[dps].strings_;
                const std::vector< hChar >& txtbuf=debugPrims_[dps].txtBuffer_;
                debugcalls[dps].text.prims=0;
                debugcalls[dps].text.start=vtxcount;
                for (hUint i=0,n=(hUint)txt.size(); i<n && mapptr<mapend; ++i) {
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
                std::vector< hDebugTexQuad >& quads = debugPrims_[dps].texQuads_;
                hColour constwhite=hColour(1.f, 1.f, 1.f, 1.f);
                for (hUint i=0,n=(hUint)quads.size(); i<n && mapptr<mapend; ++i) {
                    mapptr->vtx=quads[i].position;
                    mapptr->colour=constwhite;
                    mapptr->uv[0]=0.f;
                    mapptr->uv[1]=1.f;
                    ++mapptr;
                    mapptr->vtx=quads[i].position+quads[i].height;
                    mapptr->colour=constwhite;
                    mapptr->uv[0]=0.f;
                    mapptr->uv[1]=0.f;
                    ++mapptr;
                    mapptr->vtx=quads[i].position+quads[i].width;
                    mapptr->colour=constwhite;
                    mapptr->uv[0]=1.f;
                    mapptr->uv[1]=1.f;
                    ++mapptr;
                    //
                    mapptr->vtx=quads[i].position+quads[i].width;
                    mapptr->colour=constwhite;
                    mapptr->uv[0]=1.f;
                    mapptr->uv[1]=1.f;
                    ++mapptr;
                    mapptr->vtx=quads[i].position+quads[i].height;
                    mapptr->colour=constwhite;
                    mapptr->uv[0]=0.f;
                    mapptr->uv[1]=0.f;
                    ++mapptr;
                    mapptr->vtx=quads[i].position+quads[i].width+quads[i].height;
                    mapptr->colour=constwhite;
                    mapptr->uv[0]=1.f;
                    mapptr->uv[1]=0.f;
                    ++mapptr;

                    quads[i].startvtx=vtxcount;
                    vtxcount+=6;
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
        if (debugcalls[eDebugSet_3DDepth].tris.prims) {
            ctx->runRenderCommands(posColRdrCmds_.getFirst());
            ctx->DrawPrimitive(debugcalls[eDebugSet_3DDepth].tris.prims, debugcalls[eDebugSet_3DDepth].tris.start);
        }
        if (debugcalls[eDebugSet_3DDepth].lines.prims) {
            ctx->runRenderCommands(posColRdrLineCmds_.getFirst());
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
        if (debugcalls[eDebugSet_2DNoDepth].tris.prims) {
            ctx->runRenderCommands(posColAlphaRdrCmds_.getFirst());
            ctx->DrawPrimitive(debugcalls[eDebugSet_2DNoDepth].tris.prims, debugcalls[eDebugSet_2DNoDepth].tris.start);
        }
        if (debugcalls[eDebugSet_2DNoDepth].lines.prims) {
            ctx->runRenderCommands(posColAlphaRdrLineCmds_.getFirst());
            ctx->DrawPrimitive(debugcalls[eDebugSet_2DNoDepth].lines.prims, debugcalls[eDebugSet_2DNoDepth].lines.start);
        }
        const std::vector< hDebugTexQuad >& quads = debugPrims_[eDebugSet_2DNoDepth].texQuads_;
        if (quads.size() > 0) {
            ctx->runRenderCommands(posColUVAlphaRdrCmds_.getFirst());
            for (hUint i=0, n=(hUint)quads.size(); i<n; ++i) {
                ctx->setViewPixel(0, quads[i].srv);
                ctx->DrawPrimitive(2, quads[i].startvtx);
            }
        }
        if (debugcalls[eDebugSet_2DNoDepth].text.prims) {
            ctx->runRenderCommands(textRdrCmds_.getFirst());
            ctx->DrawPrimitive(debugcalls[eDebugSet_2DNoDepth].text.prims, debugcalls[eDebugSet_2DNoDepth].text.start);
        }

        //Reset everything
        for (hUint32 i=0; i<eDebugSet_Max; ++i) {
            debugPrims_[i].reset();
        }
    }

    template < typename _ty >
    inline void appendVectors(std::vector<_ty>& lhs, const std::vector<_ty>& rhs) {
        lhs.reserve(lhs.size()+rhs.size());
        for (auto i=rhs.begin(), n=rhs.end(); i<n; ++i) {
            lhs.push_back(*i);
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
            appendVectors(debugPrims_[i].lines_, ddprims[i].lines_); 
            appendVectors(debugPrims_[i].tris_, ddprims[i].tris_);
            appendVectors(debugPrims_[i].texQuads_, ddprims[i].texQuads_);

            hUint newtxtstart=(hUint)debugPrims_[i].txtBuffer_.size();
            appendVectors(debugPrims_[i].txtBuffer_, ddprims[i].txtBuffer_);
            hUint stringcount=(hUint)debugPrims_[i].strings_.size();
            debugPrims_[i].strings_.reserve(debugPrims_[i].strings_.size()+stringcount);
            for (hUint t=0, n=(hUint)ddprims[i].strings_.size(); t<n; ++t) {
                debugPrims_[i].strings_.push_back(ddprims[i].strings_[t]);
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