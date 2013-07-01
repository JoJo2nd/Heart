/********************************************************************

    filename:   ComplexMesh1.cpp  
    
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
#include "ComplexMesh1.h"
#include "TestUtils.h"

DEFINE_HEART_UNIT_TEST(ComplexMesh1);

#define PACKAGE_NAME ("COMPLEXMESH1")
#define RESOURCE_NAME ("LOSTEMPIRE")
#define ASSET_PATH ("COMPLEXMESH1.LOSTEMPIRE")

#define DO_Z_PRE_PASS 

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

hUint32 ComplexMesh1::RunUnitTest()
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

void ComplexMesh1::RenderUnitTest()
{
    using namespace Heart;

    hRenderSubmissionCtx* ctx = engine_->GetRenderer()->GetMainSubmissionCtx();
    hGeomLODLevel* lod = renderModel_->GetLOD(0);
    hUint32 lodobjects = lod->renderObjects_.GetSize();

    const hRenderTechniqueInfo* techinfo = zPassCamera_.getTechniquePass();
    nDrawCalls_=0;
#ifdef DO_Z_PRE_PASS
    for (hUint32 i = 0; i < lodobjects && nDrawCalls_ < MAX_DCS; ++i)
    {
        // Should a renderable simply store a draw call?
        hRenderable* renderable = &lod->renderObjects_[i];

        hFloat dist=hVec3Func::lengthFast(fpCamera_.getCameraPosition()-renderable->GetAABB().c_);
        hMaterialTechnique* tech = renderable->GetMaterial()->getGroup(0)->getTechniqueByMask(techinfo->mask_);
        if (!tech) {
            continue;
        }
        for (hUint32 pass = 0, passcount = tech->GetPassCount(); pass < passcount; ++pass ) {
            drawCall_[nDrawCalls_].sortKey_ = hBuildRenderSortKey(0/*cam*/, tech->GetLayer(), tech->GetSort(), dist, renderable->GetMaterialKey(), pass);
            hMaterialTechniquePass* passptr = tech->GetPass(pass);
            drawCall_[nDrawCalls_].blendState_ = passptr->GetBlendState();
            drawCall_[nDrawCalls_].depthState_ = passptr->GetDepthStencilState();
            drawCall_[nDrawCalls_].rasterState_ = passptr->GetRasterizerState();
            drawCall_[nDrawCalls_].progInput_ = passptr->GetRenderInputObject();
            drawCall_[nDrawCalls_].streams_=*passptr->getRenderStreamsObject();
            drawCall_[nDrawCalls_].drawPrimCount_ = renderable->GetPrimativeCount();
            drawCall_[nDrawCalls_].instanceCount_=0;
            ++nDrawCalls_;
            hcAssert(nDrawCalls_ < MAX_DCS);
            if (nDrawCalls_ >= MAX_DCS) {
                break;
            }
        }
    }

    hRenderUtility::sortDrawCalls(drawCall_, nDrawCalls_);
    hRenderUtility::submitDrawCalls(ctx, &zPassCamera_, drawCall_, nDrawCalls_, eClearTarget_Depth);
#endif
    techinfo = camera_.getTechniquePass();
    nDrawCalls_=0;

    for (hUint32 i = 0; i < lodobjects && nDrawCalls_ < MAX_DCS; ++i)
    {
        // Should a renderable simply store a draw call?
        hRenderable* renderable = &lod->renderObjects_[i];

        hFloat dist=hVec3Func::lengthFast(fpCamera_.getCameraPosition()-renderable->GetAABB().c_);
        hMaterialTechnique* tech = renderable->GetMaterial()->getGroup(0)->getTechniqueByMask(techinfo->mask_);
        if (!tech) {
            continue;
        }
        for (hUint32 pass = 0, passcount = tech->GetPassCount(); pass < passcount; ++pass ) {
            drawCall_[nDrawCalls_].sortKey_ = hBuildRenderSortKey(0/*cam*/, tech->GetLayer(), tech->GetSort(), dist, renderable->GetMaterialKey(), pass);
            hMaterialTechniquePass* passptr = tech->GetPass(pass);
            drawCall_[nDrawCalls_].blendState_ = passptr->GetBlendState();
            drawCall_[nDrawCalls_].depthState_ = passptr->GetDepthStencilState();
            drawCall_[nDrawCalls_].rasterState_ = passptr->GetRasterizerState();
            drawCall_[nDrawCalls_].progInput_ = passptr->GetRenderInputObject();
            drawCall_[nDrawCalls_].streams_=*passptr->getRenderStreamsObject();
            drawCall_[nDrawCalls_].drawPrimCount_ = renderable->GetPrimativeCount();
            drawCall_[nDrawCalls_].instanceCount_=0;
            ++nDrawCalls_;
            hcAssert(nDrawCalls_ < MAX_DCS);
            if (nDrawCalls_ >= MAX_DCS) {
                break;
            }
        }
    }
    
    hRenderUtility::sortDrawCalls(drawCall_, nDrawCalls_);
#ifdef DO_Z_PRE_PASS
    hRenderUtility::submitDrawCalls(ctx, &camera_, drawCall_, nDrawCalls_, eClearTarget_Colour);
#else
    hRenderUtility::submitDrawCalls(ctx, &camera_, drawCall_, nDrawCalls_, eClearTarget_Colour|eClearTarget_Depth);
#endif
}


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void ComplexMesh1::CreateRenderResources()
{
    using namespace Heart;
    hRenderer* renderer = engine_->GetRenderer();
    hRenderMaterialManager* matMgr=renderer->GetMaterialManager();
    hUint32 w = renderer->GetWidth();
    hUint32 h = renderer->GetHeight();
    hFloat aspect = (hFloat)w/(hFloat)h;
    hRenderViewportTargetSetup rtDesc={0};
    hTexture* bb=matMgr->getGlobalTexture("back_buffer");
#ifdef DO_Z_PRE_PASS
    hTexture* db=matMgr->getGlobalTexture("z_pre_pass");
    hTextureFormat dfmt=TFORMAT_D32F;
#else
    hTexture* db=matMgr->getGlobalTexture("depth_buffer");
    hTextureFormat dfmt=TFORMAT_D24S8F;
#endif
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
    fpCamera_.reset(Heart::hVec3(0.f, 1.f, 0.f), Heart::hVec3(0.f, 0.f, 1.f), Heart::hVec3(0.f, 40.f, -60.f));
    Heart::hMatrix vm = fpCamera_.getViewmatrix();
    camera_.Initialise(renderer);
    camera_.bindRenderTargetSetup(rtDesc);
    camera_.SetFieldOfView(45.f);
    camera_.SetProjectionParams( aspect, 0.1f, 1000.f);
    camera_.SetViewMatrix(vm);
    camera_.setViewport(vp);
#ifdef DO_Z_PRE_PASS
    camera_.SetTechniquePass(renderer->GetMaterialManager()->GetRenderTechniqueInfo("postzmain"));
#else
    camera_.SetTechniquePass(renderer->GetMaterialManager()->GetRenderTechniqueInfo("main"));
#endif

    rtDesc.nTargets_=0;
    rtDesc.depth_=dsv;
    zPassCamera_.Initialise(renderer);
    zPassCamera_.bindRenderTargetSetup(rtDesc);
    zPassCamera_.SetFieldOfView(45.f);
    zPassCamera_.SetProjectionParams( aspect, 0.1f, 1000.f);
    zPassCamera_.SetViewMatrix(vm);
    zPassCamera_.setViewport(vp);
    zPassCamera_.SetTechniquePass(renderer->GetMaterialManager()->GetRenderTechniqueInfo("zprepass"));

    renderModel_ = static_cast<hRenderModel*>(engine_->GetResourceManager()->mtGetResource(ASSET_PATH));

    // The camera hold refs to this
    rtv->DecRef();
    dsv->DecRef();

    hcAssert(renderModel_);
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void ComplexMesh1::DestroyRenderResources()
{
    using namespace Heart;

    camera_.releaseRenderTargetSetup();
    zPassCamera_.releaseRenderTargetSetup();
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void ComplexMesh1::UpdateCamera()
{
    using namespace Heart;

    fpCamera_.update(hClock::Delta());
    camera_.SetViewMatrix(fpCamera_.getViewmatrix());
    zPassCamera_.SetViewMatrix(fpCamera_.getViewmatrix());
}