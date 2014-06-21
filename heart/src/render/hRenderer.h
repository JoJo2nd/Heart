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

#include "base/hTypes.h"
#include "base/hFunctor.h"
#include "base/hClock.h"
#include "base/hLinkedList.h"
#include "base/hArray.h"
#include "core/hResource.h"
#include "math/hMatrix.h"
#include "math/hVec3.h"
#include "math/hVec4.h"
#include "render/hRenderStateBlock.h"
#include "render/hRenderSubmissionContext.h"
#include "render/hRendererCamera.h"
#include "pal/dx11/hWin32DX11.h"

namespace Heart {

class hShaderProgram;

#define HEART_DEBUG_CAMERA_ID (13)
#define HEART_DEBUGUI_CAMERA_ID (14)

hFUNCTOR_TYPEDEF(void (*)(const hChar*, void**, hUint32*), hShaderIncludeCallback);
hFUNCTOR_TYPEDEF(void (*)(hRenderSubmissionCtx*), hCustomRenderCallback);

class hSystem;

namespace hRenderer {

    static const hUint32	DEFAULT_SCRATCH_BUFFER_SIZE = 1024*1024*8;
    static const hUint32    MAX_DCBLOCKS = (64*1024);
    static const hUint32    s_resoruceUpdateLimit = 1024;
    static const hUint32    s_scratchBufferCount = 5;

    void					create(hSystem* pSystem, hUint32 width, hUint32 height, hUint32 bpp, hFloat shaderVersion, hBool fullscreen, hBool vsync);
    void					destroy();
    hFORCEINLINE hFloat                  getRatio() { return 1.f; }

    ///////////////////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////////

    const hRenderFrameStats* getRenderStats();

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    hRenderSubmissionCtx* createRenderSubmissionCtx();
    void                  destroyRenderSubmissionCtx( hRenderSubmissionCtx* ctx );
    /*
        pimpl methods
    */
    void  compileShaderStageFromSource(const hChar* shaderProg, hUint32 len, const hChar* entry, hShaderProfile profile, hShaderProgram* out);
    void  createShaderStage(const hChar* shaderProg, hUint32 len, hShaderType type, hShaderProgram* out);
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
//private:  
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
//public:
    void* allocTempRenderMemory( hUint32 size );
    /*
        end new engine design methods
    */
    void BeginRenderFrame();
    void EndRenderFrame();
    void rendererFrameSubmit();
    hFloat getLastGPUTime();
    hBool isRenderThread();

    // typedef hMap< hUint32, hBlendState >         BlendStateMapType;
    // typedef hMap< hUint32, hRasterizerState >    RasterizerStateMapType;
    // typedef hMap< hUint32, hDepthStencilState >  DepthStencilStateMapType;
    // typedef hMap< hUint32, hSamplerState >       SamplerStateMapType;

    // //
    // void                                                    CollectAndSortDrawCalls();
    // void                                                    SubmitDrawCallsMT();
    // void                                                    SubmitDrawCallsST();
    // void                                                    createDebugShadersInternal();

    // static hRenderer*    instance_;

    // // Init params
    // hSystem*												system_;
    // hUint32 												width_;
    // hUint32 												height_; 
    // hUint32 												bpp_;
    // hFloat													shaderVersion_;
    // hBool													fullscreen_;
    // hBool													vsync_;
    // hFloat                                                  gpuTime_;

    // hMutex                   resourceMutex_;
    // BlendStateMapType        blendStates_;
    // RasterizerStateMapType   rasterizerStates_;
    // DepthStencilStateMapType depthStencilStates_;
    // SamplerStateMapType      samplerStateMap_;

    // hRendererCamera         renderCameras_[HEART_MAX_RENDER_CAMERAS];
    // void*                   renderStateCache_; // todo: remove
    // hRenderSubmissionCtx    mainSubmissionCtx_;

    // hTexture*               backBuffer_;

    // hUint32                                                 scratchBufferSize_;
    // hByte                                                   drawDataScratchBuffer_[DEFAULT_SCRATCH_BUFFER_SIZE];
    // hAtomicInt                                              scratchPtrOffset_;
    // hAtomicInt                                              drawCallBlockIdx_;
    // hArray< hDrawCall, MAX_DCBLOCKS >                       drawCallBlocks_;
    // hAtomicInt                                              drawResourceUpdateCalls_;

    // hTimer              frameTimer_;
    // hRenderFrameStats   stats_;
}
}
#endif // hrRenderer_h__
