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

#define HEART_DEBUG_CAMERA_ID (13)
#define HEART_DEBUGUI_CAMERA_ID (14)

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
    class hDrawCallPusher;
	
	struct HEARTCORE_SLIBEXPORT hRenderFrameStats
	{
		hUint32			nTriangels_;
		hUint32			nRenderCmds_;
		hUint32			nDrawCalls_;
	};

	struct HEARTCORE_SLIBEXPORT hRenderFrameStatsCollection
	{
		static const hUint32 MAX_PASSES = 8;
		hRenderFrameStats passes_[ MAX_PASSES ];
	};

    struct HEARTCORE_SLIBEXPORT hRenderResourceUpdateCmd
    {
        enum hMapType
        {
            eMapTypeVtxBuffer,
            eMapTypeIdxBuffer,
        };
        union
        {
            hVertexBuffer*  vb_;
            hIndexBuffer*   ib_;
        };
        void*   data_;
        hUint32 size_;
        hUint16 flags_;
    };

    struct HEARTCORE_SLIBEXPORT hDrawCall
    {
        hUint64             sortKey_;
        union
        {
            struct  
            {
                hVertexBuffer*      vertexBuffer_;
                hIndexBuffer*       indexBuffer_;
            };
            struct  
            {
                hByte*      imIBBuffer_;
                hUint32     ibSize_;
                hByte*      imVBBuffer_;
                hUint32     vbSize_;
            };
        };            
        hMaterialInstance*      matInstance_;
        hUint16                 primCount_;
        hUint16                 startVertex_;
        hUint16                 stride_;
        hScissorRect            scissor_;
        PrimitiveType           primType_ : 4;
        hBool                   immediate_ : 1;
    };

	class HEARTCORE_SLIBEXPORT hRenderer : public hdRenderDevice
	{
	public:

        HEART_BASECLASS(hdRenderDevice);

        static const hUint32									DEFAULT_SCRATCH_BUFFER_SIZE = 1024*1024*8;
        static const hUint32                                    MAX_DCBLOCKS = (64*1024);
        static const hUint32                                    s_resoruceUpdateLimit = 1024;
        static const hUint32                                    s_scratchBufferCount = 5;

		hRenderer();
		void													Create( hSystem* pSystem, hUint32 width, hUint32 height, hUint32 bpp, hFloat shaderVersion, hBool fullscreen, hBool vsync, hResourceManager* pResourceManager );
		void													Destroy();
        hFloat                                                  GetRatio() const { return (hFloat)GetWidth()/(hFloat)GetHeight(); }
        hRendererCamera*                                        GetRenderCamera(hUint32 id) { hcAssertMsg(id < 15, "Invalid camera id access"); return &renderCameras_[id];}

		///////////////////////////////////////////////////////////////////////////////////////////////////
		///////////////////////////////////////////////////////////////////////////////////////////////////
		///////////////////////////////////////////////////////////////////////////////////////////////////

		hRenderFrameStatsCollection*							LastRenderFrameStats() { return rendererStats_ + (currentRenderStatFrame_ % 2); }
        hRenderMaterialManager*                                 GetMaterialManager() { return &techniqueManager_; }

        //////////////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////

        hTexture*                                               OnTextureLoad(hISerialiseStream* dataStream);

        //////////////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////
        hRenderSubmissionCtx*                                   CreateRenderSubmissionCtx();
        void                                                    DestroyRenderSubmissionCtx( hRenderSubmissionCtx* ctx );
		/*
			pimpl methods
		*/
		void													DestroyMaterial( hMaterial* pMat );
		void													CreateTexture( hUint32 width, hUint32 height, hUint32 levels, hMipDesc* initialData, hTextureFormat format, hUint32 flags, hTexture** outTex );
		void													DestroyTexture( hTexture* pOut );
		void													CreateIndexBuffer( hUint16* pIndices, hUint16 nIndices, hUint32 flags, PrimitiveType primType, hIndexBuffer** outIB );
		void													DestroyIndexBuffer( hIndexBuffer* pOut );
		void													CreateVertexBuffer( void* initData, hUint32 nElements, hUint32 layout, hUint32 flags, hVertexBuffer** outVB );
		void													DestroyVertexBuffer( hVertexBuffer* pOut );

        void                                                    SumbitResourceUpdateCommand(const hRenderResourceUpdateCmd& cmd);
        void                                                    SubmitDrawCallBlock(hDrawCall* block, hUint32 count);
		void*													AllocTempRenderMemory( hUint32 size );

		/*
			end new engine design methods
		*/
		void													BeginRenderFrame();
		void													EndRenderFrame();
		hUint32													CurrentFPS() { return FPS_; }
        hFloat                                                  GetLastGPUTime() { return gpuTime_; }

		//////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////

		static bool												IsRenderThread();

	private:

		friend class hVertexDeclarationManager;

		void													CreateVertexDeclaration( hVertexDeclaration*& pOut, hUint32 vtxFlags );
		void													DestroyVertexDeclaration( hVertexDeclaration* pVD );

        //
        hUint32                                                 BeginCameraRender(hRenderSubmissionCtx* ctx, hUint32 camID);
        void                                                    CollectAndSortDrawCalls();
        void                                                    SubmitDrawCallsMT();
        void                                                    SubmitDrawCallsST();
        void                                                    DoDrawResourceUpdates();
		// Init params
		hSystem*												system_;
		hUint32 												width_;
		hUint32 												height_; 
		hUint32 												bpp_;
		hFloat													shaderVersion_;
		hBool													fullscreen_;
		hBool													vsync_;
        hFloat                                                  gpuTime_;

        hRendererCamera                                         renderCameras_[15];
		hRenderState*											renderStateCache_;
		hResourceManager*										resourceManager_;
        hRenderMaterialManager                                  techniqueManager_;
        hRenderSubmissionCtx                                    mainSubmissionCtx_;
        hVertexBuffer*                                          volatileVBuffer_[s_scratchBufferCount];
        hIndexBuffer*                                           volatileIBuffer_[s_scratchBufferCount];

        hUint32                                                 scratchBufferSize_;
        hByte                                                   drawDataScratchBuffer_[DEFAULT_SCRATCH_BUFFER_SIZE];
        hAtomicInt                                              scratchPtrOffset_;
        hAtomicInt                                              drawCallBlockIdx_;
        hArray< hDrawCall, MAX_DCBLOCKS >                       drawCallBlocks_;
        hAtomicInt                                              drawResourceUpdateCalls_;
        hArray< hRenderResourceUpdateCmd, s_resoruceUpdateLimit >     drawResourceUpdates_;




        //Debug Vars
        hMaterial*                                              debugMaterial_;
        hIndexBuffer*                                           debugSphereIB_;
        hVertexBuffer*                                          debugSphereVB_;

		hUint32										            FPS_;
		hUint32													currentRenderStatFrame_;
		hRenderFrameStatsCollection								rendererStats_[2];
		hUint32													statPass_;
		
		static void*											pRenderThreadID_;    
	};

}

#endif // hrRenderer_h__