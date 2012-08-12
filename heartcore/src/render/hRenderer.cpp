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

        hRenderDeviceSetup setup;
        setup.alloc_ = RnTmpMalloc;
        setup.free_ = RnTmpFree;
        ParentClass::Create( system_, width_, height_, bpp_, shaderVersion_, fullscreen_, vsync_, setup );

        ParentClass::InitialiseMainRenderSubmissionCtx( &mainSubmissionCtx_.impl_ );

        CreateIndexBuffer( NULL, RenderUtility::GetSphereMeshIndexCount( 16, 8 ), 0, PRIMITIVETYPE_TRILIST, &debugSphereIB_ );
        CreateVertexBuffer( NULL, RenderUtility::GetSphereMeshVertexCount( 16, 8 ), hrVF_XYZ, 0, &debugSphereVB_ );
        RenderUtility::BuildSphereMesh( 16, 8, 1.f, &mainSubmissionCtx_,debugSphereIB_, debugSphereVB_ );

        CreateIndexBuffer(NULL, 65535, RESOURCEFLAG_DYNAMIC, PRIMITIVETYPE_TRILIST, &volatileIBuffer_ );
        CreateVertexBuffer(NULL, 65535, ~0U, RESOURCEFLAG_DYNAMIC, &volatileVBuffer_ );

        //Create viewport/camera for debug drawing
        hRendererCamera* camera = GetRenderCamera(HEART_DEBUGUI_CAMERA_ID);
        hRenderViewportTargetSetup rtDesc;
        rtDesc.nTargets_ = 0;
        rtDesc.width_ = GetWidth();
        rtDesc.height_ = GetHeight();
        rtDesc.targetFormat_ = Heart::TFORMAT_ARGB8;
        rtDesc.hasDepthStencil_ = hFalse;
        rtDesc.depthFormat_ = Heart::TFORMAT_D24S8F;

        hViewport vp;
        vp.x_ = 0;
        vp.y_ = 0;
        vp.width_ = GetWidth();
        vp.height_ = GetHeight();

        camera->Initialise(this);
        camera->SetRenderTargetSetup(rtDesc);
        camera->SetFieldOfView( 45.f );
        camera->SetOrthoParams(0 ,0, GetWidth(), GetHeight(), 0.1f, 100.f);
        camera->SetViewMatrix( Heart::hMatrixFunc::identity() );
        camera->SetViewport(vp);
        camera->SetTechniquePass(techniqueManager_.GetRenderTechniqueInfo("main"));

	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hRenderer::Destroy()
	{	
        DestroyIndexBuffer(debugSphereIB_);
        DestroyVertexBuffer(debugSphereVB_);
        debugSphereIB_ = NULL;
        debugSphereVB_ = NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hRenderer::BeginRenderFrame()
	{
		//ReleasePendingRenderResources();
        //Start new frame

		// clear the render buffer
		++currentRenderStatFrame_;

        //Free last frame draw calls and temporary memory
        hAtomic::AtomicSet(scratchPtrOffset_, 0);
        hAtomic::AtomicSet(drawCallBlockIdx_, 0);
	}

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    
	void hRenderer::EndRenderFrame()
	{
        ParentClass::SwapBuffers();
        ParentClass::BeginRender(&gpuTime_);
    
        CollectAndSortDrawCalls();
        SubmitDrawCallsMT();

        ParentClass::EndRender();
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	hTexture* hRenderer::OnTextureLoad(hISerialiseStream* dataStream)
	{
        hTexture* resource = hNEW(GetGlobalHeap()/*!heap*/, hTexture)(this);
        hSerialiser ser;
        ser.Deserialise( dataStream, *resource );

        hMipDesc* dataPtrs = (hMipDesc*)hAlloca(sizeof(hMipDesc)*resource->nLevels_);

        for (hUint32 i = 0; i < resource->nLevels_; ++i)
        {
            dataPtrs[i].data   = resource->levelDescs_[i].mipdata_;
            dataPtrs[i].size   = resource->levelDescs_[i].mipdataSize_;
            dataPtrs[i].width  = resource->levelDescs_[i].width_;
            dataPtrs[i].height = resource->levelDescs_[i].height_;
        }

        hdTexture* dt = ParentClass::CreateTextrue( 
            resource->levelDescs_[0].width_, 
            resource->levelDescs_[0].height_, 
            resource->nLevels_, 
            resource->format_, 
            dataPtrs,
            0 );
        hcAssert(dt);
        resource->SetImpl(dt);

        if (!resource->GetKeepCPU())
        {
            resource->ReleaseCPUTextureData();
        }

 		return resource;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hRenderer::CreateVertexDeclaration( hVertexDeclaration*& pOut, hUint32 vtxFlags )
	{
// 		pOut = hNEW ( hRendererHeap ) hVertexDeclaration();
// 		pOut->SetImpl( hNEW ( hRendererHeap ) hdVtxDecl() );
// 		pOut->vtxFlags_ = vtxFlags;
// 
// 		pImpl()->GetVertexStrideOffset( vtxFlags, pOut->stride_, pOut->elementOffsets_ );
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hRenderer::DestroyVertexDeclaration( hVertexDeclaration* pVD )
	{
// 		hcAssert( IsRenderThread() );
// 
// 		pImpl()->DestroyVertexDeclaration( pVD->pImpl() );
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hRenderer::DestroyMaterial( hMaterial* pMat )
	{
// 		hcAssert( IsRenderThread() );
// 
// 		for ( hUint32 i = 0; i < pMat->nShaderParameters_; ++i )
// 		{
// 			if ( pMat->pShaderParameters_[i].boundTexture_.HasData() )
// 			{
// 				pMat->pShaderParameters_[i].boundTexture_.Release();
// 			}
// 		}
// 
// 		pImpl()->DestoryMaterial( pMat->pImpl() );
// 		delete pMat;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hRenderer::CreateTexture( hUint32 width, hUint32 height, hUint32 levels, hMipDesc* initialData, hTextureFormat format, hUint32 flags, hTexture** outTex )
	{
        hcAssert(levels > 0);

        (*outTex) = hNEW(GetGlobalHeap()/*!heap*/, hTexture)(this);

		(*outTex)->nLevels_ = levels;
		(*outTex)->format_ = format;
        (*outTex)->levelDescs_ = levels ? hNEW_ARRAY(GetGlobalHeap()/*!heap*/, hTexture::LevelDesc, levels) : NULL;

		for (hUint32 i = 0; i < levels; ++i)
		{
			(*outTex)->levelDescs_[ i ].width_ = initialData[i].width;
			(*outTex)->levelDescs_[ i ].height_ = initialData[i].height;
            if (initialData)
            {
                (*outTex)->levelDescs_[ i ].mipdata_ = initialData[i].data;
                (*outTex)->levelDescs_[ i ].mipdataSize_ = initialData[i].size;
            }
		}

		hdTexture* dt = ParentClass::CreateTextrue( width, height, levels, format, initialData, flags );
        hcAssert(dt);
        (*outTex)->SetImpl( dt );

        if ((flags & RESOURCEFLAG_KEEPCPUDATA) == 0)
        {
            (*outTex)->ReleaseCPUTextureData();
            (*outTex)->SetKeepCPU(hFalse);
        }
        else
        {
            (*outTex)->SetKeepCPU(hTrue);
        }
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////

	void hRenderer::DestroyTexture( hTexture* pOut )
	{
		//hcAssert( IsRenderThread() );

		ParentClass::DestroyTexture(pOut->pImpl());
        pOut->SetImpl(NULL);

        hDELETE_SAFE(GetGlobalHeap()/*!heap*/, pOut);
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hRenderer::CreateIndexBuffer( hUint16* pIndices, hUint16 nIndices, hUint32 flags, PrimitiveType primType, hIndexBuffer** outIB )
	{
		hIndexBuffer* pdata = hNEW(GetGlobalHeap()/*!heap*/, hIndexBuffer)(this);
		pdata->pIndices_ = NULL;
		pdata->nIndices_ = nIndices;
		pdata->primitiveType_ = primType;

		pdata->SetImpl( ParentClass::CreateIndexBufferDevice( nIndices*sizeof(hUint16), pIndices, flags ) );

        *outIB = pdata;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hRenderer::DestroyIndexBuffer( hIndexBuffer* pOut )
	{
		//hcAssert( IsRenderThread() );

		ParentClass::DestroyIndexBufferDevice( pOut->pImpl() );
        pOut->SetImpl(NULL);
		hDELETE_SAFE(GetGlobalHeap()/*!heap*/, pOut);
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hRenderer::CreateVertexBuffer( void* initData, hUint32 nElements, hUint32 layout, hUint32 flags, hVertexBuffer** outVB )
	{
        hVertexBuffer* pdata = hNEW(GetGlobalHeap()/*!heap*/, hVertexBuffer)(this);
        pdata->vtxCount_ = nElements;
        pdata->stride_ = ParentClass::ComputeVertexLayoutStride( layout );

        pdata->SetImpl( ParentClass::CreateVertexBufferDevice( layout, nElements*pdata->stride_, initData, flags ) );

        *outVB = pdata;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hRenderer::DestroyVertexBuffer( hVertexBuffer* pOut )
	{
		ParentClass::DestroyVertexBufferDevice(pOut->pImpl());
        pOut->SetImpl(NULL);
		hDELETE_SAFE(GetGlobalHeap()/*!heap*/, pOut);
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

    void* hRenderer::AllocTempRenderMemory( hUint32 size )
    {
        hUint32 ret;
        hAtomic::AtomicAddWithPrev(scratchPtrOffset_, size, &ret);
        return drawDataScratchBuffer_+ret;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hUint32 hRenderer::BeginCameraRender(hRenderSubmissionCtx* ctx, hUint32 camID)
    {
        hRendererCamera* camera = GetRenderCamera(camID);
        camera->UpdateParameters();
        hUint32 retTechMask = camera->GetTechniqueMask();

        ctx->SetRenderTarget(0, camera->GetRenderTarget(0));
        ctx->SetRenderTarget(1, camera->GetRenderTarget(1));
        ctx->SetRenderTarget(2, camera->GetRenderTarget(2));
        ctx->SetRenderTarget(3, camera->GetRenderTarget(3));
        ctx->SetDepthTarget(camera->GetDepthTarget());
        ctx->SetViewport(camera->GetViewport());

        ctx->SetConstantBuffer(camera->GetViewportConstantBlock());

        return retTechMask;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    int drawCallCompare(const void* lhs, const void* rhs)
    {
        return ((hDrawCall*)lhs)->sortKey_ < ((hDrawCall*)rhs)->sortKey_ ? -1 : 1;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderer::CollectAndSortDrawCalls()
    {
        //TODO: wait on job chain
        
        // POSSIBLE TODO: parallel merge sort
        qsort(drawCallBlocks_.GetBuffer(), drawCallBlockIdx_.value_, sizeof(hDrawCall), drawCallCompare);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderer::SubmitDrawCallsMT()
    {
        /* TODO: Multi-thread submit, but ST is debug-able */
        SubmitDrawCallsST();
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderer::SubmitDrawCallsST()
    {
        /*Single thread submit*/
        hUint32 camera = ~0U;
        const hMaterial* material = NULL;
        hUint32 pass = ~0U;
        hUint32 tmask = ~0U;
        hUint32 dcs = drawCallBlockIdx_.value_;
        for (hUint32 dc = 0; dc < dcs; ++dc)
        {
            hDrawCall* dcall = &drawCallBlocks_[dc];
            hMaterialInstance* mat = dcall->matInstance_;
            hUint32 nCam = (dcall->sortKey_&0xF000000000000000) >> 60;
            hUint32 nPass = (dcall->sortKey_&0xF);
            if (nCam != camera)
            {
                //Begin camera pass
                tmask = BeginCameraRender(&mainSubmissionCtx_, nCam);
                camera = nCam;
            }

            hMaterialTechnique* tech = mat->GetTechniqueByMask(tmask);

            if (material != mat->GetParentMaterial() || pass != nPass)
            {
                hMaterialTechnique* tech = mat->GetTechniqueByMask(tmask);
                hMaterialTechniquePass* techpass = tech->GetPass(nPass);
                mainSubmissionCtx_.SetVertexShader( techpass->GetVertexShader() );
                mainSubmissionCtx_.SetPixelShader( techpass->GetFragmentShader() );
                mainSubmissionCtx_.SetRenderStateBlock( techpass->GetBlendState() );
                mainSubmissionCtx_.SetRenderStateBlock( techpass->GetDepthStencilState() );
                mainSubmissionCtx_.SetRenderStateBlock( techpass->GetRasterizerState() );

                material = mat->GetParentMaterial();
                pass = nPass;
            }

            for ( hUint32 i = 0; i < mat->GetConstantBufferCount(); ++i )
            {
                mainSubmissionCtx_.SetConstantBuffer( mat->GetConstantBlock(i) );
            }

            for ( hUint32 i = 0; i < mat->GetSamplerCount(); ++i )
            {
                const hSamplerParameter* samp = mat->GetSamplerParameter( i );
                hcWarningHigh( samp->boundTexture_ == NULL, "Sampler has no texture bound" );
                if (samp->boundTexture_)
                    mainSubmissionCtx_.SetSampler( samp->samplerReg_, samp->boundTexture_, samp->samplerState_ );
            }


            hIndexBuffer* ib  = dcall->indexBuffer_;
            hVertexBuffer* vb = dcall->vertexBuffer_;
            if (dcall->immediate_)
            {
                hIndexBufferMapInfo ibmap;
                hVertexBufferMapInfo vbmap;
                mainSubmissionCtx_.Map(volatileIBuffer_, &ibmap);
                mainSubmissionCtx_.Map(volatileVBuffer_, &vbmap);

                //Copy
                hMemCpy(ibmap.ptr_, dcall->imIBBuffer_, dcall->ibSize_);
                hMemCpy(vbmap.ptr_, dcall->imVBBuffer_, dcall->vbSize_);

                mainSubmissionCtx_.Unmap(&ibmap);
                mainSubmissionCtx_.Unmap(&vbmap);

                ib = volatileIBuffer_;
                vb = volatileVBuffer_;
            }

            mainSubmissionCtx_.SetPrimitiveType(dcall->primType_);
            mainSubmissionCtx_.SetIndexStream(ib);
            mainSubmissionCtx_.SetVertexStream(0, vb, dcall->stride_);
            if (dcall->indexBuffer_ == NULL)
                mainSubmissionCtx_.DrawPrimitive(dcall->primCount_, dcall->startVertex_);
            else
                mainSubmissionCtx_.DrawIndexedPrimitive(dcall->primCount_, dcall->startVertex_);
        }
    }

}