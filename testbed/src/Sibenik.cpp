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
    switch(state_)
    {
    case eBeginLoad:
        {
            hcPrintf("Loading package \"%s\"", PACKAGE_NAME);
            engine_->GetResourceManager()->loadPackage(PACKAGE_NAME);
            hcPrintf("Stub");//fpCamera_.setInput(pad);
            state_ = eLoading;
        }
        break;
    case eLoading:
        {
            if (engine_->GetResourceManager()->getIsPackageLoaded(PACKAGE_NAME) &&
                Heart::hResourceHandle("MATERIALS.DEFERRED_PS").weakPtr() && // <- Not correct way to handle this!
                Heart::hResourceHandle("MATERIALS.DEFERRED_VS").weakPtr())
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
            timer_ += Heart::hClock::delta();
            UpdateCamera();
            deferredLightManager_.setSphereLight(0, Heart::hVec3(Heart::hCos(timer_*0.5f)*0.f, 250.f, 0.f), 100.f+Heart::hSin(timer_*0.5f)*99.f);
            deferredLightManager_.setSphereLight(1, Heart::hVec3(Heart::hCos(timer_*0.5f)*1000.f, 600.f, 0.f), 100.f);
            if (getForceExitFlag()) {
                state_ = eBeginUnload;
            }
        }
        break;
    case eBeginUnload:
        {
            SetCanRender(hFalse);
            DestroyRenderResources();
            engine_->GetResourceManager()->unloadPackage(PACKAGE_NAME);
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
    const Heart::hRenderTechniqueInfo* techinfo = engine_->GetRenderer()->GetMaterialManager()->GetRenderTechniqueInfo("main");
    Heart::hDrawCall drawcall;
    drawCtx_.Begin(renderer);
    Heart::hRenderModel* renderModel;
    Heart::hResourceHandleScope<Heart::hRenderModel> handleScope(renderModelHandle_, &renderModel);

    if (renderModel) {
        for (hUint32 i=0, n=renderModel->getRenderableCount(); i<n; ++i) {
            // Should a renderable simply store a draw call?
            Heart::hRenderable* renderable=renderModel->getRenderable(i);

            hFloat dist=Heart::hVec3Func::lengthFast(fpCamera_.getCameraPosition()-renderModel->getBounds().c_);
            Heart::hMaterialGroup* group = renderable->GetMaterial()->getGroup(0);
            for (hUint t=0, nt=group->getTechCount(); t<nt; ++t) {
                Heart::hMaterialTechnique* tech=group->getTech(t);
                if (tech->GetMask()!=techinfo->mask_) {
                    continue;
                }
                for (hUint32 pass = 0, passcount = tech->GetPassCount(); pass < passcount; ++pass ) {
                    drawcall.sortKey_ = Heart::hBuildRenderSortKey(0/*cam*/, tech->GetLayer(), tech->GetSort(), dist, renderable->GetMaterialKey(), pass);
                    drawcall.rCmds_ = renderModel->getRenderCommands(renderable->getRenderCommandOffset(0, t, pass));
                    drawCtx_.SubmitDrawCall(drawcall);
                }
            }
        }

        //submit the lighting pass
        drawcall.customCallFlag_=true;
        drawcall.sortKey_=Heart::hBuildRenderSortKey(1/*cam*/, 1, 0, 0.f, 0, 0);
        drawcall.customCall_=hFUNCTOR_BINDMEMBER(Heart::hCustomRenderCallback, Heart::hLightingManager, doDeferredLightPass, &deferredLightManager_);
        drawCtx_.SubmitDrawCall(drawcall);
    }

    drawCtx_.End();

    Heart::hDebugLine orig[] = {
        {Heart::hVec3(0.f, 0.f, 0.f), Heart::hVec3(100.f, 0.f, 0.f), Heart::hColour(1.f,0.f,0.f,1.f)},
        {Heart::hVec3(0.f, 0.f, 0.f), Heart::hVec3(0.f, 100.f, 0.f), Heart::hColour(0.f,1.f,0.f,1.f)},
        {Heart::hVec3(0.f, 0.f, 0.f), Heart::hVec3(0.f, 0.f, 100.f), Heart::hColour(0.f,0.f,1.f,1.f)},

        {Heart::hVec3(0.f, 0.f, 0.f), Heart::hVec3(-100.f, 0.f, 0.f), Heart::hColour(0.f,1.f,1.f,1.f)},
        {Heart::hVec3(0.f, 0.f, 0.f), Heart::hVec3(0.f, -100.f, 0.f), Heart::hColour(1.f,0.f,1.f,1.f)},
        {Heart::hVec3(0.f, 0.f, 0.f), Heart::hVec3(0.f, 0.f, -100.f), Heart::hColour(1.f,1.f,0.f,1.f)},
    };
    Heart::hVec3 tri[] = {
        Heart::hVec3(-100.f, -50.f, 0.f), Heart::hVec3(0.f, 50.f, 0.f), Heart::hVec3(100.f, -50.f, 0.f),
    };
    Heart::hDebugDraw* dd=Heart::hDebugDraw::it();
    dd->begin();
    dd->drawLines(orig, 6, Heart::eDebugSet_3DDepth);
    dd->drawTexturedQuad(Heart::hVec3(-640, 160.f, 0.f), 355.f, 200.f, normalSRV_);
//     dd->drawLines(orig, 6, Heart::eDebugSet_2DNoDepth);
//     dd->drawTris(tri, 3, Heart::hColour(1.f,0.f,0.f,0.5f), Heart::eDebugSet_3DDepth);
//     dd->drawTris(tri, 3, Heart::hColour(0.f,0.f,1.f,0.5f), Heart::eDebugSet_2DNoDepth);
//     dd->drawText(Heart::hVec3(11.f,  9.f, 0.f), "Hello World from debug text", Heart::hColour(0.f,0.f,0.f,1.f));
//     dd->drawText(Heart::hVec3(10.f, 10.f, 0.f), "Hello World from debug text", Heart::hColour(1.f,1.f,1.f,1.f));
    dd->end();
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

    renderModelHandle_ = Heart::hResourceHandle(ASSET_PATH);
    Heart::hRenderBuffer* constblock_;
    MaterialConstants initdata = {
        Heart::hVec4(1.f, 1.f, 1.f, 1.f),
        Heart::hVec4(1.f, 1.f, 1.f, 1.f),
        64.f
    };
    renderer->createBuffer(sizeof(MaterialConstants), &initdata, eResourceFlag_ConstantBuffer, 0, &constblock_);
/*
    for (hUint i=0, n=renderModel_->GetLODCount(); i<n; ++i) {
        hGeomLODLevel* lod=renderModel_->GetLOD(i);
        for(hUint ri=0, rn=lod->renderObjects_.GetSize(); ri<rn; ++ri) {
            lod->renderObjects_[ri].GetMaterial()->bindConstanstBuffer(Heart::hCRC32::StringCRC("MaterialConstants"), constblock_);
        }
    }
*/
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

    hShaderResourceViewDesc srvdesc;
    hZeroMem(&srvdesc, sizeof(srvdesc));
    srvdesc.format_=albedo->getTextureFormat();
    srvdesc.resourceType_=albedo->getRenderType();
    srvdesc.tex2D_.topMip_=0;
    srvdesc.tex2D_.mipLevels_=~0;
    renderer->createShaderResourceView(albedo, srvdesc, &albedoSRV_);
    hZeroMem(&srvdesc, sizeof(srvdesc));
    srvdesc.format_=normal->getTextureFormat();
    srvdesc.resourceType_=normal->getRenderType();
    srvdesc.tex2D_.topMip_=0;
    srvdesc.tex2D_.mipLevels_=~0;
    renderer->createShaderResourceView(normal, srvdesc, &normalSRV_);
    hZeroMem(&srvdesc, sizeof(srvdesc));
    srvdesc.format_=spec->getTextureFormat();
    srvdesc.resourceType_=spec->getRenderType();
    srvdesc.tex2D_.topMip_=0;
    srvdesc.tex2D_.mipLevels_=~0;
    renderer->createShaderResourceView(spec, srvdesc, &specSRV_);
    hZeroMem(&srvdesc, sizeof(srvdesc));
    srvdesc.format_=eTextureFormat_R32_float;
    srvdesc.resourceType_=depth->getRenderType();
    srvdesc.tex2D_.topMip_=0;
    srvdesc.tex2D_.mipLevels_=~0;
    renderer->createShaderResourceView(depth, srvdesc, &depthSRV_);

    hLightingManager::hRenderTargetInfo lightInput;
    lightInput.albedo_=albedo;
    lightInput.normal_=normal;
    lightInput.spec_=spec;
    lightInput.depth_=depth;
    lightInput.vertexLightShader_= Heart::hResourceHandle("MATERIALS.DEFERRED_VS");//static_cast<hShaderProgram*>(engine_->GetResourceManager()->getResource("MATERIALS.DEFERRED_VS"));
    lightInput.pixelLightShader_= Heart::hResourceHandle("MATERIALS.DEFERRED_PS");//static_cast<hShaderProgram*>(engine_->GetResourceManager()->getResource("MATERIALS.DEFERRED_PS"));
    lightInput.viewCameraIndex_=0;
    deferredLightManager_.initialise(renderer, &lightInput);
    //deferredLightManager_.addDirectionalLight(Heart::hVec3Func::normalise(hVec3(2.f, -.1f, .2f)), Heart::WHITE);
//     deferredLightManager_.addQuadLight(Heart::hVec3(1200.f, 0.f, 0.f), Heart::hVec3(0.f, 0.f, 125.f), Heart::hVec3(0.f, 425.f, 0.f), Heart::WHITE);
//     deferredLightManager_.addQuadLight(Heart::hVec3(0.f, 25.f, 0.f), Heart::hVec3(25.f, 0.f, 0.f), Heart::hVec3(0.f, 15.f, 0.f), Heart::WHITE);
//     deferredLightManager_.addQuadLight(Heart::hVec3(1200.f, 0.f, 0.f), Heart::hVec3(0.f, 0.f, -75.f), Heart::hVec3(0.f, 25.f, 0.f), Heart::WHITE);
//     deferredLightManager_.addQuadLight(Heart::hVec3(1200.f, 0.f, 0.f), Heart::hVec3(0.f, 0.f, -250.f), Heart::hVec3(0.f, 925.f, 0.f), Heart::WHITE);
//     deferredLightManager_.addQuadLight(Heart::hVec3Func::normalise(Heart::hVec3(120.f, 0.f, 100.f))*20.f, Heart::hVec3Func::normalise(Heart::hVec3(0.f, 50.f, 200.f))*50.f, Heart::hVec3(175.f, 250.f, 0.f), Heart::WHITE);
    //deferredLightManager_.addDirectionalLight(Heart::hVec3Func::normalise(hVec3(0.f, -1.f, 0.f)), Heart::WHITE);
    //deferredLightManager_.addDirectionalLight(Heart::hVec3Func::normalise(hVec3(1.f, 0.f, 0.f)), Heart::WHITE);
    //deferredLightManager_.addDirectionalLight(Heart::hVec3Func::normalise(hVec3(0.f, 0.f, 0.f)), Heart::WHITE);
    deferredLightManager_.enableSphereLight(0, hTrue);
    deferredLightManager_.setSphereLight(0, hVec3(0.f, 100.f, 0.f), 50.f);
    deferredLightManager_.enableSphereLight(1, hTrue);
    deferredLightManager_.setSphereLight(1, hVec3(0.f, 600.f, 0.f), 110.f);
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

    if (albedoSRV_) {
        albedoSRV_->DecRef();
        albedoSRV_=hNullptr;
    }
    if (normalSRV_) {
        normalSRV_->DecRef();
        normalSRV_=hNullptr;
    }
    if (specSRV_) {
        specSRV_->DecRef();
        specSRV_=hNullptr;
    }
    if (depthSRV_) {
        depthSRV_->DecRef();
        depthSRV_=hNullptr;
    }
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void Sibenik::UpdateCamera()
{
    using namespace Heart;

    hRenderer* renderer = engine_->GetRenderer();
    hRendererCamera* camera = renderer->GetRenderCamera(0);

    fpCamera_.update(hClock::delta());
    camera->SetViewMatrix(fpCamera_.getViewmatrix());
}