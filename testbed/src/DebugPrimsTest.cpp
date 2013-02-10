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

#define PACKAGE_NAME ("CORE")
#define CUBE_MAT_RESID ("CORE.DEBUGMAT")

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
            CreateRenderResources();
            SetCanRender(hTrue);
            state_ = eRender;
        } break;
    case eRender: {
            UpdateCamera();
            if (pad->GetButton(HEART_PAD_BACK).buttonVal_ ||
                kb->GetButton(VK_SPACE).raisingEdge_)
            {
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
    Heart::hConstBlockMapInfo mapinfo;
    Heart::hViewportShaderConstants* viewportConsts=NULL;
    Heart::hMatrix model=modelMtx_;

    renderer->beginCameraRender(ctx, 0);

    model=Heart::hMatrixFunc::mult(modelMtx_, Heart::hMatrixFunc::translation(Heart::hVec3(-1.f, 0.f, 0.f)));
    ctx->Map(modelMtxCB_, &mapinfo);
    *(Heart::hMatrix*)mapinfo.ptr = model;
    ctx->Unmap(&mapinfo);

    Heart::hMaterialTechnique* tech = wireCubeMat_->GetTechniqueByMask(techinfo->mask_);
    for (hUint32 pass = 0, passcount = tech->GetPassCount(); pass < passcount; ++pass ) {
        Heart::hMaterialTechniquePass* passptr = tech->GetPass(pass);
        ctx->SetMaterialPass(passptr);
        ctx->DrawPrimitive(cubeVB_->getVertexCount()/3, 0);
    }

    model=Heart::hMatrixFunc::mult(modelMtx_, Heart::hMatrixFunc::translation(Heart::hVec3(1.f, 0.f, 0.f)));
    ctx->Map(modelMtxCB_, &mapinfo);
    *(Heart::hMatrix*)mapinfo.ptr = model;
    ctx->Unmap(&mapinfo);

    tech = viewLitCubeMat_->GetTechniqueByMask(techinfo->mask_);
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
    hRenderViewportTargetSetup rtDesc;
    rtDesc.nTargets_ = 0;
    rtDesc.width_ = w;
    rtDesc.height_ = h;
    rtDesc.targetFormat_ = Heart::TFORMAT_ARGB8_sRGB;
    rtDesc.hasDepthStencil_ = hFalse;
    rtDesc.depthFormat_ = Heart::TFORMAT_D24S8F;

    hViewport vp;
    vp.x_ = 0;
    vp.y_ = 0;
    vp.width_ = w;
    vp.height_ = h;

    camPos_ = Heart::hVec3(0.f, 0.f, -5.f);
    camDir_ = Heart::hVec3(0.f, 0.f, 1.f);
    camUp_  = Heart::hVec3(0.f, 1.f, 0.f);

    Heart::hMatrix vm = Heart::hMatrixFunc::LookAt(camPos_, camPos_+camDir_, camUp_);

    camera->SetRenderTargetSetup(rtDesc);
    camera->SetFieldOfView(45.f);
    camera->SetProjectionParams(aspect, 0.1f, 1000.f);
    camera->SetViewMatrix(vm);
    camera->SetViewport(vp);
    camera->SetTechniquePass(renderer->GetMaterialManager()->GetRenderTechniqueInfo("main"));

    Heart::hRenderUtility::buildDebugCubeMesh(renderer, GetGlobalHeap(), &cubeVB_);
    wireCubeMat_=matMgr->getWireframeDebug()->createMaterialInstance(0);
    viewLitCubeMat_=matMgr->getDebugViewLit()->createMaterialInstance(0);
     
    wireCubeMat_->bindVertexStream(0, cubeVB_);
    viewLitCubeMat_->bindVertexStream(0, cubeVB_);

    viewportCB_=matMgr->GetGlobalConstantBlock(hCRC32::StringCRC("ViewportConstants"));
    modelMtxCB_=matMgr->GetGlobalConstantBlock(hCRC32::StringCRC("InstanceConstants"));
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void DebugPrimsTest::DestroyRenderResources()
{
    using namespace Heart;
    hRenderer* renderer = engine_->GetRenderer();
    hRendererCamera* camera = renderer->GetRenderCamera(0);

    camera->ReleaseRenderTargetSetup();
    if(cubeVB_) {
        renderer->DestroyVertexBuffer(cubeVB_);
        cubeVB_=NULL;
    }
    if (wireCubeMat_) {
        hMaterialInstance::destroyMaterialInstance(wireCubeMat_);
        wireCubeMat_=NULL;
    }
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void DebugPrimsTest::UpdateCamera()
{
    using namespace Heart;
    using namespace Heart::hVec3Func;

    hRenderer* renderer = engine_->GetRenderer();
    hRendererCamera* camera = renderer->GetRenderCamera(0);
    hdGamepad* pad = engine_->GetControllerManager()->GetGamepad(0);

    updateCameraFirstPerson(hClock::Delta(), *pad, &camUp_, &camDir_, &camPos_);
    Heart::hMatrix vm = Heart::hMatrixFunc::LookAt(camPos_, camPos_+camDir_, camUp_);
    camera->SetViewMatrix(vm);
}