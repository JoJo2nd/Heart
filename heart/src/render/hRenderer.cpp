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
        return hMalloc(size);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    
    void RnTmpFree( void* ptr )
    {
        hFree(ptr);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hRenderer::hRenderer()
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
        frameTimer_.reset();
        materialManager_.setRenderer(this);
        materialManager_.setResourceManager(pResourceManager);

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
        texreshandler.loadProc_     =hFUNCTOR_BINDMEMBER(hResourceLoadProc, hRenderer,      textureResourceLoader,     this);
        texreshandler.postLoadProc_ =hFUNCTOR_BINDMEMBER(hResourcePostLoadProc, hRenderer,  textureResourcePostLoad,   this);
        texreshandler.preUnloadProc_=hFUNCTOR_BINDMEMBER(hResourcePreUnloadProc, hRenderer, textureResourcePreUnload,  this);
        texreshandler.unloadProc_   =hFUNCTOR_BINDMEMBER(hResourceUnloadProc, hRenderer,    textureResourceUnload,     this);
        resourceManager_->registerResourceHandler("texture", texreshandler);
        
        hResourceHandler shaderreshandler;
        shaderreshandler.loadProc_      =hFUNCTOR_BINDMEMBER(hResourceLoadProc, hRenderer,      shaderResourceLoader,    this);
        shaderreshandler.postLoadProc_  =hFUNCTOR_BINDMEMBER(hResourcePostLoadProc, hRenderer,  shaderResourcePostLoad,  this);
        shaderreshandler.preUnloadProc_ =hFUNCTOR_BINDMEMBER(hResourcePreUnloadProc, hRenderer, shaderResourcePreUnload, this);
        shaderreshandler.unloadProc_    =hFUNCTOR_BINDMEMBER(hResourceUnloadProc, hRenderer,    shaderResourceUnload,    this);
        resourceManager_->registerResourceHandler("gpu", shaderreshandler);
        
        hResourceHandler matreshandler;
        matreshandler.loadProc_      =hFUNCTOR_BINDMEMBER(hResourceLoadProc, hRenderer,      materialResourceLoader,    this);
        matreshandler.postLoadProc_  =hFUNCTOR_BINDMEMBER(hResourcePostLoadProc, hRenderer,  materialResourcePostLoad,  this);
        matreshandler.preUnloadProc_ =hFUNCTOR_BINDMEMBER(hResourcePreUnloadProc, hRenderer, materialResourcePreUnload, this);
        matreshandler.unloadProc_    =hFUNCTOR_BINDMEMBER(hResourceUnloadProc, hRenderer,    materialResourceUnload,    this);
        resourceManager_->registerResourceHandler("mfx", matreshandler);

        hResourceHandler meshreshandler;
        meshreshandler.loadProc_      =hFUNCTOR_BINDMEMBER(hResourceLoadProc, hRenderer,   meshResourceLoader, this);
        meshreshandler.postLoadProc_  =hFUNCTOR_BINDMEMBER(hResourcePostLoadProc, hRenderer,   meshResourceLink,   this);
        meshreshandler.preUnloadProc_ =hFUNCTOR_BINDMEMBER(hResourcePreUnloadProc, hRenderer, meshResourceUnlink, this);
        meshreshandler.unloadProc_    =hFUNCTOR_BINDMEMBER(hResourceUnloadProc, hRenderer, meshResourceUnload, this);
        resourceManager_->registerResourceHandler("mesh", meshreshandler);
        
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

        hShaderProgram* prog=hNullptr;
        prog=hResourceHandle(hDebugShaderResourceID_PixelWhite).weakPtr<hShaderProgram>();
        resourceManager_->removeResource(hDebugShaderResourceID_PixelWhite);
        prog->DecRef();
        prog=hResourceHandle(hDebugShaderResourceID_VertexPosOnly).weakPtr<hShaderProgram>();
        resourceManager_->removeResource(hDebugShaderResourceID_VertexPosOnly);
        prog->DecRef();
        prog=hResourceHandle(hDebugShaderResourceID_ConsoleVertex).weakPtr<hShaderProgram>();
        resourceManager_->removeResource(hDebugShaderResourceID_ConsoleVertex);
        prog->DecRef();
        prog=hResourceHandle(hDebugShaderResourceID_ConsolePixel).weakPtr<hShaderProgram>();
        resourceManager_->removeResource(hDebugShaderResourceID_ConsolePixel);
        prog->DecRef();
        prog=hResourceHandle(hDebugShaderResourceID_FontVertex).weakPtr<hShaderProgram>();
        resourceManager_->removeResource(hDebugShaderResourceID_FontVertex);
        prog->DecRef();
        prog=hResourceHandle(hDebugShaderResourceID_FontPixel).weakPtr<hShaderProgram>();
        resourceManager_->removeResource(hDebugShaderResourceID_FontPixel);
        prog->DecRef();
        prog=hResourceHandle(hDebugShaderResourceID_VertexPosNormal).weakPtr<hShaderProgram>();
        resourceManager_->removeResource(hDebugShaderResourceID_VertexPosNormal);
        prog->DecRef();
        prog=hResourceHandle(hDebugShaderResourceID_PixelWhiteViewLit).weakPtr<hShaderProgram>();
        resourceManager_->removeResource(hDebugShaderResourceID_PixelWhiteViewLit);
        prog->DecRef();
        prog=hResourceHandle(hDebugShaderResourceID_TexVertex).weakPtr<hShaderProgram>();
        resourceManager_->removeResource(hDebugShaderResourceID_TexVertex);
        prog->DecRef();
        prog=hResourceHandle(hDebugShaderResourceID_TexPixel).weakPtr<hShaderProgram>();
        resourceManager_->removeResource(hDebugShaderResourceID_TexPixel);
        prog->DecRef();
        prog=hResourceHandle(hDebugShaderResourceID_VertexPosCol).weakPtr<hShaderProgram>();
        resourceManager_->removeResource(hDebugShaderResourceID_VertexPosCol);
        prog->DecRef();
        prog=hResourceHandle(hDebugShaderResourceID_PixelPosCol).weakPtr<hShaderProgram>();
        resourceManager_->removeResource(hDebugShaderResourceID_PixelPosCol);
        prog->DecRef();

        //hDELETE_ARRAY_SAFE(GetGlobalHeap(), depthBuffer_->levelDescs_);
        //depthBuffer_->DecRef();
        ParentClass::Destroy();

        hDebugDrawRenderer::it()->destroyResources();
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderer::BeginRenderFrame()
    {
        HEART_PROFILE_FUNC();
        //ReleasePendingRenderResources();
        //Start new frame

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
        hDebugDrawRenderer::it()->render(this, &mainSubmissionCtx_);
        ParentClass::SwapBuffers(backBuffer_);

        hZeroMem(&stats_, sizeof(stats_));
        stats_.gpuTime_=0.f;
        stats_.frametime_=(hFloat)frameTimer_.elapsedMilliSec();
        frameTimer_.reset();
        mainSubmissionCtx_.appendRenderStats(&stats_);
        mainSubmissionCtx_.resetStats();
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderer::createTexture(hUint32 levels, hMipDesc* initialData, hTextureFormat format, hUint32 flags, hTexture** outTex)
    {
        hcAssert(initialData);
        hcAssert(levels > 0);

        (*outTex) = hNEW(hTexture)(
            hFUNCTOR_BINDMEMBER(hTexture::hZeroRefProc, hRenderer, destroyTexture, this),
            eRenderResourceType_Tex2D);

        (*outTex)->nLevels_ = levels;
        (*outTex)->format_ = format;
        (*outTex)->flags_ = flags;
        (*outTex)->levelDescs_ = levels ? hNEW_ARRAY(hTexture::LevelDesc, levels) : NULL;

        for (hUint32 i = 0; i < levels; ++i)
        {
            (*outTex)->levelDescs_[ i ].width_ = initialData[i].width;
            (*outTex)->levelDescs_[ i ].height_ = initialData[i].height;
            (*outTex)->levelDescs_[ i ].mipdata_ = hNullptr;//initialData[i].data;
            (*outTex)->levelDescs_[ i ].mipdataSize_ = 0;//initialData[i].size;
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

        while (w >= 1 && h >= 1 && lvls < 16 && lvls < inout->nLevels_) {
            mipsdata[lvls].width=w;
            mipsdata[lvls].height=h;
            mipsdata[lvls].data=hNullptr;
            mipsdata[lvls].size=0;
            w /= 2;
            h /= 2;
            ++lvls;
        }

        ParentClass::destroyTextureDevice(inout);
        ParentClass::createTextureDevice(lvls, inout->format_, mipsdata, inout->flags_, inout);
        hDELETE_ARRAY_SAFE(inout->levelDescs_);
        inout->nLevels_ = lvls;
        inout->levelDescs_ = lvls ? hNEW_ARRAY(hTexture::LevelDesc, lvls) : hNullptr;

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
        hDELETE_SAFE(pOut);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderer::createIndexBuffer(const void* pIndices, hUint32 nIndices, hUint32 flags, hIndexBuffer** outIB)
    {
        hUint elementSize= nIndices > 0xFFFF ? sizeof(hUint32) : sizeof(hUint16);
        hIndexBuffer* pdata = hNEW(hIndexBuffer)(
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
        hDELETE_SAFE(ib);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderer::createVertexBuffer(const void* initData, hUint32 nElements, hInputLayoutDesc* desc, hUint32 desccount, hUint32 flags, hVertexBuffer** outVB)
    {
        hVertexBuffer* pdata = hNEW(hVertexBuffer)(
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
        ParentClass::destroyVertexBufferDevice(vb);
        hDELETE_SAFE(vb);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hRenderSubmissionCtx* hRenderer::CreateRenderSubmissionCtx()
    {
        hRenderSubmissionCtx* ret = hNEW(hRenderSubmissionCtx);
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
        hDELETE_SAFE(ctx);
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

        hDebugDrawRenderer::it()->initialiseResources(this, resourceManager_);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderer::createDebugShadersInternal() {
        hShaderProgram* prog=hNEW(hShaderProgram)(this, hFUNCTOR_BINDMEMBER(hShaderProgram::hZeroProc, hRenderer, destroyShader, this));
        ParentClass::compileShaderFromSourceDevice(
            ParentClass::getDebugShaderSource(eDebugPixelWhite),
            hStrLen(ParentClass::getDebugShaderSource(eDebugPixelWhite)),
            "mainFP", eShaderProfile_ps4_0, hNullptr, hNullptr, 0, prog);
        resourceManager_->insertResource(hDebugShaderResourceID_PixelWhite, prog);

        prog=hNEW(hShaderProgram)(this, hFUNCTOR_BINDMEMBER(hShaderProgram::hZeroProc, hRenderer, destroyShader, this));
        ParentClass::compileShaderFromSourceDevice(
            ParentClass::getDebugShaderSource(eDebugVertexPosOnly),
            hStrLen(ParentClass::getDebugShaderSource(eDebugVertexPosOnly)),
            "mainVP", eShaderProfile_vs4_0, hNullptr, hNullptr, 0, prog);
        resourceManager_->insertResource(hDebugShaderResourceID_VertexPosOnly, prog);

        prog=hNEW(hShaderProgram)(this, hFUNCTOR_BINDMEMBER(hShaderProgram::hZeroProc, hRenderer, destroyShader, this));
        ParentClass::compileShaderFromSourceDevice(
            ParentClass::getDebugShaderSource(eConsoleVertex),
            hStrLen(ParentClass::getDebugShaderSource(eConsoleVertex)),
            "mainVP", eShaderProfile_vs4_0, hNullptr, hNullptr, 0, prog);
        resourceManager_->insertResource(hDebugShaderResourceID_ConsoleVertex, prog);

        prog=hNEW(hShaderProgram)(this, hFUNCTOR_BINDMEMBER(hShaderProgram::hZeroProc, hRenderer, destroyShader, this));
        ParentClass::compileShaderFromSourceDevice(
            ParentClass::getDebugShaderSource(eConsolePixel),
            hStrLen(ParentClass::getDebugShaderSource(eConsolePixel)),
            "mainFP", eShaderProfile_ps4_0, hNullptr, hNullptr, 0, prog);
        resourceManager_->insertResource(hDebugShaderResourceID_ConsolePixel, prog);

        prog=hNEW(hShaderProgram)(this, hFUNCTOR_BINDMEMBER(hShaderProgram::hZeroProc, hRenderer, destroyShader, this));
        ParentClass::compileShaderFromSourceDevice(
            ParentClass::getDebugShaderSource(eDebugFontVertex),
            hStrLen(ParentClass::getDebugShaderSource(eDebugFontVertex)),
            "mainVP", eShaderProfile_vs4_0, hNullptr, hNullptr, 0, prog);
        resourceManager_->insertResource(hDebugShaderResourceID_FontVertex, prog);

        prog=hNEW(hShaderProgram)(this, hFUNCTOR_BINDMEMBER(hShaderProgram::hZeroProc, hRenderer, destroyShader, this));
        ParentClass::compileShaderFromSourceDevice(
            ParentClass::getDebugShaderSource(eDebugFontPixel),
            hStrLen(ParentClass::getDebugShaderSource(eDebugFontPixel)),
            "mainFP", eShaderProfile_ps4_0, hNullptr, hNullptr, 0, prog);
        resourceManager_->insertResource(hDebugShaderResourceID_FontPixel, prog);

        prog=hNEW(hShaderProgram)(this, hFUNCTOR_BINDMEMBER(hShaderProgram::hZeroProc, hRenderer, destroyShader, this));
        ParentClass::compileShaderFromSourceDevice(
            ParentClass::getDebugShaderSource(eDebugVertexPosNormal),
            hStrLen(ParentClass::getDebugShaderSource(eDebugVertexPosNormal)),
            "mainVP", eShaderProfile_vs4_0, hNullptr, hNullptr, 0, prog);
        resourceManager_->insertResource(hDebugShaderResourceID_VertexPosNormal, prog);

        prog=hNEW(hShaderProgram)(this, hFUNCTOR_BINDMEMBER(hShaderProgram::hZeroProc, hRenderer, destroyShader, this));
        ParentClass::compileShaderFromSourceDevice(
            ParentClass::getDebugShaderSource(eDebugPixelWhiteViewLit),
            hStrLen(ParentClass::getDebugShaderSource(eDebugPixelWhiteViewLit)),
            "mainFP", eShaderProfile_ps4_0, hNullptr, hNullptr, 0, prog);
        resourceManager_->insertResource(hDebugShaderResourceID_PixelWhiteViewLit, prog);

        prog=hNEW(hShaderProgram)(this, hFUNCTOR_BINDMEMBER(hShaderProgram::hZeroProc, hRenderer, destroyShader, this));
        ParentClass::compileShaderFromSourceDevice(
            ParentClass::getDebugShaderSource(eDebugTexVertex),
            hStrLen(ParentClass::getDebugShaderSource(eDebugTexVertex)),
            "mainVP", eShaderProfile_vs4_0, hNullptr, hNullptr, 0, prog);
        resourceManager_->insertResource(hDebugShaderResourceID_TexVertex, prog);

        prog=hNEW(hShaderProgram)(this, hFUNCTOR_BINDMEMBER(hShaderProgram::hZeroProc, hRenderer, destroyShader, this));
        ParentClass::compileShaderFromSourceDevice(
            ParentClass::getDebugShaderSource(eDebugTexPixel),
            hStrLen(ParentClass::getDebugShaderSource(eDebugTexPixel)),
            "mainFP", eShaderProfile_ps4_0, hNullptr, hNullptr, 0, prog);
        resourceManager_->insertResource(hDebugShaderResourceID_TexPixel, prog);

        prog=hNEW(hShaderProgram)(this, hFUNCTOR_BINDMEMBER(hShaderProgram::hZeroProc, hRenderer, destroyShader, this));
        ParentClass::compileShaderFromSourceDevice(
            ParentClass::getDebugShaderSource(eDebugVertexPosCol),
            hStrLen(ParentClass::getDebugShaderSource(eDebugVertexPosCol)),
            "mainVP", eShaderProfile_vs4_0, hNullptr, hNullptr, 0, prog);
        resourceManager_->insertResource(hDebugShaderResourceID_VertexPosCol, prog);

        prog=hNEW(hShaderProgram)(this, hFUNCTOR_BINDMEMBER(hShaderProgram::hZeroProc, hRenderer, destroyShader, this));
        ParentClass::compileShaderFromSourceDevice(
            ParentClass::getDebugShaderSource(eDebugPixelPosCol),
            hStrLen(ParentClass::getDebugShaderSource(eDebugPixelPosCol)),
            "mainFP", eShaderProfile_ps4_0, hNullptr, hNullptr, 0, prog);
        resourceManager_->insertResource(hDebugShaderResourceID_PixelPosCol, prog);

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
        (*outsrv) = hNEW(hShaderResourceView)(
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
        (*outsrv) = hNEW(hShaderResourceView)(
            hFUNCTOR_BINDMEMBER(hShaderResourceView::hZeroRefProc, hRenderer, destroyShaderResourceView, this));
        ParentClass::createShaderResourseViewDevice(cb, desc, *outsrv);
        (*outsrv)->refType_=desc.resourceType_;
        (*outsrv)->refCB_=cb;
        cb->AddRef();
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderer::destroyShaderResourceView(hShaderResourceView* srv) {
        hcAssert(srv);
        hcAssert(srv->GetRefCount() == 0);
        ParentClass::destroyShaderResourceViewDevice(srv);
        hDELETE_SAFE(srv);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderer::createRenderTargetView(hTexture* tex, const hRenderTargetViewDesc& rtvd, hRenderTargetView** outrtv) {
        (*outrtv) = hNEW(hRenderTargetView)(
            hFUNCTOR_BINDMEMBER(hRenderTargetView::hZeroRefProc, hRenderer, destroyRenderTargetView, this));
        ParentClass::createRenderTargetViewDevice(tex, rtvd, *outrtv);
        (*outrtv)->bindTexture(tex);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderer::createDepthStencilView(hTexture* tex, const hDepthStencilViewDesc& dsvd, hDepthStencilView** outdsv) {
        (*outdsv) = hNEW(hDepthStencilView)(
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
        hDELETE_SAFE(view);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderer::destroyDepthStencilView(hDepthStencilView* view) {
        hcAssert(view);
        hcAssert(view->GetRefCount() == 0);
        ParentClass::destroyDepthStencilViewDevice(view);
        hDELETE_SAFE(view);
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
            state = hNEW(hBlendState)(
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
        hDELETE(state);
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
            state = hNEW(hRasterizerState)(
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
        hDELETE(state);
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
            state = hNEW(hDepthStencilState)(
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
        hDELETE(state);
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
            state = hNEW(hSamplerState)(
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
        hDELETE(state);
        resourceMutex_.Unlock();
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderer::createBuffer(hUint size, void* data, hUint flags, hUint stride, hRenderBuffer** outcb) {
        (*outcb) = hNEW(hRenderBuffer)(
            hFUNCTOR_BINDMEMBER(hRenderBuffer::hZeroRefProc, hRenderer, destroyConstantBlock, this));
        ParentClass::createBufferDevice(size, data, flags, stride, *outcb);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderer::destroyConstantBlock(hRenderBuffer* block) {
        ParentClass::destroyConstantBlockDevice(block);
        hDELETE(block);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderer::compileShaderFromSource(const hChar* shaderProg, hUint32 len, 
        const hChar* entry, hShaderProfile profile, hIIncludeHandler* includes, hShaderDefine* defines, hUint ndefines, hShaderProgram** out) {
        (*out) = hNEW(hShaderProgram)(this,
            hFUNCTOR_BINDMEMBER(hShaderProgram::hZeroProc, hRenderer, destroyShader, this));
        ParentClass::compileShaderFromSourceDevice(shaderProg, len, entry, profile, includes, defines, ndefines, *out);
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

    void hRenderer::createShader(const hChar* shaderProg, hUint32 len, hShaderType type, hShaderProgram** out) {
        (*out) = hNEW(hShaderProgram)(this,
            hFUNCTOR_BINDMEMBER(hShaderProgram::hZeroProc, hRenderer, destroyShader, this));
        ParentClass::compileShaderDevice(shaderProg, len, type, *out);
        (*out)->SetShaderType(type);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderer::destroyShader(hShaderProgram* prog) {
        ParentClass::destroyShaderDevice(prog);
        hDELETE(prog);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hResourceClassBase* hRenderer::textureResourceLoader(const hResourceSection* sections, hUint sectioncount) {
        Heart::hTexture* texture=hNullptr;
        Heart::hMipDesc mips[24];
        for (hUint i=0, n=sectioncount; i<n; ++i) {
            if (hStrCmp("texture", sections[i].sectionName_) == 0) {
                google::protobuf::io::ArrayInputStream input(sections[i].sectionData_, sections[i].sectionSize_);
                google::protobuf::io::CodedInputStream inputstream(&input);
                proto::TextureResource texresource;
                hBool ok=texresource.ParseFromCodedStream(&inputstream);
                hcAssertMsg(ok,"Failed to parse Texture Resource");
                if (!ok) {
                    return hNullptr;
                }
                for (hUint32 im = 0, nm=texresource.mips_size(); im < nm; ++im) {
                    mips[im].width=texresource.mips(im).width();
                    mips[im].height=texresource.mips(im).height();
                    mips[im].data=(hByte*)texresource.mips(im).data().c_str();
                    mips[im].size=(hUint)texresource.mips(im).data().length();
                }
                createTexture(texresource.mips_size(), mips, (Heart::hTextureFormat)texresource.format(), 0, &texture);
            }
        }

        return texture;
/*
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
        textureData = (hByte*)hHeapMalloc("general", totalTextureSize);
        for (hUint32 i = 0; i < header.mipCount; ++i) {
            mips[i].data = textureData + (hUint32)(mips[i].data);
        }
        //Read Texture data
        file->Read(textureData, totalTextureSize);
        createTexture(header.mipCount, mips, header.format, header.flags, &texutre);
        return texutre;
*/
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderer::textureResourcePostLoad(hResourceManager* manager, hResourceClassBase* texture) {
        manager->insertResource(texture->getResourceID(), texture);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderer::textureResourcePreUnload(hResourceManager* manager, hResourceClassBase* texture) {
        manager->removeResource(texture->getResourceID());
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderer::textureResourceUnload(hResourceClassBase* resource) {
        hTexture* tex=static_cast<hTexture*>(resource);
        // Package should be the only thing hold ref at this point...
        hcAssertMsg(tex->GetRefCount() == 1, "Texture ref count is %u, it should be 1", tex->GetRefCount());
        tex->DecRef();
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hResourceClassBase* hRenderer::shaderResourceLoader(const hResourceSection* sections, hUint sectioncount) {
        hShaderProgram* shaderProg=hNullptr;
        hBool prefersource=hFalse;
        proto::ShaderResource shaderres;

#if defined (HEART_ALLOW_SHADER_SOURCE_COMPILE)
        class hIncluder : public hIIncludeHandler 
        {
        public:
            hIncluder(const hResourceSection* sections, hUint sectioncount) 
                : sections_(sections)
                , sectioncount_(sectioncount)
            {}
            void findInclude(const hChar* includepath, const void** outdata, hUint* outlen) {
                *outdata=hNullptr;
                *outlen=0;
                for (hUint i=0, n=sectioncount_; i<n; ++i) {
                    if (hStrCmp(includepath, sections_[i].sectionName_) == 0) {
                        *outdata=sections_[i].sectionData_;
                        *outlen=sections_[i].sectionSize_;
                        return;
                    }
                }
            }

            const hResourceSection* sections_;
            hUint sectioncount_;
        };
        hIncluder includes(sections, sectioncount);
        for (hUint i=0, n=sectioncount; i<n; ++i) {
            if (hStrCmp("source", sections[i].sectionName_) == 0) {
                prefersource=hTrue;
            } else if (hStrCmp("shader", sections[i].sectionName_) == 0 && !prefersource) {
                google::protobuf::io::ArrayInputStream input(sections[i].sectionData_, sections[i].sectionSize_);
                google::protobuf::io::CodedInputStream inputstream(&input);
                hBool ok=shaderres.ParseFromCodedStream(&inputstream);
                hcAssertMsg(ok,"Failed to parse Shader Resource");
                if (!ok) {
                    return hNullptr;
                }
            }
        }
#endif
        for (hUint i=0, n=sectioncount; i<n; ++i) {
            if (hStrCmp("shader", sections[i].sectionName_) == 0 && !prefersource) {
                google::protobuf::io::ArrayInputStream input(sections[i].sectionData_, sections[i].sectionSize_);
                google::protobuf::io::CodedInputStream inputstream(&input);
                hBool ok=shaderres.ParseFromCodedStream(&inputstream);
                hcAssertMsg(ok,"Failed to parse Shader Resource");
                if (!ok) {
                    return hNullptr;
                }
                hShaderType type;
                switch(shaderres.type()) {
                case proto::eShaderType_Vertex:     type=ShaderType_VERTEXPROG;    break;
                case proto::eShaderType_Pixel:      type=ShaderType_FRAGMENTPROG;  break;
                case proto::eShaderType_Geometery:  type=ShaderType_GEOMETRYPROG;  break;
                case proto::eShaderType_Hull:       type=ShaderType_HULLPROG;      break;
                case proto::eShaderType_Domain:     type=ShaderType_DOMAINPROG;    break;
                case proto::eShaderType_Compute:    type=ShaderType_COMPUTEPROG;   break;
                }
                createShader((hChar*)shaderres.compiledprogram().c_str(), (hUint)shaderres.compiledprogram().length(), type, &shaderProg);
            } else if (hStrCmp("source", sections[i].sectionName_) == 0) {
#if defined (HEART_ALLOW_SHADER_SOURCE_COMPILE)
                hShaderType type;
                switch(shaderres.type()) {
                case proto::eShaderType_Vertex:     type=ShaderType_VERTEXPROG;    break;
                case proto::eShaderType_Pixel:      type=ShaderType_FRAGMENTPROG;  break;
                case proto::eShaderType_Geometery:  type=ShaderType_GEOMETRYPROG;  break;
                case proto::eShaderType_Hull:       type=ShaderType_HULLPROG;      break;
                case proto::eShaderType_Domain:     type=ShaderType_DOMAINPROG;    break;
                case proto::eShaderType_Compute:    type=ShaderType_COMPUTEPROG;   break;
                }
                hShaderDefine* defines=hNullptr;
                hUint definecount=shaderres.defines_size();
                if (definecount) {
                    defines=(hShaderDefine*)hAlloca(sizeof(hShaderDefine)*definecount);
                    for (hUint di=0; di<definecount; ++di) {
                        defines[di].define_=shaderres.defines(di).define().c_str();
                        defines[di].value_=shaderres.defines(di).value().c_str();
                    }
                }
                compileShaderFromSource((hChar*)sections[i].sectionData_, sections[i].sectionSize_, shaderres.entry().c_str(), (hShaderProfile)shaderres.profile(), &includes, defines, definecount, &shaderProg);
#endif
            }
        }
        return shaderProg;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderer::shaderResourcePostLoad(hResourceManager* manager, hResourceClassBase* resource) {
        manager->insertResource(resource->getResourceID(), resource);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderer::shaderResourcePreUnload(hResourceManager* manager, hResourceClassBase* resource) {
        manager->removeResource(resource->getResourceID());
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderer::shaderResourceUnload(hResourceClassBase* resource) {
        hShaderProgram* sp = static_cast<hShaderProgram*>(resource);
        sp->DecRef();
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hResourceClassBase* hRenderer::materialResourceLoader(const hResourceSection* sections, hUint sectioncount) {
        for (hUint i=0, n=sectioncount; i<n; ++i) {
            if (hStrCmp("material_blob", sections[i].sectionName_) == 0) {
                hcAssertFailMsg("material blob section no longer supported");
            } else if (hStrCmp("material", sections[i].sectionName_) == 0) {
                google::protobuf::io::ArrayInputStream input(sections[i].sectionData_, sections[i].sectionSize_);
                google::protobuf::io::CodedInputStream inputstream(&input);
                proto::MaterialResource materialres;
                hBool ok=materialres.ParseFromCodedStream(&inputstream);
                hcAssertMsg(ok,"Failed to parse Shader Resource");
                if (!ok) {
                    return hNullptr;
                }
                hMaterial* material = hNEW(hMaterial)(this);
                for (hUint gi=0, gn=materialres.groups_size(); gi<gn; ++gi) {
                    const proto::MaterialGroup& groupdef=materialres.groups(gi);
                    hMaterialGroup* group=material->addGroup(groupdef.groupname().c_str());
                    group->techCountHint(groupdef.technique_size());
                    for (hUint ti=0, tn=groupdef.technique_size(); ti<tn; ++ti) {
                        const proto::MaterialTechnique& techdef=groupdef.technique(ti);
                        hMaterialTechnique* tech=group->addTechnique(techdef.techniquename().c_str());
                        tech->SetLayer(techdef.layer());
                        tech->SetSort(techdef.transparent());
                        tech->SetPasses(techdef.passes_size());
                        for (hUint pi=0, pn=techdef.passes_size(); pi<pn; ++pi) {
                            const proto::MaterialPass& passdef=techdef.passes(pi);
                            hMaterialTechniquePass* pass=tech->appendPass();
                            hBlendStateDesc blenddesc;
                            hDepthStencilStateDesc depthdesc;
                            hRasterizerStateDesc rasterdesc;
                            if (passdef.has_pixel()) {
                                pass->setProgramID(ShaderType_FRAGMENTPROG, hResourceID(passdef.pixel()));
                            }
                            if (passdef.has_vertex()) {
                                pass->setProgramID(ShaderType_VERTEXPROG, hResourceID(passdef.vertex()));
                            }
                            if (passdef.has_geometry()) {
                                pass->setProgramID(ShaderType_GEOMETRYPROG, hResourceID(passdef.geometry()));
                            }
                            if (passdef.has_domain()) {
                                pass->setProgramID(ShaderType_DOMAINPROG, hResourceID(passdef.domain()));
                            }
                            if (passdef.has_hull()) {
                                pass->setProgramID(ShaderType_HULLPROG, hResourceID(passdef.hull()));
                            }
                            if (passdef.has_compute()) {
                                pass->setProgramID(ShaderType_COMPUTEPROG, hResourceID(passdef.compute()));
                            }
                            if (passdef.has_blend()) {
                                const proto::BlendState& blenddef=passdef.blend();
                                if (blenddef.has_blendenable()) {
                                    blenddesc.blendEnable_=(RENDER_STATE_VALUE)blenddef.blendenable();
                                }
                                if (blenddef.has_srcblend()) {
                                    blenddesc.srcBlend_=(RENDER_STATE_VALUE)blenddef.srcblend();
                                }
                                if (blenddef.has_destblend()) {
                                    blenddesc.destBlend_=(RENDER_STATE_VALUE)blenddef.destblend();
                                }
                                if (blenddef.has_blendop()) {
                                    blenddesc.blendOp_=(RENDER_STATE_VALUE)blenddef.blendop();
                                }
                                if (blenddef.has_srcblendalpha()) {
                                    blenddesc.srcBlendAlpha_=(RENDER_STATE_VALUE)blenddef.srcblendalpha();
                                }
                                if (blenddef.has_destblendalpha()) {
                                    blenddesc.destBlendAlpha_=(RENDER_STATE_VALUE)blenddef.destblendalpha();
                                }
                                if (blenddef.has_blendopalpha()) {
                                    blenddesc.blendOpAlpha_=(RENDER_STATE_VALUE)blenddef.blendopalpha();
                                }
                                if (blenddef.has_rendertargetwritemask()) {
                                    blenddesc.renderTargetWriteMask_=blenddef.rendertargetwritemask();
                                }
                            }
                            hBlendState* bs=createBlendState(blenddesc);
                            pass->bindBlendState(bs);
                            bs->DecRef();
                            if (passdef.has_depthstencil()) {
                                const proto::DepthStencilState& depthdef=passdef.depthstencil();
                                if (depthdef.has_depthenable()) {
                                    depthdesc.depthEnable_ = (RENDER_STATE_VALUE)depthdef.depthenable();
                                }
                                if (depthdef.has_depthwritemask()) {
                                    depthdesc.depthWriteMask_=(RENDER_STATE_VALUE)depthdef.depthwritemask();
                                }
                                if (depthdef.has_depthfunc()) {
                                    depthdesc.depthFunc_=(RENDER_STATE_VALUE)depthdef.depthfunc();
                                }
                                if (depthdef.has_stencilenable()) {
                                    depthdesc.stencilEnable_=(RENDER_STATE_VALUE)depthdef.stencilenable();
                                }
                                if (depthdef.has_stencilreadmask()) {
                                    depthdesc.stencilReadMask_=depthdef.stencilreadmask();
                                }
                                if (depthdef.has_stencilwritemask()) {
                                    depthdesc.stencilWriteMask_=depthdef.stencilwritemask();
                                }
                                if (depthdef.has_stencilfailop()) {
                                    depthdesc.stencilFailOp_=(RENDER_STATE_VALUE)depthdef.stencilfailop();
                                }
                                if (depthdef.has_stencildepthfailop()) {
                                    depthdesc.stencilDepthFailOp_=(RENDER_STATE_VALUE)depthdef.stencildepthfailop();
                                }
                                if (depthdef.has_stencilpassop()) {
                                    depthdesc.stencilPassOp_=(RENDER_STATE_VALUE)depthdef.stencilpassop();
                                }
                                if (depthdef.has_stencilfunc()) {
                                    depthdesc.stencilFunc_=(RENDER_STATE_VALUE)depthdef.stencilfunc();
                                }
                                if (depthdef.has_stencilref()) {
                                    depthdesc.stencilRef_=depthdef.stencilref();
                                }
                            }
                            hDepthStencilState* ds=createDepthStencilState(depthdesc);
                            pass->bindDepthStencilState(ds);
                            ds->DecRef();
                            if (passdef.has_rasterizer()) {
                                const proto::RasterizerState& rasterdef=passdef.rasterizer();
                                if (rasterdef.has_fillmode()) {
                                    rasterdesc.fillMode_=(RENDER_STATE_VALUE)rasterdef.fillmode();
                                }
                                if (rasterdef.has_cullmode()) {
                                    rasterdesc.cullMode_=(RENDER_STATE_VALUE)rasterdef.cullmode();
                                }
                                if (rasterdef.has_frontcounterclockwise()) {
                                    rasterdesc.frontCounterClockwise_=(RENDER_STATE_VALUE)rasterdef.frontcounterclockwise();
                                }
                                if (rasterdef.has_depthbias()) {
                                    rasterdesc.depthBias_=(RENDER_STATE_VALUE)rasterdef.depthbias();
                                }
                                if (rasterdef.has_depthbiasclamp()) {
                                    rasterdesc.depthBiasClamp_=rasterdef.depthbiasclamp();
                                }
                                if (rasterdef.has_slopescaleddepthbias()) {
                                    rasterdesc.slopeScaledDepthBias_=rasterdef.slopescaleddepthbias();
                                }
                                if (rasterdef.has_depthclipenable()) {
                                    rasterdesc.depthClipEnable_=(RENDER_STATE_VALUE)rasterdef.depthclipenable();
                                }
                                if (rasterdef.has_scissorenable()) {
                                    rasterdesc.scissorEnable_=(RENDER_STATE_VALUE)rasterdef.scissorenable();
                                }
                            }
                            hRasterizerState* rs=createRasterizerState(rasterdesc);
                            pass->bindRasterizerState(rs);
                            rs->DecRef();
                        }
                    }
                }
                //Read samplers
                for (hUint32 is=0, ns=materialres.samplers_size(); is<ns; ++is)
                {
                    const proto::MaterialSampler& matsamplerdef=materialres.samplers(is);
                   
                    hSamplerParameter sampler;
                    hSamplerStateDesc samplerdesc;
                    hSamplerState* ss;

                    if (matsamplerdef.has_samplerstate()) {
                        const proto::SamplerState& samplerdef=matsamplerdef.samplerstate();
                        if (samplerdef.has_filter()) {
                            samplerdesc.filter_=(hSAMPLER_STATE_VALUE)samplerdef.filter();
                        }
                        if (samplerdef.has_addressu()) {
                            samplerdesc.addressU_=(hSAMPLER_STATE_VALUE)samplerdef.addressu();
                        }
                        if (samplerdef.has_addressv()) {
                            samplerdesc.addressV_=(hSAMPLER_STATE_VALUE)samplerdef.addressv();
                        }
                        if (samplerdef.has_addressw()) {
                            samplerdesc.addressW_=(hSAMPLER_STATE_VALUE)samplerdef.addressw();
                        }
                        if (samplerdef.has_miplodbias()) {
                            samplerdesc.mipLODBias_=samplerdef.miplodbias();
                        }
                        if (samplerdef.has_maxanisotropy()) {
                            samplerdesc.maxAnisotropy_=samplerdef.maxanisotropy();
                        }
                        if (samplerdef.has_bordercolour()) {
                            const proto::Colour& colour=samplerdef.bordercolour();
                            hFloat alpha=colour.has_alpha() ? (colour.alpha()/255.f) : 1.f;
                            samplerdesc.borderColour_=hColour((colour.red()/255.f), (colour.green()/255.f), (colour.blue()/255.f), alpha);
                        }
                        if (samplerdef.has_minlod()) {
                            samplerdesc.minLOD_=samplerdef.minlod();
                        }
                        if (samplerdef.has_maxlod()) {
                            samplerdesc.maxLOD_=samplerdef.maxlod();
                        }
                    }
                    ss = createSamplerState(samplerdesc);

                    hStrCopy(sampler.name_, sampler.name_.GetMaxSize(), matsamplerdef.samplername().c_str());
                    sampler.paramID_=hCRC32::StringCRC(sampler.name_);
                    sampler.samplerState_=ss;
                    material->addSamplerParameter(sampler);
                }
                //Read parameters
                for (hUint pi=0, pn=materialres.parameters_size(); pi<pn; ++pi) {
                    const proto::MaterialParameter& paramdef=materialres.parameters(pi);
                    if (paramdef.has_resourceid()) {
                        material->addDefaultParameterValue(paramdef.paramname().c_str(), hResourceID(paramdef.resourceid()));
                    } else if (paramdef.floatvalues_size()) {
                        material->addDefaultParameterValue(paramdef.paramname().c_str(), paramdef.floatvalues().data(), paramdef.floatvalues_size());
                    } else if (paramdef.intvalues_size()) {
                        material->addDefaultParameterValue(paramdef.paramname().c_str(), (const hInt32*)paramdef.intvalues().data(), paramdef.intvalues_size());
                    } else if (paramdef.colourvalues_size()) {
                        proto::Colour c=paramdef.colourvalues(0);
                        hColour colour(c.red()/255.f, c.green()/255.f, c.blue()/255.f, c.has_alpha() ? c.alpha()/255.f : 1.f);
                        material->addDefaultParameterValue(paramdef.paramname().c_str(), colour);
                    }
                }

                return material;
            }
        }

        return hNullptr;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderer::materialResourcePostLoad(hResourceManager* manager, hResourceClassBase* resource) {
        hMaterial* mat = static_cast< hMaterial* >(resource);
        mat->listenToResourceEvents(manager);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderer::materialResourcePreUnload(hResourceManager* manager, hResourceClassBase* resource) {
        hcAssert(resource);
        hMaterial* mat = static_cast<hMaterial*>(resource);
        mat->stopListeningToResourceEvents();
        mat->unbind();
        manager->removeResource(mat->getResourceID());
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderer::materialResourceUnload(hResourceClassBase* resource) {
        hMaterial* mat = static_cast<hMaterial*>(resource);
        hDELETE(mat);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hResourceClassBase* hRenderer::meshResourceLoader(const hResourceSection* sections, hUint sectioncount) {
        for (hUint i=0, n=sectioncount; i<n; ++i) {
            if (hStrCmp("mesh", sections[i].sectionName_) == 0) {
                google::protobuf::io::ArrayInputStream input(sections[i].sectionData_, sections[i].sectionSize_);
                google::protobuf::io::CodedInputStream inputstream(&input);
                proto::Mesh meshres;
                hBool ok=meshres.ParseFromCodedStream(&inputstream);
                hcAssertMsg(ok,"Failed to parse Shader Resource");
                if (!ok) {
                    return hNullptr;
                }

                hRenderModel* rmodel = hNEW(hRenderModel)();
                rmodel->setRenderableCountHint(meshres.renderables_size());
                for (hUint ri=0, rn=meshres.renderables_size(); ri<rn; ++ri) {
                    const proto::Renderable& renderableres=meshres.renderables(ri);
                    hRenderable* renderable=rmodel->appendRenderable();
                    PrimitiveType primtype=(PrimitiveType)renderableres.primtype();
                    hAABB aabb;

                    aabb.reset();

                    if (renderableres.has_aabb()) {
                        const proto::AxisAlignedBoundingBox& aabbres=renderableres.aabb();
                        hVec3 bounds[] = {
                            hVec3(aabbres.minx(), aabbres.miny(), aabbres.minz()),
                            hVec3(aabbres.maxx(), aabbres.maxy(), aabbres.maxz())
                        };
                        aabb = hAABB::computeFromPointSet(bounds, (hUint)hStaticArraySize(bounds));
                    }
                    renderable->SetStartIndex(0);
                    renderable->SetPrimativeType(primtype);
                    renderable->SetPrimativeCount(renderableres.primcount());
                    renderable->SetAABB(aabb);
                    renderable->setMaterialResourceID(hResourceID(renderableres.materialresource()));

                    if (renderableres.has_indexbuffer()) {
                        hIndexBuffer* ib;
                        createIndexBuffer(renderableres.indexbuffer().data(), renderableres.indexcount(), 0, &ib);
                        renderable->SetIndexBuffer(ib);
                    }

                    rmodel->setRenderableCountHint(renderableres.vertexstreams_size());
                    for (hUint rsi=0, rsn=renderableres.vertexstreams_size(); rsi<rsn; ++rsi) {
                        hUint vtxcount=renderableres.vertexcount();
                        const proto::VertexStream& vtxstreamres=renderableres.vertexstreams(rsi);
                        hInputLayoutDesc vtxstreamdesc(
                            vtxstreamres.semantic().c_str(),
                            vtxstreamres.semanticindex(),
                            (hInputFormat)vtxstreamres.format(),
                            rsi, 0);
                        hVertexBuffer* vb=hNullptr;
                        createVertexBuffer(vtxstreamres.streamdata().data(), vtxcount, &vtxstreamdesc, 1, 0, &vb);
                        renderable->setVertexStream(rsi, vb);
                    }
                }
                return rmodel;
            }
        }

        return hNullptr;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderer::meshResourceLink(hResourceManager* manager, hResourceClassBase* resource) {
        hRenderModel* rmodel = static_cast< hRenderModel* >(resource);
        rmodel->listenForResourceEvents(manager);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderer::meshResourceUnlink(hResourceManager* manager, hResourceClassBase* resource) {
        hRenderModel* rmodel = static_cast< hRenderModel* >(resource);
        rmodel->cleanUp();
        manager->removeResource(rmodel->getResourceID());
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderer::meshResourceUnload(hResourceClassBase* resource) {
        hRenderModel* rmodel = static_cast< hRenderModel* >(resource);
        hUint32 renderablecount = rmodel->getRenderableCount();
        for(hUint32 i = 0; i < renderablecount; ++i) {
            hRenderable* renderable=rmodel->getRenderable(i);
            for (hUint32 s = 0, sc=renderable->getVertexStreamCount(); s < sc; ++s) {
                hVertexBuffer* vb=renderable->getVertexStream(s);
                if (vb) {
                    vb->DecRef();
                    vb=NULL;
                }
            }
            renderable->getIndexBuffer()->DecRef();
        }
        hDELETE_SAFE(rmodel);
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
        stopResourceEventListening();


        destroy();

        targetInfo_=*rndrinfo;
        if (targetInfo_.vertexLightShader_.getIsValid()) {
            targetInfo_.vertexLightShader_.registerForUpdates(hFUNCTOR_BINDMEMBER(hResourceEventProc, hLightingManager, resourceUpdate, this));
        }
        if (targetInfo_.pixelLightShader_.getIsValid()) {
            targetInfo_.pixelLightShader_.registerForUpdates(hFUNCTOR_BINDMEMBER(hResourceEventProc, hLightingManager, resourceUpdate, this));
        }
        renderer_=renderer;
        generateRenderCommands(renderer);

        for (hUint i=0, n=sphereLights_.GetMaxSize(); i<n; ++i) {
            freeSphereLights_.addTail(&sphereLights_[i]);
        }
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
            //depthStencilState_->DecRef();
            depthStencilState_=hNullptr;
        }
        if (samplerState_) {
            samplerState_->DecRef();
            samplerState_=hNullptr;
        }
        if (inputLayout_ && targetInfo_.vertexLightShader_.weakPtr<hShaderProgram>()) {
            targetInfo_.vertexLightShader_.weakPtr<hShaderProgram>()->destroyVertexLayout(inputLayout_);
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
        if (quadLightData_) {
            quadLightData_->DecRef();
            quadLightData_=hNullptr;
        }
        if (sphereLightData_) {
            sphereLightData_->DecRef();
            sphereLightData_=hNullptr;
        }
        if (screenQuadIB_) {
            screenQuadIB_->DecRef();
            screenQuadIB_=hNullptr;
        }
        if (screenQuadVB_) {
            screenQuadVB_->DecRef();
            screenQuadVB_=hNullptr;
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

    void hLightingManager::addQuadLight(const hVec3& halfwidth, const hVec3& halfheight, const hVec3& centre, const hColour& colour) {
        hQuadLight quad;
        quadLights_[lightInfo_.quadLightCount_].points_[0] = centre-halfwidth-halfheight;
        quadLights_[lightInfo_.quadLightCount_].points_[1] = centre+halfwidth-halfheight;
        quadLights_[lightInfo_.quadLightCount_].points_[2] = centre+halfwidth+halfheight;
        quadLights_[lightInfo_.quadLightCount_].points_[3] = centre-halfwidth+halfheight;
        quadLights_[lightInfo_.quadLightCount_].colour_ = colour;
        quadLights_[lightInfo_.quadLightCount_].centre_=centre;
        quadLights_[lightInfo_.quadLightCount_].halfv_[0]=halfwidth;
        quadLights_[lightInfo_.quadLightCount_].halfv_[1]=halfheight;
        ++lightInfo_.quadLightCount_;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hLightingManager::doDeferredLightPass(hRenderer* renderer, hRenderSubmissionCtx* ctx) {
        if (renderCmds_.isEmpty()) {
            return;
        }

        (void)renderer;
        hRenderBufferMapInfo mapinfo;
        hRendererCamera* viewcam=renderer->GetRenderCamera(targetInfo_.viewCameraIndex_);
        hMatrix view=viewcam->GetViewMatrix();
        hMatrix invView=hMatrixFunc::inverse(view);
        hMatrix project=viewcam->GetProjectionMatrix();
        hMatrix invProject=hMatrixFunc::inverse(project);

        drawDebugLightInfo();

        ctx->Map(inputLightData_, &mapinfo); {
            hInputLightData* mapptr=(hInputLightData*)mapinfo.ptr;
            mapptr->viewMatrix_=view;
            mapptr->inverseViewMtx_=invView;
            mapptr->projectionMtx_=project;
            mapptr->inverseProjectMtx_=invProject;
            mapptr->eyePos_=hMatrixFunc::getRow(invView, 3);
            mapptr->directionalLightCount_ = lightInfo_.directionalLightCount_;
            mapptr->quadLightCount_ = lightInfo_.quadLightCount_;
            mapptr->sphereLightCount_=lightInfo_.sphereLightCount_;
            ctx->Unmap(&mapinfo);
        }
        ctx->Map(directionLightData_, &mapinfo); {
            hDirectionalLight* mapptr=(hDirectionalLight*)mapinfo.ptr;
            for (hUint i=0; i<lightInfo_.directionalLightCount_; ++i) {
                mapptr[i]=directionalLights_[i];
            }
            ctx->Unmap(&mapinfo);
        }
        ctx->Map(quadLightData_, &mapinfo); {
            hQuadLight* mapptr=(hQuadLight*)mapinfo.ptr;
            for (hUint i=0; i<lightInfo_.quadLightCount_; ++i) {
                mapptr[i]=quadLights_[i];
            }
            ctx->Unmap(&mapinfo);
        }
        ctx->Map(sphereLightData_, &mapinfo); {
            hSphereLightRenderData* mapptr=(hSphereLightRenderData*)mapinfo.ptr;
            for (hSphereLight* i=activeSphereLights_.begin(), *n=activeSphereLights_.end(); i!=n; i=i->GetNext()) {
                mapptr->centreRadius_=i->centreRadius_;
                mapptr->colour_=i->colour_;
                mapptr->colour_.r_=12.57f*10.f;//light power
                mapptr->colour_.g_=12.57f*10.f;//light power
                mapptr->colour_.b_=12.57f*10.f;//light power
                mapptr->colour_.a_=12.57f*10.f;//light power
                ++mapptr;
            }
            ctx->Unmap(&mapinfo);
        }
        ctx->runRenderCommands(renderCmds_.getFirst());
    }
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hLightingManager::drawDebugLightInfo() {
        hDebugDraw* dd=hDebugDraw::it();
        dd->begin();

        for (hUint i=0; i<lightInfo_.quadLightCount_; ++i) {
            hVec3 forward=hVec3Func::cross(quadLights_[i].halfv_[0], quadLights_[i].halfv_[1]);
            forward=hVec3Func::normaliseFast(forward)*hVec3Func::lengthFast(quadLights_[i].halfv_[0]);
            Heart::hDebugLine quadlight[] = {
                //quad
                {quadLights_[i].points_[0], quadLights_[i].points_[1], hColour(1.f, 0.f, 1.f, 1.f)},
                {quadLights_[i].points_[1], quadLights_[i].points_[2], hColour(1.f, 0.f, 1.f, 1.f)},
                {quadLights_[i].points_[2], quadLights_[i].points_[3], hColour(1.f, 0.f, 1.f, 1.f)},
                {quadLights_[i].points_[3], quadLights_[i].points_[0], hColour(1.f, 0.f, 1.f, 1.f)},
                //half widths
                {quadLights_[i].centre_, quadLights_[i].centre_+quadLights_[i].halfv_[0], hColour(1.f, 0.f, 0.f, 1.f)},
                {quadLights_[i].centre_, quadLights_[i].centre_+quadLights_[i].halfv_[1], hColour(0.f, 1.f, 0.f, 1.f)},
                {quadLights_[i].centre_, quadLights_[i].centre_+forward, hColour(0.f, 0.f, 1.f, 1.f)},
            };
            dd->drawLines(quadlight, (hUint)hArraySize(quadlight), eDebugSet_3DDepth);
        }

        for (hSphereLight* i=activeSphereLights_.begin(), *n=activeSphereLights_.end(); i!=n; i=i->GetNext()) {
            const hSphereLight& l=*i;
            Heart::hDebugLine spherelines[] = {
                { l.centreRadius_+hVec3(l.centreRadius_.getW(), 0.f, 0.f), l.centreRadius_-hVec3(l.centreRadius_.getW(), 0.f, 0.f), hColour(1.f, 0.f, 0.f, 1.f) },
                { l.centreRadius_+hVec3(0.f, l.centreRadius_.getW(), 0.f), l.centreRadius_-hVec3(0.f, l.centreRadius_.getW(), 0.f), hColour(0.f, 1.f, 0.f, 1.f) },
                { l.centreRadius_+hVec3(0.f, 0.f, l.centreRadius_.getW()), l.centreRadius_-hVec3(0.f, 0.f, l.centreRadius_.getW()), hColour(0.f, 0.f, 1.f, 1.f) },
                // rings
                { l.centreRadius_+hVec3(l.centreRadius_.getW()*hCos(0.f)   ,  l.centreRadius_.getW()*hSin(0.f)   , 0.f), l.centreRadius_+hVec3(l.centreRadius_.getW()*hCos(0.785f),  l.centreRadius_.getW()*hSin(0.785f), 0.f), hColour(1.f, 0.f, 1.f, 1.f) },
                { l.centreRadius_+hVec3(l.centreRadius_.getW()*hCos(0.785f),  l.centreRadius_.getW()*hSin(0.785f), 0.f), l.centreRadius_+hVec3(l.centreRadius_.getW()*hCos(1.57f) ,  l.centreRadius_.getW()*hSin(1.57f) , 0.f), hColour(1.f, 0.f, 1.f, 1.f) },
                { l.centreRadius_+hVec3(l.centreRadius_.getW()*hCos(1.57f) ,  l.centreRadius_.getW()*hSin(1.57f) , 0.f), l.centreRadius_+hVec3(l.centreRadius_.getW()*hCos(2.355f),  l.centreRadius_.getW()*hSin(2.355f), 0.f), hColour(1.f, 0.f, 1.f, 1.f) },
                { l.centreRadius_+hVec3(l.centreRadius_.getW()*hCos(2.355f),  l.centreRadius_.getW()*hSin(2.355f), 0.f), l.centreRadius_+hVec3(l.centreRadius_.getW()*hCos(3.14f) ,  l.centreRadius_.getW()*hSin(3.14f) , 0.f), hColour(1.f, 0.f, 1.f, 1.f) },
                { l.centreRadius_+hVec3(l.centreRadius_.getW()*hCos(3.14f) ,  l.centreRadius_.getW()*hSin(3.14f) , 0.f), l.centreRadius_+hVec3(l.centreRadius_.getW()*hCos(3.925f),  l.centreRadius_.getW()*hSin(3.925f), 0.f), hColour(1.f, 0.f, 1.f, 1.f) },
                { l.centreRadius_+hVec3(l.centreRadius_.getW()*hCos(3.925f),  l.centreRadius_.getW()*hSin(3.925f), 0.f), l.centreRadius_+hVec3(l.centreRadius_.getW()*hCos(4.71f) ,  l.centreRadius_.getW()*hSin(4.71f) , 0.f), hColour(1.f, 0.f, 1.f, 1.f) },
                { l.centreRadius_+hVec3(l.centreRadius_.getW()*hCos(4.71f) ,  l.centreRadius_.getW()*hSin(4.71f) , 0.f), l.centreRadius_+hVec3(l.centreRadius_.getW()*hCos(5.495f),  l.centreRadius_.getW()*hSin(5.495f), 0.f), hColour(1.f, 0.f, 1.f, 1.f) },
                { l.centreRadius_+hVec3(l.centreRadius_.getW()*hCos(5.495f),  l.centreRadius_.getW()*hSin(5.495f), 0.f), l.centreRadius_+hVec3(l.centreRadius_.getW()*hCos(0.f)   ,  l.centreRadius_.getW()*hSin(0.f)   , 0.f), hColour(1.f, 0.f, 1.f, 1.f) },
                
                { l.centreRadius_+hVec3(l.centreRadius_.getW()*hCos(0.f)   ,  0.f, l.centreRadius_.getW()*hSin(0.f)   ), l.centreRadius_+hVec3(l.centreRadius_.getW()*hCos(0.785f), 0.f, l.centreRadius_.getW()*hSin(0.785f)), hColour(1.f, 0.f, 1.f, 1.f) },
                { l.centreRadius_+hVec3(l.centreRadius_.getW()*hCos(0.785f),  0.f, l.centreRadius_.getW()*hSin(0.785f)), l.centreRadius_+hVec3(l.centreRadius_.getW()*hCos(1.57f) , 0.f, l.centreRadius_.getW()*hSin(1.57f) ), hColour(1.f, 0.f, 1.f, 1.f) },
                { l.centreRadius_+hVec3(l.centreRadius_.getW()*hCos(1.57f) ,  0.f, l.centreRadius_.getW()*hSin(1.57f) ), l.centreRadius_+hVec3(l.centreRadius_.getW()*hCos(2.355f), 0.f, l.centreRadius_.getW()*hSin(2.355f)), hColour(1.f, 0.f, 1.f, 1.f) },
                { l.centreRadius_+hVec3(l.centreRadius_.getW()*hCos(2.355f),  0.f, l.centreRadius_.getW()*hSin(2.355f)), l.centreRadius_+hVec3(l.centreRadius_.getW()*hCos(3.14f) , 0.f, l.centreRadius_.getW()*hSin(3.14f) ), hColour(1.f, 0.f, 1.f, 1.f) },
                { l.centreRadius_+hVec3(l.centreRadius_.getW()*hCos(3.14f) ,  0.f, l.centreRadius_.getW()*hSin(3.14f) ), l.centreRadius_+hVec3(l.centreRadius_.getW()*hCos(3.925f), 0.f, l.centreRadius_.getW()*hSin(3.925f)), hColour(1.f, 0.f, 1.f, 1.f) },
                { l.centreRadius_+hVec3(l.centreRadius_.getW()*hCos(3.925f),  0.f, l.centreRadius_.getW()*hSin(3.925f)), l.centreRadius_+hVec3(l.centreRadius_.getW()*hCos(4.71f) , 0.f, l.centreRadius_.getW()*hSin(4.71f) ), hColour(1.f, 0.f, 1.f, 1.f) },
                { l.centreRadius_+hVec3(l.centreRadius_.getW()*hCos(4.71f) ,  0.f, l.centreRadius_.getW()*hSin(4.71f) ), l.centreRadius_+hVec3(l.centreRadius_.getW()*hCos(5.495f), 0.f, l.centreRadius_.getW()*hSin(5.495f)), hColour(1.f, 0.f, 1.f, 1.f) },
                { l.centreRadius_+hVec3(l.centreRadius_.getW()*hCos(5.495f),  0.f, l.centreRadius_.getW()*hSin(5.495f)), l.centreRadius_+hVec3(l.centreRadius_.getW()*hCos(0.f)   , 0.f, l.centreRadius_.getW()*hSin(0.f)   ), hColour(1.f, 0.f, 1.f, 1.f) },
            };

            dd->drawLines(spherelines, (hUint)hArraySize(spherelines), eDebugSet_3DDepth);
        }

        dd->end();
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hLightingManager::enableSphereLight(hUint light, hBool enable) {
        if (enable) {
            hcAssert(!activeSphereLights_.existInList(&sphereLights_[light]));
            freeSphereLights_.remove(&sphereLights_[light]);
            activeSphereLights_.addHead(&sphereLights_[light]);
        } else if (!enable) {
            hcAssert(activeSphereLights_.existInList(&sphereLights_[light]));
            activeSphereLights_.remove(&sphereLights_[light]);
            freeSphereLights_.addHead(&sphereLights_[light]);
        }
        lightInfo_.sphereLightCount_=activeSphereLights_.getSize();
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hLightingManager::setSphereLight(hUint light, const hVec3& centre, hFloat radius) {
        sphereLights_[light].centreRadius_=hVec4(centre, radius);
        sphereLights_[light].colour_=WHITE;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hLightingManager::generateRenderCommands(hRenderer* renderer) {
        hRenderCommandGenerator rcGen(&renderCmds_);
        if (!targetInfo_.pixelLightShader_.weakPtr() || !targetInfo_.vertexLightShader_.weakPtr()) {
            return;
        }

        destroy();

        lightInfo_.directionalLightCount_=0;
        lightInfo_.quadLightCount_=0;
        renderer->createBuffer(sizeof(hInputLightData), hNullptr, eResourceFlag_ConstantBuffer, 0, &inputLightData_);
        renderer->createBuffer(sizeof(hDirectionalLight)*s_maxDirectionalLights, hNullptr, eResourceFlag_ShaderResource | eResourceFlag_StructuredBuffer, sizeof(hDirectionalLight), &directionLightData_);
        renderer->createBuffer(sizeof(hQuadLight)*s_maxQuadLights, hNullptr, eResourceFlag_ShaderResource | eResourceFlag_StructuredBuffer, sizeof(hQuadLight), &quadLightData_);
        renderer->createBuffer(sizeof(hSphereLightRenderData)*s_maxSphereLights, hNullptr, eResourceFlag_ShaderResource | eResourceFlag_StructuredBuffer, sizeof(hSphereLightRenderData), &sphereLightData_);
        hRenderUtility::buildTessellatedQuadMesh(1.f, 1.f, 10, 10, renderer, &screenQuadIB_, &screenQuadVB_);
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
        inputLayout_=targetInfo_.vertexLightShader_.weakPtr<hShaderProgram>()->createVertexLayout(screenQuadVB_->getLayoutDesc(), screenQuadVB_->getDescCount());


        for (hUint i=0, n=targetInfo_.pixelLightShader_.weakPtr<hShaderProgram>()->getInputCount(); i<n; ++i) {
            hShaderInput shaderInput;
            targetInfo_.pixelLightShader_.weakPtr<hShaderProgram>()->getInput(i, &shaderInput);
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
            } else if (hStrCmp(shaderInput.name_, "quad_lighting")==0 && shaderInput.type_==eShaderInputType_Resource) {
                hShaderResourceViewDesc srvdesc;
                hShaderResourceView* srv;
                hZeroMem(&srvdesc, sizeof(srvdesc));
                srvdesc.resourceType_=eRenderResourceType_Buffer;
                srvdesc.format_=eTextureFormat_Unknown;
                srvdesc.buffer_.firstElement_=0;
                srvdesc.buffer_.elementOffset_=0;
                srvdesc.buffer_.elementWidth_=sizeof(hQuadLight);
                srvdesc.buffer_.numElements_=s_maxQuadLights;
                renderer->createShaderResourceView(quadLightData_, srvdesc, &srv);
                if (srv_.GetSize() < shaderInput.bindPoint_+1) {
                    srv_.Resize(shaderInput.bindPoint_+1);
                }
                srv_[shaderInput.bindPoint_]=srv;
            } else if (hStrCmp(shaderInput.name_, "sphere_lighting")==0 && shaderInput.type_==eShaderInputType_Resource) {
                hShaderResourceViewDesc srvdesc;
                hShaderResourceView* srv;
                hZeroMem(&srvdesc, sizeof(srvdesc));
                srvdesc.resourceType_=eRenderResourceType_Buffer;
                srvdesc.format_=eTextureFormat_Unknown;
                srvdesc.buffer_.firstElement_=0;
                srvdesc.buffer_.elementOffset_=0;
                srvdesc.buffer_.elementWidth_=sizeof(hSphereLightRenderData);
                srvdesc.buffer_.numElements_=s_maxSphereLights;
                renderer->createShaderResourceView(sphereLightData_, srvdesc, &srv);
                if (srv_.GetSize() < shaderInput.bindPoint_+1) {
                    srv_.Resize(shaderInput.bindPoint_+1);
                }
                srv_[shaderInput.bindPoint_]=srv;
            }
        }
        rcGen.setRenderStates(blendState_, rasterState_, depthStencilState_);
        rcGen.setShader(targetInfo_.vertexLightShader_.weakPtr<hShaderProgram>(), ShaderType_VERTEXPROG);
        rcGen.setShader(targetInfo_.pixelLightShader_.weakPtr<hShaderProgram>(), ShaderType_FRAGMENTPROG);
        rcGen.setPixelInputs(&samplerState_, 1, srv_.GetBuffer(), srv_.GetSize(), buffers_.GetBuffer(), buffers_.GetSize());
        rcGen.setStreamInputs(PRIMITIVETYPE_TRILIST, screenQuadIB_, screenQuadIB_->getIndexBufferType(), inputLayout_, &screenQuadVB_, 0, 1);
        rcGen.setDrawIndex(screenQuadIB_->GetIndexCount()/3, 0);
        rcGen.setReturn();

    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hBool hLightingManager::resourceUpdate(hResourceID , hResurceEvent event, hResourceManager*, hResourceClassBase*) {
        destroy();
        if (event == hResourceEvent_DBInsert) {
            generateRenderCommands(renderer_);
        }
        return hTrue;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hLightingManager::stopResourceEventListening() {
        if (targetInfo_.vertexLightShader_.getIsValid()) {
            targetInfo_.vertexLightShader_.unregisterForUpdates(hFUNCTOR_BINDMEMBER(hResourceEventProc, hLightingManager, resourceUpdate, this));
            targetInfo_.vertexLightShader_=hResourceHandle();
        }
        if (targetInfo_.pixelLightShader_.getIsValid()) {
            targetInfo_.pixelLightShader_.unregisterForUpdates(hFUNCTOR_BINDMEMBER(hResourceEventProc, hLightingManager, resourceUpdate, this));
            targetInfo_.pixelLightShader_=hResourceHandle();
        }
    }

}