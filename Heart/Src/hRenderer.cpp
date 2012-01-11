/********************************************************************
	created:	2008/06/22
	created:	22:6:2008   12:01
	filename: 	Renderer.cpp
	author:		James Moran
	
	purpose:	
*********************************************************************/

#include "Common.h"
#include "hRenderer.h"
#include "hStack.h"
#include "hClock.h"
#include "hVertexDeclarations.h"
#include "hResourceManager.h"
#include "hRenderState.h"
#include "hTexture.h"
#include "hIndexBuffer.h"
#include "hVertexBuffer.h"
#include "hMesh.h"
#include "hSystem.h"
#include "hRenderTargetTexture.h"
#include "hSerialiserFileStream.h"


namespace Heart
{

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void* hRenderer::pRenderThreadID_ = NULL;

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	hRenderer::hRenderer()
		: currentRenderStatFrame_( 0 )
		, pThreadFrameStats_( NULL )
		, statPass_( 0 )
	{
		SetImpl( hNEW ( hGeneralHeap ) hdRenderDevice );
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

        pImpl()->Create( system_, width_, height_, bpp_, shaderVersion_, fullscreen_, vsync_ );

		renderThread_.Begin( 
			"Rendering hThread",
			hThread::PRIORITY_NORMAL,
			Heart::Device::Thread::ThreadFunc::bind< hRenderer, &hRenderer::RenderThread >( this ), 
			NULL );

	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hRenderer::Destroy()
	{	
		hcAssert( renderThread_.IsComplete() );
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////

	hUint32 hRenderer::ReleasePendingTexturesResources()
	{
		hUint32 ret = 0;
		while( !texturesToRelease_.IsEmpty() )
		{
			hTextureBase* ptex = texturesToRelease_.peek();
			texturesToRelease_.pop();
			ptex->Release();
			++ret;
		}

		return ret;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////

	hUint32 hRenderer::ReleasePendingIndexBufferResources()
	{
		hUint32 ret = 0;
		while( !indexBuffersToRelease_.IsEmpty() )
		{
			hIndexBuffer* pib = indexBuffersToRelease_.peek();
			indexBuffersToRelease_.pop();
			pib->Release();
			++ret;
		}
		return ret;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////

	hUint32 hRenderer::ReleasePendingVertexBufferResources()
	{
		hUint32 ret = 0;
		while( !vertexBuffersToRelease_.IsEmpty() )
		{
			hVertexBuffer* pvb = vertexBuffersToRelease_.peek();
			vertexBuffersToRelease_.pop();
			pvb->Release();
			++ret;
		}
		return ret;
	}


	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	hUint32 hRenderer::ReleasePendingMeshResources()
	{
		hUint32 ret = 0;
		while ( !meshesToRelease_.IsEmpty() )
		{
			hMesh* pmesh = meshesToRelease_.peek();
			meshesToRelease_.pop();
#ifdef HEART_OLD_RENDER_SUBMISSION
			NewRenderCommand< Cmd::ReleaseMesh >( pmesh );
#endif // HEART_OLD_RENDER_SUBMISSION
			++ret;
		}
		return ret;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	hUint32 hRenderer::ReleasePendingMaterialResources()
	{
		hUint32 ret = 0;
		while( !materialsToRelease_.IsEmpty() )
		{
			hMaterial* pmat = materialsToRelease_.peek();
			materialsToRelease_.pop();
			//pmat->Release();
			++ret;
		}
		return ret;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hRenderer::BeginRenderFrame( hBool wait )
	{
		if ( wait )
		{
			while ( !renderCmdBuffer_.IsEmpty() )
			{
				Threading::ThreadYield();
			}
		}

		ReleasePendingRenderResources();

		// clear the render buffer
		++currentRenderStatFrame_;
	}

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    
	void hRenderer::EndRenderFrame()
	{
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	hUint32 hRenderer::ReleasePendingRenderResources()
	{
		hUint32 released = 0;
		released += ReleasePendingTexturesResources();
		released += ReleasePendingIndexBufferResources();
		released += ReleasePendingVertexBufferResources();
		released += ReleasePendingMeshResources();
		released += ReleasePendingMaterialResources();
		return released;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	hUint32 hRenderer::RenderThread( void* pParam )
	{
		pRenderThreadID_ = Threading::GetCurrentThreadID();
		hFloat frameCounter = 0.0f;
		hUint32 frames = 0;

        //TEMP:
        do {
            pImpl()->BeginRender();
            pImpl()->EndRender();
            pImpl()->SwapBuffers();
        } while( !renderThreadKill_.TryWait() );
/*
		renderStateCache_->defaultRenderState();

		//pImpl()->ActivateContext();

		hBool quitWaiting = hFalse;
		for(;;)
		{
			CreatePendingResources();

			if ( !renderCmdBuffer_.IsEmpty() )
			{
				Cmd::RenderCmdBase* pCmd = (Cmd::RenderCmdBase*)renderCmdBuffer_.Peek();
				//check for invalid size, not definite but normally happens when
				//size_ is not set correctly
				hcAssert( pCmd->size_ < ( DEFAULT_CMD_BUF_SIZE / 4) );
				hUint32 sizetopop = pCmd->size_;

				if ( pCmd->size_ == 0 )
				{
					//end of frame
					++frames;
					frameCounter += hClock::deltams();

					if ( frameCounter > 1000.0f )
					{
						FPS_ = frames;
						frameCounter -= 1000.0f;
						frames = 0;
					}

					sizetopop = sizeof( Cmd::EndFrame );
				}
				else 
				{
					hcAssert( pCmd->breakMe_ == hFalse );

					pCmd->Execute( this );

#ifdef HEART_COLLECT_RENDER_STATS
					//Do This After execute, as the first command  in a frame
					//should set the correct framestats pointer
					hRenderFrameStats* stats = pFrameStats();
					if ( stats )
					{
						++stats->nRenderCmds_;
					}
#endif


					//clean up
					if ( pCmd->DestroyCommand() )
					{
						pCmd->~RenderCmdBase();
					}
				}

				renderCmdBuffer_.Pop( sizetopop );
			}
			else
			{
				//Yield to another thread
				Threading::ThreadYield();
			}

			// Check for the kill command
			// We cannot leave until the last render commands have been push
			// as the renderer my have to clean up resources
			if ( renderThreadKill_.TryWait() == hTrue) 
			{
				quitWaiting = hTrue;
			}

			if ( quitWaiting && renderCmdBuffer_.IsEmpty() )
			{
				break;
			}
		}

		vertexDeclManager_->Destroy();

		pImpl()->Destroy();
        */

		return 0;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	hResourceClassBase* hRenderer::OnTextureLoad( const hChar* ext, hSerialiserFileStream* dataStream, hResourceManager* resManager )
	{
        hTexture* resource = hNEW ( hGeneralHeap ) hTexture( this );
        hSerialiser ser;
        ser.Deserialise( dataStream, *resource );
        for ( hUint32 i = 0; i < resource->nLevels_; ++i )
        {
            HEART_RESOURCE_DATA_FIXUP( void*, resource->textureData_, resource->levelDescs_[i].mipdata_ );
        }

 		return resource;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	hUint32 hRenderer::OnTextureUnload( const hChar* ext, hResourceClassBase* resource, hResourceManager* resManager )
	{
        delete resource;
		return 0;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	hResourceClassBase* hRenderer::OnMaterialLoad( const hChar* ext, hSerialiserFileStream* dataStream, hResourceManager* resManager )
	{
        hMaterial* resource = hNEW ( hGeneralHeap ) hMaterial( this );
        hSerialiser ser;
        ser.Deserialise( dataStream, *resource );

        //Fixup dependencies
        resManager->LockResourceDatabase();
        hUint32 nTech = resource->techniques_.GetSize();
        for ( hUint32 tech = 0; tech < nTech; ++tech )
        {
            hUint32 nPasses = resource->techniques_[tech].passes_.GetSize();
            for ( hUint32 pass = 0; pass < nPasses; ++pass )
            {
                hUint32 vpid = (hUint32)resource->techniques_[tech].passes_[pass].vertexProgram_;
                hUint32 fpid = (hUint32)resource->techniques_[tech].passes_[pass].fragmentProgram_;
                hShaderProgram* vp = static_cast< hShaderProgram* >( resManager->GetResource( vpid ) );
                hShaderProgram* fp = static_cast< hShaderProgram* >( resManager->GetResource( fpid ) );

                resource->techniques_[tech].passes_[pass].vertexProgram_   = vp;
                resource->techniques_[tech].passes_[pass].fragmentProgram_ = fp;

                hdShaderProgram* prog = vp->pImpl();
                for ( hUint32 i = 0; i < prog->GetConstantBufferCount(); ++i )
                {
                    resource->AddConstBufferDesc( prog->GetConstantBufferName( i ), prog->GetConstantBufferSize( i ) );
                }
                hUint32 parameterCount = vp->GetParameterCount();
                for ( hUint32 i = 0; i < parameterCount; ++i )
                {
                    hShaderParameter param;
                    hBool ok = prog->GetShaderParameter( i, &param );
                    hcAssertMsg( ok, "Shader Parameter Look up Out of Bounds" );
                    resource->FindOrAddShaderParameter( param, prog->GetShaderParameterDefaultValue( i ) );
                }

                prog = fp->pImpl();
                for ( hUint32 i = 0; i < prog->GetConstantBufferCount(); ++i )
                {
                    resource->AddConstBufferDesc( prog->GetConstantBufferName( i ), prog->GetConstantBufferSize( i ) );
                }
                parameterCount = fp->GetParameterCount();
                for ( hUint32 i = 0; i < parameterCount; ++i )
                {
                    hShaderParameter param;
                    hBool ok = prog->GetShaderParameter( i, &param );
                    hcAssertMsg( ok, "Shader Parameter Look up Out of Bounds" );
                    resource->FindOrAddShaderParameter( param, prog->GetShaderParameterDefaultValue( i ) );
                }

                CreateBlendState( resource->techniques_[tech].passes_[pass].blendStateDesc_, &resource->techniques_[tech].passes_[pass].blendState_ );
                CreateRasterizerState( resource->techniques_[tech].passes_[pass].rasterizerStateDesc_, &resource->techniques_[tech].passes_[pass].rasterizerState_ );
                CreateDepthStencilState( resource->techniques_[tech].passes_[pass].depthStencilStateDesc_, &resource->techniques_[tech].passes_[pass].depthStencilState_ );
            }
        }
        hUint32 nSamp = resource->samplers_.GetSize();
        for ( hUint32 samp = 0; samp < nSamp; ++samp )
        {
            if ( resource->samplers_[samp].boundTexture_ )
            {
                hUint32 sid = (hUint32)resource->samplers_[samp].boundTexture_;
                resource->samplers_[samp].boundTexture_ = static_cast< hTexture* >( resManager->GetResource( sid ) );
                CreateSamplerState( resource->samplers_[samp].samplerDesc_, &resource->samplers_[samp].samplerState_ );
            }
        }
        resManager->UnlockResourceDatabase();

		return resource;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	hUint32 hRenderer::OnMaterialUnload( const hChar* ext, hResourceClassBase* resource, hResourceManager* resManager )
	{
		delete resource;
		return 0;
	}

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hResourceClassBase* hRenderer::OnShaderProgramLoad( const hChar* ext, hSerialiserFileStream* dataStream, hResourceManager* resManager )
    {
        hShaderProgram* resource = hNEW ( hGeneralHeap ) hShaderProgram;
        hSerialiser ser;
        ser.Deserialise( dataStream, *resource );

        resource->SetImpl( pImpl()->CompileShader( (hChar*)resource->shaderProgram_, resource->shaderProgramLength_, resource->vertexInputLayoutFlags_, resource->shaderType_ ) );
     
        return resource;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hUint32 hRenderer::OnShaderProgramUnload( const hChar* ext, hResourceClassBase* resource, hResourceManager* resManager )
    {
        delete resource;
        return 0;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////

	hUint32 hRenderer::OnIndexBufferLoad( const hChar* pExt, void* pLoadedData, void* pUserData, Heart::hResourceManager* pResourceManager )
	{
// 		hIndexBuffer* pIB = new (pLoadedData) hIndexBuffer( this );
// 		hByte* pData = (hByte*)pLoadedData;
// 
// 		pIB->SetImpl( (hdIndexBuffer*)( pIB + 1 ) );
// 		new ( pImpl() ) hdIndexBuffer();
// 
// 		hcAssert( pIB->pIndices_ );
// 		HEART_RESOURCE_DATA_FIXUP( hUint16, pData, pIB->pIndices_ );
// 		pIB->renderer_ = this;
// 
// 		pImpl()->CreateIndexBuffer( pIB->pImpl(), pIB->pIndices_, pIB->nIndices_, pIB->primitiveType_, 0 );
// 
// 		pIB->IsDiskResource( true );

		return 0;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////

	hUint32 hRenderer::OnIndexBufferUnload( const hChar* pExt, void* pLoadedData, void* pUserData, Heart::hResourceManager* pResourceManager )
	{
// 		hIndexBuffer* pIB = (hIndexBuffer*)pLoadedData;
// 		while( indexBuffersToRelease_.IsFull() )
// 		{
// 			Threading::ThreadSleep( 1 );
// 		}
// 		indexBuffersToRelease_.push( pIB );
		return 1;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////

	hUint32 hRenderer::OnVertexBufferLoad( const hChar* pExt, void* pLoadedData, void* pUserData, Heart::hResourceManager* pResourceManager )
	{
// 		hVertexBuffer* pVB = new ( pLoadedData ) hVertexBuffer( this );
// 		hByte* pData = (hByte*)pLoadedData;
// 
// 		pVB->SetImpl( (hdVtxBuffer*)( pVB + 1 ) );
// 		new ( pImpl() ) hdVtxBuffer();
// 
// 		hcAssert( pVB->pVtxBuffer_ );
// 
// 		pVB->renderer_ = this;
// 		GetVertexDeclaration( pVB->pVtxDecl_, (hUint32)pVB->pVtxDecl_ );
// 		HEART_RESOURCE_DATA_FIXUP( void, pData, pVB->pVtxBuffer_ );
// 
// 		pImpl()->CreateVertexBuffer( pVB->pImpl(), pVB->vtxBufferSize_ / pVB->pVtxDecl_->Stride(), pVB->pVtxDecl_->Stride(), 0 );
// 
// 		//pVB->Lock();
// 		//pVB->SetData( 0, 0, 0, pVB->pVtxBuffer_, pVB->vtxBufferSize_ );
// 		//pVB->Unlock();
// 		//TODO: Use a unbound flag to get this applied on first render?
// 		pVB->FlushVertexData( pVB->pVtxBuffer_, pVB->vtxBufferSize_ );
// 
// 		pVB->IsDiskResource( true );

		return 0;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////

	hUint32 hRenderer::OnVertexBufferUnload( const hChar* pExt, void* pLoadedData, void* pUserData, Heart::hResourceManager* pResourceManager )
	{
// 		hVertexBuffer* pVB = (hVertexBuffer*)pLoadedData;
// 		while( vertexBuffersToRelease_.IsFull() )
// 		{
// 			Threading::ThreadSleep( 1 );
// 		}
// 		vertexBuffersToRelease_.push( pVB );
		return 1;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	hUint32 hRenderer::OnMeshLoad( const hChar* pExt, void* pLoadedData, void* pUserData, hResourceManager* pResourceManager_ )
	{
// 		hMesh* pmesh = (hMesh*)pLoadedData;
// 		hByte* pData = (hByte*)pLoadedData;
// 
// 		//Build the vtable
// 		new ( pmesh ) hMesh();
// 
// // 		pDepsList->GetDependency( pmesh->indices_ );
// // 		pDepsList->GetDependency( pmesh->vertices_ );
// // 		pDepsList->GetDependency( pmesh->material_ );
// 
// 		pmesh->nPrimatives_ = pmesh->indices_->IndexCount() / 3;

		return 0;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	hUint32 hRenderer::OnMeshUnload( const hChar* pExt, void* pLoadedData, void* pUserData, hResourceManager* pResourceManager_ )
	{
// 		hMesh* pmesh = (hMesh*)pLoadedData;
// 
// 		if ( pmesh->indices_.HasData() )
// 		{
// 			pmesh->indices_.Release();
// 		}
// 		if ( pmesh->vertices_.HasData() )
// 		{
// 			pmesh->vertices_.Release();
// 		}
// 		if ( pmesh->material_.HasData() )
// 		{
// 			pmesh->material_.Release();
// 		}
// 
// 		while( meshesToRelease_.IsFull() )
// 		{
// 			Threading::ThreadSleep( 1 );
// 		}
// 		meshesToRelease_.push( pmesh );
		return 1;
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

	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////

	void hRenderer::CreatePendingResources()
	{
// 		while ( !textureToCreate_.IsEmpty() )
// 		{
// 			hTexture* tex = textureToCreate_.peek();
// 			pImpl()->CreateTexture( tex->pImpl(), tex->levelDescs_[ 0 ].width_, tex->levelDescs_[ 0 ].height_, tex->nLevels_, tex->format_ );
// 
// 			hAtomic::LWMemoryBarrier();
// 			tex->created_ = hTrue;
// 			textureToCreate_.pop();
// 		}
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

	void hRenderer::CreateTexture( hUint32 width, hUint32 height, hUint32 levels, TextureFormat format, hUint32 flags, hTexture** outTex )
	{
		(*outTex) = hNEW ( hGeneralHeap ) hTexture( this );

		(*outTex)->nLevels_ = levels;
		(*outTex)->format_ = format;
		(*outTex)->levelDescs_ = hNEW ( hRendererHeap ) hTexture::LevelDesc[ levels ];
		(*outTex)->textureData_ = NULL;

		hUint32 tw = width;
		hUint32 th = height;
		for ( hUint32 i = 0; i < levels; ++i, tw >>= 1, th >>= 1 )
		{
			(*outTex)->levelDescs_[ i ].width_ = tw;
			(*outTex)->levelDescs_[ i ].height_ = th;
		}

		hdTexture* dt = pImpl()->CreateTextrue( width, height, levels, format, NULL, 0, flags );
        (*outTex)->SetImpl( dt );
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////

	void hRenderer::DestroyTexture( hTexture* pOut )
	{
		hcAssert( IsRenderThread() );

		pImpl()->DestroyTexture( pOut->pImpl() );
		delete pOut;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hRenderer::CreateIndexBuffer( hUint16* pIndices, hUint16 nIndices, hUint32 flags, PrimitiveType primType, hIndexBuffer** outIB )
	{
		hIndexBuffer* pdata = hNEW ( hRendererHeap ) hIndexBuffer( this );
		pdata->pIndices_ = NULL;
		pdata->nIndices_ = nIndices;
		pdata->primitiveType_ = primType;

		pdata->SetImpl( pImpl()->CreateIndexBuffer( nIndices*sizeof(hUint16), pIndices, flags ) );

        *outIB = pdata;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hRenderer::DestroyIndexBuffer( hIndexBuffer* pOut )
	{
// 		hcAssert( IsRenderThread() );
// 
// 		pImpl()->DestoryIndexBuffer( pOut->pImpl() );
// 		delete pOut;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hRenderer::CreateVertexBuffer( void* initData, hUint32 nElements, hUint32 stride, hUint32 layout, hUint32 flags, hVertexBuffer** outVB )
	{
        hVertexBuffer* pdata = hNEW ( hRendererHeap ) hVertexBuffer( this );
        pdata->vtxCount_ = nElements;
        pdata->stride_ = stride;

        pdata->SetImpl( pImpl()->CreateVertexBuffer( layout, nElements*stride, initData, flags ) );
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hRenderer::DestoryVertexBuffer( hVertexBuffer* pOut )
	{
// 		hcAssert( IsRenderThread() );
// 
// 		pImpl()->DestoryVertexBuffer( pOut->pImpl() );
// 		delete pOut;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void* hRenderer::AquireTempRenderMemory( hUint32 size )
	{
		//TODO: double buffer this in some way and make is fast
		return hRendererHeap.alignAlloc( size, 16 );
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hRenderer::ReleaseTempRenderMemory( void* ptr )
	{
		//TODO: double buffer this in some way and make it fast
		hRendererHeap.release( ptr );
	}

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hRenderSubmissionCtx* hRenderer::CreateRenderSubmissionCtx()
    {
        hRenderSubmissionCtx* ret = hNEW( hRendererHeap ) hRenderSubmissionCtx();
        pImpl()->InitialiseRenderSubmissionCtx( &ret->impl_ );
        return ret;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderer::DestroyRenderSubmissionCtx( hRenderSubmissionCtx* ctx )
    {
        hcAssert( ctx );
        pImpl()->DestroyRenderSubmissionCtx( &ctx->impl_ );
        delete ctx;
    }

}