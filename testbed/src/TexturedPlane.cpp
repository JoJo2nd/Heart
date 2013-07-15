/********************************************************************

    filename:   TexturedPlane.cpp  
    
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
#include "TexturedPlane.h"
#include "TestUtils.h"

DEFINE_HEART_UNIT_TEST(TexturedPlane);

#define PACKAGE_NAME ("MATERIALS")
#define RESOURCE_NAME ("MATERIALS")
#define ASSET_PATH ("MATERIALS.MINE")

#define TEST_TEXTURE_WIDTH  (265)
#define TEST_TEXTURE_HEIGHT (265)

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

hUint32 TexturedPlane::RunUnitTest()
{
    Heart::hdGamepad* pad = engine_->GetControllerManager()->GetGamepad(0);
    Heart::hdKeyboard* kb = engine_->GetControllerManager()->GetSystemKeyboard();
    switch(state_)
    {
    case eBeginLoad:
        {
            fpCamera_.setInput(pad);
            state_ = eRender;
            timer_ = 0.f;
            CreateRenderResources();
            SetCanRender(hTrue);
        }
        break;
    case eRender:
        {
            timer_ += Heart::hClock::Delta();
            UpdateCamera();
            if (getForceExitFlag()) {
                state_ = eBeginUnload;
            }
        }
        break;
    case eBeginUnload:
        {
            SetCanRender(hFalse);
            DestroyRenderResources();
            hcPrintf("End unit test %s.", s_className_);
            SetExitCode(UNIT_TEST_EXIT_CODE_OK);
            state_ = eExit;
        }
        break;
    }

    return 0;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void TexturedPlane::RenderUnitTest()
{
    Heart::hRenderer* renderer = engine_->GetRenderer();
    Heart::hRenderSubmissionCtx* ctx=renderer->GetMainSubmissionCtx();
    const Heart::hRenderTechniqueInfo* techinfo = renderer->GetMaterialManager()->GetRenderTechniqueInfo("main");
    Heart::hRenderBufferMapInfo mapinfo;

    camera_.UpdateParameters(ctx);
    ctx->setTargets(camera_.getTargetCount(), camera_.getTargets(), camera_.getDepthTarget());
    ctx->SetViewport(camera_.getTargetViewport());
    for (hUint i=0; i<camera_.getTargetCount(); ++i) {
        ctx->clearColour(camera_.getRenderTarget(i), Heart::hColour(.5f, .5f, .5f, 1.f));
    }
    ctx->clearDepth(camera_.getDepthTarget(), 1.f);

    ctx->Map(modelMtxCB_, &mapinfo);
    *(Heart::hMatrix*)mapinfo.ptr = Heart::hMatrixFunc::identity();
    ctx->Unmap(&mapinfo);

    Heart::hMaterialTechnique* tech=materialInstance_->getGroup(0)->getTechniqueByMask(techinfo->mask_);
    for (hUint32 pass = 0, passcount = tech->GetPassCount(); pass < passcount; ++pass ) {
        Heart::hMaterialTechniquePass* passptr = tech->GetPass(pass);
        ctx->SetMaterialPass(passptr);
        ctx->DrawIndexedPrimitive(quadIB_->GetIndexCount()/3, 0);
    }
}


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void TexturedPlane::CreateRenderResources()
{
    using namespace Heart;
    hRenderer* renderer = engine_->GetRenderer();
    hRenderMaterialManager* matMgr=renderer->GetMaterialManager();
    hRendererCamera* camera = &camera_;
    hUint32 w = renderer->GetWidth();
    hUint32 h = renderer->GetHeight();
    hFloat aspect = (hFloat)w/(hFloat)h;
    hRenderViewportTargetSetup rtDesc={0};
    hTexture* bb=matMgr->getGlobalTexture("back_buffer");
    hTexture* db=matMgr->getGlobalTexture("depth_buffer");
    hRenderTargetView* rtv=NULL;
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
    dsvd.format_=eTextureFormat_D32_float;
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

    hUint32* inittexdata=(hUint32*)hHeapMalloc(GetGlobalHeap(), sizeof(hUint32)*TEST_TEXTURE_WIDTH*TEST_TEXTURE_HEIGHT);
    Heart::hMatrix vm = Heart::hMatrixFunc::identity();//Heart::hMatrixFunc::LookAt(camPos_, camPos_+camDir_, camUp_);
    hMipDesc resTexInit={
        TEST_TEXTURE_WIDTH, 
        TEST_TEXTURE_HEIGHT, 
        (hByte*)inittexdata, 
        TEST_TEXTURE_WIDTH*TEST_TEXTURE_HEIGHT*sizeof(hUint32)
    };

    hUint32* ptr=inittexdata;
    for (hUint h=0; h<TEST_TEXTURE_HEIGHT; ++h) {
        for (hUint w=0; w<TEST_TEXTURE_WIDTH; ++w, ++ptr) {
            if (h < TEST_TEXTURE_HEIGHT/2) {
                if (w < TEST_TEXTURE_WIDTH/2) {
                    *ptr= 0xFF0000FF; //RED
                } else {
                    *ptr= 0xFF00FF00; //GREEN
                }
            } else {
                if (w < TEST_TEXTURE_WIDTH/2) {
                    *ptr= 0xFFFF0000; // BLUE
                } else {
                    *ptr= 0xFFFFFFFF; // WHITE
                }
            }
        }
    }

    camera->Initialise(renderer);
    camera->bindRenderTargetSetup(rtDesc);
    camera->SetFieldOfView(45.f);
    camera->SetOrthoParams(0.f, 2.f, 2.f, 0.f, 0.f, 1000.f);
    camera->SetViewMatrix(vm);
    camera->setViewport(vp);
    camera->SetTechniquePass(matMgr->GetRenderTechniqueInfo("main"));

    renderer->createTexture(1, &resTexInit, eTextureFormat_RGBA8_sRGB_unorm, 0, GetGlobalHeap(), &resTex_);
    hRenderUtility::buildTessellatedQuadMesh(2.f, 2.f, 20, 20, renderer, GetGlobalHeap(), &quadIB_, &quadVB_);
    materialInstance_=matMgr->getDebugTexMaterial()->createMaterialInstance(0);
    hShaderResourceViewDesc srvd;
    hZeroMem(&srvd, sizeof(srvd));
    srvd.format_=resTex_->getTextureFormat();
    srvd.resourceType_=resTex_->getRenderType();
    srvd.tex2D_.topMip_=0;
    srvd.tex2D_.mipLevels_=~0;
    renderer->createShaderResourceView(resTex_, srvd, &resTesSRV_);

    materialInstance_->bindResource(hCRC32::StringCRC("g_texture"), resTesSRV_);
    materialInstance_->bindInputStreams(PRIMITIVETYPE_TRILIST, quadIB_, &quadVB_, 1);

    modelMtxCB_=matMgr->GetGlobalConstantBlock(hCRC32::StringCRC("InstanceConstants"));

    // The camera hold refs to this
    rtv->DecRef();
    dsv->DecRef();
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void TexturedPlane::DestroyRenderResources()
{
    using namespace Heart;
    hRenderer* renderer = engine_->GetRenderer();
    hRendererCamera* camera = &camera_;

    if (resTex_) {
        resTex_->DecRef();
        resTex_=NULL;
    }
    hMaterialInstance::destroyMaterialInstance(materialInstance_);
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

void TexturedPlane::UpdateCamera()
{
    using namespace Heart;

    hRenderer* renderer = engine_->GetRenderer();
    hRendererCamera* camera = renderer->GetRenderCamera(0);

    fpCamera_.update(hClock::Delta());
    camera->SetViewMatrix(fpCamera_.getViewmatrix());
}
