/********************************************************************

    filename:   ComputeTest.cpp  
    
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
#include "ComputeTest.h"
#include "TestUtils.h"

DEFINE_HEART_UNIT_TEST(ComputeTest);

#define PACKAGE_NAME ("MATERIALS")
#define RESOURCE_NAME ("MATERIALS")
#define ASSET_PATH ("MATERIALS.PERLIN_CS")

#define NOISE_PER       (0.333f)
#define NOISE_OCTAVES   (8)
#define NOISE_SEED      (2013)
#define NOISE_STEP      (0.05f)

#define NOISE_SIZE_X    (8192)
#define NOISE_SIZE_Y    (8192)

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

__declspec(align(16))
struct cbNoise {
    hFloat persistence;
    hUint  octaves;
    hFloat scale;
    hUint  seed;
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

hUint32 ComputeTest::RunUnitTest()
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

void ComputeTest::RenderUnitTest()
{
    Heart::hRenderer* renderer = engine_->GetRenderer();
    Heart::hRenderSubmissionCtx* ctx=renderer->GetMainSubmissionCtx();
    ctx->setComputeInput(&computeParams_);
    ctx->dispatch(32, 32, 1);
}


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void ComputeTest::CreateRenderResources()
{
    using namespace Heart;
    hRenderer* renderer = engine_->GetRenderer();
    hRendererCamera* camera = renderer->GetRenderCamera(0);
    hUint32 w = renderer->GetWidth();
    hUint32 h = renderer->GetHeight();
    hFloat aspect = (hFloat)w/(hFloat)h;
    hRenderViewportTargetSetup rtDesc={0};
    rtDesc.nTargets_=1;
    rtDesc.targets_[0]=renderer->GetMaterialManager()->getGlobalTexture("back_buffer");
    rtDesc.depth_=renderer->GetMaterialManager()->getGlobalTexture("depth_buffer");

    hRelativeViewport vp;
    vp.x=0.f;
    vp.y=0.f;
    vp.w=1.f;
    vp.h=1.f;

    camPos_ = Heart::hVec3(0.f, 1.f, 0.f);
    camDir_ = Heart::hVec3(0.f, 0.f, 1.f);
    camUp_  = Heart::hVec3(0.f, 1.f, 0.f);

    Heart::hMatrix vm = Heart::hMatrixFunc::LookAt(camPos_, camPos_+camDir_, camUp_);
    hMipDesc resTexInit={
        8192, 8192, NULL, 0
    };
    hUint32 cbSizes=sizeof(cbNoise);

    camera->SetRenderTargetSetup(rtDesc);
    camera->SetFieldOfView(45.f);
    camera->SetProjectionParams( aspect, 0.1f, 1000.f);
    camera->SetViewMatrix(vm);
    camera->setViewport(vp);
    camera->SetTechniquePass(renderer->GetMaterialManager()->GetRenderTechniqueInfo("main"));

    computeProg_ = static_cast<hShaderProgram*>(engine_->GetResourceManager()->mtGetResource(ASSET_PATH));
    renderer->createTexture(1, &resTexInit, TFORMAT_R32F, RESOURCEFLAG_UNORDEREDACCESS, GetGlobalHeap(), &resTex_);
    noiseParams_=renderer->CreateConstantBlocks(&cbSizes, NULL, 1);

    computeParams_.bindShaderProgram(computeProg_);
    computeParams_.bindResourceView(hCRC32::StringCRC("g_textureOut"), resTex_);
    computeParams_.bindConstantBuffer(hCRC32::StringCRC("cbNoise"), noiseParams_);

    hRenderSubmissionCtx* ctx=renderer->GetMainSubmissionCtx();
    hConstBlockMapInfo mapinfo;
    ctx->Map(noiseParams_, &mapinfo);
    cbNoise* ncb=(cbNoise*)mapinfo.ptr;
    ncb->persistence = NOISE_PER;
    ncb->octaves = NOISE_OCTAVES;
    ncb->scale = NOISE_STEP;
    ncb->seed = NOISE_SEED;
    ctx->Unmap(&mapinfo);
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void ComputeTest::DestroyRenderResources()
{
    using namespace Heart;
    hRenderer* renderer = engine_->GetRenderer();
    hRendererCamera* camera = renderer->GetRenderCamera(0);

    if (resTex_) {
        renderer->destroyTexture(resTex_);
        resTex_=NULL;
    }
    if (noiseParams_) {
        renderer->DestroyConstantBlocks(noiseParams_, 1);
        noiseParams_=NULL;
    }
    camera->ReleaseRenderTargetSetup();
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void ComputeTest::UpdateCamera()
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
