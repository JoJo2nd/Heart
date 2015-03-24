/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#include "base/hTypes.h"
#include "utils/hBase64.h"
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
	hRenderer::hProgramReflectionInfo*	refInfo;
    hRenderer::hVertexBuffer*			vb;
    hRenderer::hRenderCall*				rc;
	hRenderer::hUniformBuffer*			ub;

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
            {hStringID("in_position"), 3, hRenderer::hVertexInputType::Float,				 0, hFalse, sizeof(hFloat)*7},
            {hStringID("in_colour"),   4, hRenderer::hVertexInputType::Float, sizeof(hFloat)*3, hFalse, sizeof(hFloat)*7},
        };

        shaderProg = hResourceManager::weakResource<hShaderProgram>(hStringID("/system/shader_input"));

        vert = shaderProg->getShader(hShaderProfile::ES2_vs);//hRenderer::compileShaderStageFromSource(vertSrc, hStrLen(vertSrc), "main", hShaderProfile::ES2_vs);
        frag = shaderProg->getShader(hShaderProfile::ES2_ps);//hRenderer::compileShaderStageFromSource(fragSrc, hStrLen(fragSrc), "main", hShaderProfile::ES2_ps);
        vb   = hRenderer::createVertexBuffer(verts, sizeof(hFloat)*7, 3, 0);

// 		refInfo = hRenderer::createProgramReflectionInfo(vert, frag, nullptr, nullptr, nullptr);
// 		for (hUint i=0, n=hRenderer::getUniformatBlockCount(refInfo); i<n; ++i) {
// 			auto param_info = hRenderer::getUniformatBlockInfo(refInfo, i);
// 			if (hStrCmp(param_info.name,"TimerBlock") == 0) {
// 				paramBlockSize = param_info.dynamicSize;
// 			}
// 		}
		//ub = hRenderer::createUniformBuffer(nullptr, paramBlockSize*3, (hUint32)hRenderer::hUniformBufferFlags::Dynamic);
		for (auto& i:fences) {
			i = nullptr;
		}
		currentFence = 0;


        hRenderer::hRenderCallDesc rcd;
        rcd.vertex_ = vert;
        rcd.fragment_ = frag;
        rcd.vertexBuffer_ = vb;
		//rcd.setUniformBuffer(hStringID("TimerBlock"), ub);
        rcd.setVertexBufferLayout(lo, 2);
        rc = hRenderer::createRenderCall(rcd);

		timer_.reset();
		timer_.setPause(hFalse);

        SetCanRender(hTrue);
    }
    ~ShaderInput() {
        hRenderer::destroyRenderCall(rc);
        hRenderer::destroyUniformBuffer(ub);
        hRenderer::destroyProgramReflectionInfo(refInfo);
        hRenderer::destroyVertexBuffer(vb);
    }

    virtual hUint32 RunUnitTest() override {

        if (timer_.elapsedMilliSec() > 10*1000 || getForceExitFlag()) {
            SetExitCode(UNIT_TEST_EXIT_CODE_OK);
        }

        return 0;
    }

    void RenderUnitTest() override {
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
        hRenderer::swapBuffers(cl);
        
        hRenderer::submitFrame(cl);
    }
};

DEFINE_HEART_UNIT_TEST(ShaderInput);