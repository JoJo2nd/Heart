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
            if (pad->GetButton(HEART_PAD_BACK).buttonVal_ ||
                kb->GetButton(VK_SPACE).raisingEdge_)
            {
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
    Heart::hRenderer* renderer = engine_->GetRenderer();
    Heart::hGeomLODLevel* lod = renderModel_->GetLOD(0);
    hUint32 lodobjects = lod->renderObjects_.GetSize();
    const Heart::hRenderTechniqueInfo* techinfo = engine_->GetRenderer()->GetMaterialManager()->GetRenderTechniqueInfo("main");

    drawCtx_.Begin(renderer);

    for (hUint32 i = 0; i < lodobjects; ++i)
    {
        // Should a renderable simply store a draw call?
        Heart::hRenderable* renderable = &lod->renderObjects_[i];

        hFloat dist=Heart::hVec3Func::lengthFast(camPos_-renderable->GetAABB().c_);
        Heart::hMaterialTechnique* tech = renderable->GetMaterial()->GetTechniqueByMask(techinfo->mask_);
        for (hUint32 pass = 0, passcount = tech->GetPassCount(); pass < passcount; ++pass ) {
            drawCall_.sortKey_ = Heart::hBuildRenderSortKey(0/*cam*/, tech->GetLayer(), tech->GetSort(), dist, renderable->GetMaterialKey(), pass);
            Heart::hMaterialTechniquePass* passptr = tech->GetPass(pass);
            drawCall_.blendState_ = passptr->GetBlendState();
            drawCall_.depthState_ = passptr->GetDepthStencilState();
            drawCall_.rasterState_ = passptr->GetRasterizerState();
            drawCall_.progInput_ = passptr->GetRenderInputObject();
            drawCall_.streams_=*passptr->getRenderStreamsObject();
            drawCall_.drawPrimCount_ = renderable->GetPrimativeCount();
            drawCall_.instanceCount_=0;
            drawCtx_.SubmitDrawCall(drawCall_);
        }
    }
    drawCtx_.End();
}


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void ComplexMesh1::CreateRenderResources()
{
    using namespace Heart;
    hRenderer* renderer = engine_->GetRenderer();
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

    camPos_ = Heart::hVec3(0.f, 10.f, -110.f);
    camDir_ = Heart::hVec3(0.f, 0.f, 1.f);
    camUp_  = Heart::hVec3(0.f, 1.f, 0.f);

    Heart::hMatrix vm = Heart::hMatrixFunc::LookAt(camPos_, camPos_+camDir_, camUp_);

    camera->SetRenderTargetSetup(rtDesc);
    camera->SetFieldOfView(45.f);
    camera->SetProjectionParams( aspect, 0.1f, 1000.f);
    camera->SetViewMatrix(vm);
    camera->SetViewport(vp);
    camera->SetTechniquePass(renderer->GetMaterialManager()->GetRenderTechniqueInfo("main"));

    renderModel_ = static_cast<hRenderModel*>(engine_->GetResourceManager()->mtGetResource(ASSET_PATH));

    hcAssert(renderModel_);
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void ComplexMesh1::DestroyRenderResources()
{
    using namespace Heart;
    hRenderer* renderer = engine_->GetRenderer();
    hRendererCamera* camera = renderer->GetRenderCamera(0);

    camera->ReleaseRenderTargetSetup();
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void ComplexMesh1::UpdateCamera()
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