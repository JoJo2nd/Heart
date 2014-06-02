/********************************************************************

    filename:   ComputeBlur.cpp  
    
    Copyright (c) 24:2:2013 James Moran
    
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
#include "testbed_precompiled.h"
#include "ComputeBlur.h"
#include "TestUtils.h"

DEFINE_HEART_UNIT_TEST(ComputeBlur);

#define PACKAGE_NAME ("MATERIALS")
#define RESOURCE_NAME ("MATERIALS")
#define ASSET_PATH_COL ("MATERIALS.GAUSSIAN_BLUR_COL")
#define ASSET_PATH_ROW ("MATERIALS.GAUSSIAN_BLUR_ROW")
#define ASSET_PATH_TEXTURE ("MATERIALS.NARUTO_TEST")
#define ASSET_BLIT_MAT ("MATERIALS.BLUR_BLIT")

#define TEST_TEXTURE_WIDTH  (265)
#define TEST_TEXTURE_HEIGHT (265)

//#define CREATE_TEXTURE

namespace ComputeBlurData
{
__declspec(align(16))
struct cbParams
{
    hUint inputDataSize;              // 
    hUint inputDataSizeLog2;          //
    hUint numApproxPasses;
    float halfBoxFilterWidth;			// w/2
    float fracHalfBoxFilterWidth;		// frac(w/2+0.5)
    float invFracHalfBoxFilterWidth;	// 1-frac(w/2+0.5)
    float rcpBoxFilterWidth;			// 1/w
};
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

hUint32 ComputeBlur::RunUnitTest()
{
    Heart::hdGamepad* pad = engine_->GetControllerManager()->GetGamepad(0);
    Heart::hdKeyboard* kb = engine_->GetControllerManager()->GetSystemKeyboard();
    Heart::hResourceManager* resMgr=engine_->GetResourceManager();
    switch(state_)
    {
    case eBeginLoad: {
            resMgr->mtLoadPackage(PACKAGE_NAME);
            fpCamera_.setInput(pad);
            state_ = eLoading;
        } break;
    case eLoading: {
            if (resMgr->mtIsPackageLoaded(PACKAGE_NAME)) {
                CreateRenderResources();
                SetCanRender(hTrue);
                state_=eRender;
            }
        } break;
    case eRender: {
            UpdateCamera();
            if (getForceExitFlag()) {
                state_ = eBeginUnload;
            }
        } break;
    case eBeginUnload: {
            SetCanRender(hFalse);
            DestroyRenderResources();
            resMgr->mtUnloadPackage(PACKAGE_NAME);
            state_ = eExit;
            SetExitCode(UNIT_TEST_EXIT_CODE_OK);
        } break;
    case eExit: {
        } break;
    }

    return 0;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void ComputeBlur::RenderUnitTest()
{
    Heart::hRenderer* renderer = engine_->GetRenderer();
    Heart::hRenderSubmissionCtx* ctx=renderer->GetMainSubmissionCtx();
    const Heart::hRenderTechniqueInfo* techinfo = renderer->GetMaterialManager()->GetRenderTechniqueInfo("main");
    Heart::hRenderBufferMapInfo mapinfo;
    ComputeBlurData::cbParams* cbblur=NULL;

    ctx->Map(modelMtxCB_, &mapinfo);
    *(Heart::hMatrix*)mapinfo.ptr = Heart::hMatrixFunc::identity();
    ctx->Unmap(&mapinfo);
    ctx->Map(blurParamCB_, &mapinfo);
    cbblur=(ComputeBlurData::cbParams*)mapinfo.ptr;
    cbblur->inputDataSize=renderer->GetWidth();
    cbblur->inputDataSizeLog2=hUint(logf(renderer->GetWidth())/logf(2));
    cbblur->numApproxPasses=3;
    cbblur->halfBoxFilterWidth=8.f;		
    cbblur->fracHalfBoxFilterWidth=0.f;	
    cbblur->invFracHalfBoxFilterWidth=1.f;
    cbblur->rcpBoxFilterWidth=1.f/8.f;		
    ctx->Unmap(&mapinfo);

    //Submit to blur target
    Heart::hRenderUtility::setCameraParameters(ctx, &blurCamera_);
    ctx->clearDepth(camera_.getDepthTarget(), 1.f);

    Heart::hMaterialTechnique* tech=materialInstance_->getGroup(0)->getTechniqueByMask(techinfo->mask_);
    for (hUint32 pass = 0, passcount = tech->GetPassCount(); pass < passcount; ++pass ) {
        Heart::hMaterialTechniquePass* passptr = tech->GetPass(pass);
        ctx->SetMaterialPass(passptr);
        ctx->DrawIndexedPrimitive(quadIB_->GetIndexCount()/3, 0);
    }

    /*
        Bind the back buffer against the device, unbinds the result buffer and 
        allow the target to be read by the compute shaders
    */
    Heart::hRenderUtility::setCameraParameters(ctx, &camera_);

    ctx->setComputeInput(&blurVertCObj_);
    ctx->dispatch(renderer->GetHeight(), 1, 1);

//     ctx->setComputeInput(&blurHozCObj_);
//     ctx->dispatch(TEST_TEXTURE_WIDTH/128, 1, 1);

    // Submit to back buffer
    ctx->clearColour(camera_.getRenderTarget(0), Heart::hColour(.5f, .5f, .5f, 1.f));
    ctx->clearDepth(camera_.getDepthTarget(), 1.f);

    /*
        Bind the back buffer against the device, unbinds the result buffer and 
        allow the target to be read by the compute shaders
    */
    Heart::hRenderUtility::setCameraParameters(ctx, &camera_);
    tech=blurToScreen_->getGroup(0)->getTechniqueByMask(techinfo->mask_);
    for (hUint32 pass = 0, passcount = tech->GetPassCount(); pass < passcount; ++pass ) {
        Heart::hMaterialTechniquePass* passptr = tech->GetPass(pass);
        ctx->SetMaterialPass(passptr);
        ctx->DrawIndexedPrimitive(quadIB_->GetIndexCount()/3, 0);
    }
}


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void ComputeBlur::CreateRenderResources()
{
    using namespace Heart;
    hRenderer* renderer = engine_->GetRenderer();
    hRenderMaterialManager* matMgr=renderer->GetMaterialManager();
    hResourceManager* resMgr=engine_->GetResourceManager();
    hRendererCamera* camera = &camera_;
    hUint32 w = renderer->GetWidth();
    hUint32 h = renderer->GetHeight();
    hFloat aspect = (hFloat)w/(hFloat)h;
    hRenderViewportTargetSetup rtDesc={0};
    hTexture* bb=matMgr->getGlobalTexture("back_buffer");
    hTexture* db=matMgr->getGlobalTexture("depth_buffer");
    hRenderTargetView* rtv=NULL;
    hRenderTargetView* brtv=NULL;
    hDepthStencilView* dsv=NULL;
    hRenderTargetViewDesc rtvd;
    hDepthStencilViewDesc dsvd;
    hZeroMem(&rtvd, sizeof(rtvd));
    hZeroMem(&dsvd, sizeof(dsvd));
    rtvd.format_=bb->getTextureFormat();
    rtvd.resourceType_=bb->getRenderType();
    hcAssert(bb->getRenderType()==eRenderResourceType_Tex2D);
    rtvd.tex2D_.topMip_=0;
    rtvd.tex2D_.mipLevels_=~0;
    dsvd.format_=db->getTextureFormat();
    dsvd.resourceType_=db->getRenderType();
    hcAssert(db->getRenderType()==eRenderResourceType_Tex2D);
    dsvd.tex2D_.topMip_=0;
    dsvd.tex2D_.mipLevels_=~0;
    renderer->createRenderTargetView(bb, rtvd, &rtv);
    renderer->createDepthStencilView(db, dsvd, &dsv);
    rtDesc.nTargets_=1;
    rtDesc.targetTex_=bb;
    rtDesc.targets_[0]=rtv;
    rtDesc.depth_=dsv;

    hRelativeViewport vp;
    vp.x=0.f;
    vp.y=0.f;
    vp.w=1.f;
    vp.h=1.f;

    Heart::hMatrix vm = fpCamera_.getViewmatrix();//Heart::hMatrixFunc::LookAt(camPos_, camPos_+camDir_, camUp_);

#ifdef CREATE_TEXTURE
    hUint32* inittexdata=(hUint32*)hMalloc(sizeof(hUint32)*TEST_TEXTURE_WIDTH*TEST_TEXTURE_HEIGHT);
    hMipDesc resTexInit={
        TEST_TEXTURE_WIDTH, 
        TEST_TEXTURE_HEIGHT, 
        (hByte*)inittexdata, 
        TEST_TEXTURE_WIDTH*TEST_TEXTURE_HEIGHT*sizeof(hUint32)
    };

    hUint32* ptr=inittexdata;
    for (hUint h=0; h<TEST_TEXTURE_HEIGHT; ++h) {
        for (hUint w=0; w<TEST_TEXTURE_WIDTH; ++w, ++ptr) {
#if 1
            if (h < TEST_TEXTURE_HEIGHT/2) {
                if (w < TEST_TEXTURE_WIDTH/2) {
                    *ptr= 0xFF00000A; //RED
                } else {
                    *ptr= 0xFF000A00; //GREEN
                }
            } else {
                if (w < TEST_TEXTURE_WIDTH/2) {
                    *ptr= 0xFF0A0000; // BLUE
                } else {
                    *ptr= 0xFF0A0A0A; // WHITE
                }
            }
#else
            hUint r=w % 4;
            static hUint32 colarr[] = {
                0xFF0000FF, 0xFF00FF00, 0xFFFF0000, 0xFFFFFFFF
            };
            *ptr= colarr[r];
#endif
        }
    }
    hFree(inittexdata);
    inittexdata=nullptr;
#endif //CREATE_TEXTURE

    camera->Initialise(renderer);
    camera->bindRenderTargetSetup(rtDesc);
    camera->SetFieldOfView(45.f);
    camera->SetOrthoParams(0.f, 2.f, 2.f, 0.f, 0.f, 1000.f);
    camera->SetViewMatrix(vm);
    camera->setViewport(vp);
    camera->SetTechniquePass(matMgr->GetRenderTechniqueInfo("main"));


#ifdef CREATE_TEXTURE
    renderer->createTexture(1, &resTexInit, TFORMAT_XRGB8, 0, GetGlobalHeap(), &resTex_);
#else
    resTex_=static_cast<Heart::hTexture*>(resMgr->mtGetResource(ASSET_PATH_TEXTURE));
    resTex_->AddRef();
#endif
    hRenderUtility::buildTessellatedQuadMesh(2.f, 2.f, 20, 20, renderer, GetGlobalHeap(), &quadIB_, &quadVB_);
    hMaterial* blitmat=static_cast<Heart::hMaterial*>(resMgr->mtGetResource(ASSET_BLIT_MAT));
    materialInstance_=matMgr->getDebugTexMaterial()->createMaterialInstance(0);
    blurToScreen_=blitmat->createMaterialInstance(0);

    Heart::hTexture* blurTarget=matMgr->getGlobalTexture("blur_target");
    rtvd.format_=blurTarget->getTextureFormat();
    renderer->createRenderTargetView(blurTarget, rtvd, &brtv);
    hMemSet(&rtDesc, 0, sizeof(rtDesc));
    rtDesc.nTargets_=1;
    rtDesc.targetTex_=db;
    rtDesc.targets_[0]=brtv;
    blurCamera_.Initialise(renderer);
    blurCamera_.bindRenderTargetSetup(rtDesc);
    blurCamera_.SetFieldOfView(45.f);
    blurCamera_.SetOrthoParams(0.f, 2.f, 2.f, 0.f, 0.f, 1000.f);
    blurCamera_.SetViewMatrix(vm);
    blurCamera_.setViewport(vp);
    blurCamera_.SetTechniquePass(matMgr->GetRenderTechniqueInfo("main"));
    
    hShaderParameterID textureid=hCRC32::StringCRC("g_texture");
    hTexture* rwtex=matMgr->getGlobalTexture("blur_rw_texture");
    hShaderResourceViewDesc srvd;
    hShaderResourceView* mockSceneSRV;
    hZeroMem(&srvd, sizeof(srvd));
    srvd.format_=resTex_->getTextureFormat();
    srvd.resourceType_=eRenderResourceType_Tex2D;
    srvd.tex2D_.topMip_=0;
    srvd.tex2D_.mipLevels_=~0;
    renderer->createShaderResourceView(resTex_, srvd, &mockSceneSRV);
    srvd.format_=blurTarget->getTextureFormat();
    renderer->createShaderResourceView(blurTarget, srvd, &blurTexSRV_);
    srvd.format_=rwtex->getTextureFormat();
    renderer->createShaderResourceView(rwtex, srvd, &computeBlurTexSRV_);

    materialInstance_->bindResource(textureid, mockSceneSRV);
    materialInstance_->bindInputStreams(PRIMITIVETYPE_TRILIST, quadIB_, &quadVB_, 1);
    textureid=hCRC32::StringCRC("texColorInput");
    blurToScreen_->bindResource(textureid, computeBlurTexSRV_);
    blurToScreen_->bindInputStreams(PRIMITIVETYPE_TRILIST, quadIB_, &quadVB_, 1);

    mockSceneSRV->DecRef();

    modelMtxCB_=matMgr->GetGlobalConstantBlock(hCRC32::StringCRC("InstanceConstants"));
    renderer->createBuffer(sizeof(ComputeBlurData::cbParams), NULL, eResourceFlag_ConstantBuffer, 0, &blurParamCB_);

    blurHCS_=static_cast<hShaderProgram*>(resMgr->mtGetResource(ASSET_PATH_ROW));
    blurVCS_=static_cast<hShaderProgram*>(resMgr->mtGetResource(ASSET_PATH_COL));
    hcAssert(blurHCS_ && blurVCS_);

    renderer->createComputeUAV(rwtex, eTextureFormat_R32_uint, 0, &blurUAV_);
    blurHozCObj_.bindShaderProgram(blurHCS_);
    blurHozCObj_.bindResourceView(hCRC32::StringCRC("g_texInput"), blurTexSRV_);
    blurHozCObj_.bindUAV(hCRC32::StringCRC("g_rwtOutput"), &blurUAV_);
    blurHozCObj_.bindConstantBuffer(hCRC32::StringCRC("cbParams"), blurParamCB_);

    blurVertCObj_.bindShaderProgram(blurHCS_);
    blurVertCObj_.bindResourceView(hCRC32::StringCRC("g_texInput"), blurTexSRV_);
    blurVertCObj_.bindUAV(hCRC32::StringCRC("g_rwtOutput"), &blurUAV_);
    blurVertCObj_.bindConstantBuffer(hCRC32::StringCRC("cbParams"), blurParamCB_);

    rtv->DecRef();
    brtv->DecRef();
    dsv->DecRef();
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void ComputeBlur::DestroyRenderResources()
{
    using namespace Heart;
    hRenderer* renderer = engine_->GetRenderer();
    hRendererCamera* camera = &camera_;

    renderer->destroyComputeUAV(&blurUAV_);
    hMaterialInstance::destroyMaterialInstance(materialInstance_);
    hMaterialInstance::destroyMaterialInstance(blurToScreen_);
    if (resTex_) {
        resTex_->DecRef();
        resTex_=NULL;
    }
    materialInstance_=NULL;
    camera->releaseRenderTargetSetup();
    if (quadIB_) {
        quadIB_->DecRef();
        quadIB_=NULL;
    }
    if (quadVB_) {
        quadVB_->DecRef();
        quadVB_=NULL;
    }
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void ComputeBlur::UpdateCamera()
{
    using namespace Heart;
    using namespace Heart::hVec3Func;

    fpCamera_.update(hClock::Delta());
    camera_.SetViewMatrix(fpCamera_.getViewmatrix());
}
