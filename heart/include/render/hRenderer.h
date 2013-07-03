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
    
    struct HEART_DLLEXPORT hRenderFrameStats
    {
        hUint32			nTriangels_;
        hUint32			nRenderCmds_;
        hUint32			nDrawCalls_;
    };

    struct HEART_DLLEXPORT hRenderFrameStatsCollection
    {
        static const hUint32 MAX_PASSES = 8;
        hRenderFrameStats passes_[ MAX_PASSES ];
    };

    struct HEART_DLLEXPORT hRenderResourceUpdateCmd
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

    typedef void (*hCustomRenderCallback)(hRenderer*, void*);

    struct HEART_DLLEXPORT hDrawCall
    {
        hUint64                 sortKey_;                                   //8b        -> 8b
        union {
            struct {
                // Pointers for the following three might not be the best solution
                hUint                   instanceCount_;
                hUint32                 drawPrimCount_;
                hdBlendState*           blendState_;
                hdRasterizerState*      rasterState_;
                hdDepthStencilState*    depthState_;
                hdRenderInputObject*    progInput_;
                hdRenderStreamsObject   streams_;
            };
            struct {
                hCustomRenderCallback   customCall_;
                void*                   userPtr_;
            };
            struct {
                hRCmd* rCmds_;
            };
        };
    };

    class HEART_DLLEXPORT hRenderCommandGenerator : public hdRenderCommandGenerator
    {
    public:
        hRenderCommandGenerator();
        hRenderCommandGenerator(hRenderCommands* rcmds);

        void             setRenderCommands(hRenderCommands* rcmds) { renderCommands_ = rcmds; }
        hRenderCommands* getRenderCommands() { return renderCommands_; }
        hUint resetCommands();
        hUint setJump(hRCmd* cmd);
        hUint setReturn();
        hUint setNoOp();
        hUint setDraw(hUint nPrimatives, hUint startVertex);
        hUint setDrawIndex(hUint nPrimatives, hUint startVertex);
        hUint setDrawInstance(hUint nPrimatives, hUint startVertex, hUint instancecount);
        hUint setDrawInstanceIndex(hUint nPrimatives, hUint startVertex, hUint instancecount);
        hUint setRenderStates(hBlendState* bs, hRasterizerState* rs, hDepthStencilState* dss);
        hUint setShader(hShaderProgram* shader);
        hUint setVertexInputs(hSamplerState** samplers, hUint nsamplers,
            hShaderResourceView** srv, hUint nsrv,
            hParameterConstantBlock** cb, hUint ncb);
        hUint setPixelInputs(hSamplerState** samplers, hUint nsamplers,
            hShaderResourceView** srv, hUint nsrv,
            hParameterConstantBlock** cb, hUint ncb);
        hUint setGeometryInputs(hdDX11SamplerState** samplers, hUint nsamplers,
            hShaderResourceView** srv, hUint nsrv,
            hParameterConstantBlock** cb, hUint ncb);
        hUint setHullInputs(hdDX11SamplerState** samplers, hUint nsamplers,
            hShaderResourceView** srv, hUint nsrv,
            hParameterConstantBlock** cb, hUint ncb);
        hUint setDomainInputs(hdDX11SamplerState** samplers, hUint nsamplers,
            hShaderResourceView** srv, hUint nsrv,
            hParameterConstantBlock** cb, hUint ncb);
        hUint setStreamInputs(PrimitiveType primType, hIndexBuffer* index, hIndexBufferType format,
            hdInputLayout* vertexlayout, hVertexBuffer** vtx, hUint firstStream, hUint streamCount);
        hUint updateShaderInputBuffer(hRCmd* cmd, hUint reg, hParameterConstantBlock* cb);
        hUint updateShaderInputSampler(hRCmd* cmd, hUint reg, hSamplerState* ss);
        hUint updateShaderInputView(hRCmd* cmd, hUint reg, hShaderResourceView* srv);
        hUint updateStreamInputs(hRCmd* cmd, PrimitiveType primType, hIndexBuffer* index, hIndexBufferType format,
            hdInputLayout* vertexlayout, hVertexBuffer** vtx, hUint firstStream, hUint streamCount);

    private:

        virtual hRCmd* getCmdBufferStart();
        virtual hUint  appendCmd(const hRCmd* cmd);
        virtual hUint  overwriteCmd(const hRCmd* oldcmd, const hRCmd* newcmd);
        virtual void   reset();

        hRenderCommands*    renderCommands_;
    };

    class HEART_DLLEXPORT hRenderer : public hdRenderDevice
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
        void                                                    initialiseCameras();
        hFloat                                                  GetRatio() const { return (hFloat)GetWidth()/(hFloat)GetHeight(); }
        hRendererCamera*                                        GetRenderCamera(hUint32 id) { hcAssertMsg(id < HEART_MAX_RENDER_CAMERAS, "Invalid camera id access"); return &renderCameras_[id];}
        hRenderSubmissionCtx*                                   GetMainSubmissionCtx() { return &mainSubmissionCtx_; };
        hUint32                                                 beginCameraRender(hRenderSubmissionCtx* ctx, hUint32 camID);

        ///////////////////////////////////////////////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////////////////////////////////

        hRenderFrameStatsCollection*							LastRenderFrameStats() { return rendererStats_ + (currentRenderStatFrame_ % 2); }
        hRenderMaterialManager*                                 GetMaterialManager() { return &materialManager_; }

        //////////////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////
        hRenderSubmissionCtx*                                   CreateRenderSubmissionCtx();
        void                                                    DestroyRenderSubmissionCtx( hRenderSubmissionCtx* ctx );
        /*
            pimpl methods
        */
        void  compileShaderFromSource(hMemoryHeapBase* heap, const hChar* shaderProg, hUint32 len, const hChar* entry, hShaderProfile profile, hShaderProgram** out);
        void  createShader(hMemoryHeapBase* heap, const hChar* shaderProg, hUint32 len, hShaderType type, hShaderProgram** out);
        void  createTexture(hUint32 levels, hMipDesc* initialData, hTextureFormat format, hUint32 flags, hMemoryHeapBase* heap, hTexture** outTex);
        void  resizeTexture(hUint32 width, hUint32 height, hTexture* inout);
        void  createIndexBuffer(void* pIndices, hUint32 nIndices, hUint32 flags, hIndexBuffer** outIB);
        void  createVertexBuffer(void* initData, hUint32 nElements, hInputLayoutDesc* desc, hUint32 desccount, hUint32 flags, hMemoryHeapBase* heap, hVertexBuffer** outVB);
        void  createShaderResourceView(hTexture* tex, const hShaderResourceViewDesc& desc, hShaderResourceView** outsrv);
        void  createShaderResourceView(hParameterConstantBlock* cb, const hShaderResourceViewDesc& desc, hShaderResourceView** outsrv);
        void  createRenderTargetView(hTexture* tex, const hRenderTargetViewDesc& rtvd, hRenderTargetView** outrtv);
        void  createDepthStencilView(hTexture* tex, const hDepthStencilViewDesc& dsvd, hDepthStencilView** outdsv);
        hBlendState*        createBlendState( const hBlendStateDesc& desc );
        hRasterizerState*   createRasterizerState( const hRasterizerStateDesc& desc );
        hDepthStencilState* createDepthStencilState( const hDepthStencilStateDesc& desc );
        hSamplerState*      createSamplerState( const hSamplerStateDesc& desc );
        void  createConstantBlock(hUint size, void* data, hParameterConstantBlock** outcb);
    private:  
        void  destroyShader(hShaderProgram* prog);
        void  destroyTexture(hTexture* pOut);
        void  destroyIndexBuffer(hIndexBuffer* pOut);
        void  destroyVertexBuffer(hVertexBuffer* pOut);
        void  destroyShaderResourceView(hShaderResourceView* srv);  
        void  destroyRenderTargetView(hRenderTargetView* view);
        void  destroyDepthStencilView(hDepthStencilView* view);
        void  destroyBlendState( hBlendState* state );
        void  destoryRasterizerState( hRasterizerState* state );
        void  destroyDepthStencilState( hDepthStencilState* state );
        void  destroySamplerState( hSamplerState* state );
        void  destroyConstantBlock(hParameterConstantBlock* block);
    public:
        void  SubmitDrawCallBlock(hDrawCall* block, hUint32 count);
        void* allocTempRenderMemory( hUint32 size );

        /*
            end new engine design methods
        */
        void                                                    BeginRenderFrame();
        void                                                    EndRenderFrame();
        void                                                    rendererFrameSubmit();

        hUint32													CurrentFPS() { return FPS_; }
        hFloat                                                  GetLastGPUTime() { return gpuTime_; }
        hShaderProgram*                                         getDebugShader(hDebugShaderID shaderID);

        //////////////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////

        static bool                                             IsRenderThread();

    private:
        typedef hMap< hUint32, hBlendState >         BlendStateMapType;
        typedef hMap< hUint32, hRasterizerState >    RasterizerStateMapType;
        typedef hMap< hUint32, hDepthStencilState >  DepthStencilStateMapType;
        typedef hMap< hUint32, hSamplerState >       SamplerStateMapType;

        //
        void                                                    CollectAndSortDrawCalls();
        void                                                    SubmitDrawCallsMT();
        void                                                    SubmitDrawCallsST();
        void                                                    createDebugShadersInternal();

        //////////////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////
        hResourceClassBase*  textureResourceLoader(hIFile* file, hResourceMemAlloc* memalloc);
        hBool                textureResourceLink(hResourceClassBase* texture, hResourceMemAlloc* memalloc);
        void                 textureResourceUnlink(hResourceClassBase* texture, hResourceMemAlloc* memalloc);
        void                 textureResourceUnload(hResourceClassBase* texture, hResourceMemAlloc* memalloc);

        hResourceClassBase*  shaderResourceLoader(hIFile* file, hResourceMemAlloc* memalloc);
        hBool                shaderResourceLink(hResourceClassBase* resource, hResourceMemAlloc* memalloc);
        void                 shaderResourceUnlink(hResourceClassBase* resource, hResourceMemAlloc* memalloc);
        void                 shaderResourceUnload(hResourceClassBase* resource, hResourceMemAlloc* memalloc);

        hResourceClassBase*  materialResourceLoader(hIFile* file, hResourceMemAlloc* memalloc);
        hBool                materialResourceLink(hResourceClassBase* resource, hResourceMemAlloc* memalloc);
        void                 materialResourceUnlink(hResourceClassBase* resource, hResourceMemAlloc* memalloc);
        void                 materialResourceUnload(hResourceClassBase* resource, hResourceMemAlloc* memalloc);

        hResourceClassBase*  meshResourceLoader(hIFile* file, hResourceMemAlloc* memalloc);
        hBool                meshResourceLink(hResourceClassBase* resource, hResourceMemAlloc* memalloc);
        void                 meshResourceUnlink(hResourceClassBase* resource, hResourceMemAlloc* memalloc);
        void                 meshResourceUnload(hResourceClassBase* resource, hResourceMemAlloc* memalloc);

        // Init params
        hSystem*												system_;
        hUint32 												width_;
        hUint32 												height_; 
        hUint32 												bpp_;
        hFloat													shaderVersion_;
        hBool													fullscreen_;
        hBool													vsync_;
        hFloat                                                  gpuTime_;

        hMutex                   resourceMutex_;
        BlendStateMapType        blendStates_;
        RasterizerStateMapType   rasterizerStates_;
        DepthStencilStateMapType depthStencilStates_;
        SamplerStateMapType      samplerStateMap_;

        hRendererCamera         renderCameras_[HEART_MAX_RENDER_CAMERAS];
        hRenderState*           renderStateCache_;
        hResourceManager*       resourceManager_;
        hRenderMaterialManager  materialManager_;
        hRenderSubmissionCtx    mainSubmissionCtx_;
        //hTexture*               depthBuffer_;
        hShaderProgram*         debugShaders_[eDebugShaderMax];

        hTexture*               backBuffer_;

        hUint32                                                 scratchBufferSize_;
        hByte                                                   drawDataScratchBuffer_[DEFAULT_SCRATCH_BUFFER_SIZE];
        hAtomicInt                                              scratchPtrOffset_;
        hAtomicInt                                              drawCallBlockIdx_;
        hArray< hDrawCall, MAX_DCBLOCKS >                       drawCallBlocks_;
        hAtomicInt                                              drawResourceUpdateCalls_;
        hArray< hRenderResourceUpdateCmd, s_resoruceUpdateLimit >     drawResourceUpdates_;

        hUint32										            FPS_;
        hUint32													currentRenderStatFrame_;
        hRenderFrameStatsCollection								rendererStats_[2];
        hUint32													statPass_;
        
        static void*											pRenderThreadID_;
    };

}

#endif // hrRenderer_h__