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

    hFUNCTOR_TYPEDEF(void (*)(const hChar*, void**, hUint32*), hShaderIncludeCallback);
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
        hUint setShader(hShaderProgram* shader, hShaderType type);
        hUint setVertexInputs(hSamplerState** samplers, hUint nsamplers,
            hShaderResourceView** srv, hUint nsrv,
            hRenderBuffer** cb, hUint ncb);
        hUint updateVertexInputs(hRCmd* cmd, hSamplerState** samplers, hUint nsamplers,
            hShaderResourceView** srv, hUint nsrv,
            hRenderBuffer** cb, hUint ncb);
        hUint setPixelInputs(hSamplerState** samplers, hUint nsamplers,
            hShaderResourceView** srv, hUint nsrv,
            hRenderBuffer** cb, hUint ncb);
        hUint updatePixelInputs(hRCmd* cmd, hSamplerState** samplers, hUint nsamplers,
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

        static hRenderer* get() { // meh... do not want
            return instance_;
        }

        hRenderer();
        void													Create( hSystem* pSystem, hUint32 width, hUint32 height, hUint32 bpp, hFloat shaderVersion, hBool fullscreen, hBool vsync);
        void													Destroy();
        void                                                    initialiseCameras();
        hFloat                                                  GetRatio() const { return (hFloat)GetWidth()/(hFloat)GetHeight(); }
        hRendererCamera*                                        GetRenderCamera(hUint32 id) { hcAssertMsg(id < HEART_MAX_RENDER_CAMERAS, "Invalid camera id access"); return &renderCameras_[id];}
        hRenderSubmissionCtx*                                   GetMainSubmissionCtx() { return &mainSubmissionCtx_; };
        hUint32                                                 beginCameraRender(hRenderSubmissionCtx* ctx, hUint32 camID);

        ///////////////////////////////////////////////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////////////////////////////////

        const hRenderFrameStats*                                getRenderStats() const { return &stats_; }
        hRenderMaterialManager*                                 GetMaterialManager() { return &materialManager_; }

        //////////////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////
        hRenderSubmissionCtx*                                   CreateRenderSubmissionCtx();
        void                                                    DestroyRenderSubmissionCtx( hRenderSubmissionCtx* ctx );
        /*
            pimpl methods
        */
        void  compileShaderFromSource(const hChar* shaderProg, hUint32 len, const hChar* entry, hShaderProfile profile, hIIncludeHandler* includes, hShaderDefine* defines, hUint ndefines, hShaderProgram* out);
        void  createShader(const hChar* shaderProg, hUint32 len, hShaderType type, hShaderProgram* out);
        void  createTexture(hUint32 levels, hMipDesc* initialData, hTextureFormat format, hUint32 flags, hTexture** outTex);
        void  resizeTexture(hUint32 width, hUint32 height, hTexture* inout);
        void  createIndexBuffer(const void* pIndices, hUint32 nIndices, hUint32 flags, hIndexBuffer** outIB);
        void  createVertexBuffer(const void* initData, hUint32 nElements, hInputLayoutDesc* desc, hUint32 desccount, hUint32 flags, hVertexBuffer** outVB);
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

        hFloat                                                  GetLastGPUTime() { return gpuTime_; }

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

#if 0
        //////////////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////
        hResourceClassBase*  textureResourceLoader(const hResourceSection* sections, hUint sectioncount);
        void                 textureResourcePostLoad(hResourceManager* manager, hResourceClassBase* texture);
        void                 textureResourcePreUnload(hResourceManager* manager, hResourceClassBase* texture);
        void                 textureResourceUnload(hResourceClassBase* texture);

        hResourceClassBase*  shaderResourceLoader(const hResourceSection* sections, hUint sectioncount);
        void                 shaderResourcePostLoad(hResourceManager* manager, hResourceClassBase* resource);
        void                 shaderResourcePreUnload(hResourceManager* manager, hResourceClassBase* resource);
        void                 shaderResourceUnload(hResourceClassBase* resource);

        hResourceClassBase*  materialResourceLoader(const hResourceSection* sections, hUint sectioncount);
        void                 materialResourcePostLoad(hResourceManager* manager, hResourceClassBase* resource);
        void                 materialResourcePreUnload(hResourceManager* manager, hResourceClassBase* resource);
        void                 materialResourceUnload(hResourceClassBase* resource);

        hResourceClassBase*  meshResourceLoader(const hResourceSection* sections, hUint sectioncount);
        void                 meshResourceLink(hResourceManager* manager, hResourceClassBase* resource);
        void                 meshResourceUnlink(hResourceManager* manager, hResourceClassBase* resource);
        void                 meshResourceUnload(hResourceClassBase* resource);
#endif

        static hRenderer*    instance_;

        // Init params
        hSystem*												system_;
        hUint32 												width_;
        hUint32 												height_; 
        hUint32 												bpp_;
        hFloat													shaderVersion_;
        hBool													fullscreen_;
        hBool													vsync_;
        hFloat                                                  gpuTime_;

        gal::Device*             gal_;

        hdMutex                   resourceMutex_;
        BlendStateMapType        blendStates_;
        RasterizerStateMapType   rasterizerStates_;
        DepthStencilStateMapType depthStencilStates_;
        SamplerStateMapType      samplerStateMap_;

        hRendererCamera         renderCameras_[HEART_MAX_RENDER_CAMERAS];
        hRenderState*           renderStateCache_;
        hRenderMaterialManager  materialManager_;
        hRenderSubmissionCtx    mainSubmissionCtx_;

        hTexture*               backBuffer_;

        hUint32                                                 scratchBufferSize_;
        hByte                                                   drawDataScratchBuffer_[DEFAULT_SCRATCH_BUFFER_SIZE];
        hAtomicInt                                              scratchPtrOffset_;
        hAtomicInt                                              drawCallBlockIdx_;
        hArray< hDrawCall, MAX_DCBLOCKS >                       drawCallBlocks_;
        hAtomicInt                                              drawResourceUpdateCalls_;

        hTimer              frameTimer_;
        hRenderFrameStats   stats_;
        
        static void*											pRenderThreadID_;
    };

    class hLightingManager
    {
    public:

        struct hRenderTargetInfo
        {
            hResourceHandle vertexLightShader_;
            hResourceHandle pixelLightShader_;
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
            , quadLightData_(hNullptr)
            , sphereLightData_(hNullptr)
            , screenQuadIB_(hNullptr)
            , screenQuadVB_(hNullptr)
            , blendState_(hNullptr)
            , rasterState_(hNullptr)
            , depthStencilState_(hNullptr)
            , samplerState_(hNullptr)
        {
        }
        ~hLightingManager() {
            stopResourceEventListening();
            destroy();
        }

        void initialise(hRenderer* renderer, const hRenderTargetInfo* rndrinfo);

        void stopResourceEventListening();

        void destroy();
        void addDirectionalLight(const hVec3& direction, const hColour& colour);
        void addQuadLight(const hVec3& halfwidth, const hVec3& halfheight, const hVec3& centre, const hColour& colour);
        void enableSphereLight(hUint light, hBool enable);
        void setSphereLight(hUint light, const hVec3& centre, hFloat radius);

        void doDeferredLightPass(hRenderer* renderer, hRenderSubmissionCtx* ctx);
        hBool resourceUpdate(hStringID , hResurceEvent);

    private:

        struct hInputLightData
        {
            hMatrix viewMatrix_;
            hMatrix inverseViewMtx_;
            hMatrix projectionMtx_;
            hMatrix inverseProjectMtx_;
            hVec4   eyePos_;
            hUint   directionalLightCount_;
            hUint   quadLightCount_;
            hUint   sphereLightCount_;
        };

        struct hDirectionalLight
        {
            hVec3   direction_;
            hColour colour_;
        };

        struct hQuadLight 
        {
            hVec3   points_[4];
            hVec3   centre_;
            hVec3   halfv_[2];
            hColour colour_;
        };

        struct hSphereLightRenderData
        {
            hVec4   centreRadius_;
            hColour colour_;
        };

        struct hSphereLight : public hLinkedListElement<hSphereLight>
        {
            hVec4   centreRadius_;
            hColour colour_;
        };

        static const hUint s_maxDirectionalLights = 15;
        static const hUint s_maxQuadLights = 32;
        static const hUint s_maxSphereLights = 64;

        void drawDebugLightInfo();
        void generateRenderCommands(hRenderer* renderer);

        hRenderer*                                          renderer_;
        hRenderTargetInfo                                   targetInfo_;
        hdInputLayout*                                      inputLayout_;
        hRenderBuffer*                                      inputLightData_;
        hRenderBuffer*                                      directionLightData_;
        hRenderBuffer*                                      quadLightData_;
        hRenderBuffer*                                      sphereLightData_;
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
        hArray<hQuadLight, s_maxDirectionalLights>          quadLights_;
        hArray<hSphereLight, s_maxSphereLights>             sphereLights_;
        hLinkedList<hSphereLight>                           freeSphereLights_;
        hLinkedList<hSphereLight>                           activeSphereLights_;
    };
}

#endif // hrRenderer_h__