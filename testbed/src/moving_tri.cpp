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
#include "UnitTestFactory.h"

struct TimerBlock { 
	float timeSecs;
	float pad[3];
};

namespace Heart {
class hHeartEngine;
}

using namespace Heart;

const hChar vertSrc[] = 
"\
#version 330\n\
layout(std140) uniform TimerBlock {\n\
	float timeSecs;\n\
	vec3  pad;\n\
};\n\
layout(location=0) in vec3 in_position;\n\
layout(location=1) in vec4 in_colour;\n\
out vec4 inout_colour;\n\
void main() {\n\
    inout_colour = in_colour;\n\
		gl_Position.xyz = in_position.xyz*sin(timeSecs);\n\
    gl_Position.w = 1;\n\
}\n\
";
const hChar fragSrc[] = 
"\
#version 330\n\
in vec4 inout_colour;\n\
out vec4 fragColour;\n\
void main() {\n\
    fragColour = inout_colour;\n\
}\n\
";

class MovingTri : public IUnitTest {
    
	hTimer								timer_;
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
    MovingTri( Heart::hHeartEngine* engine ) 
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

        vert = hRenderer::compileShaderStageFromSource(vertSrc, hStrLen(vertSrc), "main", eShaderProfile_vs4_0);
        frag = hRenderer::compileShaderStageFromSource(fragSrc, hStrLen(fragSrc), "main", eShaderProfile_ps4_0);
        vb   = hRenderer::createVertexBuffer(verts, sizeof(hFloat)*7, 3, 0);

		refInfo = hRenderer::createProgramReflectionInfo(vert, frag, nullptr, nullptr, nullptr);
		for (hUint i=0, n=hRenderer::getUniformatBlockCount(refInfo); i<n; ++i) {
			auto param_info = hRenderer::getUniformatBlockInfo(refInfo, i);
			if (hStrCmp(param_info.name,"TimerBlock") == 0) {
				paramBlockSize = hMax(param_info.size, 256);
			}
		}
		ub = hRenderer::createUniformBuffer(nullptr, paramBlockSize*3, (hUint32)hRenderer::hUniformBufferFlags::Dynamic);
		for (auto& i:fences) {
			i = nullptr;
		}
		currentFence = 0;


        hRenderer::hRenderCallDesc rcd;
        rcd.vertex_ = vert;
        rcd.fragment_ = frag;
        rcd.vertexBuffer_ = vb;
		rcd.setUniformBuffer(hStringID("TimerBlock"), ub);
        rcd.setVertexBufferLayout(lo, 2);
        rc = hRenderer::createRenderCall(rcd);

		timer_.reset();
		timer_.setPause(hFalse);

        SetCanRender(hTrue);
    }
    ~MovingTri() {}

    virtual hUint32 RunUnitTest() override {

        // if (timer_.elapsedSec() > 30 || getForceExitFlag()) {
        //     SetExitCode(UNIT_TEST_EXIT_CODE_OK);
        // }

        return 0;
    }

    void RenderUnitTest() override {
        auto* cl = hRenderer::createCmdList();
        hRenderer::clear(cl, hColour(0.f, 0.f, 0.f, 1.f), 1.f);
		if (fences[currentFence]) {
			hRenderer::wait(fences[currentFence]);
			fences[currentFence] = nullptr;
		}
		auto* ubdata = (TimerBlock*) (((hByte*)hRenderer::getMappingPtr(ub)) + (currentFence*paramBlockSize));
		ubdata->timeSecs = timer_.elapsedMilliSec()/1000.f;
		hRenderer::flushUnibufferMemoryRange(cl, ub, (currentFence*paramBlockSize), paramBlockSize);
        hRenderer::draw(cl, rc, hRenderer::Primative::Triangles, 1);
		fences[currentFence] = hRenderer::fence(cl);
		currentFence = (currentFence+1)%FENCE_COUNT;
        hRenderer::swapBuffers(cl);
        
        hRenderer::submitFrame(cl);
    }
};

DEFINE_HEART_UNIT_TEST(MovingTri);