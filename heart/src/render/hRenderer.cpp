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

#include "render/hRenderer.h"
#include "render/hDebugDraw.h"
#include "base/hProfiler.h"
#include "base/hFunctor.h"
#include "render/hTexture.h"
#include "render/hIndexBuffer.h"
#include "render/hVertexBuffer.h"
#include "render/hViews.h"
#include "base/hStringUtil.h"
#include "base/hCRC32.h"
#include "render/hParameterConstBlock.h"
#include "render/hRenderShaderProgram.h"
#include "render/hRenderUtility.h"

namespace Heart {
namespace hRenderer {

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    
    void* RnTmpMalloc( hUint32 size )
    {
        return hMalloc(size);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    
    void RnTmpFree( void* ptr )
    {
        hFree(ptr);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void Create( 
        hSystem* pSystem, 
        hUint32 width, 
        hUint32 height, 
        hUint32 bpp, 
        hFloat shaderVersion, 
        hBool fullscreen, 
        hBool vsync)
    {
#if 0
        // frameTimer_.reset();
 
        // width_			= width;
        // height_			= height;
        // bpp_			= bpp;
        // shaderVersion_	= shaderVersion;
        // fullscreen_		= fullscreen;
        // vsync_			= vsync;
        // system_			=  pSystem;
        // backBuffer_     = hNullptr;
 
        // hAtomic::AtomicSet(scratchPtrOffset_, 0);
        // hAtomic::AtomicSet(drawCallBlockIdx_, 0);

        //depthBuffer_=new GetGlobalHeap(), hTexture(
        //    hFUNCTOR_BINDMEMBER(hTexture::hZeroRefProc, hRenderer, destroyTexture, this), eRenderResourceType_Tex2D, GetGlobalHeap());

        hRenderDeviceSetup setup;
        setup.alloc_ = RnTmpMalloc;
        setup.free_ = RnTmpFree;
        setup.depthBufferTex_ = hNullptr;
        //setup.depthBufferTex_= depthBuffer_;
        //ParentClass::Create( system_, width_, height_, fullscreen_, vsync_, setup );

        ParentClass::InitialiseMainRenderSubmissionCtx(&mainSubmissionCtx_.impl_);

//         depthBuffer_->format_=TFORMAT_D24S8F;
//         depthBuffer_->nLevels_=1;
//         depthBuffer_->levelDescs_=new GetGlobalHeap(), hTexture::LevelDesc[1];
//         depthBuffer_->levelDescs_->mipdata_=NULL;
//         depthBuffer_->levelDescs_->mipdataSize_=0;
//         depthBuffer_->levelDescs_->width_ = GetWidth();
//         depthBuffer_->levelDescs_->height_ = GetHeight();


//         const hChar* bbalias[] = {
//             "back_buffer",
//             "g_back_buffer",
//         };
//         materialManager_.registerGlobalTexture("back_buffer", backBuffer_, bbalias, hStaticArraySize(bbalias));
//         const hChar* dbalias[] = {
//             "depth_buffer",
//             "g_depth_buffer",
//             "z_buffer",
//             "g_z_buffer",
//         };
//         materialManager_.registerGlobalTexture("depth_buffer", depthBuffer_, dbalias, hStaticArraySize(dbalias));

        createDebugShadersInternal();
#else
        hStub();
#endif    
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void Destroy()
    {
#if 0
        instance_ = nullptr;
        for (hUint32 i = 0; i < HEART_MAX_RENDER_CAMERAS; ++i)
        {
            GetRenderCamera(i)->releaseRenderTargetSetup();
        }

        hShaderProgram* prog=hNullptr;
        prog=hResourceHandle(hDebugShaderResourceID_PixelWhite).weakPtr<hShaderProgram>();
        resourceManager_->removeResource(hDebugShaderResourceID_PixelWhite);
        prog->DecRef();
        prog=hResourceHandle(hDebugShaderResourceID_VertexPosOnly).weakPtr<hShaderProgram>();
        resourceManager_->removeResource(hDebugShaderResourceID_VertexPosOnly);
        prog->DecRef();
        prog=hResourceHandle(hDebugShaderResourceID_ConsoleVertex).weakPtr<hShaderProgram>();
        resourceManager_->removeResource(hDebugShaderResourceID_ConsoleVertex);
        prog->DecRef();
        prog=hResourceHandle(hDebugShaderResourceID_ConsolePixel).weakPtr<hShaderProgram>();
        resourceManager_->removeResource(hDebugShaderResourceID_ConsolePixel);
        prog->DecRef();
        prog=hResourceHandle(hDebugShaderResourceID_FontVertex).weakPtr<hShaderProgram>();
        resourceManager_->removeResource(hDebugShaderResourceID_FontVertex);
        prog->DecRef();
        prog=hResourceHandle(hDebugShaderResourceID_FontPixel).weakPtr<hShaderProgram>();
        resourceManager_->removeResource(hDebugShaderResourceID_FontPixel);
        prog->DecRef();
        prog=hResourceHandle(hDebugShaderResourceID_VertexPosNormal).weakPtr<hShaderProgram>();
        resourceManager_->removeResource(hDebugShaderResourceID_VertexPosNormal);
        prog->DecRef();
        prog=hResourceHandle(hDebugShaderResourceID_PixelWhiteViewLit).weakPtr<hShaderProgram>();
        resourceManager_->removeResource(hDebugShaderResourceID_PixelWhiteViewLit);
        prog->DecRef();
        prog=hResourceHandle(hDebugShaderResourceID_TexVertex).weakPtr<hShaderProgram>();
        resourceManager_->removeResource(hDebugShaderResourceID_TexVertex);
        prog->DecRef();
        prog=hResourceHandle(hDebugShaderResourceID_TexPixel).weakPtr<hShaderProgram>();
        resourceManager_->removeResource(hDebugShaderResourceID_TexPixel);
        prog->DecRef();
        prog=hResourceHandle(hDebugShaderResourceID_VertexPosCol).weakPtr<hShaderProgram>();
        resourceManager_->removeResource(hDebugShaderResourceID_VertexPosCol);
        prog->DecRef();
        prog=hResourceHandle(hDebugShaderResourceID_PixelPosCol).weakPtr<hShaderProgram>();
        resourceManager_->removeResource(hDebugShaderResourceID_PixelPosCol);
        prog->DecRef();

        //delete[] GetGlobalHeap(), depthBuffer_->levelDescs_;  = nullptr;
        //depthBuffer_->DecRef();

        ParentClass::Destroy();

        hDebugDrawRenderer::it()->destroyResources();
#else
        hStub();
#endif
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void BeginRenderFrame()
    {
#if 0        
        HEART_PROFILE_FUNC();
        //ReleasePendingRenderResources();
        //Start new frame

        //Free last frame draw calls and temporary memory
        hAtomic::AtomicSet(scratchPtrOffset_, 0);
        hAtomic::AtomicSet(drawCallBlockIdx_, 0);
        hAtomic::AtomicSet(drawResourceUpdateCalls_, 0);

        ParentClass::BeginRender(&gpuTime_);
#else
        hStub();
#endif
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    
    void EndRenderFrame()
    {
#if 0
        HEART_PROFILE_FUNC();


        ParentClass::EndRender();
        hDebugDrawRenderer::it()->render(this, &mainSubmissionCtx_);
        ParentClass::SwapBuffers(backBuffer_);

        hZeroMem(&stats_, sizeof(stats_));
        stats_.gpuTime_=0.f;
        stats_.frametime_=(hFloat)frameTimer_.elapsedMilliSec();
        frameTimer_.reset();
        mainSubmissionCtx_.appendRenderStats(&stats_);
        mainSubmissionCtx_.resetStats();
#else
        hStub();
#endif        
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void createTexture(hUint32 levels, hMipDesc* initialData, hTextureFormat format, hUint32 flags, hTexture** outTex)
    {
#if 0
        hcAssert(initialData);
        hcAssert(levels > 0);

        (*outTex) = new hTexture(
            hFUNCTOR_BINDMEMBER(hTexture::hZeroRefProc, hRenderer, destroyTexture, this),
            eRenderResourceType_Tex2D);

        (*outTex)->nLevels_ = levels;
        (*outTex)->format_ = format;
        (*outTex)->flags_ = flags;
        (*outTex)->levelDescs_ = levels ? new hTexture::LevelDesc[levels] : NULL;

        for (hUint32 i = 0; i < levels; ++i)
        {
            (*outTex)->levelDescs_[ i ].width_ = initialData[i].width;
            (*outTex)->levelDescs_[ i ].height_ = initialData[i].height;
            (*outTex)->levelDescs_[ i ].mipdata_ = hNullptr;//initialData[i].data;
            (*outTex)->levelDescs_[ i ].mipdataSize_ = 0;//initialData[i].size;
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
#else
        hStub();
#endif
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void resizeTexture(hUint32 width, hUint32 height, hTexture* inout)
    {
#if 0
        hcAssert(width > 0 && height > 0 && inout);
        //TODO: do a down-size render? Atm this is only used for render targets so content is throw away.
        if (inout == backBuffer_) {
            backBuffer_=hNullptr;
        }
        hMipDesc mipsdata[16];// Based on the largest texture dim is 64K (is that even possible? 13 should be safe)
        hUint lvls=0;
        hUint w=width;
        hUint h=height;

        while (w >= 1 && h >= 1 && lvls < 16 && lvls < inout->nLevels_) {
            mipsdata[lvls].width=w;
            mipsdata[lvls].height=h;
            mipsdata[lvls].data=hNullptr;
            mipsdata[lvls].size=0;
            w /= 2;
            h /= 2;
            ++lvls;
        }

        ParentClass::destroyTextureDevice(inout);
        ParentClass::createTextureDevice(lvls, inout->format_, mipsdata, inout->flags_, inout);
        delete[] inout->levelDescs_;
        inout->levelDescs_ = nullptr;
        inout->nLevels_ = lvls;
        inout->levelDescs_ = lvls ? new hTexture::LevelDesc[lvls] : hNullptr;

        for (hUint32 i = 0; i < lvls; ++i) {
            inout->levelDescs_[ i ].width_       = mipsdata[i].width;
            inout->levelDescs_[ i ].height_      = mipsdata[i].height;
            inout->levelDescs_[ i ].mipdata_     = mipsdata[i].data;
            inout->levelDescs_[ i ].mipdataSize_ = mipsdata[i].size;
        }
#else
        hStub();
#endif
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////////

    void destroyTexture(hTexture* pOut)
    {
#if 0
        hcAssert(pOut->GetRefCount() == 0);
        ParentClass::destroyTextureDevice(pOut);
        delete pOut; pOut = nullptr;
#else
        hStub();
#endif
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void createIndexBuffer(const void* pIndices, hUint32 nIndices, hUint32 flags, hIndexBuffer** outIB)
    {
#if 0
        hUint elementSize= nIndices > 0xFFFF ? sizeof(hUint32) : sizeof(hUint16);
        hIndexBuffer* pdata = new hIndexBuffer(
            hFUNCTOR_BINDMEMBER(hIndexBuffer::hZeroProc, hRenderer, destroyIndexBuffer, this));
        pdata->pIndices_ = NULL;
        pdata->nIndices_ = nIndices;
        pdata->type_= nIndices > 0xFFFF ? hIndexBufferType_Index32 : hIndexBufferType_Index16;
        ParentClass::createIndexBufferDevice(nIndices*elementSize, pIndices, flags, pdata);
        *outIB = pdata;
#else
        hStub();
#endif
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void destroyIndexBuffer(hIndexBuffer* ib)
    {
#if 0
        hcAssert(ib);
        ParentClass::destroyIndexBufferDevice(ib);
        delete ib; ib = nullptr;
#else
        hStub();
#endif
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void createVertexBuffer(const void* initData, hUint32 nElements, hInputLayoutDesc* desc, hUint32 desccount, hUint32 flags, hVertexBuffer** outVB)
    {
#if 0        
        hVertexBuffer* pdata = new hVertexBuffer(
            hFUNCTOR_BINDMEMBER(hVertexBufferZeroProc, hRenderer, destroyVertexBuffer, this));
        pdata->vtxCount_ = nElements;
        pdata->stride_ = ParentClass::computeVertexLayoutStride( desc, desccount );
        ParentClass::createVertexBufferDevice(desc, desccount, pdata->stride_, nElements*pdata->stride_, initData, flags, pdata);
        *outVB = pdata;
#else
        hStub();
#endif        
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void destroyVertexBuffer(hVertexBuffer* vb)
    {
#if 0        
        hcAssert(vb);
        ParentClass::destroyVertexBufferDevice(vb);
        delete vb; vb = nullptr;
#else
        hStub();
#endif        
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hRenderSubmissionCtx* CreateRenderSubmissionCtx()
    {
#if 0
        hRenderSubmissionCtx* ret = new hRenderSubmissionCtx;
        ret->Initialise( this );
       
        ParentClass::InitialiseRenderSubmissionCtx( &ret->impl_ );

        return ret;
#else
        hStub();
        return nullptr;
#endif 
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void DestroyRenderSubmissionCtx( hRenderSubmissionCtx* ctx )
    {
#if 0        
        hcAssert( ctx );
        ParentClass::DestroyRenderSubmissionCtx( &ctx->impl_ );
        delete ctx; ctx = nullptr;
#else
        hStub();
#endif        
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hBool isRenderThread()
    {
#if 0        
        //means nothing now...
        return pRenderThreadID_ == Device::GetCurrentThreadID();
#else
        hStub();
        return hFalse;
#endif        
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void SubmitDrawCallBlock( hDrawCall* block, hUint32 count )
    {
#if 0  
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
#else
        hStub();
#endif         
    }
  

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void* allocTempRenderMemory( hUint32 size )
    {
#if 0        
        hUint32 ret;
        hAtomic::AtomicAddWithPrev(scratchPtrOffset_, size, &ret);
        return drawDataScratchBuffer_+ret;
#else
        hStub();
        return nullptr;
#endif        
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void createDebugShadersInternal() {
#if 0
        hShaderProgram* prog=new hShaderProgram)(this, hFUNCTOR_BINDMEMBER(hShaderProgram::hZeroProc, hRenderer, destroyShader, this);
        ParentClass::compileShaderFromSourceDevice(
            ParentClass::getDebugShaderSource(eDebugPixelWhite),
            hStrLen(ParentClass::getDebugShaderSource(eDebugPixelWhite)),
            "mainFP", eShaderProfile_ps4_0, hNullptr, hNullptr, 0, prog);
        resourceManager_->insertResource(hDebugShaderResourceID_PixelWhite, prog);

        prog=new hShaderProgram)(this, hFUNCTOR_BINDMEMBER(hShaderProgram::hZeroProc, hRenderer, destroyShader, this);
        ParentClass::compileShaderFromSourceDevice(
            ParentClass::getDebugShaderSource(eDebugVertexPosOnly),
            hStrLen(ParentClass::getDebugShaderSource(eDebugVertexPosOnly)),
            "mainVP", eShaderProfile_vs4_0, hNullptr, hNullptr, 0, prog);
        resourceManager_->insertResource(hDebugShaderResourceID_VertexPosOnly, prog);

        prog=new hShaderProgram)(this, hFUNCTOR_BINDMEMBER(hShaderProgram::hZeroProc, hRenderer, destroyShader, this);
        ParentClass::compileShaderFromSourceDevice(
            ParentClass::getDebugShaderSource(eConsoleVertex),
            hStrLen(ParentClass::getDebugShaderSource(eConsoleVertex)),
            "mainVP", eShaderProfile_vs4_0, hNullptr, hNullptr, 0, prog);
        resourceManager_->insertResource(hDebugShaderResourceID_ConsoleVertex, prog);

        prog=new hShaderProgram)(this, hFUNCTOR_BINDMEMBER(hShaderProgram::hZeroProc, hRenderer, destroyShader, this);
        ParentClass::compileShaderFromSourceDevice(
            ParentClass::getDebugShaderSource(eConsolePixel),
            hStrLen(ParentClass::getDebugShaderSource(eConsolePixel)),
            "mainFP", eShaderProfile_ps4_0, hNullptr, hNullptr, 0, prog);
        resourceManager_->insertResource(hDebugShaderResourceID_ConsolePixel, prog);

        prog=new hShaderProgram)(this, hFUNCTOR_BINDMEMBER(hShaderProgram::hZeroProc, hRenderer, destroyShader, this);
        ParentClass::compileShaderFromSourceDevice(
            ParentClass::getDebugShaderSource(eDebugFontVertex),
            hStrLen(ParentClass::getDebugShaderSource(eDebugFontVertex)),
            "mainVP", eShaderProfile_vs4_0, hNullptr, hNullptr, 0, prog);
        resourceManager_->insertResource(hDebugShaderResourceID_FontVertex, prog);

        prog=new hShaderProgram)(this, hFUNCTOR_BINDMEMBER(hShaderProgram::hZeroProc, hRenderer, destroyShader, this);
        ParentClass::compileShaderFromSourceDevice(
            ParentClass::getDebugShaderSource(eDebugFontPixel),
            hStrLen(ParentClass::getDebugShaderSource(eDebugFontPixel)),
            "mainFP", eShaderProfile_ps4_0, hNullptr, hNullptr, 0, prog);
        resourceManager_->insertResource(hDebugShaderResourceID_FontPixel, prog);

        prog=new hShaderProgram)(this, hFUNCTOR_BINDMEMBER(hShaderProgram::hZeroProc, hRenderer, destroyShader, this);
        ParentClass::compileShaderFromSourceDevice(
            ParentClass::getDebugShaderSource(eDebugVertexPosNormal),
            hStrLen(ParentClass::getDebugShaderSource(eDebugVertexPosNormal)),
            "mainVP", eShaderProfile_vs4_0, hNullptr, hNullptr, 0, prog);
        resourceManager_->insertResource(hDebugShaderResourceID_VertexPosNormal, prog);

        prog=new hShaderProgram)(this, hFUNCTOR_BINDMEMBER(hShaderProgram::hZeroProc, hRenderer, destroyShader, this);
        ParentClass::compileShaderFromSourceDevice(
            ParentClass::getDebugShaderSource(eDebugPixelWhiteViewLit),
            hStrLen(ParentClass::getDebugShaderSource(eDebugPixelWhiteViewLit)),
            "mainFP", eShaderProfile_ps4_0, hNullptr, hNullptr, 0, prog);
        resourceManager_->insertResource(hDebugShaderResourceID_PixelWhiteViewLit, prog);

        prog=new hShaderProgram)(this, hFUNCTOR_BINDMEMBER(hShaderProgram::hZeroProc, hRenderer, destroyShader, this);
        ParentClass::compileShaderFromSourceDevice(
            ParentClass::getDebugShaderSource(eDebugTexVertex),
            hStrLen(ParentClass::getDebugShaderSource(eDebugTexVertex)),
            "mainVP", eShaderProfile_vs4_0, hNullptr, hNullptr, 0, prog);
        resourceManager_->insertResource(hDebugShaderResourceID_TexVertex, prog);

        prog=new hShaderProgram)(this, hFUNCTOR_BINDMEMBER(hShaderProgram::hZeroProc, hRenderer, destroyShader, this);
        ParentClass::compileShaderFromSourceDevice(
            ParentClass::getDebugShaderSource(eDebugTexPixel),
            hStrLen(ParentClass::getDebugShaderSource(eDebugTexPixel)),
            "mainFP", eShaderProfile_ps4_0, hNullptr, hNullptr, 0, prog);
        resourceManager_->insertResource(hDebugShaderResourceID_TexPixel, prog);

        prog=new hShaderProgram)(this, hFUNCTOR_BINDMEMBER(hShaderProgram::hZeroProc, hRenderer, destroyShader, this);
        ParentClass::compileShaderFromSourceDevice(
            ParentClass::getDebugShaderSource(eDebugVertexPosCol),
            hStrLen(ParentClass::getDebugShaderSource(eDebugVertexPosCol)),
            "mainVP", eShaderProfile_vs4_0, hNullptr, hNullptr, 0, prog);
        resourceManager_->insertResource(hDebugShaderResourceID_VertexPosCol, prog);

        prog=new hShaderProgram)(this, hFUNCTOR_BINDMEMBER(hShaderProgram::hZeroProc, hRenderer, destroyShader, this);
        ParentClass::compileShaderFromSourceDevice(
            ParentClass::getDebugShaderSource(eDebugPixelPosCol),
            hStrLen(ParentClass::getDebugShaderSource(eDebugPixelPosCol)),
            "mainFP", eShaderProfile_ps4_0, hNullptr, hNullptr, 0, prog);
        resourceManager_->insertResource(hDebugShaderResourceID_PixelPosCol, prog);
#else
        hcPrintf("Stub "__FUNCTION__);
#endif

    }

    void rendererFrameSubmit()
    {
#if 0        
        HEART_PROFILE_FUNC();
        CollectAndSortDrawCalls();
        SubmitDrawCallsMT();
#else
        hStub();
#endif        
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void createShaderResourceView(hTexture* tex, const hShaderResourceViewDesc& desc, hShaderResourceView** outsrv) {
#if 0
        (*outsrv) = new hShaderResourceView(
            hFUNCTOR_BINDMEMBER(hShaderResourceViewZeroRefProc, hRenderer, destroyShaderResourceView, this));
        ParentClass::createShaderResourseViewDevice(tex, desc, *outsrv);
        (*outsrv)->refType_=desc.resourceType_;
        (*outsrv)->refTex_=tex;
        tex->AddRef();
#else
        hStub();
#endif
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void createShaderResourceView(hRenderBuffer* cb, const hShaderResourceViewDesc& desc, hShaderResourceView** outsrv) {
#if 0
        (*outsrv) = new hShaderResourceView(
            hFUNCTOR_BINDMEMBER(hShaderResourceViewZeroRefProc, hRenderer, destroyShaderResourceView, this));
        ParentClass::createShaderResourseViewDevice(cb, desc, *outsrv);
        (*outsrv)->refType_=desc.resourceType_;
        (*outsrv)->refCB_=cb;
        cb->AddRef();
#else
        hStub();
#endif
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void destroyShaderResourceView(hShaderResourceView* srv) {
#if 0
        hcAssert(srv);
        hcAssert(srv->GetRefCount() == 0);
        ParentClass::destroyShaderResourceViewDevice(srv);
        delete srv; srv = nullptr;
#else
        hStub();
#endif        
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void createRenderTargetView(hTexture* tex, const hRenderTargetViewDesc& rtvd, hRenderTargetView** outrtv) {
#if 0
        (*outrtv) = new hRenderTargetView(
            hFUNCTOR_BINDMEMBER(hRenderTargetViewZeroRefProc, hRenderer, destroyRenderTargetView, this));
        ParentClass::createRenderTargetViewDevice(tex, rtvd, *outrtv);
        (*outrtv)->bindTexture(tex);
#else
        hStub();
#endif
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void createDepthStencilView(hTexture* tex, const hDepthStencilViewDesc& dsvd, hDepthStencilView** outdsv) {
#if 0
        (*outdsv) = new hDepthStencilView(
            hFUNCTOR_BINDMEMBER(hDepthStencilViewZeroRefProc, hRenderer, destroyDepthStencilView, this));
        ParentClass::createDepthStencilViewDevice(tex, dsvd, *outdsv);
        (*outdsv)->bindTexture(tex);
#else
        hStub();
#endif
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void destroyRenderTargetView(hRenderTargetView* view) {
#if 0
        hcAssert(view);
        hcAssert(view->GetRefCount() == 0);
        ParentClass::destroyRenderTargetViewDevice(view);
        delete view; view = nullptr;
#else
        hStub();
#endif
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void destroyDepthStencilView(hDepthStencilView* view) {
#if 0
        hcAssert(view);
        hcAssert(view->GetRefCount() == 0);
        ParentClass::destroyDepthStencilViewDevice(view);
        delete view; view = nullptr;
#else
        hStub();
#endif
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hBlendState* createBlendState(const hBlendStateDesc& desc) {
#if 0
        //Build state key
        hUint32 stateKey = hCRC32::FullCRC( (const hChar*)&desc, sizeof(desc) );
        resourceMutex_.Lock();
        hBlendState* state = blendStates_.Find(stateKey);
        if ( !state ) {
            state = new hBlendState(
                hFUNCTOR_BINDMEMBER(hBlendZeroRefProc, hRenderer, destroyBlendState, this));
            ParentClass::createBlendStateDevice(desc, state);
            blendStates_.Insert(stateKey, state);
        }
        else {
            state->AddRef();
        }
        resourceMutex_.Unlock();
        return state;
#else
        hStub();
        return nullptr;
#endif
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void destroyBlendState(hBlendState* state) {
#if 0
        hcAssert(state->GetRefCount() == 0);
        resourceMutex_.Lock();
        blendStates_.Remove(state->GetKey());
        ParentClass::destroyBlendStateDevice(state);
        delete state;
        resourceMutex_.Unlock();
#else
        hStub();
#endif
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hRasterizerState* createRasterizerState(const hRasterizerStateDesc& desc) {
#if 0
        //Build state key
        hUint32 stateKey = hCRC32::FullCRC( (const hChar*)&desc, sizeof(desc) );
        resourceMutex_.Lock();
        hRasterizerState* state = rasterizerStates_.Find(stateKey);
        if ( !state ) {
            state = new hRasterizerState(
                hFUNCTOR_BINDMEMBER(hRasterizerZeroRefProc, hRenderer, destoryRasterizerState, this));
            ParentClass::createRasterizerStateDevice(desc, state);
            rasterizerStates_.Insert(stateKey, state);
        }
        else {
            state->AddRef();
        }
        resourceMutex_.Unlock();
        return state;
#else
        hStub();
        return nullptr;
#endif
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void destoryRasterizerState(hRasterizerState* state) {
#if 0
        hcAssert(state->GetRefCount() == 0);
        resourceMutex_.Lock();
        rasterizerStates_.Remove(state->GetKey());
        ParentClass::destroyRasterizerStateDevice(state);
        delete state;
        resourceMutex_.Unlock();
#else
        hStub();
#endif
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hDepthStencilState* createDepthStencilState(const hDepthStencilStateDesc& desc) {
#if 0
        //Build state key
        hUint32 stateKey = hCRC32::FullCRC( (const hChar*)&desc, sizeof(desc) );
        resourceMutex_.Lock();
        hDepthStencilState* state = depthStencilStates_.Find(stateKey);
        if ( !state ) {
            state = new hDepthStencilState(
                hFUNCTOR_BINDMEMBER(hDepthStencilZeroRefProc, hRenderer, destroyDepthStencilState, this));
            ParentClass::createDepthStencilStateDevice(desc, state);
            depthStencilStates_.Insert(stateKey, state);
        }
        else {
            state->AddRef();
        }
        resourceMutex_.Unlock();
        return state;
#else
        hStub();
        return nullptr;
#endif
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void destroyDepthStencilState(hDepthStencilState* state) {
#if 0
        hcAssert(state->GetRefCount() == 0);
        resourceMutex_.Lock();
        depthStencilStates_.Remove(state->GetKey());
        ParentClass::destroyDepthStencilStateDevice(state);
        delete state;
        resourceMutex_.Unlock();
#else
        hStub();
#endif
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hSamplerState* createSamplerState(const hSamplerStateDesc& desc) {
#if 0
        //Build state key
        hUint32 stateKey = hCRC32::FullCRC( (const hChar*)&desc, sizeof(desc) );
        resourceMutex_.Lock();
        hSamplerState* state = samplerStateMap_.Find(stateKey);
        if ( !state ) {
            state = new hSamplerState(
                hFUNCTOR_BINDMEMBER(hSamplerZeroRefProc, hRenderer, destroySamplerState, this));
            ParentClass::createSamplerStateDevice(desc, state);
            samplerStateMap_.Insert(stateKey, state);
        }
        else {
            state->AddRef();
        }
        resourceMutex_.Unlock();
        return state;
#else
        hStub();
        return nullptr;
#endif
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void destroySamplerState(hSamplerState* state) {
#if 0
        hcAssert(state->GetRefCount() == 0);
        resourceMutex_.Lock();
        samplerStateMap_.Remove(state->GetKey());
        ParentClass::destroySamplerStateDevice(state);
        delete state;
        resourceMutex_.Unlock();
#else
        hStub();
#endif
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void createBuffer(hUint size, void* data, hUint flags, hUint stride, hRenderBuffer** outcb) {
#if 0
        (*outcb) = new hRenderBuffer(
            hFUNCTOR_BINDMEMBER(hRenderBuffer::hZeroRefProc, hRenderer, destroyConstantBlock, this));
        ParentClass::createBufferDevice(size, data, flags, stride, *outcb);
#else
        hStub();
#endif
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void destroyConstantBlock(hRenderBuffer* block) {
#if 0
        ParentClass::destroyConstantBlockDevice(block);
        delete block;
#else
        hStub();
#endif
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void compileShaderFromSource(const hChar* shaderProg, hUint32 len, 
        const hChar* entry, hShaderProfile profile, hIIncludeHandler* includes, hShaderDefine* defines, hUint ndefines, hShaderProgram* out) {
#if 0
        ParentClass::compileShaderFromSourceDevice(shaderProg, len, entry, profile, includes, defines, ndefines, out);
        if (profile >= eShaderProfile_vs4_0 && profile <= eShaderProfile_vs5_0) {
            out->SetShaderType(ShaderType_VERTEXPROG);
        } else if (profile >= eShaderProfile_ps4_0 && profile <= eShaderProfile_ps5_0) {
            out->SetShaderType(ShaderType_FRAGMENTPROG);
        } else if (profile >= eShaderProfile_gs4_0 && profile <= eShaderProfile_gs5_0) {
            out->SetShaderType(ShaderType_GEOMETRYPROG);
        } else if (profile >= eShaderProfile_cs4_0 && profile <= eShaderProfile_cs5_0) {
            out->SetShaderType(ShaderType_COMPUTEPROG);
        } else if (profile == eShaderProfile_hs5_0) {
            out->SetShaderType(ShaderType_HULLPROG);
        } else if (profile == eShaderProfile_ds5_0) {
            out->SetShaderType(ShaderType_DOMAINPROG);
        }
#else
        hStub();
#endif
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void createShader(const hChar* shaderProg, hUint32 len, hShaderType type, hShaderProgram* out) {
#if 0
        ParentClass::compileShaderDevice(shaderProg, len, type, out);
        out->SetShaderType(type);
#else
        hStub();
#endif
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void destroyShader(hShaderProgram* prog) {
#if 0        
        ParentClass::destroyShaderDevice(prog);
        delete prog;
#endif
    }

#if 0
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hResourceClassBase* hRenderer::textureResourceLoader(const hResourceSection* sections, hUint sectioncount) {
        Heart::hTexture* texture=hNullptr;
        Heart::hMipDesc mips[24];
        for (hUint i=0, n=sectioncount; i<n; ++i) {
            if (hStrCmp("texture", sections[i].sectionName_) == 0) {
                google::protobuf::io::ArrayInputStream input(sections[i].sectionData_, sections[i].sectionSize_);
                google::protobuf::io::CodedInputStream inputstream(&input);
                proto::TextureResource texresource;
                hBool ok=texresource.ParseFromCodedStream(&inputstream);
                hcAssertMsg(ok,"Failed to parse Texture Resource");
                if (!ok) {
                    return hNullptr;
                }
                for (hUint32 im = 0, nm=texresource.mips_size(); im < nm; ++im) {
                    mips[im].width=texresource.mips(im).width();
                    mips[im].height=texresource.mips(im).height();
                    mips[im].data=(hByte*)texresource.mips(im).data().c_str();
                    mips[im].size=(hUint)texresource.mips(im).data().length();
                }
                createTexture(texresource.mips_size(), mips, (Heart::hTextureFormat)texresource.format(), 0, &texture);
            }
        }

        return texture;
/*
        Heart::hTexture* texutre;
        Heart::hMipDesc* mips = NULL;
        hUint32 totalTextureSize = 0;
        hByte* textureData = NULL;
        TextureHeader header = {0};

        file->Read(&header, sizeof(header));
        mips = (Heart::hMipDesc*)hAlloca(header.mipCount*sizeof(Heart::hMipDesc));
        //Read mip info
        file->Read(mips, header.mipCount*sizeof(Heart::hMipDesc));
        //Add up the size need for textures
        for (hUint32 i = 0; i < header.mipCount; ++i) {
            mips[i].data = (hByte*)totalTextureSize;
            totalTextureSize += mips[i].size;
        }
        textureData = (hByte*)hMalloc("general", totalTextureSize);
        for (hUint32 i = 0; i < header.mipCount; ++i) {
            mips[i].data = textureData + (hUint32)(mips[i].data);
        }
        //Read Texture data
        file->Read(textureData, totalTextureSize);
        createTexture(header.mipCount, mips, header.format, header.flags, &texutre);
        return texutre;
*/
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderer::textureResourcePostLoad(hResourceManager* manager, hResourceClassBase* texture) {
        manager->insertResource(texture->getResourceID(), texture);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderer::textureResourcePreUnload(hResourceManager* manager, hResourceClassBase* texture) {
        manager->removeResource(texture->getResourceID());
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderer::textureResourceUnload(hResourceClassBase* resource) {
        hTexture* tex=static_cast<hTexture*>(resource);
        // Package should be the only thing hold ref at this point...
        hcAssertMsg(tex->GetRefCount() == 1, "Texture ref count is %u, it should be 1", tex->GetRefCount());
        tex->DecRef();
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hResourceClassBase* hRenderer::shaderResourceLoader(const hResourceSection* sections, hUint sectioncount) {
        hShaderProgram* shaderProg=hNullptr;
        hBool prefersource=hFalse;
        proto::ShaderResource shaderres;
#if 0
#if defined (HEART_ALLOW_SHADER_SOURCE_COMPILE)
        class hIncluder : public hIIncludeHandler 
        {
        public:
            hIncluder(const hResourceSection* sections, hUint sectioncount) 
                : sections_(sections)
                , sectioncount_(sectioncount)
            {}
            void findInclude(const hChar* includepath, const void** outdata, hUint* outlen) {
                *outdata=hNullptr;
                *outlen=0;
                for (hUint i=0, n=sectioncount_; i<n; ++i) {
                    if (hStrCmp(includepath, sections_[i].sectionName_) == 0) {
                        *outdata=sections_[i].sectionData_;
                        *outlen=sections_[i].sectionSize_;
                        return;
                    }
                }
            }

            const hResourceSection* sections_;
            hUint sectioncount_;
        };
        hIncluder includes(sections, sectioncount);
        for (hUint i=0, n=sectioncount; i<n; ++i) {
            if (hStrCmp("source", sections[i].sectionName_) == 0) {
                prefersource=hTrue;
            } else if (hStrCmp("shader", sections[i].sectionName_) == 0 && !prefersource) {
                google::protobuf::io::ArrayInputStream input(sections[i].sectionData_, sections[i].sectionSize_);
                google::protobuf::io::CodedInputStream inputstream(&input);
                hBool ok=shaderres.ParseFromCodedStream(&inputstream);
                hcAssertMsg(ok,"Failed to parse Shader Resource");
                if (!ok) {
                    return hNullptr;
                }
            }
        }
#endif
        for (hUint i=0, n=sectioncount; i<n; ++i) {
            if (hStrCmp("shader", sections[i].sectionName_) == 0 && !prefersource) {
                google::protobuf::io::ArrayInputStream input(sections[i].sectionData_, sections[i].sectionSize_);
                google::protobuf::io::CodedInputStream inputstream(&input);
                hBool ok=shaderres.ParseFromCodedStream(&inputstream);
                hcAssertMsg(ok,"Failed to parse Shader Resource");
                if (!ok) {
                    return hNullptr;
                }
                hShaderType type;
                switch(shaderres.type()) {
                case proto::eShaderType_Vertex:     type=ShaderType_VERTEXPROG;    break;
                case proto::eShaderType_Pixel:      type=ShaderType_FRAGMENTPROG;  break;
                case proto::eShaderType_Geometry:   type=ShaderType_GEOMETRYPROG;  break;
                case proto::eShaderType_Hull:       type=ShaderType_HULLPROG;      break;
                case proto::eShaderType_Domain:     type=ShaderType_DOMAINPROG;    break;
                case proto::eShaderType_Compute:    type=ShaderType_COMPUTEPROG;   break;
                }
                createShader((hChar*)shaderres.compiledprogram().c_str(), (hUint)shaderres.compiledprogram().length(), type, &shaderProg);
            } else if (hStrCmp("source", sections[i].sectionName_) == 0) {
#if defined (HEART_ALLOW_SHADER_SOURCE_COMPILE)
                hShaderType type;
                switch(shaderres.type()) {
                case proto::eShaderType_Vertex:     type=ShaderType_VERTEXPROG;    break;
                case proto::eShaderType_Pixel:      type=ShaderType_FRAGMENTPROG;  break;
                case proto::eShaderType_Geometry:   type=ShaderType_GEOMETRYPROG;  break;
                case proto::eShaderType_Hull:       type=ShaderType_HULLPROG;      break;
                case proto::eShaderType_Domain:     type=ShaderType_DOMAINPROG;    break;
                case proto::eShaderType_Compute:    type=ShaderType_COMPUTEPROG;   break;
                }
                hShaderDefine* defines=hNullptr;
                hUint definecount=shaderres.defines_size();
                if (definecount) {
                    defines=(hShaderDefine*)hAlloca(sizeof(hShaderDefine)*definecount);
                    for (hUint di=0; di<definecount; ++di) {
                        defines[di].define_=shaderres.defines(di).define().c_str();
                        defines[di].value_=shaderres.defines(di).value().c_str();
                    }
                }
                compileShaderFromSource((hChar*)sections[i].sectionData_, sections[i].sectionSize_, shaderres.entry().c_str(), (hShaderProfile)shaderres.profile(), &includes, defines, definecount, &shaderProg);
#endif
            }
        }
#endif
        return shaderProg;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderer::shaderResourcePostLoad(hResourceManager* manager, hResourceClassBase* resource) {
        manager->insertResource(resource->getResourceID(), resource);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderer::shaderResourcePreUnload(hResourceManager* manager, hResourceClassBase* resource) {
        manager->removeResource(resource->getResourceID());
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderer::shaderResourceUnload(hResourceClassBase* resource) {
        hShaderProgram* sp = static_cast<hShaderProgram*>(resource);
        sp->DecRef();
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hResourceClassBase* hRenderer::materialResourceLoader(const hResourceSection* sections, hUint sectioncount) {
        for (hUint i=0, n=sectioncount; i<n; ++i) {
            if (hStrCmp("material_blob", sections[i].sectionName_) == 0) {
                hcAssertFailMsg("material blob section no longer supported");
            } else if (hStrCmp("material", sections[i].sectionName_) == 0) {
                google::protobuf::io::ArrayInputStream input(sections[i].sectionData_, sections[i].sectionSize_);
                google::protobuf::io::CodedInputStream inputstream(&input);
                proto::MaterialResource materialres;
                hBool ok=materialres.ParseFromCodedStream(&inputstream);
                hcAssertMsg(ok,"Failed to parse Shader Resource");
                if (!ok) {
                    return hNullptr;
                }
                hMaterial* material = new hMaterial)(this;
                for (hUint gi=0, gn=materialres.groups_size(); gi<gn; ++gi) {
                    const proto::MaterialGroup& groupdef=materialres.groups(gi);
                    hMaterialGroup* group=material->addGroup(groupdef.groupname().c_str());
                    group->techCountHint(groupdef.technique_size());
                    for (hUint ti=0, tn=groupdef.technique_size(); ti<tn; ++ti) {
                        const proto::MaterialTechnique& techdef=groupdef.technique(ti);
                        hMaterialTechnique* tech=group->addTechnique(techdef.techniquename().c_str());
                        tech->SetLayer(techdef.layer());
                        tech->SetSort(techdef.transparent());
                        tech->SetPasses(techdef.passes_size());
                        for (hUint pi=0, pn=techdef.passes_size(); pi<pn; ++pi) {
                            const proto::MaterialPass& passdef=techdef.passes(pi);
                            hMaterialTechniquePass* pass=tech->appendPass();
                            hBlendStateDesc blenddesc;
                            hDepthStencilStateDesc depthdesc;
                            hRasterizerStateDesc rasterdesc;
                            if (passdef.has_pixel()) {
                                pass->setProgramID(ShaderType_FRAGMENTPROG, hResourceID::buildResourceID(passdef.pixel().c_str()));
                            }
                            if (passdef.has_vertex()) {
                                pass->setProgramID(ShaderType_VERTEXPROG, hResourceID::buildResourceID(passdef.vertex().c_str()));
                            }
                            if (passdef.has_geometry()) {
                                pass->setProgramID(ShaderType_GEOMETRYPROG, hResourceID::buildResourceID(passdef.geometry().c_str()));
                            }
                            if (passdef.has_domain()) {
                                pass->setProgramID(ShaderType_DOMAINPROG, hResourceID::buildResourceID(passdef.domain().c_str()));
                            }
                            if (passdef.has_hull()) {
                                pass->setProgramID(ShaderType_HULLPROG, hResourceID::buildResourceID(passdef.hull().c_str()));
                            }
                            if (passdef.has_compute()) {
                                pass->setProgramID(ShaderType_COMPUTEPROG, hResourceID::buildResourceID(passdef.compute().c_str()));
                            }
                            if (passdef.has_blend()) {
                                const proto::BlendState& blenddef=passdef.blend();
                                if (blenddef.has_blendenable()) {
                                    blenddesc.blendEnable_=(RENDER_STATE_VALUE)blenddef.blendenable();
                                }
                                if (blenddef.has_srcblend()) {
                                    blenddesc.srcBlend_=(RENDER_STATE_VALUE)blenddef.srcblend();
                                }
                                if (blenddef.has_destblend()) {
                                    blenddesc.destBlend_=(RENDER_STATE_VALUE)blenddef.destblend();
                                }
                                if (blenddef.has_blendop()) {
                                    blenddesc.blendOp_=(RENDER_STATE_VALUE)blenddef.blendop();
                                }
                                if (blenddef.has_srcblendalpha()) {
                                    blenddesc.srcBlendAlpha_=(RENDER_STATE_VALUE)blenddef.srcblendalpha();
                                }
                                if (blenddef.has_destblendalpha()) {
                                    blenddesc.destBlendAlpha_=(RENDER_STATE_VALUE)blenddef.destblendalpha();
                                }
                                if (blenddef.has_blendopalpha()) {
                                    blenddesc.blendOpAlpha_=(RENDER_STATE_VALUE)blenddef.blendopalpha();
                                }
                                if (blenddef.has_rendertargetwritemask()) {
                                    blenddesc.renderTargetWriteMask_=blenddef.rendertargetwritemask();
                                }
                            }
                            hBlendState* bs=createBlendState(blenddesc);
                            pass->bindBlendState(bs);
                            bs->DecRef();
                            if (passdef.has_depthstencil()) {
                                const proto::DepthStencilState& depthdef=passdef.depthstencil();
                                if (depthdef.has_depthenable()) {
                                    depthdesc.depthEnable_ = (RENDER_STATE_VALUE)depthdef.depthenable();
                                }
                                if (depthdef.has_depthwritemask()) {
                                    depthdesc.depthWriteMask_=(RENDER_STATE_VALUE)depthdef.depthwritemask();
                                }
                                if (depthdef.has_depthfunc()) {
                                    depthdesc.depthFunc_=(RENDER_STATE_VALUE)depthdef.depthfunc();
                                }
                                if (depthdef.has_stencilenable()) {
                                    depthdesc.stencilEnable_=(RENDER_STATE_VALUE)depthdef.stencilenable();
                                }
                                if (depthdef.has_stencilreadmask()) {
                                    depthdesc.stencilReadMask_=depthdef.stencilreadmask();
                                }
                                if (depthdef.has_stencilwritemask()) {
                                    depthdesc.stencilWriteMask_=depthdef.stencilwritemask();
                                }
                                if (depthdef.has_stencilfailop()) {
                                    depthdesc.stencilFailOp_=(RENDER_STATE_VALUE)depthdef.stencilfailop();
                                }
                                if (depthdef.has_stencildepthfailop()) {
                                    depthdesc.stencilDepthFailOp_=(RENDER_STATE_VALUE)depthdef.stencildepthfailop();
                                }
                                if (depthdef.has_stencilpassop()) {
                                    depthdesc.stencilPassOp_=(RENDER_STATE_VALUE)depthdef.stencilpassop();
                                }
                                if (depthdef.has_stencilfunc()) {
                                    depthdesc.stencilFunc_=(RENDER_STATE_VALUE)depthdef.stencilfunc();
                                }
                                if (depthdef.has_stencilref()) {
                                    depthdesc.stencilRef_=depthdef.stencilref();
                                }
                            }
                            hDepthStencilState* ds=createDepthStencilState(depthdesc);
                            pass->bindDepthStencilState(ds);
                            ds->DecRef();
                            if (passdef.has_rasterizer()) {
                                const proto::RasterizerState& rasterdef=passdef.rasterizer();
                                if (rasterdef.has_fillmode()) {
                                    rasterdesc.fillMode_=(RENDER_STATE_VALUE)rasterdef.fillmode();
                                }
                                if (rasterdef.has_cullmode()) {
                                    rasterdesc.cullMode_=(RENDER_STATE_VALUE)rasterdef.cullmode();
                                }
                                if (rasterdef.has_frontcounterclockwise()) {
                                    rasterdesc.frontCounterClockwise_=(RENDER_STATE_VALUE)rasterdef.frontcounterclockwise();
                                }
                                if (rasterdef.has_depthbias()) {
                                    rasterdesc.depthBias_=(RENDER_STATE_VALUE)rasterdef.depthbias();
                                }
                                if (rasterdef.has_depthbiasclamp()) {
                                    rasterdesc.depthBiasClamp_=rasterdef.depthbiasclamp();
                                }
                                if (rasterdef.has_slopescaleddepthbias()) {
                                    rasterdesc.slopeScaledDepthBias_=rasterdef.slopescaleddepthbias();
                                }
                                if (rasterdef.has_depthclipenable()) {
                                    rasterdesc.depthClipEnable_=(RENDER_STATE_VALUE)rasterdef.depthclipenable();
                                }
                                if (rasterdef.has_scissorenable()) {
                                    rasterdesc.scissorEnable_=(RENDER_STATE_VALUE)rasterdef.scissorenable();
                                }
                            }
                            hRasterizerState* rs=createRasterizerState(rasterdesc);
                            pass->bindRasterizerState(rs);
                            rs->DecRef();
                        }
                    }
                }
                //Read samplers
                for (hUint32 is=0, ns=materialres.samplers_size(); is<ns; ++is)
                {
                    const proto::MaterialSampler& matsamplerdef=materialres.samplers(is);
                   
                    hSamplerParameter sampler;
                    hSamplerStateDesc samplerdesc;
                    hSamplerState* ss;

                    if (matsamplerdef.has_samplerstate()) {
                        const proto::SamplerState& samplerdef=matsamplerdef.samplerstate();
                        if (samplerdef.has_filter()) {
                            samplerdesc.filter_=(hSAMPLER_STATE_VALUE)samplerdef.filter();
                        }
                        if (samplerdef.has_addressu()) {
                            samplerdesc.addressU_=(hSAMPLER_STATE_VALUE)samplerdef.addressu();
                        }
                        if (samplerdef.has_addressv()) {
                            samplerdesc.addressV_=(hSAMPLER_STATE_VALUE)samplerdef.addressv();
                        }
                        if (samplerdef.has_addressw()) {
                            samplerdesc.addressW_=(hSAMPLER_STATE_VALUE)samplerdef.addressw();
                        }
                        if (samplerdef.has_miplodbias()) {
                            samplerdesc.mipLODBias_=samplerdef.miplodbias();
                        }
                        if (samplerdef.has_maxanisotropy()) {
                            samplerdesc.maxAnisotropy_=samplerdef.maxanisotropy();
                        }
                        if (samplerdef.has_bordercolour()) {
                            const proto::Colour& colour=samplerdef.bordercolour();
                            hFloat alpha=colour.has_alpha() ? (colour.alpha()/255.f) : 1.f;
                            samplerdesc.borderColour_=hColour((colour.red()/255.f), (colour.green()/255.f), (colour.blue()/255.f), alpha);
                        }
                        if (samplerdef.has_minlod()) {
                            samplerdesc.minLOD_=samplerdef.minlod();
                        }
                        if (samplerdef.has_maxlod()) {
                            samplerdesc.maxLOD_=samplerdef.maxlod();
                        }
                    }
                    ss = createSamplerState(samplerdesc);

                    hStrCopy(sampler.name_, sampler.name_.GetMaxSize(), matsamplerdef.samplername().c_str());
                    sampler.paramID_=hCRC32::StringCRC(sampler.name_);
                    sampler.samplerState_=ss;
                    material->addSamplerParameter(sampler);
                }
                //Read parameters
                for (hUint pi=0, pn=materialres.parameters_size(); pi<pn; ++pi) {
                    const proto::MaterialParameter& paramdef=materialres.parameters(pi);
                    if (paramdef.has_resourceid()) {
                        material->addDefaultParameterValue(paramdef.paramname().c_str(), hResourceID::buildResourceID(paramdef.resourceid().c_str()));
                    } else if (paramdef.floatvalues_size()) {
                        material->addDefaultParameterValue(paramdef.paramname().c_str(), paramdef.floatvalues().data(), paramdef.floatvalues_size());
                    } else if (paramdef.intvalues_size()) {
                        material->addDefaultParameterValue(paramdef.paramname().c_str(), (const hInt32*)paramdef.intvalues().data(), paramdef.intvalues_size());
                    } else if (paramdef.colourvalues_size()) {
                        proto::Colour c=paramdef.colourvalues(0);
                        hColour colour(c.red()/255.f, c.green()/255.f, c.blue()/255.f, c.has_alpha() ? c.alpha()/255.f : 1.f);
                        material->addDefaultParameterValue(paramdef.paramname().c_str(), colour);
                    }
                }

                return material;
            }
        }

        return hNullptr;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderer::materialResourcePostLoad(hResourceManager* manager, hResourceClassBase* resource) {
        hMaterial* mat = static_cast< hMaterial* >(resource);
        mat->listenToResourceEvents(manager);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderer::materialResourcePreUnload(hResourceManager* manager, hResourceClassBase* resource) {
        hcAssert(resource);
        hMaterial* mat = static_cast<hMaterial*>(resource);
        mat->stopListeningToResourceEvents();
        mat->unbind();
        manager->removeResource(mat->getResourceID());
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderer::materialResourceUnload(hResourceClassBase* resource) {
        hMaterial* mat = static_cast<hMaterial*>(resource);
        delete mat;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hResourceClassBase* hRenderer::meshResourceLoader(const hResourceSection* sections, hUint sectioncount) {
        for (hUint i=0, n=sectioncount; i<n; ++i) {
            if (hStrCmp("mesh", sections[i].sectionName_) == 0) {
                google::protobuf::io::ArrayInputStream input(sections[i].sectionData_, sections[i].sectionSize_);
                google::protobuf::io::CodedInputStream inputstream(&input);
                proto::Mesh meshres;
                hBool ok=meshres.ParseFromCodedStream(&inputstream);
                hcAssertMsg(ok,"Failed to parse Shader Resource");
                if (!ok) {
                    return hNullptr;
                }

                hRenderModel* rmodel = new hRenderModel)(;
                rmodel->setRenderableCountHint(meshres.renderables_size());
                for (hUint ri=0, rn=meshres.renderables_size(); ri<rn; ++ri) {
                    const proto::Renderable& renderableres=meshres.renderables(ri);
                    hRenderable* renderable=rmodel->appendRenderable();
                    PrimitiveType primtype=(PrimitiveType)renderableres.primtype();
                    hAABB aabb;

                    aabb.reset();

                    if (renderableres.has_aabb()) {
                        const proto::AxisAlignedBoundingBox& aabbres=renderableres.aabb();
                        hVec3 bounds[] = {
                            hVec3(aabbres.minx(), aabbres.miny(), aabbres.minz()),
                            hVec3(aabbres.maxx(), aabbres.maxy(), aabbres.maxz())
                        };
                        aabb = hAABB::computeFromPointSet(bounds, (hUint)hStaticArraySize(bounds));
                    }
                    renderable->SetStartIndex(0);
                    renderable->SetPrimativeType(primtype);
                    renderable->SetPrimativeCount(renderableres.primcount());
                    renderable->SetAABB(aabb);
                    renderable->setMaterialResourceID(hResourceID::buildResourceID(renderableres.materialresource().c_str()));

                    if (renderableres.has_indexbuffer()) {
                        hIndexBuffer* ib;
                        createIndexBuffer(renderableres.indexbuffer().data(), renderableres.indexcount(), 0, &ib);
                        renderable->SetIndexBuffer(ib);
                    }

                    rmodel->setRenderableCountHint(renderableres.vertexstreams_size());
                    for (hUint rsi=0, rsn=renderableres.vertexstreams_size(); rsi<rsn; ++rsi) {
                        hUint vtxcount=renderableres.vertexcount();
                        const proto::VertexStream& vtxstreamres=renderableres.vertexstreams(rsi);
                        hInputLayoutDesc vtxstreamdesc(
                            vtxstreamres.semantic().c_str(),
                            vtxstreamres.semanticindex(),
                            (hInputFormat)vtxstreamres.format(),
                            rsi, 0);
                        hVertexBuffer* vb=hNullptr;
                        createVertexBuffer(vtxstreamres.streamdata().data(), vtxcount, &vtxstreamdesc, 1, 0, &vb);
                        renderable->setVertexStream(rsi, vb);
                    }
                }
                return rmodel;
            }
        }

        return hNullptr;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderer::meshResourceLink(hResourceManager* manager, hResourceClassBase* resource) {
        hRenderModel* rmodel = static_cast< hRenderModel* >(resource);
        rmodel->listenForResourceEvents(manager);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderer::meshResourceUnlink(hResourceManager* manager, hResourceClassBase* resource) {
        hRenderModel* rmodel = static_cast< hRenderModel* >(resource);
        rmodel->cleanUp();
        manager->removeResource(rmodel->getResourceID());
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderer::meshResourceUnload(hResourceClassBase* resource) {
        hRenderModel* rmodel = static_cast< hRenderModel* >(resource);
        hUint32 renderablecount = rmodel->getRenderableCount();
        for(hUint32 i = 0; i < renderablecount; ++i) {
            hRenderable* renderable=rmodel->getRenderable(i);
            for (hUint32 s = 0, sc=renderable->getVertexStreamCount(); s < sc; ++s) {
                hVertexBuffer* vb=renderable->getVertexStream(s);
                if (vb) {
                    vb->DecRef();
                    vb=NULL;
                }
            }
            renderable->getIndexBuffer()->DecRef();
        }
        delete rmodel; rmodel = nullptr;
    }
#endif

#if 0 //!!JM
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderCommands::reserveSpace(hUint size) {
        if (allocatedSize_ < size) {
            cmds_=(hRCmd*)hRealloc(cmds_, hAlign(size, 1024));
            allocatedSize_=hAlign(size, 1024);
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderCommands::insertCommand(hUint where, const hRCmd* command, hBool overwrite) {
        hcAssert(command);
        hcAssert(where <= cmdSize_);
        if (where+command->size_ >= cmdSize_) {
            reserveSpace(cmdSize_+command->size_);
        }
        hRCmd* cmd=(hRCmd*)((hByte*)cmds_+where);
        hRCmd* nextcmd=(hRCmd*)((hByte*)cmds_+where+cmd->size_);
        hUint remainingBytes=cmdSize_-where;
        if (overwrite) {
            hcAssert(cmd->opCode_ < eRenderCmd_End);
            hByte oldcmdsize=cmd->size_;
            if (oldcmdsize < command->size_) {
                hMemMove(((hByte*)cmd+command->size_), nextcmd, remainingBytes);
                cmdSize_+=command->size_-oldcmdsize;
            }
            hMemSet(cmd, 0xAC, cmd->size_);
            hMemCpy(cmd, command, command->size_);
            if (oldcmdsize > command->size_) {
                cmd->size_=oldcmdsize;
            }
        } else {
            if (remainingBytes > 0) {
                hMemMove(nextcmd, cmd, remainingBytes);
            }
            hMemCpy(cmd, command, command->size_);
            cmdSize_+=command->size_;
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hRenderCommandGenerator::hRenderCommandGenerator() 
        : renderCommands_(hNullptr)
    {

    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hRenderCommandGenerator::hRenderCommandGenerator(hRenderCommands* rcmds)
        : renderCommands_(rcmds)
    {
        hcAssert(renderCommands_);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hRCmd* hRenderCommandGenerator::getCmdBufferStart() {
        return renderCommands_ ? renderCommands_->cmds_ : hNullptr;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hUint hRenderCommandGenerator::appendCmd(const hRCmd* cmd) {
        hcAssert(renderCommands_);
        hUint cmdstart=renderCommands_->cmdSize_;
        renderCommands_->insertCommand(renderCommands_->cmdSize_, cmd, false);
        return cmdstart;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hUint hRenderCommandGenerator::overwriteCmd(const hRCmd* oldcmd, const hRCmd* newcmd) {
        hcAssert(renderCommands_);
        hUint sizediff=oldcmd->size_ < newcmd->size_ ? newcmd->size_-oldcmd->size_ : 0;
        hUint cmdstart=(hUint)((ptrdiff_t)oldcmd-(ptrdiff_t)renderCommands_->cmds_);
        renderCommands_->insertCommand(cmdstart, newcmd, true);
        return sizediff;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hRenderCommandGenerator::reset() {
        hcAssert(renderCommands_);
        renderCommands_->cmdSize_=0;
#ifdef HEART_DEBUG
        hMemSet(renderCommands_->cmds_, 0xDC, renderCommands_->allocatedSize_);
#endif
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hUint hRenderCommandGenerator::resetCommands() {
        reset();
        return 0;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hUint hRenderCommandGenerator::setJump(hRCmd* cmd) {
        hcAssert(renderCommands_);
        hRCmdJump jumpcmd(cmd);
        return appendCmd(&jumpcmd);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hUint hRenderCommandGenerator::setReturn() {
        hcAssert(renderCommands_);
        hRCmdReturn retcmd;
        return appendCmd(&retcmd);
    }
    
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hUint hRenderCommandGenerator::setNoOp() {
        hcAssert(renderCommands_);
        hRCmdNOOP cmd;
        return appendCmd(&cmd);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hUint hRenderCommandGenerator::setDraw(hUint nPrimatives, hUint startVertex) {
        hcAssert(renderCommands_);
        hRCmdDraw cmd(nPrimatives, startVertex);
        return appendCmd(&cmd);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hUint hRenderCommandGenerator::setDrawIndex(hUint nPrimatives, hUint startVertex) {
        hcAssert(renderCommands_);
        hRCmdDrawIndex cmd(nPrimatives, startVertex);
        return appendCmd(&cmd);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hUint hRenderCommandGenerator::setDrawInstance(hUint nPrimatives, hUint startVertex, hUint instancecount) {
        hcAssert(renderCommands_);
        hRCmdDrawInstanced cmd(nPrimatives, startVertex, instancecount);
        return appendCmd(&cmd);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hUint hRenderCommandGenerator::setDrawInstanceIndex(hUint nPrimatives, hUint startVertex, hUint instancecount) {
        hcAssert(renderCommands_);
        hRCmdDrawInstancedIndex cmd(nPrimatives, startVertex, instancecount);
        return appendCmd(&cmd);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hUint hRenderCommandGenerator::setRenderStates(hBlendState* bs, hRasterizerState* rs, hDepthStencilState* dss) {
        hcAssert(renderCommands_);
        return hdRenderCommandGenerator::setRenderStates(bs, rs, dss);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hUint hRenderCommandGenerator::setShader(hShaderProgram* shader, hShaderType type) {
        hcAssert(renderCommands_);
        return hdRenderCommandGenerator::setShader(shader, type);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hUint hRenderCommandGenerator::setVertexInputs(hSamplerState** samplers, hUint nsamplers, hShaderResourceView** srv, hUint nsrv, hRenderBuffer** cb, hUint ncb) {
        hcAssert(renderCommands_);
        hdSamplerState** dsamplers      =(hdSamplerState**)hAlloca(sizeof(hdSamplerState*)*nsamplers);
        hdShaderResourceView** dsrv     =(hdShaderResourceView**)hAlloca(sizeof(hdShaderResourceView*)*nsrv);
        hdRenderBuffer** dcb  =(hdRenderBuffer**)hAlloca(sizeof(hdRenderBuffer*)*ncb);
        for (hUint i=0, n=nsamplers; i<n; ++i) {
            dsamplers[i]=samplers[i];
        }
        for (hUint i=0, n=nsrv; i<n; ++i) {
            dsrv[i]=srv[i];
        }
        for (hUint i=0, n=ncb; i<n; ++i) {
            dcb[i]=cb[i];
        }
        return hdRenderCommandGenerator::setVertexInputs(dsamplers, nsamplers, dsrv, nsrv, dcb, ncb);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hUint hRenderCommandGenerator::updateVertexInputs(hRCmd* cmd, hSamplerState** samplers, hUint nsamplers, hShaderResourceView** srv, hUint nsrv, hRenderBuffer** cb, hUint ncb) {
        hdSamplerState** dsamplers      =(hdSamplerState**)hAlloca(sizeof(hdSamplerState*)*nsamplers);
        hdShaderResourceView** dsrv     =(hdShaderResourceView**)hAlloca(sizeof(hdShaderResourceView*)*nsrv);
        hdRenderBuffer** dcb  =(hdRenderBuffer**)hAlloca(sizeof(hdRenderBuffer*)*ncb);
        for (hUint i=0, n=nsamplers; i<n; ++i) {
            dsamplers[i]=samplers[i];
        }
        for (hUint i=0, n=nsrv; i<n; ++i) {
            dsrv[i]=srv[i];
        }
        for (hUint i=0, n=ncb; i<n; ++i) {
            dcb[i]=cb[i];
        }
        return hdRenderCommandGenerator::updateVertexInputs(cmd, dsamplers, nsamplers, dsrv, nsrv, dcb, ncb);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hUint hRenderCommandGenerator::setPixelInputs(hSamplerState** samplers, hUint nsamplers, hShaderResourceView** srv, hUint nsrv, hRenderBuffer** cb, hUint ncb) {
        hcAssert(renderCommands_);
        hdSamplerState** dsamplers      =(hdSamplerState**)hAlloca(sizeof(hdSamplerState*)*nsamplers);
        hdShaderResourceView** dsrv     =(hdShaderResourceView**)hAlloca(sizeof(hdShaderResourceView*)*nsrv);
        hdRenderBuffer** dcb  =(hdRenderBuffer**)hAlloca(sizeof(hdRenderBuffer*)*ncb);
        for (hUint i=0, n=nsamplers; i<n; ++i) {
            dsamplers[i]=samplers[i];
        }
        for (hUint i=0, n=nsrv; i<n; ++i) {
            dsrv[i]=srv[i];
        }
        for (hUint i=0, n=ncb; i<n; ++i) {
            dcb[i]=cb[i];
        }
        return hdRenderCommandGenerator::setPixelInputs(dsamplers, nsamplers, dsrv, nsrv, dcb, ncb);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hUint hRenderCommandGenerator::updatePixelInputs(hRCmd* cmd, hSamplerState** samplers, hUint nsamplers, hShaderResourceView** srv, hUint nsrv, hRenderBuffer** cb, hUint ncb) {
        hcAssert(renderCommands_);
        hdSamplerState** dsamplers      =(hdSamplerState**)hAlloca(sizeof(hdSamplerState*)*nsamplers);
        hdShaderResourceView** dsrv     =(hdShaderResourceView**)hAlloca(sizeof(hdShaderResourceView*)*nsrv);
        hdRenderBuffer** dcb  =(hdRenderBuffer**)hAlloca(sizeof(hdRenderBuffer*)*ncb);
        for (hUint i=0, n=nsamplers; i<n; ++i) {
            dsamplers[i]=samplers[i];
        }
        for (hUint i=0, n=nsrv; i<n; ++i) {
            dsrv[i]=srv[i];
        }
        for (hUint i=0, n=ncb; i<n; ++i) {
            dcb[i]=cb[i];
        }
        return hdRenderCommandGenerator::updatePixelInputs(cmd, dsamplers, nsamplers, dsrv, nsrv, dcb, ncb);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hUint hRenderCommandGenerator::setGeometryInputs(hdDX11SamplerState** samplers, hUint nsamplers, hShaderResourceView** srv, hUint nsrv, hRenderBuffer** cb, hUint ncb) {
        hcAssert(renderCommands_);
        hdSamplerState** dsamplers      =(hdSamplerState**)hAlloca(sizeof(hdSamplerState*)*nsamplers);
        hdShaderResourceView** dsrv     =(hdShaderResourceView**)hAlloca(sizeof(hdShaderResourceView*)*nsrv);
        hdRenderBuffer** dcb  =(hdRenderBuffer**)hAlloca(sizeof(hdRenderBuffer*)*ncb);
        for (hUint i=0, n=nsamplers; i<n; ++i) {
            dsamplers[i]=samplers[i];
        }
        for (hUint i=0, n=nsrv; i<n; ++i) {
            dsrv[i]=srv[i];
        }
        for (hUint i=0, n=ncb; i<n; ++i) {
            dcb[i]=cb[i];
        }
        return hdRenderCommandGenerator::setGeometryInputs(dsamplers, nsamplers, dsrv, nsrv, dcb, ncb);
    }
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hUint hRenderCommandGenerator::setHullInputs(hdDX11SamplerState** samplers, hUint nsamplers, hShaderResourceView** srv, hUint nsrv, hRenderBuffer** cb, hUint ncb) {
        hcAssert(renderCommands_);
        hdSamplerState** dsamplers      =(hdSamplerState**)hAlloca(sizeof(hdSamplerState*)*nsamplers);
        hdShaderResourceView** dsrv     =(hdShaderResourceView**)hAlloca(sizeof(hdShaderResourceView*)*nsrv);
        hdRenderBuffer** dcb  =(hdRenderBuffer**)hAlloca(sizeof(hdRenderBuffer*)*ncb);
        for (hUint i=0, n=nsamplers; i<n; ++i) {
            dsamplers[i]=samplers[i];
        }
        for (hUint i=0, n=nsrv; i<n; ++i) {
            dsrv[i]=srv[i];
        }
        for (hUint i=0, n=ncb; i<n; ++i) {
            dcb[i]=cb[i];
        }
        return hdRenderCommandGenerator::setHullInputs(dsamplers, nsamplers, dsrv, nsrv, dcb, ncb);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hUint hRenderCommandGenerator::setDomainInputs(hdDX11SamplerState** samplers, hUint nsamplers, hShaderResourceView** srv, hUint nsrv, hRenderBuffer** cb, hUint ncb) {
        hcAssert(renderCommands_);
        hdSamplerState** dsamplers      =(hdSamplerState**)hAlloca(sizeof(hdSamplerState*)*nsamplers);
        hdShaderResourceView** dsrv     =(hdShaderResourceView**)hAlloca(sizeof(hdShaderResourceView*)*nsrv);
        hdRenderBuffer** dcb  =(hdRenderBuffer**)hAlloca(sizeof(hdRenderBuffer*)*ncb);
        for (hUint i=0, n=nsamplers; i<n; ++i) {
            dsamplers[i]=samplers[i];
        }
        for (hUint i=0, n=nsrv; i<n; ++i) {
            dsrv[i]=srv[i];
        }
        for (hUint i=0, n=ncb; i<n; ++i) {
            dcb[i]=cb[i];
        }
        return hdRenderCommandGenerator::setDomainInputs(dsamplers, nsamplers, dsrv, nsrv, dcb, ncb);
    }
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hUint hRenderCommandGenerator::setStreamInputs(PrimitiveType primType, hIndexBuffer* index, hIndexBufferType format, hdInputLayout* vertexlayout, hVertexBuffer** vtx, hUint firstStream, hUint streamCount) {
        hcAssert(renderCommands_);
        hdVtxBuffer** dvtx=(hdVtxBuffer**)hAlloca(sizeof(hdVtxBuffer*)*streamCount);
        for (hUint i=0, n=streamCount; i<n; ++i) {
            dvtx[i]=vtx[i];
        }
        return hdRenderCommandGenerator::setStreamInputs(primType, index, format, vertexlayout, dvtx, firstStream, streamCount);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hUint hRenderCommandGenerator::updateShaderInputBuffer(hRCmd* cmd, hUint reg, hRenderBuffer* cb) {
        hcAssert(cmd >= renderCommands_->cmds_ && (hByte*)cmd < (hByte*)renderCommands_->cmds_+renderCommands_->cmdSize_);
        hdRenderCommandGenerator::updateShaderInputBuffer(cmd, reg, cb);
        return 0;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hUint hRenderCommandGenerator::updateShaderInputSampler(hRCmd* cmd, hUint reg, hSamplerState* ss) {
        hcAssert(cmd >= renderCommands_->cmds_ && (hByte*)cmd < (hByte*)renderCommands_->cmds_+renderCommands_->cmdSize_);
        hdRenderCommandGenerator::updateShaderInputSampler(cmd, reg, ss);
        return 0;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hUint hRenderCommandGenerator::updateShaderInputView(hRCmd* cmd, hUint reg, hShaderResourceView* srv) {
        hcAssert(cmd >= renderCommands_->cmds_ && (hByte*)cmd < (hByte*)renderCommands_->cmds_+renderCommands_->cmdSize_);
        hdRenderCommandGenerator::updateShaderInputView(cmd, reg, srv);
        return 0;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hUint hRenderCommandGenerator::updateStreamInputs(hRCmd* cmd, PrimitiveType primType, hIndexBuffer* index, hIndexBufferType format, hdInputLayout* vertexlayout, hVertexBuffer** vtx, hUint firstStream, hUint streamCount) {
        hcAssert(cmd >= renderCommands_->cmds_ && (hByte*)cmd < (hByte*)renderCommands_->cmds_+renderCommands_->cmdSize_);
        hdVtxBuffer** dvtx=(hdVtxBuffer**)hAlloca(sizeof(hdVtxBuffer*)*streamCount);
        for (hUint i=0, n=streamCount; i<n; ++i) {
            dvtx[i]=vtx[i];
        }
        return hdRenderCommandGenerator::updateStreamInputs(cmd, primType, index, format, vertexlayout, dvtx, firstStream, streamCount);
    }
#endif
}
}