/********************************************************************

    filename:   DebugPrimsTest.cpp  
    
    Copyright (c) 2:2:2013 James Moran
    
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
#include "DebugPrimsTest.h"
#include "TestUtils.h"

DEFINE_HEART_UNIT_TEST(DebugPrimsTest);

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

hUint32 DebugPrimsTest::RunUnitTest()
{
    Heart::hdGamepad* pad = engine_->GetControllerManager()->GetGamepad(0);
    Heart::hdKeyboard* kb = engine_->GetControllerManager()->GetSystemKeyboard();
    timer_ += Heart::hClock::Delta();
    modelMtx_ = Heart::hMatrixFunc::rotate(timer_, timer_, timer_);

    switch(state_)
    {
    case eBegin: {
            fpCamera_.setInput(pad);
            CreateRenderResources();
            SetCanRender(hTrue);
            state_ = eRender;
        } break;
    case eRender: {
            UpdateCamera();
            if (getForceExitFlag()) {
                state_ = eExit;
            }
        } break;
    case eExit: {
            hcPrintf("End unit test %s.", s_className_);
            DestroyRenderResources();
            SetExitCode(UNIT_TEST_EXIT_CODE_OK);
        } break;
    }

    return 0;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void DebugPrimsTest::RenderUnitTest()
{
    Heart::hRenderer* renderer = engine_->GetRenderer();
    Heart::hRenderSubmissionCtx* ctx=renderer->GetMainSubmissionCtx();
    const Heart::hRenderTechniqueInfo* techinfo = engine_->GetRenderer()->GetMaterialManager()->GetRenderTechniqueInfo("main");
    Heart::hRenderBufferMapInfo mapinfo;
    Heart::hViewportShaderConstants* viewportConsts=NULL;
    Heart::hMatrix model=modelMtx_;

    renderer->beginCameraRender(ctx, 0);

    model=Heart::hMatrixFunc::mult(modelMtx_, Heart::hMatrixFunc::translation(Heart::hVec3(-4.f, 0.f, 0.f)));
    ctx->Map(modelMtxCB_, &mapinfo);
    *(Heart::hMatrix*)mapinfo.ptr = model;
    ctx->Unmap(&mapinfo);

    Heart::hMaterialTechnique* tech = wireConeMat_->getGroup(0)->getTechniqueByMask(techinfo->mask_);
    for (hUint32 pass = 0, passcount = tech->GetPassCount(); pass < passcount; ++pass ) {
        Heart::hMaterialTechniquePass* passptr = tech->GetPass(pass);
        ctx->SetMaterialPass(passptr);
        ctx->DrawPrimitive(coneVB_->getVertexCount()/3, 0);
    }

    model=Heart::hMatrixFunc::mult(Heart::hMatrixFunc::RotationX(HEART_PI/2.f),Heart::hMatrixFunc::translation(Heart::hVec3(-10.f, 0.f, -10.f)));
    ctx->Map(modelMtxCB_, &mapinfo);
    *(Heart::hMatrix*)mapinfo.ptr = model;
    ctx->Unmap(&mapinfo);

    tech = wirePlaneMat_->getGroup(0)->getTechniqueByMask(techinfo->mask_);
    for (hUint32 pass = 0, passcount = tech->GetPassCount(); pass < passcount; ++pass ) {
        Heart::hMaterialTechniquePass* passptr = tech->GetPass(pass);
        ctx->SetMaterialPass(passptr);
        ctx->DrawIndexedPrimitive(planeIB_->GetIndexCount()/3, 0);
    }

    model=Heart::hMatrixFunc::mult(modelMtx_, Heart::hMatrixFunc::translation(Heart::hVec3(-2.5f, 0.f, 0.f)));
    ctx->Map(modelMtxCB_, &mapinfo);
    *(Heart::hMatrix*)mapinfo.ptr = model;
    ctx->Unmap(&mapinfo);

    tech = wireSphereMat_->getGroup(0)->getTechniqueByMask(techinfo->mask_);
    for (hUint32 pass = 0, passcount = tech->GetPassCount(); pass < passcount; ++pass ) {
        Heart::hMaterialTechniquePass* passptr = tech->GetPass(pass);
        ctx->SetMaterialPass(passptr);
        ctx->DrawIndexedPrimitive(sphereIB_->GetIndexCount()/3, 0);
    }

    model=Heart::hMatrixFunc::mult(modelMtx_, Heart::hMatrixFunc::translation(Heart::hVec3(-1.f, 0.f, 0.f)));
    ctx->Map(modelMtxCB_, &mapinfo);
    *(Heart::hMatrix*)mapinfo.ptr = model;
    ctx->Unmap(&mapinfo);

    tech = wireCubeMat_->getGroup(0)->getTechniqueByMask(techinfo->mask_);
    for (hUint32 pass = 0, passcount = tech->GetPassCount(); pass < passcount; ++pass ) {
        Heart::hMaterialTechniquePass* passptr = tech->GetPass(pass);
        ctx->SetMaterialPass(passptr);
        ctx->DrawPrimitive(cubeVB_->getVertexCount()/3, 0);
    }

    model=Heart::hMatrixFunc::mult(modelMtx_, Heart::hMatrixFunc::translation(Heart::hVec3(4.f, 0.f, 0.f)));
    ctx->Map(modelMtxCB_, &mapinfo);
    *(Heart::hMatrix*)mapinfo.ptr = model;
    ctx->Unmap(&mapinfo);

    tech = viewLitConeMat_->getGroup(0)->getTechniqueByMask(techinfo->mask_);
    for (hUint32 pass = 0, passcount = tech->GetPassCount(); pass < passcount; ++pass ) {
        Heart::hMaterialTechniquePass* passptr = tech->GetPass(pass);
        ctx->SetMaterialPass(passptr);
        ctx->DrawPrimitive(coneVB_->getVertexCount()/3, 0);
    }

    model=Heart::hMatrixFunc::mult(modelMtx_, Heart::hMatrixFunc::translation(Heart::hVec3(2.5f, 0.f, 0.f)));
    ctx->Map(modelMtxCB_, &mapinfo);
    *(Heart::hMatrix*)mapinfo.ptr = model;
    ctx->Unmap(&mapinfo);

    tech = viewLitSphereMat_->getGroup(0)->getTechniqueByMask(techinfo->mask_);
    for (hUint32 pass = 0, passcount = tech->GetPassCount(); pass < passcount; ++pass ) {
        Heart::hMaterialTechniquePass* passptr = tech->GetPass(pass);
        ctx->SetMaterialPass(passptr);
        ctx->DrawIndexedPrimitive(sphereIB_->GetIndexCount()/3, 0);
    }

    model=Heart::hMatrixFunc::mult(modelMtx_, Heart::hMatrixFunc::translation(Heart::hVec3(1.f, 0.f, 0.f)));
    ctx->Map(modelMtxCB_, &mapinfo);
    *(Heart::hMatrix*)mapinfo.ptr = model;
    ctx->Unmap(&mapinfo);

    tech = viewLitCubeMat_->getGroup(0)->getTechniqueByMask(techinfo->mask_);
    for (hUint32 pass = 0, passcount = tech->GetPassCount(); pass < passcount; ++pass ) {
        Heart::hMaterialTechniquePass* passptr = tech->GetPass(pass);
        ctx->SetMaterialPass(passptr);
        ctx->DrawPrimitive(cubeVB_->getVertexCount()/3, 0);
    }
}


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void DebugPrimsTest::CreateRenderResources()
{
    using namespace Heart;
    hRenderer* renderer = engine_->GetRenderer();
    hResourceManager* resMgr=engine_->GetResourceManager();
    hRenderMaterialManager* matMgr=renderer->GetMaterialManager();
    hRendererCamera* camera = renderer->GetRenderCamera(0);
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

    fpCamera_.reset(Heart::hVec3(0.f, 1.f, 0.f), Heart::hVec3(0.f, 0.f, 1.f), Heart::hVec3(0.f, 0.f, -7.f));
    Heart::hMatrix vm = fpCamera_.getViewmatrix();

    camera->bindRenderTargetSetup(rtDesc);
    camera->SetFieldOfView(45.f);
    camera->SetProjectionParams(aspect, 0.1f, 1000.f);
    camera->SetViewMatrix(vm);
    camera->setViewport(vp);
    camera->SetTechniquePass(renderer->GetMaterialManager()->GetRenderTechniqueInfo("main"));

    Heart::hRenderUtility::buildDebugCubeMesh(renderer, GetGlobalHeap(), &cubeVB_);
    Heart::hRenderUtility::buildSphereMesh(8, 8, .5f, renderer, GetGlobalHeap(), &sphereIB_, &sphereVB_);
    Heart::hRenderUtility::buildConeMesh(8, .5f, 1.f, renderer, GetGlobalHeap(), &coneVB_);
    Heart::hRenderUtility::buildTessellatedQuadMesh(20.f, 20.f, 20, 20, renderer, GetGlobalHeap(), &planeIB_, &planeVB_);

    wireCubeMat_=matMgr->getWireframeDebug()->createMaterialInstance(0);
    wireSphereMat_=matMgr->getWireframeDebug()->createMaterialInstance(0);
    wireConeMat_=matMgr->getWireframeDebug()->createMaterialInstance(0);
    wirePlaneMat_=matMgr->getWireframeDebug()->createMaterialInstance(0);
    viewLitCubeMat_=matMgr->getDebugViewLit()->createMaterialInstance(0);
    viewLitSphereMat_=matMgr->getDebugViewLit()->createMaterialInstance(0);
    viewLitConeMat_=matMgr->getDebugViewLit()->createMaterialInstance(0);
     
    wireCubeMat_->bindInputStreams(PRIMITIVETYPE_TRILIST, hNullptr, &cubeVB_, 1);
    viewLitCubeMat_->bindInputStreams(PRIMITIVETYPE_TRILIST, hNullptr, &cubeVB_, 1);
    wireSphereMat_->bindInputStreams(PRIMITIVETYPE_TRILIST, sphereIB_, &sphereVB_, 1);
    viewLitSphereMat_->bindInputStreams(PRIMITIVETYPE_TRILIST, sphereIB_, &sphereVB_, 1);
    wireConeMat_->bindInputStreams(PRIMITIVETYPE_TRILIST, hNullptr, &coneVB_, 1);
    viewLitConeMat_->bindInputStreams(PRIMITIVETYPE_TRILIST, hNullptr, &coneVB_, 1);;
    wirePlaneMat_->bindInputStreams(PRIMITIVETYPE_TRILIST, planeIB_, &planeVB_, 1);

    viewportCB_=matMgr->GetGlobalConstantBlock(hCRC32::StringCRC("ViewportConstants"));
    modelMtxCB_=matMgr->GetGlobalConstantBlock(hCRC32::StringCRC("InstanceConstants"));

    timer_ = 0.f;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void DebugPrimsTest::DestroyRenderResources()
{
    using namespace Heart;
    hRenderer* renderer = engine_->GetRenderer();
    hRendererCamera* camera = renderer->GetRenderCamera(0);

    camera->releaseRenderTargetSetup();
    if(cubeVB_) {
        cubeVB_->DecRef();
        cubeVB_=NULL;
    }
    if (wireCubeMat_) {
        hMaterialInstance::destroyMaterialInstance(wireCubeMat_);
        wireCubeMat_=NULL;
    }
    if (viewLitCubeMat_) {
        hMaterialInstance::destroyMaterialInstance(viewLitCubeMat_);
        viewLitCubeMat_=NULL;
    }
    if (sphereIB_) {
        sphereIB_->DecRef();
        sphereIB_=NULL;
    }
    if (sphereVB_) {
        sphereVB_->DecRef();
        sphereVB_=NULL;
    }
    if (wireSphereMat_) {
        hMaterialInstance::destroyMaterialInstance(wireSphereMat_);
        wireSphereMat_=NULL;
    }
    if (viewLitSphereMat_) {
        hMaterialInstance::destroyMaterialInstance(viewLitSphereMat_);
        viewLitSphereMat_=NULL;
    }
    if(coneVB_) {
        coneVB_->DecRef();
        coneVB_=NULL;
    }
    if (wireConeMat_) {
        hMaterialInstance::destroyMaterialInstance(wireConeMat_);
        wireConeMat_=NULL;
    }
    if (viewLitConeMat_) {
        hMaterialInstance::destroyMaterialInstance(viewLitConeMat_);
        viewLitConeMat_=NULL;
    }
    if (wirePlaneMat_) {
        hMaterialInstance::destroyMaterialInstance(wirePlaneMat_);
        wirePlaneMat_=NULL;
    }
    if (planeIB_) {
        planeIB_->DecRef();
        planeIB_=NULL;
    }
    if (planeVB_) {
        planeVB_->DecRef();
        planeVB_=NULL;
    }
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void DebugPrimsTest::UpdateCamera()
{
    using namespace Heart;

    hRenderer* renderer = engine_->GetRenderer();
    hRendererCamera* camera = renderer->GetRenderCamera(0);

    fpCamera_.update(hClock::Delta());
    camera->SetViewMatrix(fpCamera_.getViewmatrix());
}