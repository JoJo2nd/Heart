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
    hShaderProgram*                     shaderProg;
	hRenderer::hShaderStage*			vert;
    hRenderer::hShaderStage*			frag;
	//hRenderer::hProgramReflectionInfo*	refInfo;
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

        shaderProg = hResourceManager::weakResource<hShaderProgram>(hStringID("/system/moving_tri"));

        vert = shaderProg->getShader(hRenderer::getActiveProfile(hShaderFrequency::Vertex));
        frag = shaderProg->getShader(hRenderer::getActiveProfile(hShaderFrequency::Pixel));
        vb   = hRenderer::createVertexBuffer(verts, sizeof(hFloat)*7, 3, 0);

        hRenderer::hUniformLayoutDesc ublo[] = {
            {"timeSecs", hRenderer::ShaderParamType::Float,    0},
            {"pad",      hRenderer::ShaderParamType::Float3,   4},
        };

        paramBlockSize = (hUint)sizeof(TimerBlock);
		ub = hRenderer::createUniformBuffer(nullptr, ublo, (hUint)hStaticArraySize(ublo), (hUint)sizeof(TimerBlock), 3, (hUint32)hRenderer::hUniformBufferFlags::Dynamic);
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
    ~MovingTri() {
        hRenderer::destroyRenderCall(rc);
        hRenderer::destroyUniformBuffer(ub);
        //hRenderer::destroyProgramReflectionInfo(refInfo);
        hRenderer::destroyVertexBuffer(vb);
    }

    const hChar* getHelpString() override {
        return "Single triangle scaled by time in the shader.\nTime parameter is passed by shader input parameters.";
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
        if (fences[currentFence]) {
            hRenderer::wait(fences[currentFence]);
            fences[currentFence] = nullptr;
        }
        auto* ubdata = (TimerBlock*) (((hByte*)hRenderer::getMappingPtr(ub)) + (currentFence*paramBlockSize));
        ubdata->timeSecs = timer_.elapsedMilliSec()/1000.f;
        hRenderer::flushUnibufferMemoryRange(cl, ub, (currentFence*paramBlockSize), paramBlockSize);
        hRenderer::draw(cl, rc, hRenderer::Primative::Triangles, 1, 0);
        fences[currentFence] = hRenderer::fence(cl);
        currentFence = (currentFence+1)%FENCE_COUNT;
        return cl;
    }
};

DEFINE_HEART_UNIT_TEST(MovingTri);
