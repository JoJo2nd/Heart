/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#include "base/hTypes.h"
#include "base/hBase64.h"
#include "base/hClock.h"
#include "base/hStringUtil.h"
#include "render/hPipelineStateDesc.h"
#include "render/hRenderer.h"
#include "render/hVertexBufferLayout.h"
#include "render/hRenderPrim.h"
#include "render/hUniformBufferFlags.h"
#include "render/hRenderShaderProgram.h"
#include "UnitTestFactory.h"


namespace Heart {
class hHeartEngine;
}

using namespace Heart;

class SingleTri : public IUnitTest {
    
    hTimer                     timer_;
    hShaderProgram*            shaderProg;
    hRenderer::hShaderStage*   vert;
    hRenderer::hShaderStage*   frag;
    hRenderer::hVertexBuffer*  vb;
    hRenderer::hPipelineState* pls;
    hRenderer::hCmdList*       cl;

public:
    SingleTri( Heart::hHeartEngine* engine ) 
        : IUnitTest( engine ) {

        struct Vtx{
            hFloat x,y,z;
            hFloat r, g, b, a;
        } verts[] = {
            //pos            //colour
             0.f,  .5f, 0.f, /**/ 1.f, 0.f, 0.f, 1.f,
             .5f, -.5f, 0.f, /**/ 0.f, 1.f, 0.f, 1.f,
            -.5f, -.5f, 0.f, /**/ 0.f, 0.f, 1.f, 1.f,
        };
        hRenderer::hVertexBufferLayout lo[] = {
            {hStringID("in_position"),hRenderer::hSemantic::Position, 0, 3, hRenderer::hVertexInputType::Float,                0, hFalse, sizeof(Vtx)},
            {hStringID("in_colour"),  hRenderer::hSemantic::Texcoord, 0, 4, hRenderer::hVertexInputType::Float, sizeof(hFloat)*3, hFalse, sizeof(Vtx)},
        };

        shaderProg = hResourceManager::weakResource<hShaderProgram>(hStringID("/system/single_tri"));

        vert = shaderProg->getShader(hRenderer::getActiveProfile(hShaderFrequency::Vertex));
        frag = shaderProg->getShader(hRenderer::getActiveProfile(hShaderFrequency::Pixel));
        vb   = hRenderer::createVertexBuffer(verts, sizeof(Vtx), 3, 0);

        hRenderer::hPipelineStateDesc rcd;
        rcd.vertex_ = vert;
        rcd.fragment_ = frag;
        rcd.vertexBuffer_ = vb;
        rcd.setVertexBufferLayout(lo, hStaticArraySize(lo));
        pls = hRenderer::createRenderPipelineState(rcd);

        SetCanRender(hTrue);
    }
    ~SingleTri() {
        hRenderer::destroyRenderPipelineState(pls);
        hRenderer::destroyVertexBuffer(vb);
    }

    virtual hUint32 RunUnitTest() override {

        if (timer_.elapsedMilliSec() > 10*1000 || getForceExitFlag()) {
            SetExitCode(UNIT_TEST_EXIT_CODE_OK);
        }

        return 0;
    }

    Heart::hRenderer::hCmdList* RenderUnitTest() override {
        cl = hRenderer::createCmdList();
        hRenderer::clear(cl, hColour(1.f, 0.f, 1.f, 1.f), 1.f);
        hRenderer::draw(cl, pls, nullptr, hRenderer::Primative::Triangles, 1, 0);
        return cl;
        //hRenderer::swapBuffers(cl);
        //hRenderer::submitFrame(cl);
    }
};

DEFINE_HEART_UNIT_TEST(SingleTri);
