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

class MovingTriCPU : public IUnitTest {
    
	hTimer								timer_;
    hShaderProgram*                     shaderProg;
	hRenderer::hShaderStage*			vert;
    hRenderer::hShaderStage*			frag;
    hRenderer::hVertexBuffer*			vb;
    hRenderer::hRenderCall*				rc;

	static const hUint FENCE_COUNT = 3;
	hUint								paramBlockSize;
	hRenderer::hRenderFence*			fences[FENCE_COUNT];
	hUint								currentFence;

public:
    MovingTriCPU( Heart::hHeartEngine* engine ) 
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

        shaderProg = hResourceManager::weakResource<hShaderProgram>(hStringID("/system/single_tri"));

        vert = shaderProg->getShader(hRenderer::getActiveProfile(hShaderFrequency::Vertex));
        frag = shaderProg->getShader(hRenderer::getActiveProfile(hShaderFrequency::Pixel));
        vb   = hRenderer::createVertexBuffer(nullptr, sizeof(hFloat)*7, 3*FENCE_COUNT, (hUint)hRenderer::hUniformBufferFlags::Dynamic);

		for (auto& i:fences) {
			i = nullptr;
		}
		currentFence = 0;


        hRenderer::hRenderCallDesc rcd;
        rcd.vertex_ = vert;
        rcd.fragment_ = frag;
        rcd.vertexBuffer_ = vb;
        rcd.setVertexBufferLayout(lo, 2);
        rc = hRenderer::createRenderCall(rcd);

		timer_.reset();
		timer_.setPause(hFalse);

        SetCanRender(hTrue);
    }
    ~MovingTriCPU() {
        hRenderer::destroyRenderCall(rc);
        hRenderer::destroyVertexBuffer(vb);
    }

    const hChar* getHelpString() override {
        return "Single triangle scaled by time on the CPU.\nVertex buffer is dynamic and written to by the CPU.";
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

        hFloat verts[] = {
            //pos            //colour
            0.f, .5f, 0.f, /**/ 1.f, 0.f, 0.f, 1.f,
            .5f, -.5f, 0.f, /**/ 0.f, 1.f, 0.f, 1.f,
            -.5f, -.5f, 0.f, /**/ 0.f, 0.f, 1.f, 1.f,
        };

        auto* vertsptr = (hFloat*)hRenderer::getMappingPtr(vb, nullptr);
        vertsptr += (21*currentFence);
        hMemCpy(vertsptr, verts, sizeof(verts));
        vertsptr[0] *= sinf(timer_.elapsedMilliSec()/1000.f);
        vertsptr[1] *= sinf(timer_.elapsedMilliSec()/1000.f);
        vertsptr[2] *= sinf(timer_.elapsedMilliSec()/1000.f);
        vertsptr[3] *= sinf(timer_.elapsedMilliSec() / 1000.f);

        vertsptr[7] *= sinf(timer_.elapsedMilliSec()/1000.f);
        vertsptr[8] *= sinf(timer_.elapsedMilliSec()/1000.f);
        vertsptr[9] *= sinf(timer_.elapsedMilliSec()/1000.f);

        vertsptr[14] *= sinf(timer_.elapsedMilliSec()/1000.f);
        vertsptr[15] *= sinf(timer_.elapsedMilliSec()/1000.f);
        vertsptr[16] *= sinf(timer_.elapsedMilliSec()/1000.f);
        hRenderer::flushVertexBufferMemoryRange(cl, vb, (21*sizeof(hFloat)*currentFence), 21*sizeof(hFloat));

        hRenderer::draw(cl, rc, hRenderer::Primative::Triangles, 1, 3*currentFence);
        fences[currentFence] = hRenderer::fence(cl);
        currentFence = (currentFence+1)%FENCE_COUNT;
        return cl;
        //hRenderer::swapBuffers(cl);
        //hRenderer::submitFrame(cl);
    }
};

DEFINE_HEART_UNIT_TEST(MovingTriCPU);
