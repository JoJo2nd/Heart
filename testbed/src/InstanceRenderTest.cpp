/********************************************************************

    filename:   InstanceRenderTest.cpp  
    
    Copyright (c) 27:1:2013 James Moran
    
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
#include "InstanceRenderTest.h"

DEFINE_HEART_UNIT_TEST(InstanceRenderTest);

#define INSTANCE_ROWS   (20)
#define INSTANCE_COLS   (INSTANCE_ROWS)
#define INSTANCE_INC    (50.f)
#define INSTANCE_START  (-(INSTANCE_INC)*(INSTANCE_ROWS/2))
#define INSTANCE_COUNT  (INSTANCE_ROWS*INSTANCE_COLS)

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

hUint32 InstanceRenderTest::RunUnitTest()
{
    Heart::hdGamepad* pad = engine_->GetControllerManager()->GetGamepad(0);
    Heart::hdKeyboard* kb = engine_->GetControllerManager()->GetSystemKeyboard();
    switch(state_)
    {
    case eBeginLoad:
        {
            hcPrintf("Loading package \"INSTANCETEST\"");
            engine_->GetResourceManager()->mtLoadPackage("INSTANCETEST");
            state_ = eLoading;
        }
        break;
    case eLoading:
        {
            if (engine_->GetResourceManager()->mtIsPackageLoaded("INSTANCETEST"))
            {
                hcPrintf("Loaded package \"INSTANCETEST\"");
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
            engine_->GetResourceManager()->mtUnloadPackage("INSTANCETEST");
            hcPrintf("Unloading package \"INSTANCETEST\"");
            state_ = eExit;
        }
        break;
    case eExit:
        {
            hcPrintf("End unit test %s package load test.", s_className_);
            SetExitCode(UNIT_TEST_EXIT_CODE_OK);
        }
        break;
    }

    return 0;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void InstanceRenderTest::RenderUnitTest()
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

        drawCall_.sortKey_ = Heart::hBuildRenderSortKey(0/*cam*/, 0/*layer*/, hFalse, 10.f, renderable->GetMaterialKey(), 0);

        Heart::hMaterialTechnique* tech = renderable->GetMaterial()->GetTechniqueByMask(techinfo->mask_);
        for (hUint32 pass = 0, passcount = tech->GetPassCount(); pass < passcount; ++pass ) {
            Heart::hMaterialTechniquePass* passptr = tech->GetPass(pass);
            drawCall_.blendState_ = passptr->GetBlendState();
            drawCall_.depthState_ = passptr->GetDepthStencilState();
            drawCall_.rasterState_ = passptr->GetRasterizerState();
            drawCall_.progInput_ = *passptr->GetRenderInputObject();
            drawCall_.streams_=*passptr->getRenderStreamsObject();
            drawCall_.drawPrimCount_ = renderable->GetPrimativeCount();
            drawCall_.instanceCount_=INSTANCE_COUNT;
            drawCtx_.SubmitDrawCall(drawCall_);
        }
    }
    drawCtx_.End();
}


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void InstanceRenderTest::CreateRenderResources()
{
    using namespace Heart;
#pragma pack(push, 1)
    struct PostionVec {
        hFloat x,y,z;
    };
#pragma pack(pop)

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

    camPos_ = Heart::hVec3(0.f, 120.f, -650.f);
    camDir_ = Heart::hVec3(.022f, -.22f, 1.f);
    camUp_  = Heart::hVec3(0.f, 1.9f, .45f);

    Heart::hMatrix vm = Heart::hMatrixFunc::LookAt(camPos_, camPos_+camDir_, camUp_);

    camera->SetRenderTargetSetup(rtDesc);
    camera->SetFieldOfView(45.f);
    camera->SetProjectionParams( aspect, 0.1f, 2000.f);
    camera->SetViewMatrix(vm);
    camera->SetViewport(vp);
    camera->SetTechniquePass(renderer->GetMaterialManager()->GetRenderTechniqueInfo("main"));

    renderModel_ = static_cast<hRenderModel*>(engine_->GetResourceManager()->mtGetResource("INSTANCETEST.BOCO"));
    hcAssert(renderModel_);

    PostionVec positions[INSTANCE_COUNT];
    Heart::hInputLayoutDesc instLayout[] ={
        {eIS_INSTANCE, 0, eIF_FLOAT3, 1, 1},
    };

    hFloat gridz=INSTANCE_START;
    for (hUint i=0, idx=0; i<INSTANCE_ROWS; ++i, gridz+=INSTANCE_INC) {
        hFloat gridx=INSTANCE_START;
        for (hUint j=0; j<INSTANCE_COLS; ++j, gridx+=INSTANCE_INC, ++idx) {
            positions[idx].x=gridx;
            positions[idx].y=0.f;
            positions[idx].z=gridz;
        }
    }

    renderer->CreateVertexBuffer(
        positions, INSTANCE_COUNT, instLayout, hStaticArraySize(instLayout), 
        Heart::RESOURCEFLAG_DYNAMIC, GetGlobalHeap(), &instanceStream_);


    renderModel_->bindVertexStream(1, instanceStream_);
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void InstanceRenderTest::DestroyRenderResources()
{
    using namespace Heart;
    hRenderer* renderer = engine_->GetRenderer();
    hRendererCamera* camera = renderer->GetRenderCamera(0);

    camera->ReleaseRenderTargetSetup();
    renderer->DestroyVertexBuffer(instanceStream_);
    instanceStream_=NULL;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void InstanceRenderTest::UpdateCamera()
{
    using namespace Heart;
    using namespace Heart::hVec3Func;

    hFloat delta = hClock::Delta();
    hdGamepad* pad = engine_->GetControllerManager()->GetGamepad(0);
    hRenderer* renderer = engine_->GetRenderer();
    hRendererCamera* camera = renderer->GetRenderCamera(0);
    hVec3 camRight = cross(camUp_, camDir_);
    hVec3 movement, angleXZ, angleYZ;
    hFloat speed = 5.f, angleSpeed = .314f;

    camRight = normaliseFast(camRight);

    speed -= pad->GetAxis(HEART_PAD_LEFT_TRIGGER).anologueVal_*5.f;
    speed += pad->GetAxis(HEART_PAD_RIGHT_TRIGGER).anologueVal_*10.f;
    speed *= delta;

    angleSpeed -= pad->GetAxis(HEART_PAD_LEFT_TRIGGER).anologueVal_*.628f;
    angleSpeed += pad->GetAxis(HEART_PAD_RIGHT_TRIGGER).anologueVal_*1.256f;
    angleSpeed *= delta;

    movement =  scale(camRight, pad->GetAxis(HEART_PAD_LEFT_STICKX).anologueVal_*speed);
    movement += scale(camDir_, pad->GetAxis(HEART_PAD_LEFT_STICKY).anologueVal_*speed);

    movement += (pad->GetButton(HEART_PAD_DPAD_UP).buttonVal_ ? scale(camUp_,speed) : hVec3Func::zeroVector());
    movement += (pad->GetButton(HEART_PAD_DPAD_DOWN).buttonVal_ ? scale(camUp_,-speed) : hVec3Func::zeroVector());

    angleXZ = hMatrixFunc::mult(camDir_, hMatrixFunc::rotate(angleSpeed*pad->GetAxis(HEART_PAD_RIGHT_STICKX).anologueVal_, camUp_));
    angleYZ = hMatrixFunc::mult(camDir_, hMatrixFunc::rotate(angleSpeed*pad->GetAxis(HEART_PAD_RIGHT_STICKY).anologueVal_, camRight));

    camPos_ += movement;
    camDir_ = angleXZ + angleYZ;
    camUp_  = cross(camDir_, camRight);

    camDir_ = normaliseFast(camDir_);
    camUp_  = normaliseFast(camUp_);

    Heart::hMatrix vm = Heart::hMatrixFunc::LookAt(camPos_, camPos_+camDir_, camUp_);
    camera->SetViewMatrix(vm);
}