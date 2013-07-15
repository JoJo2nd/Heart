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

    //submit the lighting pass
    drawcall.customCallFlag_=true;
    drawcall.sortKey_=Heart::hBuildRenderSortKey(1/*cam*/, 1, 0, 0.f, 0, 0);
    drawcall.customCall_=hFUNCTOR_BINDMEMBER(Heart::hCustomRenderCallback, Heart::hLightingManager, doDeferredLightPass, &deferredLightManager_);
    drawCtx_.SubmitDrawCall(drawcall);

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
    hTexture* backbuffer=matMgr->getGlobalTexture("back_buffer");
    hTexture* albedo=matMgr->getGlobalTexture("gbuffer_albedo");
    hTexture* normal=matMgr->getGlobalTexture("gbuffer_normal");
    hTexture* spec=matMgr->getGlobalTexture("gbuffer_spec");
    hTexture* depth=matMgr->getGlobalTexture("depth_buffer");
    hRenderTargetView* rtv[3]={hNullptr, hNullptr, hNullptr};
    hDepthStencilView* dsv=NULL;
    hRenderTargetViewDesc rtvd;
    hDepthStencilViewDesc dsvd;

    hZeroMem(&rtvd, sizeof(rtvd));
    rtvd.format_=albedo->getTextureFormat();
    rtvd.resourceType_=albedo->getRenderType();
    hcAssert(albedo->getRenderType()==eRenderResourceType_Tex2D);
    rtvd.tex2D_.topMip_=0;
    rtvd.tex2D_.mipLevels_=~0;
    renderer->createRenderTargetView(albedo, rtvd, &rtv[0]);

    hZeroMem(&rtvd, sizeof(rtvd));
    rtvd.format_=normal->getTextureFormat();
    rtvd.resourceType_=normal->getRenderType();
    hcAssert(normal->getRenderType()==eRenderResourceType_Tex2D);
    rtvd.tex2D_.topMip_=0;
    rtvd.tex2D_.mipLevels_=~0;
    renderer->createRenderTargetView(normal, rtvd, &rtv[1]);

    hZeroMem(&rtvd, sizeof(rtvd));
    rtvd.format_=spec->getTextureFormat();
    rtvd.resourceType_=spec->getRenderType();
    hcAssert(spec->getRenderType()==eRenderResourceType_Tex2D);
    rtvd.tex2D_.topMip_=0;
    rtvd.tex2D_.mipLevels_=~0;
    renderer->createRenderTargetView(spec, rtvd, &rtv[2]);

    hZeroMem(&dsvd, sizeof(dsvd));
    dsvd.format_=eTextureFormat_D32_float;
    dsvd.resourceType_=depth->getRenderType();
    hcAssert(depth->getRenderType()==eRenderResourceType_Tex2D);
    dsvd.tex2D_.topMip_=0;
    dsvd.tex2D_.mipLevels_=~0;
    renderer->createDepthStencilView(depth, dsvd, &dsv);

    rtDesc.nTargets_=3;
    rtDesc.targetTex_=albedo;
    rtDesc.targets_[0]=rtv[0];
    rtDesc.targets_[1]=rtv[1];
    rtDesc.targets_[2]=rtv[2];
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
    Heart::hRenderBuffer* constblock_;
    MaterialConstants initdata = {
        Heart::hVec4(1.f, 1.f, 1.f, 1.f),
        Heart::hVec4(1.f, 1.f, 1.f, 1.f),
        64.f
    };
    renderer->createBuffer(sizeof(MaterialConstants), &initdata, eResourceFlag_ConstantBuffer, 0, &constblock_);

    for (hUint i=0, n=renderModel_->GetLODCount(); i<n; ++i) {
        hGeomLODLevel* lod=renderModel_->GetLOD(i);
        for(hUint ri=0, rn=lod->renderObjects_.GetSize(); ri<rn; ++ri) {
            lod->renderObjects_[ri].GetMaterial()->bindConstanstBuffer(Heart::hCRC32::StringCRC("MaterialConstants"), constblock_);
        }
    }

    //material hold this so let go
    constblock_->DecRef();

    // The camera hold refs to this
    for (hUint i=0; i<hStaticArraySize(rtv); ++i)
    {
        rtv[i]->DecRef();
    }
    dsv->DecRef();

    // setup the deferred camera
    camera = renderer->GetRenderCamera(1);
    hZeroMem(&rtvd, sizeof(rtvd));
    rtvd.format_=backbuffer->getTextureFormat();
    rtvd.resourceType_=backbuffer->getRenderType();
    hcAssert(backbuffer->getRenderType()==eRenderResourceType_Tex2D);
    rtvd.tex2D_.topMip_=0;
    rtvd.tex2D_.mipLevels_=~0;
    renderer->createRenderTargetView(backbuffer, rtvd, &rtv[0]);

    rtDesc.nTargets_=1;
    rtDesc.targetTex_=backbuffer;
    rtDesc.targets_[0]=rtv[0];
    rtDesc.depth_=hNullptr;

    vp.x=0.f;
    vp.y=0.f;
    vp.w=1.f;
    vp.h=1.f;

    vm = fpCamera_.getViewmatrix();
    camera->bindRenderTargetSetup(rtDesc);
    camera->SetFieldOfView(45.f);
    camera->SetOrthoParams(0.f, 0.f, 1.f, 1.f, 0.f, 1000.f);
    camera->SetViewMatrix(vm);
    camera->setViewport(vp);
    camera->setClearScreenFlag(true);
    camera->SetTechniquePass(renderer->GetMaterialManager()->GetRenderTechniqueInfo("lighting"));
    
    rtv[0]->DecRef();

    hLightingManager::hRenderTargetInfo lightInput;
    lightInput.albedo_=albedo;
    lightInput.normal_=normal;
    lightInput.spec_=spec;
    lightInput.depth_=depth;
    lightInput.vertexLightShader_=static_cast<hShaderProgram*>(engine_->GetResourceManager()->mtGetResource("MATERIALS.DEFERRED_VS"));
    lightInput.pixelLightShader_=static_cast<hShaderProgram*>(engine_->GetResourceManager()->mtGetResource("MATERIALS.DEFERRED_PS"));
    lightInput.viewCameraIndex_=0;
    deferredLightManager_.initialise(renderer, &lightInput);
    deferredLightManager_.addDirectionalLight(Heart::hVec3Func::normalise(hVec3(2.f, -.1f, .2f)), Heart::WHITE);
    //deferredLightManager_.addDirectionalLight(Heart::hVec3Func::normalise(hVec3(1.f, 0.f, 0.f)), Heart::WHITE);
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