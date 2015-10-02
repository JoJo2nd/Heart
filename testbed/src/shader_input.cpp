/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#include "base/hTypes.h"
#include "base/hBase64.h"
#include "base/hClock.h"
#include "base/hStringUtil.h"
#include "render/hRenderCallDesc.h"
#include "render/hRenderer.h"
#include "render/hVertexBufferLayout.h"
#include "render/hRenderPrim.h"
#include "render/hUniformBufferFlags.h"
#include "render/hProgramReflectionInfo.h"
#include "render/hRenderShaderProgram.h"
#include "UnitTestFactory.h"


namespace Heart {
class hHeartEngine;
}

using namespace Heart;

class ShaderInput : public IUnitTest {
    
	hTimer								timer_;
    hShaderProgram*                     shaderProg;
	hRenderer::hShaderStage*			vert;
    hRenderer::hShaderStage*			frag;
	//hRenderer::hProgramReflectionInfo*	refInfo;
    hRenderer::hVertexBuffer*			vb;
    hRenderer::hRenderCall*				rc;
	hRenderer::hUniformBuffer*			ub;
    hRenderer::hUniformBuffer*			ub2;
    hRenderer::hUniformBuffer*			ub3;

	static const hUint FENCE_COUNT = 3;
	hUint								paramBlockSize;
	hRenderer::hRenderFence*			fences[FENCE_COUNT];
	hUint								currentFence;

public:
    ShaderInput( Heart::hHeartEngine* engine ) 
        : IUnitTest( engine ) {

        hFloat verts[] = {
            //pos            //colour
             0.f,  .5f, 0.f, /**/ 1.f, 0.f, 0.f, 1.f,
             .5f, -.5f, 0.f, /**/ 0.f, 1.f, 0.f, 1.f,
            -.5f, -.5f, 0.f, /**/ 0.f, 0.f, 1.f, 1.f,
        };
        hRenderer::hVertexBufferLayout lo[] = {
            {hStringID("in_position"), 3, hRenderer::hVertexInputType::Float,				 0, hFalse, sizeof(hFloat)*9},
            {hStringID("in_colour"),   4, hRenderer::hVertexInputType::Float, sizeof(hFloat)*3, hFalse, sizeof(hFloat)*9},
            {hStringID("in_uv"),       2, hRenderer::hVertexInputType::Float, sizeof(hFloat)*7, hFalse, sizeof(hFloat)*9},
        };

        shaderProg = hResourceManager::weakResource<hShaderProgram>(hStringID("/system/shader_input"));

        vert = shaderProg->getShader(hRenderer::getActiveProfile(hShaderFrequency::Vertex));
        frag = shaderProg->getShader(hRenderer::getActiveProfile(hShaderFrequency::Pixel));
        vb   = hRenderer::createVertexBuffer(verts, sizeof(hFloat)*7, 3, 0);

        hRenderer::hUniformLayoutDesc ublo[] = {
            {"timeSecs",    hRenderer::ShaderParamType::Float,   0},
            {"pos_shift",   hRenderer::ShaderParamType::Float3,  4},
            {"uv_shift",    hRenderer::ShaderParamType::Float2, 16},
            {"alpha",       hRenderer::ShaderParamType::Float,  24},
            {"colour_tint", hRenderer::ShaderParamType::Float3, 28},
        };
		ub = hRenderer::createUniformBuffer(nullptr, ublo, (hUint)hStaticArraySize(ublo), 40, 3, (hUint32)hRenderer::hUniformBufferFlags::Dynamic);
        hRenderer::hUniformLayoutDesc ublo2[] = {
            { "g_View", hRenderer::ShaderParamType::Float44, 0 },
            { "g_ViewInverse", hRenderer::ShaderParamType::Float44, 64},
            { "g_ViewInverseTranspose", hRenderer::ShaderParamType::Float44, 128},
            { "g_Projection", hRenderer::ShaderParamType::Float44, 192},
            { "g_ProjectionInverse", hRenderer::ShaderParamType::Float44, 256},
            { "g_ViewProjection", hRenderer::ShaderParamType::Float44, 320},
            { "g_ViewProjectionInverse", hRenderer::ShaderParamType::Float44, 384},
            { "g_viewportSize", hRenderer::ShaderParamType::Float4, 448},
        };
        ub2 = hRenderer::createUniformBuffer(nullptr, ublo2, (hUint)hStaticArraySize(ublo2), 454, 3, (hUint32)hRenderer::hUniformBufferFlags::Dynamic);
        hRenderer::hUniformLayoutDesc ublo3[] = {
            { "g_World", hRenderer::ShaderParamType::Float44, 0 },
        };
        ub3 = hRenderer::createUniformBuffer(nullptr, ublo3, (hUint)hStaticArraySize(ublo3), 64, 3, (hUint32)hRenderer::hUniformBufferFlags::Dynamic);
		for (auto& i:fences) {
			i = nullptr;
		}
		currentFence = 0;


        hRenderer::hRenderCallDesc rcd;
        rcd.vertex_ = vert;
        rcd.fragment_ = frag;
        rcd.vertexBuffer_ = vb;
		rcd.setUniformBuffer(hStringID("ParamBlock"), ub);
        rcd.setUniformBuffer(hStringID("ViewportConstants"), ub2);
        rcd.setUniformBuffer(hStringID("InstanceConstants"), ub3);
        rcd.setVertexBufferLayout(lo, (hUint)hStaticArraySize(lo));
        rc = hRenderer::createRenderCall(rcd);

		timer_.reset();
		timer_.setPause(hFalse);

        SetCanRender(hTrue);
    }
    ~ShaderInput() {
        hRenderer::destroyRenderCall(rc);
        hRenderer::destroyUniformBuffer(ub);
        //hRenderer::destroyProgramReflectionInfo(refInfo);
        hRenderer::destroyVertexBuffer(vb);
    }

    virtual hUint32 RunUnitTest() override {

        if (timer_.elapsedMilliSec() > 10*1000 || getForceExitFlag()) {
            SetExitCode(UNIT_TEST_EXIT_CODE_OK);
        }

        return 0;
    }

    Heart::hRenderer::hCmdList* RenderUnitTest() override {
        auto* cl = hRenderer::createCmdList();
        hRenderer::clear(cl, hColour(0.f, 0.f, 0.f, 1.f), 1.f);
// 		if (fences[currentFence]) {
// 			hRenderer::wait(fences[currentFence]);
// 			fences[currentFence] = nullptr;
// 		}
// 		auto* ubdata = (TimerBlock*) (((hByte*)hRenderer::getMappingPtr(ub)) + (currentFence*paramBlockSize));
// 		ubdata->timeSecs = timer_.elapsedMilliSec()/1000.f;
// 		hRenderer::flushUnibufferMemoryRange(cl, ub, (currentFence*paramBlockSize), paramBlockSize);
//         hRenderer::draw(cl, rc, hRenderer::Primative::Triangles, 1);
// 		fences[currentFence] = hRenderer::fence(cl);
// 		currentFence = (currentFence+1)%FENCE_COUNT;
        return cl;
        //hRenderer::swapBuffers(cl);
        //hRenderer::submitFrame(cl);
    }
};

DEFINE_HEART_UNIT_TEST(ShaderInput);
