/********************************************************************

    filename:   Sibenik.cpp  
    
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
#include "Sibenik.h"
#include "TestUtils.h"

DEFINE_HEART_UNIT_TEST(Sibenik);

#define PACKAGE_NAME ("SPONZA")
#define RESOURCE_NAME ("SPONZA")
#define ASSET_PATH ("SPONZA.SPONZA")

namespace {
    struct MaterialConstants {
        Heart::hVec4 matColour;
        Heart::hVec4 matSpecColour;
        hFloat       matSpecPower;
    };
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

hUint32 Sibenik::RunUnitTest()
{
    Heart::hdGamepad* pad = engine_->GetControllerManager()->GetGamepad(0);
    Heart::hdKeyboard* kb = engine_->GetControllerManager()->GetSystemKeyboard();
    switch(state_)
    {
    case eBeginLoad:
        {
            hcPrintf("Loading package \"%s\"", PACKAGE_NAME);
            engine_->GetResourceManager()->mtLoadPackage(PACKAGE_NAME);
            fpCamera_.setInput(pad);
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

void Sibenik::RenderUnitTest()
{
    Heart::hRenderer* renderer = engine_->GetRenderer();
    Heart::hGeomLODLevel* lod = renderModel_->GetLOD(0);
    hUint32 lodobjects = lod->renderObjects_.GetSize();
    const Heart::hRenderTechniqueInfo* techinfo = engine_->GetRenderer()->GetMaterialManager()->GetRenderTechniqueInfo("main");
    Heart::hDrawCall drawcall;
    drawCtx_.Begin(renderer);

    for (hUint32 i = 0; i < lodobjects; ++i) {
        // Should a renderable simply store a draw call?
        Heart::hRenderable* renderable = &lod->renderObjects_[i];

        hFloat dist=Heart::hVec3Func::lengthFast(fpCamera_.getCameraPosition()-renderable->GetAABB().c_);
        Heart::hMaterialGroup* group = renderable->GetMaterial()->getGroup(0);
        for (hUint t=0, nt=group->getTechCount(); t<nt; ++t) {
            Heart::hMaterialTechnique* tech=group->getTech(t);
            if (tech->GetMask()!=techinfo->mask_) {
                continue;
            }
            for (hUint32 pass = 0, passcount = tech->GetPassCount(); pass < passcount; ++pass ) {
                drawcall.sortKey_ = Heart::hBuildRenderSortKey(0/*cam*/, tech->GetLayer(), tech->GetSort(), dist, renderable->GetMaterialKey(), pass);
                drawcall.rCmds_ = renderModel_->getRenderCommands(renderable->getRenderCommandOffset(0, t, pass));
                drawCtx_.SubmitDrawCall(drawcall);
            }
        }
    }
    drawCtx_.End();
}


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void Sibenik::CreateRenderResources()
{
    using namespace Heart;
    hRenderer* renderer = engine_->GetRenderer();
    hRendererCamera* camera = renderer->GetRenderCamera(0);
    hRenderMaterialManager* matMgr=renderer->GetMaterialManager();
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
    dsvd.format_=TFORMAT_D32F;
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

    fpCamera_.setMoveSpeed(5.f);
    Heart::hMatrix vm = fpCamera_.getViewmatrix();

    camera->bindRenderTargetSetup(rtDesc);
    camera->SetFieldOfView(45.f);
    camera->SetProjectionParams( aspect, 1.f, 10000.f);
    camera->SetViewMatrix(vm);
    camera->setViewport(vp);
    camera->SetTechniquePass(renderer->GetMaterialManager()->GetRenderTechniqueInfo("main"));

    renderModel_ = static_cast<hRenderModel*>(engine_->GetResourceManager()->mtGetResource(ASSET_PATH));
    hcAssert(renderModel_);
    Heart::hParameterConstantBlock* constblock_;
    MaterialConstants initdata = {
        Heart::hVec4(1.f, 1.f, 1.f, 1.f),
        Heart::hVec4(1.f, 1.f, 1.f, 1.f),
        64.f
    };
    renderer->createConstantBlock(sizeof(MaterialConstants), &initdata, &constblock_);

    for (hUint i=0, n=renderModel_->GetLODCount(); i<n; ++i) {
        hGeomLODLevel* lod=renderModel_->GetLOD(i);
        for(hUint ri=0, rn=lod->renderObjects_.GetSize(); ri<rn; ++ri) {
            lod->renderObjects_[ri].GetMaterial()->bindConstanstBuffer(Heart::hCRC32::StringCRC("MaterialConstants"), constblock_);
        }
    }

    //material hold this so let go
    constblock_->DecRef();

    // The camera hold refs to this
    rtv->DecRef();
    dsv->DecRef();
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void Sibenik::DestroyRenderResources()
{
    using namespace Heart;
    hRenderer* renderer = engine_->GetRenderer();
    hRendererCamera* camera = renderer->GetRenderCamera(0);

    camera->releaseRenderTargetSetup();
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void Sibenik::UpdateCamera()
{
    using namespace Heart;

    hRenderer* renderer = engine_->GetRenderer();
    hRendererCamera* camera = renderer->GetRenderCamera(0);

    fpCamera_.update(hClock::Delta());
    camera->SetViewMatrix(fpCamera_.getViewmatrix());
}