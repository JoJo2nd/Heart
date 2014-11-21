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
#include "UnitTestFactory.h"

namespace Heart {
class hHeartEngine;
}

using namespace Heart;

const hChar vertSrc[] = 
"\
#version 330\n\
layout(location=0) in vec3 in_position;\n\
layout(location=1) in vec4 in_colour;\n\
out vec4 inout_colour;\n\
void main() {\n\
    inout_colour = /*vec4(0, 1, 0, 1);*/in_colour;\n\
    gl_Position.xyz = in_position.xyz;\n\
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

class SingleTri : public IUnitTest {
    
    hTimer                    timer_;
    hRenderer::hShaderStage*  vert;
    hRenderer::hShaderStage*  frag;
    hRenderer::hVertexBuffer* vb;
    hRenderer::hRenderCall*   rc;
    hRenderer::hCmdList*      cl;

public:
    SingleTri( Heart::hHeartEngine* engine ) 
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

        hRenderer::hRenderCallDesc rcd;
        rcd.vertex_ = vert;
        rcd.fragment_ = frag;
        rcd.vertexBuffer_ = vb;
        rcd.setVertexBufferLayout(lo, 2);

        rc = hRenderer::createRenderCall(rcd);

        cl = hRenderer::createCmdList();
        hRenderer::clear(cl, hColour(1.f, 0.f, 0.f, 1.f), 1.f);
		hRenderer::draw(cl, rc, hRenderer::Primative::Triangles, 1);
        hRenderer::swapBuffers(cl);

        SetCanRender(hTrue);
    }
    ~SingleTri() {}

    virtual hUint32 RunUnitTest() override {

        // if (timer_.elapsedSec() > 30 || getForceExitFlag()) {
        //     SetExitCode(UNIT_TEST_EXIT_CODE_OK);
        // }

        return 0;
    }

    void RenderUnitTest() override {
        hRenderer::submitFrame(cl);
    }
};

DEFINE_HEART_UNIT_TEST(SingleTri);