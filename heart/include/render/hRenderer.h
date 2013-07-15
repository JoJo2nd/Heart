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

    hFUNCTOR_TYPEDEF(void (*)(hRenderer*, hRenderSubmissionCtx*), hCustomRenderCallback);

    struct HEART_DLLEXPORT hDrawCall
    {
        hDrawCall() : customCallFlag_(false) {}
        hUint64                 sortKey_;//8b        -> 8b
        union {
            struct {
                hCustomRenderCallback   customCall_;
            };
            struct {
                hRCmd* rCmds_;
            };
        };
        hBool                   customCallFlag_;
    };

    class HEART_DLLEXPORT hRenderCommandGenerator : public hdRenderCommandGenerator
    {
    public:
        hRenderCommandGenerator();
        hRenderCommandGenerator(hRenderCommands* rcmds);

        void             setRenderCommands(hRenderCommands* rcmds) { renderCommands_ = rcmds; }
        hRenderCommands* getRenderCommands() { return renderCommands_; }
        hUint            getRenderCommandsSize() const { return renderCommands_->cmdSize_; }
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
            hRenderBuffer** cb, hUint ncb);
        hUint setPixelInputs(hSamplerState** samplers, hUint nsamplers,
            hShaderResourceView** srv, hUint nsrv,
            hRenderBuffer** cb, hUint ncb);
        hUint setGeometryInputs(hdDX11SamplerState** samplers, hUint nsamplers,
            hShaderResourceView** srv, hUint nsrv,
            hRenderBuffer** cb, hUint ncb);
        hUint setHullInputs(hdDX11SamplerState** samplers, hUint nsamplers,
            hShaderResourceView** srv, hUint nsrv,
            hRenderBuffer** cb, hUint ncb);
        hUint setDomainInputs(hdDX11SamplerState** samplers, hUint nsamplers,
            hShaderResourceView** srv, hUint nsrv,
            hRenderBuffer** cb, hUint ncb);
        hUint setStreamInputs(PrimitiveType primType, hIndexBuffer* index, hIndexBufferType format,
            hdInputLayout* vertexlayout, hVertexBuffer** vtx, hUint firstStream, hUint streamCount);
        hUint updateShaderInputBuffer(hRCmd* cmd, hUint reg, hRenderBuffer* cb);
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
        void  createShaderResourceView(hRenderBuffer* cb, const hShaderResourceViewDesc& desc, hShaderResourceView** outsrv);
        void  createRenderTargetView(hTexture* tex, const hRenderTargetViewDesc& rtvd, hRenderTargetView** outrtv);
        void  createDepthStencilView(hTexture* tex, const hDepthStencilViewDesc& dsvd, hDepthStencilView** outdsv);
        hBlendState*        createBlendState( const hBlendStateDesc& desc );
        hRasterizerState*   createRasterizerState( const hRasterizerStateDesc& desc );
        hDepthStencilState* createDepthStencilState( const hDepthStencilStateDesc& desc );
        hSamplerState*      createSamplerState( const hSamplerStateDesc& desc );
        void  createBuffer(hUint size, void* data, hUint flags, hUint stride, hRenderBuffer** outcb);
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
        void  destroyConstantBlock(hRenderBuffer* block);
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

        hUint32										            FPS_;
        hUint32													currentRenderStatFrame_;
        hRenderFrameStatsCollection								rendererStats_[2];
        hUint32													statPass_;
        
        static void*											pRenderThreadID_;
    };

    class hLightingManager
    {
    public:

        struct hRenderTargetInfo
        {
            hShaderProgram* vertexLightShader_;
            hShaderProgram* pixelLightShader_;
            hTexture*       albedo_;
            hTexture*       normal_;
            hTexture*       spec_;
            hTexture*       depth_;
            hUint           viewCameraIndex_;
        };

        hLightingManager()
            : inputLayout_(hNullptr)
            , inputLightData_(hNullptr)
            , directionLightData_(hNullptr)
            , screenQuadIB_(hNullptr)
            , screenQuadVB_(hNullptr)
            , blendState_(hNullptr)
            , rasterState_(hNullptr)
            , depthStencilState_(hNullptr)
            , samplerState_(hNullptr)
            , samplerBindPoint_(0)
        {
            
        }
        ~hLightingManager()
        {
            destroy();
        }

        void initialise(hRenderer* renderer, const hRenderTargetInfo* rndrinfo);
        void destroy();
        void addDirectionalLight(const hVec3& direction, const hColour& colour);

        void doDeferredLightPass(hRenderer* renderer, hRenderSubmissionCtx* ctx);

    private:

        struct hInputLightData
        {
            hMatrix viewMatrix_;
            hMatrix inverseViewMtx_;
            hMatrix projectionMtx_;
            hMatrix inverseProjectMtx_;
            hVec4   eyePos_;
            hUint   directionalLightCount_;
        };

        struct hDirectionalLight
        {
            hVec3   direction_;
            hColour colour_;
        };

        static const hUint s_maxDirectionalLights = 15;

        hRenderTargetInfo                                   targetInfo_;
        hdInputLayout*                                      inputLayout_;
        hRenderBuffer*                                      inputLightData_;
        hRenderBuffer*                                      directionLightData_;
        hIndexBuffer*                                       screenQuadIB_;
        hVertexBuffer*                                      screenQuadVB_;
        hBlendState*                                        blendState_;
        hRasterizerState*                                   rasterState_;
        hDepthStencilState*                                 depthStencilState_;
        hSamplerState*                                      samplerState_;
        hUint                                               samplerBindPoint_;
        hVector<hShaderResourceView*>                       srv_;
        hVector<hRenderBuffer*>                             buffers_;
        hRenderCommands                                     renderCmds_;

        hInputLightData                                     lightInfo_;
        hArray<hDirectionalLight, s_maxDirectionalLights>   directionalLights_;
    };
}

#endif // hrRenderer_h__