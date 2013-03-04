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
        return GetGlobalHeap()/*!heap*/->alignAlloc( size, 16 );
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    
    void RnTmpFree( void* ptr )
    {
        GetGlobalHeap()/*!heap*/->release( ptr );
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

        hAtomic::AtomicSet(scratchPtrOffset_, 0);
        hAtomic::AtomicSet(drawCallBlockIdx_, 0);

        backBuffer_=hNEW(GetGlobalHeap(), hTexture)(this, GetGlobalHeap());
        depthBuffer_=hNEW(GetGlobalHeap(), hTexture)(this, GetGlobalHeap());

        hRenderDeviceSetup setup;
        setup.alloc_ = RnTmpMalloc;
        setup.free_ = RnTmpFree;
        setup.backBufferTex_ = backBuffer_;
        setup.depthBufferTex_= depthBuffer_;
        ParentClass::Create( system_, width_, height_, fullscreen_, vsync_, setup );

        ParentClass::InitialiseMainRenderSubmissionCtx(&mainSubmissionCtx_.impl_);

        backBuffer_->nLevels_=1;
        backBuffer_->levelDescs_=hNEW_ARRAY(GetGlobalHeap(), hTexture::LevelDesc, 1);
        backBuffer_->levelDescs_->mipdata_=NULL;
        backBuffer_->levelDescs_->mipdataSize_=0;
        backBuffer_->levelDescs_->width_ = GetWidth();
        backBuffer_->levelDescs_->height_ = GetHeight();
        depthBuffer_->nLevels_=1;
        depthBuffer_->levelDescs_=hNEW_ARRAY(GetGlobalHeap(), hTexture::LevelDesc, 1);
        depthBuffer_->levelDescs_->mipdata_=NULL;
        depthBuffer_->levelDescs_->mipdataSize_=0;
        depthBuffer_->levelDescs_->width_ = GetWidth();
        depthBuffer_->levelDescs_->height_ = GetHeight();


        const hChar* bbalias[] = {
            "back_buffer",
            "g_back_buffer",
        };
        materialManager_.registerGlobalTexture("back_buffer", backBuffer_, bbalias, hStaticArraySize(bbalias));
        const hChar* dbalias[] = {
            "depth_buffer",
            "g_depth_buffer",
            "z_buffer",
            "g_z_buffer",
        };
        materialManager_.registerGlobalTexture("depth_buffer", depthBuffer_, dbalias, hStaticArraySize(dbalias));

        createDebugShadersInternal();

    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderer::Destroy()
    {
        materialManager_.destroyRenderResources();
        for (hUint i=0; i<eDebugShaderMax; ++i) {
            DestroyShader(GetGlobalHeap(), debugShaders_[i]);
            hDELETE_SAFE(GetGlobalHeap(), debugShaders_[i]);
        }
        ParentClass::Destroy();
        hDELETE_ARRAY_SAFE(GetGlobalHeap(), backBuffer_->levelDescs_);
        hDELETE_SAFE(GetGlobalHeap(), backBuffer_);
        hDELETE_ARRAY_SAFE(GetGlobalHeap(), depthBuffer_->levelDescs_);
        hDELETE_SAFE(GetGlobalHeap(), depthBuffer_);
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

        ParentClass::EndRender();
        ParentClass::SwapBuffers();

        backBuffer_->levelDescs_->width_ = GetWidth();
        backBuffer_->levelDescs_->height_ = GetHeight();
        depthBuffer_->levelDescs_->width_ = GetWidth();
        depthBuffer_->levelDescs_->height_ = GetHeight();
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderer::createTexture(hUint32 levels, hMipDesc* initialData, hTextureFormat format, hUint32 flags, hMemoryHeapBase* heap, hTexture** outTex)
    {
        hcAssert(initialData);
        hcAssert(levels > 0);

        (*outTex) = hNEW(heap, hTexture)(this, heap);

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
        ParentClass::destroyTextureDevice(pOut);
        hDELETE_SAFE(pOut->heap_, pOut);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderer::CreateIndexBuffer(void* pIndices, hUint32 nIndices, hUint32 flags, hIndexBuffer** outIB)
    {
        hUint elementSize= nIndices > 0xFFFF ? sizeof(hUint32) : sizeof(hUint16);
        hIndexBuffer* pdata = hNEW(GetGlobalHeap(), hIndexBuffer)(this);
        pdata->pIndices_ = NULL;
        pdata->nIndices_ = nIndices;
        pdata->type_= nIndices > 0xFFFF ? hIndexBufferType_Index32 : hIndexBufferType_Index16;

        ParentClass::createIndexBufferDevice(nIndices*elementSize, pIndices, flags, pdata);

        *outIB = pdata;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderer::DestroyIndexBuffer(hIndexBuffer* ib)
    {
        hcAssert(ib);
        ParentClass::destroyIndexBufferDevice(ib);
        hDELETE_SAFE(GetGlobalHeap(), ib);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderer::CreateVertexBuffer(void* initData, hUint32 nElements, hInputLayoutDesc* desc, hUint32 desccount, hUint32 flags, hMemoryHeapBase* heap, hVertexBuffer** outVB)
    {
        hVertexBuffer* pdata = hNEW(heap, hVertexBuffer)(heap);
        pdata->vtxCount_ = nElements;
        pdata->stride_ = ParentClass::computeVertexLayoutStride( desc, desccount );
        ParentClass::createVertexBufferDevice(desc, desccount, nElements*pdata->stride_, initData, flags, pdata);
        *outVB = pdata;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderer::DestroyVertexBuffer(hVertexBuffer* vb)
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

    void hRenderer::SumbitResourceUpdateCommand( const hRenderResourceUpdateCmd& cmd )
    {
        hcAssertMsg(drawResourceUpdateCalls_.value_ < s_resoruceUpdateLimit, "Too many resource updates calls");
        if (drawResourceUpdateCalls_.value_ >= s_resoruceUpdateLimit)
            return;

        hUint32 wIdx;
        hAtomic::AtomicAddWithPrev(drawResourceUpdateCalls_, 1, &wIdx);

        //copy calls
        drawResourceUpdates_[wIdx] = cmd;
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

    void* hRenderer::AllocTempRenderMemory( hUint32 size )
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

        for (hUint32 dc = 0; dc < dcs; ++dc)
        {
            hDrawCall* dcall = &drawCallBlocks_[dc];
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

            hBool newMaterial = matKey != lastMatKey;
            lastMatKey = matKey;
            //if (newMaterial){ //TODO flush correctly
            mainSubmissionCtx_.SetRenderStateBlock(dcall->blendState_);
            mainSubmissionCtx_.SetRenderStateBlock(dcall->depthState_);
            mainSubmissionCtx_.SetRenderStateBlock(dcall->rasterState_);

            mainSubmissionCtx_.SetRenderInputObject(dcall->progInput_);
            mainSubmissionCtx_.SetInputStreams(&dcall->streams_);
            if (dcall->instanceCount_) {
                mainSubmissionCtx_.DrawIndexedPrimitiveInstanced(dcall->instanceCount_, dcall->drawPrimCount_, 0);
            }else{
                mainSubmissionCtx_.DrawIndexedPrimitive(dcall->drawPrimCount_, 0);
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

        //Create viewport/camera for debug drawing
        hRendererCamera* camera = GetRenderCamera(HEART_DEBUGUI_CAMERA_ID);
        hRenderViewportTargetSetup rtDesc = {0};
        rtDesc.nTargets_=1;
        rtDesc.targets_[0]=materialManager_.getGlobalTexture("back_buffer");
        rtDesc.depth_=materialManager_.getGlobalTexture("depth_buffer");

        hRelativeViewport vp;
        vp.x=0.f;
        vp.y=0.f;
        vp.w=1.f;
        vp.h=1.f;

        camera->SetRenderTargetSetup(rtDesc);
        camera->SetFieldOfView(45.f);
        camera->SetOrthoParams(0.f, 0.f, (hFloat)GetWidth(), (hFloat)GetHeight(), 0.1f, 100.f);
        camera->SetViewMatrix( Heart::hMatrixFunc::identity() );
        camera->setViewport(vp);
        camera->SetTechniquePass(materialManager_.GetRenderTechniqueInfo("main"));
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
        debugShaders_[eDebugPixelWhite]=hNEW(GetGlobalHeap(), hShaderProgram)(this);
        ParentClass::compileShaderFromSource(
            GetGlobalHeap(), ParentClass::getDebugShaderSource(eDebugPixelWhite),
            hStrLen(ParentClass::getDebugShaderSource(eDebugPixelWhite)),
            "mainFP", eShaderProfile_ps4_0, debugShaders_[eDebugPixelWhite]);
        debugShaders_[eDebugVertexPosOnly]=hNEW(GetGlobalHeap(), hShaderProgram)(this);
        ParentClass::compileShaderFromSource(
            GetGlobalHeap(), ParentClass::getDebugShaderSource(eDebugVertexPosOnly),
            hStrLen(ParentClass::getDebugShaderSource(eDebugVertexPosOnly)),
            "mainVP", eShaderProfile_vs4_0, debugShaders_[eDebugVertexPosOnly]);
        debugShaders_[eConsoleVertex]=hNEW(GetGlobalHeap(), hShaderProgram)(this);
        debugShaders_[eConsoleVertex]->shaderType_=ShaderType_VERTEXPROG;
        ParentClass::compileShaderFromSource(
            GetGlobalHeap(), ParentClass::getDebugShaderSource(eConsoleVertex),
            hStrLen(ParentClass::getDebugShaderSource(eConsoleVertex)),
            "mainVP", eShaderProfile_vs4_0, debugShaders_[eConsoleVertex]);
        debugShaders_[eConsolePixel]=hNEW(GetGlobalHeap(), hShaderProgram)(this);
        ParentClass::compileShaderFromSource(
            GetGlobalHeap(), ParentClass::getDebugShaderSource(eConsolePixel),
            hStrLen(ParentClass::getDebugShaderSource(eConsolePixel)),
            "mainFP", eShaderProfile_ps4_0, debugShaders_[eConsolePixel]);
        debugShaders_[eDebugFontVertex]=hNEW(GetGlobalHeap(), hShaderProgram)(this);
        ParentClass::compileShaderFromSource(
            GetGlobalHeap(), ParentClass::getDebugShaderSource(eDebugFontVertex),
            hStrLen(ParentClass::getDebugShaderSource(eDebugFontVertex)),
            "mainVP", eShaderProfile_vs4_0, debugShaders_[eDebugFontVertex]);
        debugShaders_[eDebugFontPixel]=hNEW(GetGlobalHeap(), hShaderProgram)(this);
        ParentClass::compileShaderFromSource(
            GetGlobalHeap(), ParentClass::getDebugShaderSource(eDebugFontPixel),
            hStrLen(ParentClass::getDebugShaderSource(eDebugFontPixel)),
            "mainFP", eShaderProfile_ps4_0, debugShaders_[eDebugFontPixel]);
        debugShaders_[eDebugVertexPosNormal]=hNEW(GetGlobalHeap(), hShaderProgram)(this);
        ParentClass::compileShaderFromSource(
            GetGlobalHeap(), ParentClass::getDebugShaderSource(eDebugVertexPosNormal),
            hStrLen(ParentClass::getDebugShaderSource(eDebugVertexPosNormal)),
            "mainVP", eShaderProfile_vs4_0, debugShaders_[eDebugVertexPosNormal]);
        debugShaders_[eDebugPixelWhiteViewLit]=hNEW(GetGlobalHeap(), hShaderProgram)(this);
        ParentClass::compileShaderFromSource(
            GetGlobalHeap(), ParentClass::getDebugShaderSource(eDebugPixelWhiteViewLit),
            hStrLen(ParentClass::getDebugShaderSource(eDebugPixelWhiteViewLit)),
            "mainFP", eShaderProfile_ps4_0, debugShaders_[eDebugPixelWhiteViewLit]);
        debugShaders_[eDebugTexVertex]=hNEW(GetGlobalHeap(), hShaderProgram)(this);
        ParentClass::compileShaderFromSource(
            GetGlobalHeap(), ParentClass::getDebugShaderSource(eDebugTexVertex),
            hStrLen(ParentClass::getDebugShaderSource(eDebugTexVertex)),
            "mainVP", eShaderProfile_vs4_0, debugShaders_[eDebugTexVertex]);
        debugShaders_[eDebugTexPixel]=hNEW(GetGlobalHeap(), hShaderProgram)(this);
        ParentClass::compileShaderFromSource(
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

}