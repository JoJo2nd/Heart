/********************************************************************

    filename:   LoadTextureTest.cpp  
    
    Copyright (c) 30:1:2013 James Moran
    
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
#include "LoadTextureTest.h"
#include "TestUtils.h"

DEFINE_HEART_UNIT_TEST(LoadTextureTest);

#define PACKAGE_NAME ("MATERIALS")
#define RESOURCE_NAME ("NARUTO_TEST")
#define ASSET_PATH ("MATERIALS.NARUTO_TEST")

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

hUint32 LoadTextureTest::RunUnitTest()
{
    Heart::hdGamepad* pad = engine_->GetControllerManager()->GetGamepad(0);
    Heart::hdKeyboard* kb = engine_->GetControllerManager()->GetSystemKeyboard();
    switch(state_)
    {
    case eBeginLoad:
        {
            hcPrintf("Loading package \"%s\"", PACKAGE_NAME);
            fpCamera_.setInput(pad);
            engine_->GetResourceManager()->mtLoadPackage(PACKAGE_NAME);
            state_ = eLoading;
        }
        break;
    case eLoading:
        {
            if (engine_->GetResourceManager()->mtIsPackageLoaded(PACKAGE_NAME))
            {
                hcPrintf("Loaded package \"%s\"", PACKAGE_NAME);
                state_ = eRender;
                timer_ = 0.f;
                CreateRenderResources();
                SetCanRender(hTrue);
            }
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
            engine_->GetResourceManager()->mtUnloadPackage(PACKAGE_NAME);
            hcPrintf("Unloading package \"%s\"", PACKAGE_NAME);
            state_ = eExit;
        }
        break;
    case eExit:
        {
            hcPrintf("End unit test %s.", s_className_);
            SetExitCode(UNIT_TEST_EXIT_CODE_OK);
        }
        break;
    }

    return 0;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void LoadTextureTest::RenderUnitTest()
{
    using namespace Heart;

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

void LoadTextureTest::CreateRenderResources()
{
    using namespace Heart;
    hRenderer* renderer = engine_->GetRenderer();
    hRenderMaterialManager* matMgr=renderer->GetMaterialManager();
    hUint32 w = renderer->GetWidth();
    hUint32 h = renderer->GetHeight();
    hFloat aspect = (hFloat)w/(hFloat)h;
    hRenderViewportTargetSetup rtDesc={0};
    hTexture* bb=matMgr->getGlobalTexture("back_buffer");
    hTexture* db=matMgr->getGlobalTexture("depth_buffer");
    hTextureFormat dfmt=eTextureFormat_D32_float;
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
    dsvd.format_=dfmt;
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
    vp.x= 0.f;
    vp.y= 0.f;
    vp.w= 1.f;
    vp.h= 1.f;

    Heart::hMatrix vm = Heart::hMatrixFunc::identity();;
    camera_.Initialise(renderer);
    camera_.bindRenderTargetSetup(rtDesc);
    camera_.SetFieldOfView(45.f);
    camera_.SetOrthoParams(0.f, 1.f, 2.f, -1.f, 0.f, 1000.f);
    camera_.SetViewMatrix(vm);
    camera_.setViewport(vp);
    camera_.SetTechniquePass(renderer->GetMaterialManager()->GetRenderTechniqueInfo("main"));

    rtDesc.nTargets_=0;
    rtDesc.depth_=dsv;

    resTex_=static_cast<hTexture*>(engine_->GetResourceManager()->mtGetResource(ASSET_PATH));
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

void LoadTextureTest::DestroyRenderResources()
{
    using namespace Heart;

    camera_.releaseRenderTargetSetup();
    if (resTex_) {
        resTex_->DecRef();
        resTex_=NULL;
    }
    hMaterialInstance::destroyMaterialInstance(materialInstance_);
    materialInstance_=NULL;
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

void LoadTextureTest::UpdateCamera()
{
    using namespace Heart;

    hRenderer* renderer = engine_->GetRenderer();
    hRendererCamera* camera = renderer->GetRenderCamera(0);

    fpCamera_.update(hClock::Delta());
    camera->SetViewMatrix(fpCamera_.getViewmatrix());
}