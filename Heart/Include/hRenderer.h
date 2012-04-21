/********************************************************************

	filename: 	hRenderer.h	
	
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

#ifndef hrRenderer_h__
#define hrRenderer_h__

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

    enum hRenderCmdType
    {
        RENDERCMD_CMD_BUFFER,
        RENDERCMD_CMD_BUFFER_RELEASE,
        RENDERCMD_SWAP
    };

    struct hRendererCmd
    {
        hRenderCmdType      type_;
        union
        {
            hdRenderCommandBuffer   commandBuffer_;
        };
    };

	class hRenderer : public hPtrImpl< hdRenderDevice >
	{
	public:

		static const hUint32									DEFAULT_CMD_BUF_SIZE = 1024 * 1024;
		static const hUint32									DEFAULT_MODIFY_BUFFER_SIZE = 1024*1024*8;
		static const hUint32									MAX_BONES = 12;
		static const hUint32									RT_BACK_BUFFER = hErrorCode;
		static const hUint32									MAX_RT = 8;
		static const hUint32									MAX_RESOURCES = 128;
		//static const hUint32									NUM_RENDER_BUFFERS = 3;

		hRenderer();
		void													Create( hSystem* pSystem, hUint32 width, hUint32 height, hUint32 bpp, hFloat shaderVersion, hBool fullscreen, hBool vsync, hResourceManager* pResourceManager );
		void													Destroy();
        hUint32                                                 GetWidth() const { return pImpl()->Width(); }
        hUint32                                                 GetHeight() const { return pImpl()->Height(); }
        hFloat                                                  GetRatio() const { return (hFloat)GetWidth()/(hFloat)GetHeight(); }
		///////////////////////////////////////////////////////////////////////////////////////////////////
		///////////////////////////////////////////////////////////////////////////////////////////////////
		///////////////////////////////////////////////////////////////////////////////////////////////////

		hRenderFrameStatsCollection*							LastRenderFrameStats() { return rendererStats_ + (currentRenderStatFrame_ % 2); }
        hRenderMaterialManager*                                 GetMaterialManager() { return &techniqueManager_; }

		//default resource loaders
		hResourceClassBase*										OnTextureLoad( const hChar* ext, hUint32 resID, hSerialiserFileStream* dataStream, hResourceManager* resManager );
		hUint32													OnTextureUnload( const hChar* ext, hResourceClassBase* resource, hResourceManager* resManager );
		hResourceClassBase*										OnMaterialLoad( const hChar* ext, hUint32 resID, hSerialiserFileStream* dataStream, hResourceManager* resManager );
		hUint32													OnMaterialUnload( const hChar* ext, hResourceClassBase* resource, hResourceManager* resManager );
        hResourceClassBase*										OnShaderProgramLoad( const hChar* ext, hUint32 resID, hSerialiserFileStream* dataStream, hResourceManager* resManager );
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
        hdParameterConstantBlock*                               CreateConstantBuffers( const hUint32* sizes, const hUint32* regs, hUint32 count ) { return pImpl()->CreateConstantBlocks( sizes, regs, count ); }
        void                                                    UpdateConstantBlockParameters( hdParameterConstantBlock* constBlock, hShaderParameter* params, hUint32 parameters ) { pImpl()->UpdateConstantBlockParameters( constBlock, params, parameters ); }
        void                                                    DestroyConstantBuffers( hdParameterConstantBlock* blocks, hUint32 count ) { pImpl()->DestroyConstantBlocks( blocks, count ); }
		void													DestroyMaterial( hMaterial* pMat );
		void													CreateTexture( hUint32 width, hUint32 height, hUint32 levels, void** initialData, hUint32* initDataSize, hTextureFormat format, hUint32 flags, hTexture** outTex );
		void													DestroyTexture( hTexture* pOut );
		void													CreateIndexBuffer( hUint16* pIndices, hUint16 nIndices, hUint32 flags, PrimitiveType primType, hIndexBuffer** outIB );
		void													DestroyIndexBuffer( hIndexBuffer* pOut );
		void													CreateVertexBuffer( void* initData, hUint32 nElements, hUint32 layout, hUint32 flags, hVertexBuffer** outVB );
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
        void                                                    SubmitRenderCommandBuffer( hdRenderCommandBuffer cmdBuf, hBool releaseBuf );
		hUint32													ReleasePendingRenderResources();
		void													EndRenderFrame();
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
			return pRenderThreadID_ == hThreading::GetCurrentThreadID();
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
        hRenderMaterialManager                                  techniqueManager_;
        hRenderSubmissionCtx                                    mainSubmissionCtx_;

        //Debug Vars
        hMaterial*                                              debugMaterial_;
        hIndexBuffer*                                           debugSphereIB_;
        hVertexBuffer*                                          debugSphereVB_;

		volatile hUint32										FPS_;
		hUint32													currentRenderStatFrame_;
		hRenderFrameStatsCollection*							pThreadFrameStats_;
		hRenderFrameStatsCollection								rendererStats_[2];
		hUint32													statPass_;

		//Render Threading
		hThread													renderThread_;
		hThreadEvent											renderThreadKill_;

		//Simpler Render Buffer
		hLocklessFixedComPipe< hTexture*, MAX_RESOURCES >	    texturesToRelease_;
		hLocklessFixedComPipe< hVertexBuffer*, MAX_RESOURCES >	vertexBuffersToRelease_;
		hLocklessFixedComPipe< hIndexBuffer*, MAX_RESOURCES >	indexBuffersToRelease_;
		hLocklessFixedComPipe< hMesh*, MAX_RESOURCES >			meshesToRelease_;
		hLocklessFixedComPipe< hMaterial*, MAX_RESOURCES >		materialsToRelease_;

		hLocklessFixedComPipe< hRendererCmd, DEFAULT_CMD_BUF_SIZE > renderCmdPipe_;
		//Create Resource Pipes
		hLocklessFixedComPipe< hTexture*, MAX_RESOURCES >		textureToCreate_;
		hLocklessFixedComPipe< hIndexBuffer*, MAX_RESOURCES >	indexBuffersToCreate_;
		hLocklessFixedComPipe< hVertexBuffer*, MAX_RESOURCES >	vertexBuffersToCreate_;
		
		static void*											pRenderThreadID_;

#ifdef HEART_ALLOW_PIX_MT_DEBUGGING
        hMutex                                                  pixMutex_;
#endif
        
	};
}

#endif // hrRenderer_h__