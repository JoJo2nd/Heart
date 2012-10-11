/********************************************************************

	filename: 	ModelRenderTest.cpp	
	
	Copyright (c) 10:10:2012 James Moran
	
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

#include "ModelRenderTest.h"
#include "Gwen/UnitTest/UnitTest.h"

DEFINE_HEART_UNIT_TEST(ModelRenderTest);

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

hUint32 ModelRenderTest::RunUnitTest()
{
    switch(state_)
    {
    case eBeginLoad:
        {
            hcPrintf("Loading package \"UNITTEST\"");
            engine_->GetResourceManager()->mtLoadPackage("UNITTEST");
            state_ = eLoading;
        }
        break;
    case eLoading:
        {
            if (engine_->GetResourceManager()->mtIsPackageLoaded("UNITTEST"))
            {
                hcPrintf("Loaded package \"UNITTEST\"");
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
            if (timer_ > 30.f)
            {
                state_ = eBeginUnload;
            }
        }
        break;
    case eBeginUnload:
        {
            SetCanRender(hFalse);
            DestroyRenderResources();
            engine_->GetResourceManager()->mtUnloadPackage("UNITTEST");
            hcPrintf("Unloading package \"UNITTEST\"");
            state_ = eExit;
        }
        break;
    case eExit:
        {
            hcPrintf("End unit test resource package load test.");
            SetExitCode(UNIT_TEST_EXIT_CODE_OK);
        }
        break;
    }

    return 0;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void ModelRenderTest::RenderUnitTest()
{
    Heart::hRenderer* renderer = engine_->GetRenderer();
    Heart::hGeomLODLevel* lod = renderModel_->GetLODLevel(0.f);
    hUint32 lodobjects = lod->renderObjects_.GetSize();

    drawCtx_.Begin(renderer);

    for (hUint32 i = 0; i < lodobjects; ++i)
    {
        // Should a renderable simply store a draw call?
        Heart::hRenderable* renderable = &lod->renderObjects_[i];
        drawCall_.sortKey_ = Heart::hBuildRenderSortKey(0/*cam*/, 0/*layer*/, hFalse, 10.f, renderable->GetMaterialInstance()->GetMatKey(), 0);
        drawCall_.indexBuffer_ = renderable->GetIndexBuffer();
        for (hUint32 vs = 0; vs < Heart::hDrawCall::MAX_VERT_STREAMS; ++vs)
        {
            drawCall_.vertexBuffer_[vs] = renderable->GetVertexBuffer(vs);
        }
        drawCall_.matInstance_ = renderable->GetMaterialInstance();
        drawCall_.primType_ = renderable->GetPrimativeType();
        drawCall_.startVertex_ = renderable->GetStartIndex();
        drawCall_.primCount_ = renderable->GetPrimativeCount();

        drawCtx_.SubmitDrawCall(drawCall_);
    }
    drawCtx_.End();
}


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void ModelRenderTest::CreateRenderResources()
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

    camera->SetRenderTargetSetup(rtDesc);
    camera->SetFieldOfView(45.f);
    camera->SetProjectionParams( aspect, 0.1f, 100.f);
    camera->SetViewMatrix(Heart::hMatrixFunc::identity());
    camera->SetViewport(vp);
    camera->SetTechniquePass(renderer->GetMaterialManager()->GetRenderTechniqueInfo("main"));

    renderModel_ = static_cast<hRenderModel*>(engine_->GetResourceManager()->mtGetResource("UNITTEST.BOCO"));

    hcAssert(renderModel_);
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void ModelRenderTest::DestroyRenderResources()
{
    using namespace Heart;
    hRenderer* renderer = engine_->GetRenderer();
    hRendererCamera* camera = renderer->GetRenderCamera(0);

    camera->ReleaseRenderTargetSetup();
}
