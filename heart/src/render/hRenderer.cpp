/********************************************************************

    filename: 	hRenderer.cpp	
    
    Copyright (c) 1:4:2012 James Moran
    
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

    void* hRenderer::pRenderThreadID_ = NULL;

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    
    void* RnTmpMalloc( hUint32 size )
    {
        return GetGlobalHeap()->alloc( size, 16 );
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    
    void RnTmpFree( void* ptr )
    {
        GetGlobalHeap()->release( ptr );
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hRenderer::hRenderer()
        : currentRenderStatFrame_( 0 )
        , statPass_( 0 )
    {
        //SetImpl( hNEW(GetGlobalHeap(), hdRenderDevice) );
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderer::Create( 
        hSystem* pSystem, 
        hUint32 width, 
        hUint32 height, 
        hUint32 bpp, 
        hFloat shaderVersion, 
        hBool fullscreen, 
        hBool vsync,
        hResourceManager* pResourceManager	)
    {

        materialManager_.SetRenderer(this);

        width_			= width;
        height_			= height;
        bpp_			= bpp;
        shaderVersion_	= shaderVersion;
        fullscreen_		= fullscreen;
        vsync_			= vsync;
        resourceManager_ = pResourceManager;
        system_			=  pSystem;
        backBuffer_     = hNullptr;

        hAtomic::AtomicSet(scratchPtrOffset_, 0);
        hAtomic::AtomicSet(drawCallBlockIdx_, 0);

        //depthBuffer_=hNEW(GetGlobalHeap(), hTexture)(
        //    hFUNCTOR_BINDMEMBER(hTexture::hZeroRefProc, hRenderer, destroyTexture, this), eRenderResourceType_Tex2D, GetGlobalHeap());

        hRenderDeviceSetup setup;
        setup.alloc_ = RnTmpMalloc;
        setup.free_ = RnTmpFree;
        setup.depthBufferTex_ = hNullptr;
        //setup.depthBufferTex_= depthBuffer_;
        ParentClass::Create( system_, width_, height_, fullscreen_, vsync_, setup );

        ParentClass::InitialiseMainRenderSubmissionCtx(&mainSubmissionCtx_.impl_);

//         depthBuffer_->format_=TFORMAT_D24S8F;
//         depthBuffer_->nLevels_=1;
//         depthBuffer_->levelDescs_=hNEW_ARRAY(GetGlobalHeap(), hTexture::LevelDesc, 1);
//         depthBuffer_->levelDescs_->mipdata_=NULL;
//         depthBuffer_->levelDescs_->mipdataSize_=0;
//         depthBuffer_->levelDescs_->width_ = GetWidth();
//         depthBuffer_->levelDescs_->height_ = GetHeight();


//         const hChar* bbalias[] = {
//             "back_buffer",
//             "g_back_buffer",
//         };
//         materialManager_.registerGlobalTexture("back_buffer", backBuffer_, bbalias, hStaticArraySize(bbalias));
//         const hChar* dbalias[] = {
//             "depth_buffer",
//             "g_depth_buffer",
//             "z_buffer",
//             "g_z_buffer",
//         };
//         materialManager_.registerGlobalTexture("depth_buffer", depthBuffer_, dbalias, hStaticArraySize(dbalias));

        createDebugShadersInternal();

        hResourceHandler texreshandler;
        texreshandler.loadProc_  =hFUNCTOR_BINDMEMBER(hResourceLoadProc, hRenderer,   textureResourceLoader, this);
        texreshandler.linkProc_  =hFUNCTOR_BINDMEMBER(hResourceLinkProc, hRenderer,   textureResourceLink,   this);
        texreshandler.unlinkProc_=hFUNCTOR_BINDMEMBER(hResourceUnlinkProc, hRenderer, textureResourceUnlink, this);
        texreshandler.unloadProc_=hFUNCTOR_BINDMEMBER(hResourceUnloadProc, hRenderer, textureResourceUnload, this);
        resourceManager_->registerResourceHandler(hResourceType(TEXTURE_MAGIC_NUM), texreshandler);

        hResourceHandler shaderreshandler;
        shaderreshandler.loadProc_  =hFUNCTOR_BINDMEMBER(hResourceLoadProc, hRenderer,   shaderResourceLoader, this);
        shaderreshandler.linkProc_  =hFUNCTOR_BINDMEMBER(hResourceLinkProc, hRenderer,   shaderResourceLink,   this);
        shaderreshandler.unlinkProc_=hFUNCTOR_BINDMEMBER(hResourceUnlinkProc, hRenderer, shaderResourceUnlink, this);
        shaderreshandler.unloadProc_=hFUNCTOR_BINDMEMBER(hResourceUnloadProc, hRenderer, shaderResourceUnload, this);
        resourceManager_->registerResourceHandler(hResourceType(SHADER_MAGIC_NUM), shaderreshandler);

        hResourceHandler matreshandler;
        matreshandler.loadProc_  =hFUNCTOR_BINDMEMBER(hResourceLoadProc, hRenderer,   materialResourceLoader, this);
        matreshandler.linkProc_  =hFUNCTOR_BINDMEMBER(hResourceLinkProc, hRenderer,   materialResourceLink,   this);
        matreshandler.unlinkProc_=hFUNCTOR_BINDMEMBER(hResourceUnlinkProc, hRenderer, materialResourceUnlink, this);
        matreshandler.unloadProc_=hFUNCTOR_BINDMEMBER(hResourceUnloadProc, hRenderer, materialResourceUnload, this);
        resourceManager_->registerResourceHandler(hResourceType(MATERIAL_MAGIC_NUM), matreshandler);

        hResourceHandler meshreshandler;
        meshreshandler.loadProc_  =hFUNCTOR_BINDMEMBER(hResourceLoadProc, hRenderer,   meshResourceLoader, this);
        meshreshandler.linkProc_  =hFUNCTOR_BINDMEMBER(hResourceLinkProc, hRenderer,   meshResourceLink,   this);
        meshreshandler.unlinkProc_=hFUNCTOR_BINDMEMBER(hResourceUnlinkProc, hRenderer, meshResourceUnlink, this);
        meshreshandler.unloadProc_=hFUNCTOR_BINDMEMBER(hResourceUnloadProc, hRenderer, meshResourceUnload, this);
        resourceManager_->registerResourceHandler(hResourceType(MESH_MAGIC_NUM), meshreshandler);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderer::Destroy()
    {
        for (hUint32 i = 0; i < HEART_MAX_RENDER_CAMERAS; ++i)
        {
            GetRenderCamera(i)->releaseRenderTargetSetup();
        }
        materialManager_.destroyRenderResources();
        for (hUint i=0; i<eDebugShaderMax; ++i) {
            debugShaders_[i]->DecRef();
        }
        //hDELETE_ARRAY_SAFE(GetGlobalHeap(), depthBuffer_->levelDescs_);
        //depthBuffer_->DecRef();
        ParentClass::Destroy();
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderer::BeginRenderFrame()
    {
        HEART_PROFILE_FUNC();
        //ReleasePendingRenderResources();
        //Start new frame

        // clear the render buffer
        ++currentRenderStatFrame_;

        //Free last frame draw calls and temporary memory
        hAtomic::AtomicSet(scratchPtrOffset_, 0);
        hAtomic::AtomicSet(drawCallBlockIdx_, 0);
        hAtomic::AtomicSet(drawResourceUpdateCalls_, 0);

        ParentClass::BeginRender(&gpuTime_);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    
    void hRenderer::EndRenderFrame()
    {
        HEART_PROFILE_FUNC();
        if (!backBuffer_) {
            backBuffer_=materialManager_.getGlobalTexture("back_buffer");
        }

        ParentClass::EndRender();
        ParentClass::SwapBuffers(backBuffer_);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderer::createTexture(hUint32 levels, hMipDesc* initialData, hTextureFormat format, hUint32 flags, hMemoryHeapBase* heap, hTexture** outTex)
    {
        hcAssert(initialData);
        hcAssert(levels > 0);

        (*outTex) = hNEW(heap, hTexture)(
            hFUNCTOR_BINDMEMBER(hTexture::hZeroRefProc, hRenderer, destroyTexture, this),
            eRenderResourceType_Tex2D, heap);

        (*outTex)->nLevels_ = levels;
        (*outTex)->format_ = format;
        (*outTex)->flags_ = flags;
        (*outTex)->levelDescs_ = levels ? hNEW_ARRAY(heap, hTexture::LevelDesc, levels) : NULL;

        for (hUint32 i = 0; i < levels; ++i)
        {
            (*outTex)->levelDescs_[ i ].width_ = initialData[i].width;
            (*outTex)->levelDescs_[ i ].height_ = initialData[i].height;
            (*outTex)->levelDescs_[ i ].mipdata_ = initialData[i].data;
            (*outTex)->levelDescs_[ i ].mipdataSize_ = initialData[i].size;
        }

        ParentClass::createTextureDevice(levels, format, initialData, flags, (*outTex));

        if ((flags & (RESOURCEFLAG_KEEPCPUDATA|RESOURCEFLAG_DONTOWNCPUDATA)) == 0) {
            (*outTex)->ReleaseCPUTextureData();
            (*outTex)->SetKeepCPU(hFalse);
        } else if ((flags & RESOURCEFLAG_DONTOWNCPUDATA) == 0) {
            (*outTex)->SetKeepCPU(hTrue);
        } else if (flags & RESOURCEFLAG_DONTOWNCPUDATA) {
            for (hUint32 i = 0; i < levels; ++i) {
                (*outTex)->levelDescs_[ i ].mipdata_ = NULL;
                (*outTex)->levelDescs_[ i ].mipdataSize_ = 0;
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderer::resizeTexture(hUint32 width, hUint32 height, hTexture* inout)
    {
        hcAssert(width > 0 && height > 0 && inout);
        //TODO: do a down-size render? Atm this is only used for render targets so content is throw away.
        if (inout == backBuffer_) {
            backBuffer_=hNullptr;
        }
        hMipDesc mipsdata[16];// Based on the largest texture dim is 64K (is that even possible? 13 should be safe)
        hUint lvls=0;
        hUint w=width;
        hUint h=height;
        hMemoryHeapBase* heap=inout->heap_;
        while (w >= 1 && h >= 1 && lvls < 16 && lvls < inout->nLevels_) {
            mipsdata[lvls].width=w;
            mipsdata[lvls].height=h;
            mipsdata[lvls].data=NULL;
            mipsdata[lvls].size=0;
            w /= 2;
            h /= 2;
            ++lvls;
        }

        ParentClass::destroyTextureDevice(inout);
        ParentClass::createTextureDevice(lvls, inout->format_, mipsdata, inout->flags_, inout);
        hDELETE_ARRAY_SAFE(heap, inout->levelDescs_);
        inout->nLevels_ = lvls;
        inout->levelDescs_ = lvls ? hNEW_ARRAY(heap, hTexture::LevelDesc, lvls) : NULL;

        for (hUint32 i = 0; i < lvls; ++i) {
            inout->levelDescs_[ i ].width_       = mipsdata[i].width;
            inout->levelDescs_[ i ].height_      = mipsdata[i].height;
            inout->levelDescs_[ i ].mipdata_     = mipsdata[i].data;
            inout->levelDescs_[ i ].mipdataSize_ = mipsdata[i].size;
        }
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////////

    void hRenderer::destroyTexture(hTexture* pOut)
    {
        hcAssert(pOut->GetRefCount() == 0);
        ParentClass::destroyTextureDevice(pOut);
        hDELETE_SAFE(pOut->heap_, pOut);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderer::createIndexBuffer(void* pIndices, hUint32 nIndices, hUint32 flags, hIndexBuffer** outIB)
    {
        hUint elementSize= nIndices > 0xFFFF ? sizeof(hUint32) : sizeof(hUint16);
        hIndexBuffer* pdata = hNEW(GetGlobalHeap(), hIndexBuffer)(
            hFUNCTOR_BINDMEMBER(hIndexBuffer::hZeroProc, hRenderer, destroyIndexBuffer, this));
        pdata->pIndices_ = NULL;
        pdata->nIndices_ = nIndices;
        pdata->type_= nIndices > 0xFFFF ? hIndexBufferType_Index32 : hIndexBufferType_Index16;
        ParentClass::createIndexBufferDevice(nIndices*elementSize, pIndices, flags, pdata);
        *outIB = pdata;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderer::destroyIndexBuffer(hIndexBuffer* ib)
    {
        hcAssert(ib);
        ParentClass::destroyIndexBufferDevice(ib);
        hDELETE_SAFE(GetGlobalHeap(), ib);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderer::createVertexBuffer(void* initData, hUint32 nElements, hInputLayoutDesc* desc, hUint32 desccount, hUint32 flags, hMemoryHeapBase* heap, hVertexBuffer** outVB)
    {
        hVertexBuffer* pdata = hNEW(heap, hVertexBuffer)(heap,
            hFUNCTOR_BINDMEMBER(hVertexBuffer::hZeroProc, hRenderer, destroyVertexBuffer, this));
        pdata->vtxCount_ = nElements;
        pdata->stride_ = ParentClass::computeVertexLayoutStride( desc, desccount );
        ParentClass::createVertexBufferDevice(desc, desccount, pdata->stride_, nElements*pdata->stride_, initData, flags, pdata);
        *outVB = pdata;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderer::destroyVertexBuffer(hVertexBuffer* vb)
    {
        hcAssert(vb);
        hMemoryHeapBase* heap = vb->heap_;
        ParentClass::destroyVertexBufferDevice(vb);
        hDELETE_SAFE(heap, vb);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hRenderSubmissionCtx* hRenderer::CreateRenderSubmissionCtx()
    {
        hRenderSubmissionCtx* ret = hNEW(GetGlobalHeap()/*!heap*/, hRenderSubmissionCtx);
        ret->Initialise( this );
        ParentClass::InitialiseRenderSubmissionCtx( &ret->impl_ );

        return ret;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderer::DestroyRenderSubmissionCtx( hRenderSubmissionCtx* ctx )
    {
        hcAssert( ctx );
        ParentClass::DestroyRenderSubmissionCtx( &ctx->impl_ );
        hDELETE_SAFE(GetGlobalHeap()/*!heap*/, ctx);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    bool hRenderer::IsRenderThread()
    {
        //means nothing now...
        return pRenderThreadID_ == Device::GetCurrentThreadID();
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderer::SubmitDrawCallBlock( hDrawCall* block, hUint32 count )
    {
        hcAssertMsg(drawCallBlockIdx_.value_+count < MAX_DCBLOCKS, "Too many draw calls");
        if (drawCallBlockIdx_.value_+count >= MAX_DCBLOCKS)
            return;

        hUint32 wIdx;
        hAtomic::AtomicAddWithPrev(drawCallBlockIdx_, count, &wIdx);

        //copy calls
        count += wIdx;
        for (; wIdx < count; ++wIdx, ++block)
        {
            drawCallBlocks_[wIdx] = *block;
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void* hRenderer::allocTempRenderMemory( hUint32 size )
    {
        hUint32 ret;
        hAtomic::AtomicAddWithPrev(scratchPtrOffset_, size, &ret);
        return drawDataScratchBuffer_+ret;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hUint32 hRenderer::beginCameraRender(hRenderSubmissionCtx* ctx, hUint32 camID)
    {
        hRendererCamera* camera = GetRenderCamera(camID);
        camera->UpdateParameters(ctx);
        hUint32 retTechMask = camera->GetTechniqueMask();

        ctx->setTargets(camera->getTargetCount(), camera->getTargets(), camera->getDepthTarget());
        ctx->SetViewport(camera->getTargetViewport());
        if (camera->getClearScreenFlag()) {
            for (hUint t=0, n=camera->getTargetCount(); t<n; ++t) {
                ctx->clearColour(camera->getTargets()[t], BLACK);
            }
            if (camera->getDepthTarget()) {
                ctx->clearDepth(camera->getDepthTarget(), 1.f);
            }
        }

        return retTechMask;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    int drawCallCompare(const void* lhs, const void* rhs)
    {
        if (((hDrawCall*)lhs)->sortKey_ < ((hDrawCall*)rhs)->sortKey_) {
            return -1;
        } else if ((((hDrawCall*)lhs)->sortKey_ > ((hDrawCall*)rhs)->sortKey_)) {
            return 1;
        } else {
            return 0;
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderer::CollectAndSortDrawCalls()
    {
        HEART_PROFILE_FUNC();
        //TODO: wait on job chain
        
        // POSSIBLE TODO: parallel merge sort
        qsort(drawCallBlocks_.GetBuffer(), drawCallBlockIdx_.value_, sizeof(hDrawCall), drawCallCompare);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderer::SubmitDrawCallsMT()
    {
        HEART_PROFILE_FUNC();
        /* TODO: Multi-thread submit, but ST is debug-able */
        SubmitDrawCallsST();
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderer::SubmitDrawCallsST()
    {
        HEART_PROFILE_FUNC();
        /*Single thread submit*/
        hUint32 camera = ~0U;
        const hMaterial* material = NULL;
        hMaterial* materialInst = NULL;
        hUint32 lastMatKey = 0;
        hUint32 pass = ~0U;
        hUint32 tmask = ~0U;
        hUint32 dcs = drawCallBlockIdx_.value_;

        for (hUint32 dc = 0; dc < dcs; ++dc) {
            hDrawCall* dcall=&drawCallBlocks_[dc];
            // For masks check hBuildRenderSortKey()
            hUint32 nCam = (dcall->sortKey_&0xF000000000000000) >> 60;
            hUint32 nPass = (dcall->sortKey_&0xF);
            hUint32 matKey = (dcall->sortKey_&0x3FFFFF); // stored in lower 28 bits
            if (nCam != camera)
            {
                //Begin camera pass
                tmask = beginCameraRender(&mainSubmissionCtx_, nCam);
                camera = nCam;
            }
            if (dcall->customCallFlag_) {
                dcall->customCall_(this, &mainSubmissionCtx_);
            } else {
                mainSubmissionCtx_.runRenderCommands(dcall->rCmds_);
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderer::initialiseCameras()
    {
        // TODO: power this by a lua script
        for (hUint32 i = 0; i < HEART_MAX_RENDER_CAMERAS; ++i)
        {
            GetRenderCamera(i)->Initialise(this);
        }

        hTexture* bb=materialManager_.getGlobalTexture("back_buffer");
        hTexture* db=materialManager_.getGlobalTexture("depth_buffer");
        hRenderTargetView* rtv=NULL;
        hDepthStencilView* dsv=NULL;
        hRenderTargetViewDesc rtvd;
        hDepthStencilViewDesc dsvd;
        hZeroMem(&rtvd, sizeof(rtvd));
        hZeroMem(&dsvd, sizeof(dsvd));
        rtvd.format_=bb->getTextureFormat();
        rtvd.resourceType_=bb->getRenderType();
        hcAssert(bb->getRenderType()==eRenderResourceType_Tex2D);
        rtvd.tex2D_.topMip_=0;
        rtvd.tex2D_.mipLevels_=~0;
        dsvd.format_=eTextureFormat_D32_float;//db->getTextureFormat();
        dsvd.resourceType_=db->getRenderType();
        hcAssert(db->getRenderType()==eRenderResourceType_Tex2D);
        dsvd.tex2D_.topMip_=0;
        dsvd.tex2D_.mipLevels_=~0;
        createRenderTargetView(bb, rtvd, &rtv);
        createDepthStencilView(db, dsvd, &dsv);

        //Create viewport/camera for debug drawing
        hRendererCamera* camera = GetRenderCamera(HEART_DEBUGUI_CAMERA_ID);
        hRenderViewportTargetSetup rtDesc = {0};
        rtDesc.nTargets_=1;
        rtDesc.targetTex_=bb;
        rtDesc.targets_[0]=rtv;
        rtDesc.depth_=dsv;

        hRelativeViewport vp;
        vp.x=0.f;
        vp.y=0.f;
        vp.w=1.f;
        vp.h=1.f;

        camera->bindRenderTargetSetup(rtDesc);
        camera->SetFieldOfView(45.f);
        camera->SetOrthoParams(0.f, 0.f, (hFloat)GetWidth(), (hFloat)GetHeight(), 0.1f, 100.f);
        camera->SetViewMatrix( Heart::hMatrixFunc::identity() );
        camera->setViewport(vp);
        camera->SetTechniquePass(materialManager_.GetRenderTechniqueInfo("main"));

        rtv->DecRef();
        dsv->DecRef();
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hShaderProgram* hRenderer::getDebugShader(hDebugShaderID shaderID) {
        hcAssert(shaderID < eDebugShaderMax);
        return debugShaders_[shaderID];
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderer::createDebugShadersInternal() {
        debugShaders_[eDebugPixelWhite]=hNEW(GetGlobalHeap(), hShaderProgram)(this, GetGlobalHeap(),
            hFUNCTOR_BINDMEMBER(hShaderProgram::hZeroProc, hRenderer, destroyShader, this));
        ParentClass::compileShaderFromSourceDevice(
            GetGlobalHeap(), ParentClass::getDebugShaderSource(eDebugPixelWhite),
            hStrLen(ParentClass::getDebugShaderSource(eDebugPixelWhite)),
            "mainFP", eShaderProfile_ps4_0, debugShaders_[eDebugPixelWhite]);
        debugShaders_[eDebugVertexPosOnly]=hNEW(GetGlobalHeap(), hShaderProgram)(this, GetGlobalHeap(),
            hFUNCTOR_BINDMEMBER(hShaderProgram::hZeroProc, hRenderer, destroyShader, this));
        ParentClass::compileShaderFromSourceDevice(
            GetGlobalHeap(), ParentClass::getDebugShaderSource(eDebugVertexPosOnly),
            hStrLen(ParentClass::getDebugShaderSource(eDebugVertexPosOnly)),
            "mainVP", eShaderProfile_vs4_0, debugShaders_[eDebugVertexPosOnly]);
        debugShaders_[eConsoleVertex]=hNEW(GetGlobalHeap(), hShaderProgram)(this, GetGlobalHeap(),
            hFUNCTOR_BINDMEMBER(hShaderProgram::hZeroProc, hRenderer, destroyShader, this));
        debugShaders_[eConsoleVertex]->shaderType_=ShaderType_VERTEXPROG;
        ParentClass::compileShaderFromSourceDevice(
            GetGlobalHeap(), ParentClass::getDebugShaderSource(eConsoleVertex),
            hStrLen(ParentClass::getDebugShaderSource(eConsoleVertex)),
            "mainVP", eShaderProfile_vs4_0, debugShaders_[eConsoleVertex]);
        debugShaders_[eConsolePixel]=hNEW(GetGlobalHeap(), hShaderProgram)(this, GetGlobalHeap(),
            hFUNCTOR_BINDMEMBER(hShaderProgram::hZeroProc, hRenderer, destroyShader, this));
        ParentClass::compileShaderFromSourceDevice(
            GetGlobalHeap(), ParentClass::getDebugShaderSource(eConsolePixel),
            hStrLen(ParentClass::getDebugShaderSource(eConsolePixel)),
            "mainFP", eShaderProfile_ps4_0, debugShaders_[eConsolePixel]);
        debugShaders_[eDebugFontVertex]=hNEW(GetGlobalHeap(), hShaderProgram)(this, GetGlobalHeap(),
            hFUNCTOR_BINDMEMBER(hShaderProgram::hZeroProc, hRenderer, destroyShader, this));
        ParentClass::compileShaderFromSourceDevice(
            GetGlobalHeap(), ParentClass::getDebugShaderSource(eDebugFontVertex),
            hStrLen(ParentClass::getDebugShaderSource(eDebugFontVertex)),
            "mainVP", eShaderProfile_vs4_0, debugShaders_[eDebugFontVertex]);
        debugShaders_[eDebugFontPixel]=hNEW(GetGlobalHeap(), hShaderProgram)(this, GetGlobalHeap(),
            hFUNCTOR_BINDMEMBER(hShaderProgram::hZeroProc, hRenderer, destroyShader, this));
        ParentClass::compileShaderFromSourceDevice(
            GetGlobalHeap(), ParentClass::getDebugShaderSource(eDebugFontPixel),
            hStrLen(ParentClass::getDebugShaderSource(eDebugFontPixel)),
            "mainFP", eShaderProfile_ps4_0, debugShaders_[eDebugFontPixel]);
        debugShaders_[eDebugVertexPosNormal]=hNEW(GetGlobalHeap(), hShaderProgram)(this, GetGlobalHeap(),
            hFUNCTOR_BINDMEMBER(hShaderProgram::hZeroProc, hRenderer, destroyShader, this));
        ParentClass::compileShaderFromSourceDevice(
            GetGlobalHeap(), ParentClass::getDebugShaderSource(eDebugVertexPosNormal),
            hStrLen(ParentClass::getDebugShaderSource(eDebugVertexPosNormal)),
            "mainVP", eShaderProfile_vs4_0, debugShaders_[eDebugVertexPosNormal]);
        debugShaders_[eDebugPixelWhiteViewLit]=hNEW(GetGlobalHeap(), hShaderProgram)(this, GetGlobalHeap(),
            hFUNCTOR_BINDMEMBER(hShaderProgram::hZeroProc, hRenderer, destroyShader, this));
        ParentClass::compileShaderFromSourceDevice(
            GetGlobalHeap(), ParentClass::getDebugShaderSource(eDebugPixelWhiteViewLit),
            hStrLen(ParentClass::getDebugShaderSource(eDebugPixelWhiteViewLit)),
            "mainFP", eShaderProfile_ps4_0, debugShaders_[eDebugPixelWhiteViewLit]);
        debugShaders_[eDebugTexVertex]=hNEW(GetGlobalHeap(), hShaderProgram)(this, GetGlobalHeap(),
            hFUNCTOR_BINDMEMBER(hShaderProgram::hZeroProc, hRenderer, destroyShader, this));
        ParentClass::compileShaderFromSourceDevice(
            GetGlobalHeap(), ParentClass::getDebugShaderSource(eDebugTexVertex),
            hStrLen(ParentClass::getDebugShaderSource(eDebugTexVertex)),
            "mainVP", eShaderProfile_vs4_0, debugShaders_[eDebugTexVertex]);
        debugShaders_[eDebugTexPixel]=hNEW(GetGlobalHeap(), hShaderProgram)(this, GetGlobalHeap(),
            hFUNCTOR_BINDMEMBER(hShaderProgram::hZeroProc, hRenderer, destroyShader, this));
        ParentClass::compileShaderFromSourceDevice(
            GetGlobalHeap(), ParentClass::getDebugShaderSource(eDebugTexPixel),
            hStrLen(ParentClass::getDebugShaderSource(eDebugTexPixel)),
            "mainFP", eShaderProfile_ps4_0, debugShaders_[eDebugTexPixel]);
    }

    void hRenderer::rendererFrameSubmit()
    {
        HEART_PROFILE_FUNC();
        CollectAndSortDrawCalls();
        SubmitDrawCallsMT();
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderer::createShaderResourceView(hTexture* tex, const hShaderResourceViewDesc& desc, hShaderResourceView** outsrv) {
        (*outsrv) = hNEW(GetGlobalHeap(), hShaderResourceView)(
            hFUNCTOR_BINDMEMBER(hShaderResourceView::hZeroRefProc, hRenderer, destroyShaderResourceView, this));
        ParentClass::createShaderResourseViewDevice(tex, desc, *outsrv);
        (*outsrv)->refType_=desc.resourceType_;
        (*outsrv)->refTex_=tex;
        tex->AddRef();
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderer::createShaderResourceView(hRenderBuffer* cb, const hShaderResourceViewDesc& desc, hShaderResourceView** outsrv) {
        (*outsrv) = hNEW(GetGlobalHeap(), hShaderResourceView)(
            hFUNCTOR_BINDMEMBER(hShaderResourceView::hZeroRefProc, hRenderer, destroyShaderResourceView, this));
        ParentClass::createShaderResourseViewDevice(cb, desc, *outsrv);
        (*outsrv)->refType_=desc.resourceType_;
        (*outsrv)->refCB_=cb;
        //cb->AddRef();
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderer::destroyShaderResourceView(hShaderResourceView* srv) {
        hcAssert(srv);
        hcAssert(srv->GetRefCount() == 0);
        ParentClass::destroyShaderResourceViewDevice(srv);
        hDELETE_SAFE(GetGlobalHeap(), srv);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderer::createRenderTargetView(hTexture* tex, const hRenderTargetViewDesc& rtvd, hRenderTargetView** outrtv) {
        (*outrtv) = hNEW(GetGlobalHeap(), hRenderTargetView)(
            hFUNCTOR_BINDMEMBER(hRenderTargetView::hZeroRefProc, hRenderer, destroyRenderTargetView, this));
        ParentClass::createRenderTargetViewDevice(tex, rtvd, *outrtv);
        (*outrtv)->bindTexture(tex);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderer::createDepthStencilView(hTexture* tex, const hDepthStencilViewDesc& dsvd, hDepthStencilView** outdsv) {
        (*outdsv) = hNEW(GetGlobalHeap(), hDepthStencilView)(
            hFUNCTOR_BINDMEMBER(hDepthStencilView::hZeroRefProc, hRenderer, destroyDepthStencilView, this));
        ParentClass::createDepthStencilViewDevice(tex, dsvd, *outdsv);
        (*outdsv)->bindTexture(tex);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderer::destroyRenderTargetView(hRenderTargetView* view) {
        hcAssert(view);
        hcAssert(view->GetRefCount() == 0);
        ParentClass::destroyRenderTargetViewDevice(view);
        hDELETE_SAFE(GetGlobalHeap(), view);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderer::destroyDepthStencilView(hDepthStencilView* view) {
        hcAssert(view);
        hcAssert(view->GetRefCount() == 0);
        ParentClass::destroyDepthStencilViewDevice(view);
        hDELETE_SAFE(GetGlobalHeap(), view);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hBlendState* hRenderer::createBlendState(const hBlendStateDesc& desc) {
        //Build state key
        hUint32 stateKey = hCRC32::FullCRC( (const hChar*)&desc, sizeof(desc) );
        resourceMutex_.Lock();
        hBlendState* state = blendStates_.Find(stateKey);
        if ( !state ) {
            state = hNEW(GetGlobalHeap(), hBlendState)(
                hFUNCTOR_BINDMEMBER(hBlendState::hZeroRefProc, hRenderer, destroyBlendState, this));
            ParentClass::createBlendStateDevice(desc, state);
            blendStates_.Insert(stateKey, state);
        }
        else {
            state->AddRef();
        }
        resourceMutex_.Unlock();
        return state;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderer::destroyBlendState(hBlendState* state) {
        hcAssert(state->GetRefCount() == 0);
        resourceMutex_.Lock();
        blendStates_.Remove(state->GetKey());
        ParentClass::destroyBlendStateDevice(state);
        hDELETE(GetGlobalHeap(), state);
        resourceMutex_.Unlock();
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hRasterizerState* hRenderer::createRasterizerState(const hRasterizerStateDesc& desc) {
        //Build state key
        hUint32 stateKey = hCRC32::FullCRC( (const hChar*)&desc, sizeof(desc) );
        resourceMutex_.Lock();
        hRasterizerState* state = rasterizerStates_.Find(stateKey);
        if ( !state ) {
            state = hNEW(GetGlobalHeap(), hRasterizerState)(
                hFUNCTOR_BINDMEMBER(hRasterizerState::hZeroRefProc, hRenderer, destoryRasterizerState, this));
            ParentClass::createRasterizerStateDevice(desc, state);
            rasterizerStates_.Insert(stateKey, state);
        }
        else {
            state->AddRef();
        }
        resourceMutex_.Unlock();
        return state;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderer::destoryRasterizerState(hRasterizerState* state) {
        hcAssert(state->GetRefCount() == 0);
        resourceMutex_.Lock();
        rasterizerStates_.Remove(state->GetKey());
        ParentClass::destroyRasterizerStateDevice(state);
        hDELETE(GetGlobalHeap(), state);
        resourceMutex_.Unlock();
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hDepthStencilState* hRenderer::createDepthStencilState(const hDepthStencilStateDesc& desc) {
        //Build state key
        hUint32 stateKey = hCRC32::FullCRC( (const hChar*)&desc, sizeof(desc) );
        resourceMutex_.Lock();
        hDepthStencilState* state = depthStencilStates_.Find(stateKey);
        if ( !state ) {
            state = hNEW(GetGlobalHeap(), hDepthStencilState)(
                hFUNCTOR_BINDMEMBER(hDepthStencilState::hZeroRefProc, hRenderer, destroyDepthStencilState, this));
            ParentClass::createDepthStencilStateDevice(desc, state);
            depthStencilStates_.Insert(stateKey, state);
        }
        else {
            state->AddRef();
        }
        resourceMutex_.Unlock();
        return state;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderer::destroyDepthStencilState(hDepthStencilState* state) {
        hcAssert(state->GetRefCount() == 0);
        resourceMutex_.Lock();
        depthStencilStates_.Remove(state->GetKey());
        ParentClass::destroyDepthStencilStateDevice(state);
        hDELETE(GetGlobalHeap(), state);
        resourceMutex_.Unlock();
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hSamplerState* hRenderer::createSamplerState(const hSamplerStateDesc& desc) {
        //Build state key
        hUint32 stateKey = hCRC32::FullCRC( (const hChar*)&desc, sizeof(desc) );
        resourceMutex_.Lock();
        hSamplerState* state = samplerStateMap_.Find(stateKey);
        if ( !state ) {
            state = hNEW(GetGlobalHeap(), hSamplerState)(
                hFUNCTOR_BINDMEMBER(hSamplerState::hZeroRefProc, hRenderer, destroySamplerState, this));
            ParentClass::createSamplerStateDevice(desc, state);
            samplerStateMap_.Insert(stateKey, state);
        }
        else {
            state->AddRef();
        }
        resourceMutex_.Unlock();
        return state;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderer::destroySamplerState(hSamplerState* state) {
        hcAssert(state->GetRefCount() == 0);
        resourceMutex_.Lock();
        samplerStateMap_.Remove(state->GetKey());
        ParentClass::destroySamplerStateDevice(state);
        hDELETE(GetGlobalHeap(), state);
        resourceMutex_.Unlock();
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderer::createBuffer(hUint size, void* data, hUint flags, hUint stride, hRenderBuffer** outcb) {
        (*outcb) = hNEW(GetGlobalHeap(), hRenderBuffer)(
            hFUNCTOR_BINDMEMBER(hRenderBuffer::hZeroRefProc, hRenderer, destroyConstantBlock, this));
        ParentClass::createBufferDevice(size, data, flags, stride, *outcb);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderer::destroyConstantBlock(hRenderBuffer* block) {
        ParentClass::destroyConstantBlockDevice(block);
        hDELETE(GetGlobalHeap(), block);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderer::compileShaderFromSource(hMemoryHeapBase* heap, const hChar* shaderProg, hUint32 len, 
    const hChar* entry, hShaderProfile profile, hShaderProgram** out) {
        (*out) = hNEW(heap, hShaderProgram)(this, heap,
            hFUNCTOR_BINDMEMBER(hShaderProgram::hZeroProc, hRenderer, destroyShader, this));
        ParentClass::compileShaderFromSourceDevice(heap, shaderProg, len, entry, profile, *out);
        if (profile >= eShaderProfile_vs4_0 && profile <= eShaderProfile_vs5_0) {
            (*out)->SetShaderType(ShaderType_VERTEXPROG);
        } else if (profile >= eShaderProfile_ps4_0 && profile <= eShaderProfile_ps5_0) {
            (*out)->SetShaderType(ShaderType_FRAGMENTPROG);
        } else if (profile >= eShaderProfile_gs4_0 && profile <= eShaderProfile_gs5_0) {
            (*out)->SetShaderType(ShaderType_GEOMETRYPROG);
        } else if (profile >= eShaderProfile_cs4_0 && profile <= eShaderProfile_cs5_0) {
            (*out)->SetShaderType(ShaderType_COMPUTEPROG);
        } else if (profile == eShaderProfile_hs5_0) {
            (*out)->SetShaderType(ShaderType_HULLPROG);
        } else if (profile == eShaderProfile_ds5_0) {
            (*out)->SetShaderType(ShaderType_DOMAINPROG);
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderer::createShader(hMemoryHeapBase* heap, const hChar* shaderProg, hUint32 len, hShaderType type, hShaderProgram** out) {
        (*out) = hNEW(heap, hShaderProgram)(this, heap,
            hFUNCTOR_BINDMEMBER(hShaderProgram::hZeroProc, hRenderer, destroyShader, this));
        ParentClass::compileShaderDevice(heap, shaderProg, len, type, *out);
        (*out)->SetShaderType(type);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderer::destroyShader(hShaderProgram* prog) {
        ParentClass::destroyShaderDevice(prog->heap_, prog);
        hDELETE(prog->heap_, prog);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hResourceClassBase* hRenderer::textureResourceLoader(hIFile* file, hResourceMemAlloc* memalloc) {
        Heart::hTexture* texutre;
        Heart::hMipDesc* mips = NULL;
        hUint32 totalTextureSize = 0;
        hByte* textureData = NULL;
        TextureHeader header = {0};

        file->Read(&header, sizeof(header));
        mips = (Heart::hMipDesc*)hAlloca(header.mipCount*sizeof(Heart::hMipDesc));
        //Read mip info
        file->Read(mips, header.mipCount*sizeof(Heart::hMipDesc));
        //Add up the size need for textures
        for (hUint32 i = 0; i < header.mipCount; ++i) {
            mips[i].data = (hByte*)totalTextureSize;
            totalTextureSize += mips[i].size;
        }
        textureData = (hByte*)hHeapMalloc(memalloc->resourcePakHeap_, totalTextureSize);
        for (hUint32 i = 0; i < header.mipCount; ++i) {
            mips[i].data = textureData + (hUint32)(mips[i].data);
        }
        //Read Texture data
        file->Read(textureData, totalTextureSize);
        createTexture(header.mipCount, mips, header.format, header.flags, memalloc->resourcePakHeap_, &texutre);
        return texutre;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hBool hRenderer::textureResourceLink(hResourceClassBase* texture, hResourceMemAlloc* memalloc) {
        return hTrue; //Nothing to do
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderer::textureResourceUnlink(hResourceClassBase* texture, hResourceMemAlloc* memalloc) {
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderer::textureResourceUnload(hResourceClassBase* resource, hResourceMemAlloc* memalloc) {
        hTexture* tex=static_cast<hTexture*>(resource);
        // Package should be the only thing hold ref at this point...
        hcAssertMsg(tex->GetRefCount() == 1, "Texture ref count is %u, it should be 1", tex->GetRefCount());
        tex->DecRef();
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hResourceClassBase* hRenderer::shaderResourceLoader(hIFile* file, hResourceMemAlloc* memalloc) {
        hShaderProgram* shaderProg=NULL;
        ShaderHeader header;
        hInputLayoutDesc* inLayout = NULL;
        void* shaderBlob = NULL;
        file->Read(&header, sizeof(ShaderHeader));
        hcAssert(header.version == SHADER_VERSION);
        if (header.inputLayoutElements) {
            inLayout = (hInputLayoutDesc*)hAlloca(sizeof(hInputLayoutDesc)*header.inputLayoutElements);
            file->Read(inLayout, sizeof(hInputLayoutDesc)*header.inputLayoutElements);
        }

        shaderBlob = hHeapMalloc(memalloc->tempHeap_, header.shaderBlobSize);
        file->Read(shaderBlob, header.shaderBlobSize);

        createShader(
            memalloc->resourcePakHeap_,
            (hChar*)shaderBlob, header.shaderBlobSize,
            header.type, &shaderProg);

        hHeapFreeSafe(memalloc->tempHeap_, shaderBlob);

        return shaderProg;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hBool hRenderer::shaderResourceLink(hResourceClassBase* resource, hResourceMemAlloc* memalloc) {
        return hTrue;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderer::shaderResourceUnlink(hResourceClassBase* resource, hResourceMemAlloc* memalloc) {

    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderer::shaderResourceUnload(hResourceClassBase* resource, hResourceMemAlloc* memalloc) {
        hShaderProgram* sp = static_cast<hShaderProgram*>(resource);
        sp->DecRef();
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hResourceClassBase* hRenderer::materialResourceLoader(hIFile* file, hResourceMemAlloc* memalloc) {
        hMaterial* material = hNEW(memalloc->resourcePakHeap_, hMaterial)(memalloc->resourcePakHeap_, this);
        MaterialHeader header;
        file->Read(&header, sizeof(header));
        //TODO: handle this...
        hcAssert(header.version == MATERIAL_VERSION);
        //Read samplers
        for (hUint32 i = 0, imax = header.samplerCount; i < imax; ++i)
        {
            SamplerDefinition samplerDef;
            hSamplerParameter sampler;
            file->Read(&samplerDef, sizeof(samplerDef));

            hStrCopy(sampler.name_, sampler.name_.GetMaxSize(), samplerDef.samplerName);
            sampler.samplerState_ = createSamplerState(samplerDef.samplerState);

            material->AddSamplerParameter(sampler);
        }
        //Read parameters
        for (hUint32 i = 0, imax = header.parameterCount; i < imax; ++i)
        {
            ParameterDefinition paramDef;
            file->Read(&paramDef, sizeof(paramDef));
            material->addDefaultParameterValue(paramDef);
        }
        //Add Groups, Techniques & Passes
        for (hUint32 groupidx = 0, groupCount = header.groupCount; groupidx < groupCount; ++groupidx)
        {
            GroupDefinition groupDef;
            hMaterialGroup* group = NULL;
            file->Read(&groupDef, sizeof(groupDef));

            group = material->AddGroup(groupDef.groupName);

            group->techniques_.Reserve(groupDef.techniques);
            group->techniques_.Resize(groupDef.techniques);
            for (hUint32 techniqueIdx = 0, techniqueCount = groupDef.techniques; techniqueIdx < techniqueCount; ++techniqueIdx)
            {
                TechniqueDefinition techDef;
                file->Read(&techDef, sizeof(techDef));
                hMaterialTechnique* tech = &group->techniques_[techniqueIdx];

                tech->SetName(techDef.technqiueName);
                tech->SetPasses(techDef.passes);
                tech->SetLayer(techDef.layer);
                tech->SetSort(techDef.transparent > 0);

                for (hUint32 passIdx = 0, passCount = techDef.passes; passIdx < passCount; ++passIdx)
                {
                    PassDefintion passDef;
                    hMaterialTechniquePass pass;

                    file->Read(&passDef, sizeof(passDef));

                    hBlendState* bs=createBlendState(passDef.blendState);
                    hDepthStencilState* ds=createDepthStencilState(passDef.depthState);
                    hRasterizerState* rs=createRasterizerState(passDef.rasterizerState);
                    pass.bindBlendState(bs);
                    pass.bindDepthStencilState(ds);
                    pass.bindRasterizerState(rs);
                    pass.SetVertexShaderResID(passDef.vertexProgramID);
                    pass.SetFragmentShaderResID(passDef.fragmentProgramID);
                    bs->DecRef();
                    ds->DecRef();
                    rs->DecRef();
                    tech->AppendPass(pass);
                }
            }
        }
        
        return material;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hBool hRenderer::materialResourceLink(hResourceClassBase* resource, hResourceMemAlloc* memalloc) {
        hMaterial* mat = static_cast< hMaterial* >(resource);
        return mat->Link(resourceManager_, this, &materialManager_);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderer::materialResourceUnlink(hResourceClassBase* resource, hResourceMemAlloc* memalloc) {
        hcAssert(resource);
        hMaterial* mat = static_cast<hMaterial*>(resource);
        mat->unbind();
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderer::materialResourceUnload(hResourceClassBase* resource, hResourceMemAlloc* memalloc) {
        hMaterial* mat = static_cast<hMaterial*>(resource);
        hDELETE(memalloc->resourcePakHeap_, mat);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hResourceClassBase* hRenderer::meshResourceLoader(hIFile* file, hResourceMemAlloc* memalloc) {
        MeshHeader header = {0};
        hRenderModel* rmodel = hNEW(memalloc->resourcePakHeap_, hRenderModel)();
        hInputLayoutDesc inputDesc[32];
        hInputLayoutDesc streamInputDesc[32];
        void* tmpBuffer = NULL;
        hUint32 tmpbufsize = 0;

        file->Read(&header, sizeof(header));

        rmodel->SetLODCount(header.lodCount);

        for (hUint32 lIdx = 0; lIdx < header.lodCount; ++lIdx)
        {
            LODHeader lodHeader;
            hGeomLODLevel* lod = rmodel->GetLOD(lIdx);
            file->Read(&lodHeader, sizeof(lodHeader));

            lod->minRange_ = lodHeader.minRange;
            lod->renderObjects_.Resize(lodHeader.renderableCount);

            for (hUint32 rIdx = 0; rIdx < lodHeader.renderableCount; ++rIdx)
            {
                RenderableHeader rHeader={0};
                hRenderable* renderable = &lod->renderObjects_[rIdx];
                hIndexBuffer* ib;
                hVertexBuffer* vb;
                hVec3 bounds[2];//min,max;
                hAABB aabb;

                file->Read(&rHeader, sizeof(rHeader));
                hcAssert(rHeader.inputElements < 32);
                file->Read(inputDesc, sizeof(hInputLayoutDesc)*rHeader.inputElements);

                tmpbufsize = hMax(tmpbufsize, rHeader.ibSize);
                tmpBuffer = hHeapRealloc(memalloc->tempHeap_, tmpBuffer, tmpbufsize);

                bounds[0] = hVec3(rHeader.boundsMin[0], rHeader.boundsMin[1], rHeader.boundsMin[2]);
                bounds[1] = hVec3(rHeader.boundsMax[0], rHeader.boundsMax[1], rHeader.boundsMax[2]);
                aabb = hAABB::computeFromPointSet(bounds, 2);

                file->Read(tmpBuffer, rHeader.ibSize);
                createIndexBuffer(tmpBuffer, rHeader.nPrimatives*3, 0, &ib);

                renderable->SetStartIndex(rHeader.startIndex);
                renderable->SetPrimativeCount(rHeader.nPrimatives);
                renderable->SetPrimativeType((PrimitiveType)rHeader.primType);
                renderable->SetAABB(aabb);
                renderable->SetIndexBuffer(ib);
                renderable->SetMaterialResourceID(rHeader.materialID);

                hUint32 streams = rHeader.streams;
                for (hUint32 streamIdx = 0; streamIdx < streams; ++streamIdx)
                {
                    StreamHeader sHeader = {0};
                    file->Read(&sHeader, sizeof(sHeader));

                    tmpbufsize = hMax(tmpbufsize, sHeader.size);
                    tmpBuffer = hHeapRealloc(memalloc->tempHeap_, tmpBuffer, tmpbufsize);

                    file->Read(tmpBuffer, sHeader.size);

                    //Builder stream inputDesc
                    hUint32 sidc = rHeader.inputElements;
                    hUint32 side = 0;
                    for (hUint32 sid = 0; sid < sidc; ++sid)
                    {
                        if (inputDesc[sid].inputStream_ == sHeader.index)
                        {
                            streamInputDesc[side] = inputDesc[sid];
                            ++side;
                        }
                    }

                    createVertexBuffer(tmpBuffer, rHeader.verts, streamInputDesc, side, 0, memalloc->resourcePakHeap_, &vb);
                    renderable->SetVertexBuffer(sHeader.index, vb);
                }
            }
        }

        hHeapFreeSafe(memalloc->tempHeap_, tmpBuffer);
        return rmodel;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hBool hRenderer::meshResourceLink(hResourceClassBase* resource, hResourceMemAlloc* memalloc) {
        hRenderModel* rmodel = static_cast< hRenderModel* >(resource);
        hUint32 lodcount = rmodel->GetLODCount();
        for(hUint32 i = 0; i < lodcount; ++i) {
            hGeomLODLevel* lod = rmodel->GetLOD(i);
            hUint32 objcount = lod->renderObjects_.GetSize();
            for (hUint32 j = 0; j < objcount; ++j) {
                if (lod->renderObjects_[j].GetMaterial() == 0) {
                    hMaterial* mat = static_cast<hMaterial*>(resourceManager_->ltGetResource(lod->renderObjects_[j].GetMaterialResourceID()));
                    // Possible the material won't have loaded just yet...
                    if (!mat) {
                        return hFalse;
                    }
                    lod->renderObjects_[j].SetMaterial(mat);
                    lod->renderObjects_[j].bind();
                }
            }
        }

        rmodel->initialiseRenderCommands();
        return hTrue;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderer::meshResourceUnlink(hResourceClassBase* resource, hResourceMemAlloc* memalloc) {
        hRenderModel* rmodel = static_cast< hRenderModel* >(resource);
        for (hUint32 lIdx = 0, lodc = rmodel->GetLODCount(); lIdx < lodc; ++lIdx) {
            hGeomLODLevel* lod = rmodel->GetLOD(lIdx);
            for (hUint32 rIdx = 0, rCnt = lod->renderObjects_.GetSize(); rIdx < rCnt; ++rIdx) {
                lod->renderObjects_[rIdx].SetMaterial(hNullptr);
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderer::meshResourceUnload(hResourceClassBase* resource, hResourceMemAlloc* memalloc) {
        hRenderModel* rmodel = static_cast< hRenderModel* >(resource);
        for (hUint32 lIdx = 0, lodc = rmodel->GetLODCount(); lIdx < lodc; ++lIdx) {
            hGeomLODLevel* lod = rmodel->GetLOD(lIdx);
            for (hUint32 rIdx = 0, rCnt = lod->renderObjects_.GetSize(); rIdx < rCnt; ++rIdx) {
                for (hUint32 s = 0, sc=lod->renderObjects_[rIdx].getVertexBufferCount(); s < sc; ++s) {
                    hVertexBuffer* vb=lod->renderObjects_[rIdx].GetVertexBuffer(s);
                    if (vb) {
                        vb->DecRef();
                        vb=NULL;
                    }
                }
                lod->renderObjects_[rIdx].GetIndexBuffer()->DecRef();
            }
        }
        hDELETE_SAFE(memalloc->resourcePakHeap_, resource);
    }


    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderCommands::reserveSpace(hUint size) {
        if (allocatedSize_ < size) {
            cmds_=(hRCmd*)hRealloc(cmds_, hAlign(size, 1024));
            allocatedSize_=hAlign(size, 1024);
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderCommands::insertCommand(hUint where, const hRCmd* command, hBool overwrite) {
        hcAssert(command);
        hcAssert(where <= cmdSize_);
        if (where+command->size_ >= cmdSize_) {
            reserveSpace(cmdSize_+command->size_);
        }
        hRCmd* cmd=(hRCmd*)((hByte*)cmds_+where);
        hRCmd* nextcmd=(hRCmd*)((hByte*)cmds_+where+cmd->size_);
        hUint remainingBytes=cmdSize_-where;
        if (overwrite) {
            hcAssert(cmd->opCode_ < eRenderCmd_End);
            hByte oldcmdsize=cmd->size_;
            if (oldcmdsize < command->size_) {
                hMemMove(((hByte*)cmd+command->size_), nextcmd, remainingBytes);
                cmdSize_+=command->size_-oldcmdsize;
            }
            hMemSet(cmd, 0xAC, cmd->size_);
            hMemCpy(cmd, command, command->size_);
            if (oldcmdsize > command->size_) {
                cmd->size_=oldcmdsize;
            }
        } else {
            if (remainingBytes > 0) {
                hMemMove(nextcmd, cmd, remainingBytes);
            }
            hMemCpy(cmd, command, command->size_);
            cmdSize_+=command->size_;
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hRenderCommandGenerator::hRenderCommandGenerator() 
        : renderCommands_(hNullptr)
    {

    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hRenderCommandGenerator::hRenderCommandGenerator(hRenderCommands* rcmds)
        : renderCommands_(rcmds)
    {
        hcAssert(renderCommands_);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hRCmd* hRenderCommandGenerator::getCmdBufferStart() {
        return renderCommands_ ? renderCommands_->cmds_ : hNullptr;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hUint hRenderCommandGenerator::appendCmd(const hRCmd* cmd) {
        hcAssert(renderCommands_);
        hUint cmdstart=renderCommands_->cmdSize_;
        renderCommands_->insertCommand(renderCommands_->cmdSize_, cmd, false);
        return cmdstart;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hUint hRenderCommandGenerator::overwriteCmd(const hRCmd* oldcmd, const hRCmd* newcmd) {
        hcAssert(renderCommands_);
        hUint sizediff=oldcmd->size_ < newcmd->size_ ? newcmd->size_-oldcmd->size_ : 0;
        hUint cmdstart=(hUint)((ptrdiff_t)oldcmd-(ptrdiff_t)renderCommands_->cmds_);
        renderCommands_->insertCommand(cmdstart, newcmd, true);
        return sizediff;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderCommandGenerator::reset() {
        hcAssert(renderCommands_);
        renderCommands_->cmdSize_=0;
#ifdef HEART_DEBUG
        hMemSet(renderCommands_->cmds_, 0xDC, renderCommands_->allocatedSize_);
#endif
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hUint hRenderCommandGenerator::resetCommands() {
        reset();
        return 0;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hUint hRenderCommandGenerator::setJump(hRCmd* cmd) {
        hcAssert(renderCommands_);
        hRCmdJump jumpcmd(cmd);
        return appendCmd(&jumpcmd);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hUint hRenderCommandGenerator::setReturn() {
        hcAssert(renderCommands_);
        hRCmdReturn retcmd;
        return appendCmd(&retcmd);
    }
    
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hUint hRenderCommandGenerator::setNoOp() {
        hcAssert(renderCommands_);
        hRCmdNOOP cmd;
        return appendCmd(&cmd);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hUint hRenderCommandGenerator::setDraw(hUint nPrimatives, hUint startVertex) {
        hcAssert(renderCommands_);
        hRCmdDraw cmd(nPrimatives, startVertex);
        return appendCmd(&cmd);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hUint hRenderCommandGenerator::setDrawIndex(hUint nPrimatives, hUint startVertex) {
        hcAssert(renderCommands_);
        hRCmdDrawIndex cmd(nPrimatives, startVertex);
        return appendCmd(&cmd);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hUint hRenderCommandGenerator::setDrawInstance(hUint nPrimatives, hUint startVertex, hUint instancecount) {
        hcAssert(renderCommands_);
        hRCmdDrawInstanced cmd(nPrimatives, startVertex, instancecount);
        return appendCmd(&cmd);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hUint hRenderCommandGenerator::setDrawInstanceIndex(hUint nPrimatives, hUint startVertex, hUint instancecount) {
        hcAssert(renderCommands_);
        hRCmdDrawInstancedIndex cmd(nPrimatives, startVertex, instancecount);
        return appendCmd(&cmd);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hUint hRenderCommandGenerator::setRenderStates(hBlendState* bs, hRasterizerState* rs, hDepthStencilState* dss) {
        hcAssert(renderCommands_);
        return hdRenderCommandGenerator::setRenderStates(bs, rs, dss);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hUint hRenderCommandGenerator::setShader(hShaderProgram* shader, hShaderType type) {
        hcAssert(renderCommands_);
        return hdRenderCommandGenerator::setShader(shader, type);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hUint hRenderCommandGenerator::setVertexInputs(hSamplerState** samplers, hUint nsamplers, hShaderResourceView** srv, hUint nsrv, hRenderBuffer** cb, hUint ncb) {
        hcAssert(renderCommands_);
        hdSamplerState** dsamplers      =(hdSamplerState**)hAlloca(sizeof(hdSamplerState*)*nsamplers);
        hdShaderResourceView** dsrv     =(hdShaderResourceView**)hAlloca(sizeof(hdShaderResourceView*)*nsrv);
        hdRenderBuffer** dcb  =(hdRenderBuffer**)hAlloca(sizeof(hdRenderBuffer*)*ncb);
        for (hUint i=0, n=nsamplers; i<n; ++i) {
            dsamplers[i]=samplers[i];
        }
        for (hUint i=0, n=nsrv; i<n; ++i) {
            dsrv[i]=srv[i];
        }
        for (hUint i=0, n=ncb; i<n; ++i) {
            dcb[i]=cb[i];
        }
        return hdRenderCommandGenerator::setVertexInputs(dsamplers, nsamplers, dsrv, nsrv, dcb, ncb);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hUint hRenderCommandGenerator::updateVertexInputs(hRCmd* cmd, hSamplerState** samplers, hUint nsamplers, hShaderResourceView** srv, hUint nsrv, hRenderBuffer** cb, hUint ncb) {
        hdSamplerState** dsamplers      =(hdSamplerState**)hAlloca(sizeof(hdSamplerState*)*nsamplers);
        hdShaderResourceView** dsrv     =(hdShaderResourceView**)hAlloca(sizeof(hdShaderResourceView*)*nsrv);
        hdRenderBuffer** dcb  =(hdRenderBuffer**)hAlloca(sizeof(hdRenderBuffer*)*ncb);
        for (hUint i=0, n=nsamplers; i<n; ++i) {
            dsamplers[i]=samplers[i];
        }
        for (hUint i=0, n=nsrv; i<n; ++i) {
            dsrv[i]=srv[i];
        }
        for (hUint i=0, n=ncb; i<n; ++i) {
            dcb[i]=cb[i];
        }
        return hdRenderCommandGenerator::updateVertexInputs(cmd, dsamplers, nsamplers, dsrv, nsrv, dcb, ncb);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hUint hRenderCommandGenerator::setPixelInputs(hSamplerState** samplers, hUint nsamplers, hShaderResourceView** srv, hUint nsrv, hRenderBuffer** cb, hUint ncb) {
        hcAssert(renderCommands_);
        hdSamplerState** dsamplers      =(hdSamplerState**)hAlloca(sizeof(hdSamplerState*)*nsamplers);
        hdShaderResourceView** dsrv     =(hdShaderResourceView**)hAlloca(sizeof(hdShaderResourceView*)*nsrv);
        hdRenderBuffer** dcb  =(hdRenderBuffer**)hAlloca(sizeof(hdRenderBuffer*)*ncb);
        for (hUint i=0, n=nsamplers; i<n; ++i) {
            dsamplers[i]=samplers[i];
        }
        for (hUint i=0, n=nsrv; i<n; ++i) {
            dsrv[i]=srv[i];
        }
        for (hUint i=0, n=ncb; i<n; ++i) {
            dcb[i]=cb[i];
        }
        return hdRenderCommandGenerator::setPixelInputs(dsamplers, nsamplers, dsrv, nsrv, dcb, ncb);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hUint hRenderCommandGenerator::updatePixelInputs(hRCmd* cmd, hSamplerState** samplers, hUint nsamplers, hShaderResourceView** srv, hUint nsrv, hRenderBuffer** cb, hUint ncb) {
        hcAssert(renderCommands_);
        hdSamplerState** dsamplers      =(hdSamplerState**)hAlloca(sizeof(hdSamplerState*)*nsamplers);
        hdShaderResourceView** dsrv     =(hdShaderResourceView**)hAlloca(sizeof(hdShaderResourceView*)*nsrv);
        hdRenderBuffer** dcb  =(hdRenderBuffer**)hAlloca(sizeof(hdRenderBuffer*)*ncb);
        for (hUint i=0, n=nsamplers; i<n; ++i) {
            dsamplers[i]=samplers[i];
        }
        for (hUint i=0, n=nsrv; i<n; ++i) {
            dsrv[i]=srv[i];
        }
        for (hUint i=0, n=ncb; i<n; ++i) {
            dcb[i]=cb[i];
        }
        return hdRenderCommandGenerator::updatePixelInputs(cmd, dsamplers, nsamplers, dsrv, nsrv, dcb, ncb);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hUint hRenderCommandGenerator::setGeometryInputs(hdDX11SamplerState** samplers, hUint nsamplers, hShaderResourceView** srv, hUint nsrv, hRenderBuffer** cb, hUint ncb) {
        hcAssert(renderCommands_);
        hdSamplerState** dsamplers      =(hdSamplerState**)hAlloca(sizeof(hdSamplerState*)*nsamplers);
        hdShaderResourceView** dsrv     =(hdShaderResourceView**)hAlloca(sizeof(hdShaderResourceView*)*nsrv);
        hdRenderBuffer** dcb  =(hdRenderBuffer**)hAlloca(sizeof(hdRenderBuffer*)*ncb);
        for (hUint i=0, n=nsamplers; i<n; ++i) {
            dsamplers[i]=samplers[i];
        }
        for (hUint i=0, n=nsrv; i<n; ++i) {
            dsrv[i]=srv[i];
        }
        for (hUint i=0, n=ncb; i<n; ++i) {
            dcb[i]=cb[i];
        }
        return hdRenderCommandGenerator::setGeometryInputs(dsamplers, nsamplers, dsrv, nsrv, dcb, ncb);
    }
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hUint hRenderCommandGenerator::setHullInputs(hdDX11SamplerState** samplers, hUint nsamplers, hShaderResourceView** srv, hUint nsrv, hRenderBuffer** cb, hUint ncb) {
        hcAssert(renderCommands_);
        hdSamplerState** dsamplers      =(hdSamplerState**)hAlloca(sizeof(hdSamplerState*)*nsamplers);
        hdShaderResourceView** dsrv     =(hdShaderResourceView**)hAlloca(sizeof(hdShaderResourceView*)*nsrv);
        hdRenderBuffer** dcb  =(hdRenderBuffer**)hAlloca(sizeof(hdRenderBuffer*)*ncb);
        for (hUint i=0, n=nsamplers; i<n; ++i) {
            dsamplers[i]=samplers[i];
        }
        for (hUint i=0, n=nsrv; i<n; ++i) {
            dsrv[i]=srv[i];
        }
        for (hUint i=0, n=ncb; i<n; ++i) {
            dcb[i]=cb[i];
        }
        return hdRenderCommandGenerator::setHullInputs(dsamplers, nsamplers, dsrv, nsrv, dcb, ncb);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hUint hRenderCommandGenerator::setDomainInputs(hdDX11SamplerState** samplers, hUint nsamplers, hShaderResourceView** srv, hUint nsrv, hRenderBuffer** cb, hUint ncb) {
        hcAssert(renderCommands_);
        hdSamplerState** dsamplers      =(hdSamplerState**)hAlloca(sizeof(hdSamplerState*)*nsamplers);
        hdShaderResourceView** dsrv     =(hdShaderResourceView**)hAlloca(sizeof(hdShaderResourceView*)*nsrv);
        hdRenderBuffer** dcb  =(hdRenderBuffer**)hAlloca(sizeof(hdRenderBuffer*)*ncb);
        for (hUint i=0, n=nsamplers; i<n; ++i) {
            dsamplers[i]=samplers[i];
        }
        for (hUint i=0, n=nsrv; i<n; ++i) {
            dsrv[i]=srv[i];
        }
        for (hUint i=0, n=ncb; i<n; ++i) {
            dcb[i]=cb[i];
        }
        return hdRenderCommandGenerator::setDomainInputs(dsamplers, nsamplers, dsrv, nsrv, dcb, ncb);
    }
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hUint hRenderCommandGenerator::setStreamInputs(PrimitiveType primType, hIndexBuffer* index, hIndexBufferType format, hdInputLayout* vertexlayout, hVertexBuffer** vtx, hUint firstStream, hUint streamCount) {
        hcAssert(renderCommands_);
        hdVtxBuffer** dvtx=(hdVtxBuffer**)hAlloca(sizeof(hdVtxBuffer*)*streamCount);
        for (hUint i=0, n=streamCount; i<n; ++i) {
            dvtx[i]=vtx[i];
        }
        return hdRenderCommandGenerator::setStreamInputs(primType, index, format, vertexlayout, dvtx, firstStream, streamCount);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hUint hRenderCommandGenerator::updateShaderInputBuffer(hRCmd* cmd, hUint reg, hRenderBuffer* cb) {
        hcAssert(cmd >= renderCommands_->cmds_ && (hByte*)cmd < (hByte*)renderCommands_->cmds_+renderCommands_->cmdSize_);
        hdRenderCommandGenerator::updateShaderInputBuffer(cmd, reg, cb);
        return 0;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hUint hRenderCommandGenerator::updateShaderInputSampler(hRCmd* cmd, hUint reg, hSamplerState* ss) {
        hcAssert(cmd >= renderCommands_->cmds_ && (hByte*)cmd < (hByte*)renderCommands_->cmds_+renderCommands_->cmdSize_);
        hdRenderCommandGenerator::updateShaderInputSampler(cmd, reg, ss);
        return 0;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hUint hRenderCommandGenerator::updateShaderInputView(hRCmd* cmd, hUint reg, hShaderResourceView* srv) {
        hcAssert(cmd >= renderCommands_->cmds_ && (hByte*)cmd < (hByte*)renderCommands_->cmds_+renderCommands_->cmdSize_);
        hdRenderCommandGenerator::updateShaderInputView(cmd, reg, srv);
        return 0;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hUint hRenderCommandGenerator::updateStreamInputs(hRCmd* cmd, PrimitiveType primType, hIndexBuffer* index, hIndexBufferType format, hdInputLayout* vertexlayout, hVertexBuffer** vtx, hUint firstStream, hUint streamCount) {
        hcAssert(cmd >= renderCommands_->cmds_ && (hByte*)cmd < (hByte*)renderCommands_->cmds_+renderCommands_->cmdSize_);
        hdVtxBuffer** dvtx=(hdVtxBuffer**)hAlloca(sizeof(hdVtxBuffer*)*streamCount);
        for (hUint i=0, n=streamCount; i<n; ++i) {
            dvtx[i]=vtx[i];
        }
        return hdRenderCommandGenerator::updateStreamInputs(cmd, primType, index, format, vertexlayout, dvtx, firstStream, streamCount);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hLightingManager::initialise(hRenderer* renderer, const hRenderTargetInfo* rndrinfo) {
        destroy();

        hRenderCommandGenerator rcGen(&renderCmds_);
        targetInfo_=*rndrinfo;
        lightInfo_.directionalLightCount_=0;
        renderer->createBuffer(sizeof(hInputLightData), hNullptr, eResourceFlag_ConstantBuffer, 0, &inputLightData_);
        renderer->createBuffer(sizeof(hDirectionalLight)*s_maxDirectionalLights, hNullptr, eResourceFlag_ShaderResource | eResourceFlag_StructuredBuffer, sizeof(hDirectionalLight), &directionLightData_);
        hRenderUtility::buildTessellatedQuadMesh(1.f, 1.f, 10, 10, renderer, GetGlobalHeap(), &screenQuadIB_, &screenQuadVB_);
        hBlendStateDesc blendstatedesc;
        hZeroMem(&blendstatedesc, sizeof(blendstatedesc));
        blendstatedesc.blendEnable_           = RSV_DISABLE;
        blendstatedesc.srcBlend_              = RSV_BLEND_OP_ONE;
        blendstatedesc.destBlend_             = RSV_BLEND_OP_ZERO;
        blendstatedesc.blendOp_               = RSV_BLEND_FUNC_ADD;
        blendstatedesc.srcBlendAlpha_         = RSV_BLEND_OP_ONE;
        blendstatedesc.destBlendAlpha_        = RSV_BLEND_OP_ZERO;
        blendstatedesc.blendOpAlpha_          = RSV_BLEND_FUNC_ADD;
        blendstatedesc.renderTargetWriteMask_ = RSV_COLOUR_WRITE_FULL;
        blendState_=renderer->createBlendState(blendstatedesc);
        hRasterizerStateDesc rasterstatedesc;
        hZeroMem(&rasterstatedesc, sizeof(rasterstatedesc));
        rasterstatedesc.fillMode_              = RSV_FILL_MODE_SOLID;
        rasterstatedesc.cullMode_              = RSV_CULL_MODE_NONE;
        rasterstatedesc.frontCounterClockwise_ = RSV_DISABLE;
        rasterstatedesc.depthBias_             = 0;
        rasterstatedesc.depthBiasClamp_        = 0.f;
        rasterstatedesc.slopeScaledDepthBias_  = 0.f;
        rasterstatedesc.depthClipEnable_       = RSV_ENABLE;
        rasterstatedesc.scissorEnable_         = RSV_DISABLE;
        rasterState_=renderer->createRasterizerState(rasterstatedesc);
        hDepthStencilStateDesc depthstatedesc;
        hZeroMem(&depthstatedesc, sizeof(depthstatedesc));
        depthstatedesc.depthEnable_        = RSV_DISABLE;
        depthstatedesc.depthWriteMask_     = RSV_DISABLE;
        depthstatedesc.depthFunc_          = RSV_Z_CMP_LESS;
        depthstatedesc.stencilEnable_      = RSV_DISABLE;
        depthstatedesc.stencilReadMask_    = ~0U;
        depthstatedesc.stencilWriteMask_   = ~0U;
        depthstatedesc.stencilFailOp_      = RSV_SO_KEEP;
        depthstatedesc.stencilDepthFailOp_ = RSV_SO_KEEP;
        depthstatedesc.stencilPassOp_      = RSV_SO_KEEP;
        depthstatedesc.stencilFunc_        = RSV_SF_CMP_ALWAYS;
        depthstatedesc.stencilRef_         = 0;  
        depthStencilState_=renderer->createDepthStencilState(depthstatedesc);
        hSamplerStateDesc samplerstatedesc;
        samplerstatedesc.filter_        = SSV_POINT;
        samplerstatedesc.addressU_      = SSV_CLAMP;
        samplerstatedesc.addressV_      = SSV_CLAMP;
        samplerstatedesc.addressW_      = SSV_CLAMP;
        samplerstatedesc.mipLODBias_    = 0;
        samplerstatedesc.maxAnisotropy_ = 16;
        samplerstatedesc.borderColour_  = WHITE;
        samplerstatedesc.minLOD_        = -FLT_MAX;
        samplerstatedesc.maxLOD_        = FLT_MAX;
        samplerState_=renderer->createSamplerState(samplerstatedesc);
        inputLayout_=targetInfo_.vertexLightShader_->createVertexLayout(screenQuadVB_->getLayoutDesc(), screenQuadVB_->getDescCount());

        
        for (hUint i=0, n=targetInfo_.pixelLightShader_->getInputCount(); i<n; ++i) {
            hShaderInput shaderInput;
            targetInfo_.pixelLightShader_->getInput(i, &shaderInput);
            if (hStrCmp(shaderInput.name_, "gbuffer_albedo")==0 && shaderInput.type_==eShaderInputType_Resource) {
                hShaderResourceViewDesc srvdesc;
                hShaderResourceView* srv;
                hZeroMem(&srvdesc, sizeof(srvdesc));
                srvdesc.resourceType_=eRenderResourceType_Tex2D;
                srvdesc.format_=targetInfo_.albedo_->getTextureFormat();
                srvdesc.tex2D_.mipLevels_=~0;
                srvdesc.tex2D_.topMip_=0;
                renderer->createShaderResourceView(targetInfo_.albedo_, srvdesc, &srv);
                if (srv_.GetSize() < shaderInput.bindPoint_+1) {
                    srv_.Resize(shaderInput.bindPoint_+1);
                }
                srv_[shaderInput.bindPoint_]=srv;
            } else if (hStrCmp(shaderInput.name_, "gbuffer_normal")==0 && shaderInput.type_==eShaderInputType_Resource) {
                hShaderResourceViewDesc srvdesc;
                hShaderResourceView* srv;
                hZeroMem(&srvdesc, sizeof(srvdesc));
                srvdesc.resourceType_=eRenderResourceType_Tex2D;
                srvdesc.format_=targetInfo_.normal_->getTextureFormat();
                srvdesc.tex2D_.mipLevels_=~0;
                srvdesc.tex2D_.topMip_=0;
                renderer->createShaderResourceView(targetInfo_.normal_, srvdesc, &srv);
                if (srv_.GetSize() < shaderInput.bindPoint_+1) {
                    srv_.Resize(shaderInput.bindPoint_+1);
                }
                srv_[shaderInput.bindPoint_]=srv;
            } else if (hStrCmp(shaderInput.name_, "gbuffer_spec")==0 && shaderInput.type_==eShaderInputType_Resource) {
                    hShaderResourceViewDesc srvdesc;
                    hShaderResourceView* srv;
                    hZeroMem(&srvdesc, sizeof(srvdesc));
                    srvdesc.resourceType_=eRenderResourceType_Tex2D;
                    srvdesc.format_=targetInfo_.spec_->getTextureFormat();
                    srvdesc.tex2D_.mipLevels_=~0;
                    srvdesc.tex2D_.topMip_=0;
                    renderer->createShaderResourceView(targetInfo_.spec_, srvdesc, &srv);
                    if (srv_.GetSize() < shaderInput.bindPoint_+1) {
                        srv_.Resize(shaderInput.bindPoint_+1);
                    }
                    srv_[shaderInput.bindPoint_]=srv;
            } else if (hStrCmp(shaderInput.name_, "gbuffer_depth")==0 && shaderInput.type_==eShaderInputType_Resource) {
                hShaderResourceViewDesc srvdesc;
                hShaderResourceView* srv;
                hZeroMem(&srvdesc, sizeof(srvdesc));
                srvdesc.resourceType_=eRenderResourceType_Tex2D;
                srvdesc.format_=eTextureFormat_R32_float;
                srvdesc.tex2D_.mipLevels_=~0;
                srvdesc.tex2D_.topMip_=0;
                renderer->createShaderResourceView(targetInfo_.depth_, srvdesc, &srv);
                if (srv_.GetSize() < shaderInput.bindPoint_+1) {
                    srv_.Resize(shaderInput.bindPoint_+1);
                }
                srv_[shaderInput.bindPoint_]=srv;
            } else if (hStrCmp(shaderInput.name_, "tex_sampler")==0 && shaderInput.type_==eShaderInputType_Sampler) {
                samplerBindPoint_=shaderInput.bindPoint_;
            } else if (hStrCmp(shaderInput.name_, "lighting_setup")==0 && shaderInput.type_==eShaderInputType_Buffer) {
                if (buffers_.GetSize() < shaderInput.bindPoint_+1) {
                    buffers_.Resize(shaderInput.bindPoint_+1);
                }
                buffers_[shaderInput.bindPoint_]=inputLightData_;
            } else if (hStrCmp(shaderInput.name_, "direction_lighting")==0 && shaderInput.type_==eShaderInputType_Resource) {
                hShaderResourceViewDesc srvdesc;
                hShaderResourceView* srv;
                hZeroMem(&srvdesc, sizeof(srvdesc));
                srvdesc.resourceType_=eRenderResourceType_Buffer;
                srvdesc.format_=eTextureFormat_Unknown;
                srvdesc.buffer_.firstElement_=0;
                srvdesc.buffer_.elementOffset_=0;
                srvdesc.buffer_.elementWidth_=sizeof(hDirectionalLight);
                srvdesc.buffer_.numElements_=s_maxDirectionalLights;
                renderer->createShaderResourceView(directionLightData_, srvdesc, &srv);
                if (srv_.GetSize() < shaderInput.bindPoint_+1) {
                    srv_.Resize(shaderInput.bindPoint_+1);
                }
                srv_[shaderInput.bindPoint_]=srv;
            }
        }
        rcGen.setRenderStates(blendState_, rasterState_, depthStencilState_);
        rcGen.setShader(targetInfo_.vertexLightShader_, ShaderType_VERTEXPROG);
        rcGen.setShader(targetInfo_.pixelLightShader_, ShaderType_FRAGMENTPROG);
        rcGen.setPixelInputs(&samplerState_, 1, srv_.GetBuffer(), srv_.GetSize(), buffers_.GetBuffer(), buffers_.GetSize());
        rcGen.setStreamInputs(PRIMITIVETYPE_TRILIST, screenQuadIB_, screenQuadIB_->getIndexBufferType(), inputLayout_, &screenQuadVB_, 0, 1);
        rcGen.setDrawIndex(screenQuadIB_->GetIndexCount()/3, 0);
        rcGen.setReturn();
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hLightingManager::destroy() {
        for (hUint i=0, n=srv_.GetSize(); i<n; ++i) {
            if (srv_[i]) {
                srv_[i]->DecRef();
                srv_[i]=hNullptr;
            }
        }
        srv_.Resize(0);
        buffers_.Resize(0);
        if (blendState_) {
            blendState_->DecRef();
            blendState_=hNullptr;
        }
        if (rasterState_) {
            rasterState_->DecRef();
            rasterState_=hNullptr;
        }
        if (depthStencilState_) {
            depthStencilState_->DecRef();
            depthStencilState_=hNullptr;
        }
        if (samplerState_) {
            samplerState_->DecRef();
            samplerState_=hNullptr;
        }
        if (inputLayout_) {
            targetInfo_.vertexLightShader_->destroyVertexLayout(inputLayout_);
            inputLayout_=hNullptr;
        }
        if (inputLightData_) {
            inputLightData_->DecRef();
            inputLightData_=hNullptr;
        }
        if (directionLightData_) {
            directionLightData_->DecRef();
            directionLightData_=hNullptr;
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hLightingManager::addDirectionalLight(const hVec3& direction, const hColour& colour) {
        directionalLights_[lightInfo_.directionalLightCount_].direction_=direction;
        directionalLights_[lightInfo_.directionalLightCount_].colour_=colour;
        ++lightInfo_.directionalLightCount_;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hLightingManager::doDeferredLightPass(hRenderer* renderer, hRenderSubmissionCtx* ctx) {
        (void)renderer;
        hRenderBufferMapInfo mapinfo;
        hRendererCamera* viewcam=renderer->GetRenderCamera(targetInfo_.viewCameraIndex_);
        hMatrix view=viewcam->GetViewMatrix();
        hMatrix invView=hMatrixFunc::inverse(view);
        hMatrix project=viewcam->GetProjectionMatrix();
        hMatrix invProject=hMatrixFunc::inverse(project);

        ctx->Map(inputLightData_, &mapinfo); {
            hInputLightData* mapptr=(hInputLightData*)mapinfo.ptr;
            mapptr->viewMatrix_=view;
            mapptr->inverseViewMtx_=invView;
            mapptr->projectionMtx_=project;
            mapptr->inverseProjectMtx_=invProject;
            mapptr->eyePos_=hMatrixFunc::getRow(invView, 3);
            mapptr->directionalLightCount_ = lightInfo_.directionalLightCount_;
            ctx->Unmap(&mapinfo);
        }
        ctx->Map(directionLightData_, &mapinfo); {
            hDirectionalLight* mapptr=(hDirectionalLight*)mapinfo.ptr;
            for (hUint i=0; i<lightInfo_.directionalLightCount_; ++i) {
                mapptr[i]=directionalLights_[i];
            }
            ctx->Unmap(&mapinfo);
        }
        ctx->runRenderCommands(renderCmds_.getFirst());
    }

}