/********************************************************************
	created:	2008/06/15
	created:	15:6:2008   1:17
	filename: 	Renderer.h
	author:		James Moran
	
	purpose:	
*********************************************************************/
#ifndef hrRenderer_h__
#define hrRenderer_h__

#include "hTypes.h"
#include "hMemory.h"
#include "hMath.h"
#include "hThread.h"
#include "hLocklessComPipe.h"
#include "hSemaphore.h"
#include "hThreadEvent.h"
#include "hResource.h"
#include "hRendererConstants.h"
#include "hDebugRenderer.h"
#include "hRenderSubmissionContext.h"

namespace Heart
{

	class hResourceManager;
	class hResourceDependencyList;
    class hSerialiserFileStream;
	class hSystem;
	class hRenderState;
	struct hViewport;
	class hTexture;
    struct hTextureMapInfo;
	class hTextureBase;
	class hRenderTargetTexture;
	class DepthSurface;
	class hVertexDeclaration;
	class hMaterial;
	class hIndexBuffer;
    struct hIndexBufferMapInfo;
	class hVertexBuffer;
    struct hVertexBufferMapInfo;
	class hMesh;
	
	struct hRenderFrameStats
	{
		hUint32			nTriangels_;
		hUint32			nRenderCmds_;
		hUint32			nDrawCalls_;
	};

	struct hRenderFrameStatsCollection
	{
		static const hUint32 MAX_PASSES = 8;
		hRenderFrameStats passes_[ MAX_PASSES ];
	};

	class hRenderer : public pimpl< hdRenderDevice >
	{
	public:

		static const hUint32									DEFAULT_CMD_BUF_SIZE = 1024 * 1024;
		static const hUint32									DEFAULT_MODIFY_BUFFER_SIZE = 1024*1024*8;
		static const hUint32									MAX_BONES = 12;
		static const hUint32									RT_BACK_BUFFER = hErrorCode;
		static const hUint32									MAX_RT = 8;
		static const hUint32									MAX_RESOURCES = 128;
		static const hUint32									NUM_RENDER_BUFFERS = NUM_RENDER_BUFFERS;

		hRenderer();
		void													Create( hSystem* pSystem, hUint32 width, hUint32 height, hUint32 bpp, hFloat shaderVersion, hBool fullscreen, hBool vsync, hResourceManager* pResourceManager );
		void													Destroy();
		///////////////////////////////////////////////////////////////////////////////////////////////////
		///////////////////////////////////////////////////////////////////////////////////////////////////
		///////////////////////////////////////////////////////////////////////////////////////////////////
		hRenderFrameStatsCollection*							LastRenderFrameStats() { return rendererStats_ + (currentRenderStatFrame_ % 2); }

		//default resource loaders
		hResourceClassBase*										OnTextureLoad( const hChar* ext, hSerialiserFileStream* dataStream, hResourceManager* resManager );
		hUint32													OnTextureUnload( const hChar* ext, hResourceClassBase* resource, hResourceManager* resManager );
		hResourceClassBase*										OnMaterialLoad( const hChar* ext, hSerialiserFileStream* dataStream, hResourceManager* resManager );
		hUint32													OnMaterialUnload( const hChar* ext, hResourceClassBase* resource, hResourceManager* resManager );
        hResourceClassBase*										OnShaderProgramLoad( const hChar* ext, hSerialiserFileStream* dataStream, hResourceManager* resManager );
        hUint32													OnShaderProgramUnload( const hChar* ext, hResourceClassBase* resource, hResourceManager* resManager );
		hUint32													OnIndexBufferLoad( const hChar* pExt, void* pLoadedData, void* pUserData, hResourceManager* pResourceManager );
		hUint32													OnIndexBufferUnload( const hChar* pExt, void* pLoadedData, void* pUserData, hResourceManager* pResourceManager );
		hUint32													OnVertexBufferLoad( const hChar* pExt, void* pLoadedData, void* pUserData, hResourceManager* pResourceManager );
		hUint32													OnVertexBufferUnload( const hChar* pExt, void* pLoadedData, void* pUserData, hResourceManager* pResourceManager );
		hUint32													OnMeshLoad( const hChar* pExt, void* pLoadedData, void* pUserData, hResourceManager* pResourceManager_ );
		hUint32													OnMeshUnload( const hChar* pExt, void* pLoadedData, void* pUserData, hResourceManager* pResourceManager_ );
        //////////////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////
        hRenderSubmissionCtx*                                   CreateRenderSubmissionCtx();
        void                                                    DestroyRenderSubmissionCtx( hRenderSubmissionCtx* ctx );
		/*
			pimpl methods
		*/
        void                                                    CreateBlendState( const hBlendStateDesc& desc, hdBlendState** state ) { *state = pImpl()->CreateBlendState( desc ); }
        void                                                    DestroyBlendState( hdBlendState* state ) { pImpl()->DestroyBlendState( state ); } 
        void                                                    CreateRasterizerState( const hRasterizerStateDesc& desc, hdRasterizerState** state ) { *state = pImpl()->CreateRasterizerState( desc ); }
        void                                                    DestoryRasterizerState( hdRasterizerState* state ) { pImpl()->DestoryRasterizerState( state ); } 
        void                                                    CreateDepthStencilState( const hDepthStencilStateDesc& desc, hdDepthStencilState** state ) { *state = pImpl()->CreateDepthStencilState( desc ); }
        void                                                    DestoryDepthStencilState( hdDepthStencilState* state ) { pImpl()->DestoryDepthStencilState( state ); } 
        void                                                    CreateSamplerState( const hSamplerStateDesc& desc, hdSamplerState** state ) { *state = pImpl()->CreateSamplerState( desc ); }
        void                                                    DestorySamplerState( hdSamplerState* state ) { pImpl()->DestroySamplerState( state ); } 
        hdParameterConstantBlock*                               CreateConstantBuffers( const hUint32* sizes, hUint32 count ) { return pImpl()->CreateConstantBlocks( sizes, count ); }
        void                                                    UpdateConstantBlockParameters( hdParameterConstantBlock* constBlock, hShaderParameter* params, hUint32 parameters ) { pImpl()->UpdateConstantBlockParameters( constBlock, params, parameters ); }
        void                                                    DestroyConstantBuffers( hdParameterConstantBlock* blocks, hUint32 count ) { pImpl()->DestroyConstantBlocks( blocks, count ); }
		void													DestroyMaterial( hMaterial* pMat );
		void													CreateTexture( hUint32 width, hUint32 height, hUint32 levels, TextureFormat format, hUint32 flags, hTexture** outTex );
		void													DestroyTexture( hTexture* pOut );
		void													CreateIndexBuffer( hUint16* pIndices, hUint16 nIndices, hUint32 flags, PrimitiveType primType, hIndexBuffer** outIB );
		void													DestroyIndexBuffer( hIndexBuffer* pOut );
		void													CreateVertexBuffer( void* initData, hUint32 nElements, hUint32 stride, hUint32 layout, hUint32 flags, hVertexBuffer** outVB );
		void													DestoryVertexBuffer( hVertexBuffer* pOut );

		void*													AquireTempRenderMemory( hUint32 size );
		void													ReleaseTempRenderMemory( void* ptr );
		///////////////////////////////////////////////////////////////////////////////////////////////////
		// Render Thread only methods /////////////////////////////////////////////////////////////////////
		///////////////////////////////////////////////////////////////////////////////////////////////////
		hRenderFrameStats*										pFrameStats() { return pThreadFrameStats_->passes_ + statPass_; }
		void													pFrameStats( hRenderFrameStatsCollection* framestats ) { pThreadFrameStats_ = framestats; }
		void													NextStatCollectPass() { ++statPass_; }

		/*
			end new engine design methods
		*/
		void													BeginRenderFrame( hBool wait );
		hUint32													ReleasePendingRenderResources();
		void													EndRenderFrame();
		hUint32													Width() { return 0/*pImpl()->Width()*/; }
		hUint32													Height() { return 0/*pImpl()->Height()*/; }
		hUint32													CurrentFPS() { return FPS_; }
		void													StopRenderThread()
		{
			ReleasePendingRenderResources();

			renderThreadKill_.Signal();

			while( renderThread_.IsComplete() == hFalse ) { Sleep( 3 ); }
		}

		//////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////

		static bool												IsRenderThread()
		{
			return pRenderThreadID_ == Threading::GetCurrentThreadID();
		}

	private:

		friend class hVertexDeclarationManager;

		void													CreateVertexDeclaration( hVertexDeclaration*& pOut, hUint32 vtxFlags );
		void													DestroyVertexDeclaration( hVertexDeclaration* pVD );
		hUint32													RenderThread( void* pParam );
		void													CreatePendingResources();
		hUint32													ReleasePendingTexturesResources();
		hUint32													ReleasePendingIndexBufferResources();
		hUint32													ReleasePendingVertexBufferResources();
		hUint32													ReleasePendingMeshResources();
		hUint32													ReleasePendingMaterialResources();


		// Init params
		hSystem*												system_;
		hUint32 												width_;
		hUint32 												height_; 
		hUint32 												bpp_;
		hFloat													shaderVersion_;
		hBool													fullscreen_;
		hBool													vsync_;

		hRenderState*											renderStateCache_;
		hResourceManager*										resourceManager_;

		hUint32													nCmds_;

		volatile hUint32										FPS_;
		hUint32													currentRenderStatFrame_;
		hRenderFrameStatsCollection*							pThreadFrameStats_;
		hRenderFrameStatsCollection								rendererStats_[2];
		hUint32													statPass_;

		//Render Threading
		hThread													renderThread_;
		hThreadEvent											renderThreadKill_;

		//Simpler Render Buffer
		hLocklessFixedComPipe< hTextureBase*, MAX_RESOURCES >	texturesToRelease_;
		hLocklessFixedComPipe< hVertexBuffer*, MAX_RESOURCES >	vertexBuffersToRelease_;
		hLocklessFixedComPipe< hIndexBuffer*, MAX_RESOURCES >	indexBuffersToRelease_;
		hLocklessFixedComPipe< hMesh*, MAX_RESOURCES >			meshesToRelease_;
		hLocklessFixedComPipe< hMaterial*, MAX_RESOURCES >		materialsToRelease_;
		hLocklessComPipe< DEFAULT_CMD_BUF_SIZE >				renderCmdBuffer_;
		//Create Resource Pipes
		hLocklessFixedComPipe< hTexture*, MAX_RESOURCES >		textureToCreate_;
		hLocklessFixedComPipe< hIndexBuffer*, MAX_RESOURCES >	indexBuffersToCreate_;
		hLocklessFixedComPipe< hVertexBuffer*, MAX_RESOURCES >	vertexBuffersToCreate_;
		
		static void*											pRenderThreadID_;
	};
}

#endif // hrRenderer_h__