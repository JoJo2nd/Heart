/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#include "base/hTypes.h"
#include "base/hBase64.h"
#include "base/hClock.h"
#include "base/hStringUtil.h"
#include "core/hResourceManager.h"
#include "render/hPipelineStateDesc.h"
#include "render/hRenderer.h"
#include "render/hVertexBufferLayout.h"
#include "render/hRenderPrim.h"
#include "render/hUniformBufferFlags.h"
#include "render/hTextureFlags.h"
#include "render/hProgramReflectionInfo.h"
#include "render/hRenderShaderProgram.h"
#include "render/hMipDesc.h"
#include "render/hTextureResource.h"
#include "math/hMatrix.h"
#include "math/hVector.h"
#include "UnitTestFactory.h"

namespace Heart {
class hHeartEngine;
}

#include "shaders/ViewportConstants.hpp"
#include "shaders/InstanceConstants.hpp"

using namespace Heart;

class RenderTarget : public IUnitTest {
    
    hTimer                              timer_;
    hShaderProgram*                     shaderProg;
    hShaderProgram*                     shaderProg2;
    hRenderer::hShaderStage*            vert;
    hRenderer::hShaderStage*            frag;
    hRenderer::hShaderStage*            vert2;
    hRenderer::hShaderStage*            frag2;
    hRenderer::hVertexBuffer*           vb;
    hRenderer::hIndexBuffer*            ib;
    hRenderer::hPipelineState*          rc[2];
    hRenderer::hInputState*             is[2];
    hRenderer::hUniformBuffer*          ub1[2];
    hRenderer::hUniformBuffer*          ub2;
    hTextureResource*                   rt_res;
    hRenderer::hTexture2D*              t2d;
    hRenderer::hRenderTarget*           rt;

    static const hUint FENCE_COUNT = 3;
    hUint                               paramBlockSize;
    hRenderer::hRenderFence*            fences[FENCE_COUNT];
    hUint                               currentFence;

public:
    RenderTarget( Heart::hHeartEngine* engine ) 
        : IUnitTest( engine ) {

        struct Vtx{
            hFloat x,y,z;
            hFloat r,g,b,a;
            hFloat u, v;
        } cube_verts[] = {
            //pos            //colour
            // .5f,  .5f, .5f, /**/ 1.f, 0.f, 0.f, 1.f, /*0*/ 
            // .5f,  .5f,-.5f, /**/ 0.f, 1.f, 0.f, 1.f, /*1*/ 
            // .5f, -.5f, .5f, /**/ 0.f, 0.f, 1.f, 1.f, /*2*/ 
            // .5f, -.5f,-.5f, /**/ 0.f, 0.f, 0.f, 1.f, /*3*/ 
            //-.5f,  .5f, .5f, /**/ 1.f, 0.f, 0.f, 1.f, /*4*/ 
            //-.5f,  .5f,-.5f, /**/ 0.f, 1.f, 0.f, 1.f, /*5*/ 
            //-.5f, -.5f, .5f, /**/ 0.f, 0.f, 1.f, 1.f, /*6*/ 
            //-.5f, -.5f,-.5f, /**/ 0.f, 0.f, 0.f, 1.f, /*7*/ 
            // Front face
            -1.0, -1.0,  1.0, 1.f, 0.f, 0.f, 1.f, 0.f, 0.f,  
             1.0, -1.0,  1.0, 0.f, 1.f, 0.f, 1.f, 1.f, 0.f,
             1.0,  1.0,  1.0, 0.f, 0.f, 1.f, 1.f, 1.f, 1.f,
            -1.0,  1.0,  1.0, 0.f, 0.f, 0.f, 1.f, 0.f, 1.f,
            
            // Back face
            -1.0, -1.0, -1.0, 1.f, 0.f, 0.f, 1.f, 0.f, 0.f, 
            -1.0,  1.0, -1.0, 0.f, 1.f, 0.f, 1.f, 1.f, 0.f,
             1.0,  1.0, -1.0, 0.f, 0.f, 1.f, 1.f, 1.f, 1.f,
             1.0, -1.0, -1.0, 0.f, 0.f, 0.f, 1.f, 0.f, 1.f,
            
            // Top face
            -1.0,  1.0, -1.0, 1.f, 0.f, 0.f, 1.f, 0.f, 0.f, 
            -1.0,  1.0,  1.0, 0.f, 1.f, 0.f, 1.f, 1.f, 0.f,
             1.0,  1.0,  1.0, 0.f, 0.f, 1.f, 1.f, 1.f, 1.f,
             1.0,  1.0, -1.0, 0.f, 0.f, 0.f, 1.f, 0.f, 1.f,
            
            // Bottom face
            -1.0, -1.0, -1.0, 1.f, 0.f, 0.f, 1.f, 0.f, 0.f, 
             1.0, -1.0, -1.0, 0.f, 1.f, 0.f, 1.f, 1.f, 0.f,
             1.0, -1.0,  1.0, 0.f, 0.f, 1.f, 1.f, 1.f, 1.f,
            -1.0, -1.0,  1.0, 0.f, 0.f, 0.f, 1.f, 0.f, 1.f,
            
            // Right face
             1.0, -1.0, -1.0, 1.f, 0.f, 0.f, 1.f, 0.f, 0.f, 
             1.0,  1.0, -1.0, 0.f, 1.f, 0.f, 1.f, 1.f, 0.f,
             1.0,  1.0,  1.0, 0.f, 0.f, 1.f, 1.f, 1.f, 1.f,
             1.0, -1.0,  1.0, 0.f, 0.f, 0.f, 1.f, 0.f, 1.f,
            
            // Left face
            -1.0, -1.0, -1.0, 1.f, 0.f, 0.f, 1.f, 0.f, 0.f, 
            -1.0, -1.0,  1.0, 0.f, 1.f, 0.f, 1.f, 1.f, 0.f,
            -1.0,  1.0,  1.0, 0.f, 0.f, 1.f, 1.f, 1.f, 1.f,
            -1.0,  1.0, -1.0, 0.f, 0.f, 0.f, 1.f, 0.f, 1.f,
        };
        hUint16 cube_indices[] = {
            /*
            0,2,3, 0,3,1,
            1,3,7, 1,7,5,
            5,7,6, 5,6,4,
            4,6,2, 4,2,0,
            5,0,1, 5,4,0,
            7,3,2, 7,2,6,
            */
            0,  1,  2,      0,  2,  3,    // front
            4,  5,  6,      4,  6,  7,    // back
            8,  9,  10,     8,  10, 11,   // top
            12, 13, 14,     12, 14, 15,   // bottom
            16, 17, 18,     16, 18, 19,   // right
            20, 21, 22,     20, 22, 23    // left
        };
        hRenderer::hVertexBufferLayout lo[] = {
            {hStringID("in_position"), hRenderer::hSemantic::Position, 0, 3, hRenderer::hVertexInputType::Float,                0, hFalse, sizeof(Vtx)},
            {hStringID("in_colour") ,  hRenderer::hSemantic::Texcoord, 0, 4, hRenderer::hVertexInputType::Float, sizeof(hFloat)*3, hFalse, sizeof(Vtx)},
            {hStringID("in_uv")     ,  hRenderer::hSemantic::Texcoord, 1, 2, hRenderer::hVertexInputType::Float, sizeof(hFloat)*7, hFalse, sizeof(Vtx)},
        };

        shaderProg = hResourceManager::weakResource<hShaderProgram>(hStringID("/system/render_target_1"));
        shaderProg2 = hResourceManager::weakResource<hShaderProgram>(hStringID("/system/render_target_2"));

        vert = shaderProg->getShader(hRenderer::getActiveProfile(hShaderFrequency::Vertex));
        frag = shaderProg->getShader(hRenderer::getActiveProfile(hShaderFrequency::Pixel));
        vert2 = shaderProg2->getShader(hRenderer::getActiveProfile(hShaderFrequency::Vertex));
        frag2 = shaderProg2->getShader(hRenderer::getActiveProfile(hShaderFrequency::Pixel));
        vb = hRenderer::createVertexBuffer(cube_verts, sizeof(Vtx), hStaticArraySize(cube_verts), 0);
        ib = hRenderer::createIndexBuffer(cube_indices, hStaticArraySize(cube_indices), 0);
        rt_res = hResourceManager::weakResource<hTextureResource>(hStringID("/system/test_render_target"));
        hcAssert(rt_res && rt_res->isRenderTarget());
        t2d = rt_res->getTexture2D();//hRenderer::createTexture2D(1, rt_mip, hTextureFormat::RGBA8_unorm, (hUint32)hRenderer::TextureFlags::RenderTarget);
        rt = rt_res->getRenderTarget();//hRenderer::createRenderTarget(t2d, 0);


        ub1[0] = hRenderer::createUniformBuffer(nullptr, ViewportConstants::getLayout(), ViewportConstants::getLayoutCount(), (hUint)sizeof(ViewportConstants), (hUint32)hRenderer::hUniformBufferFlags::Dynamic);
        ub1[1] = hRenderer::createUniformBuffer(nullptr, ViewportConstants::getLayout(), ViewportConstants::getLayoutCount(), (hUint)sizeof(ViewportConstants), (hUint32)hRenderer::hUniformBufferFlags::Dynamic);
        ub2 = hRenderer::createUniformBuffer(nullptr, InstanceConstants::getLayout(), InstanceConstants::getLayoutCount(), (hUint)sizeof(InstanceConstants), (hUint32)hRenderer::hUniformBufferFlags::Dynamic);
        for (auto& i:fences) {
            i = nullptr;
        }
        currentFence = 0;

        hRenderer::hPipelineStateDesc rcd;
        hRenderer::hInputStateDesc isd;
        rcd.depthStencil_.depthEnable_ = hTrue;
        rcd.rasterizer_.cullMode_ = proto::renderstate::CullClockwise;
        rcd.vertex_ = vert;
        rcd.fragment_ = frag;
        rcd.vertexBuffer_ = vb;
        rcd.indexBuffer_ = ib;
        rcd.setVertexBufferLayout(lo, 3);
        rc[0] = hRenderer::createRenderPipelineState(rcd);
        isd.setUniformBuffer(hStringID("ViewportConstants"), ub1[1]);
        isd.setUniformBuffer(hStringID("InstanceConstants"), ub2);
        is[0] = hRenderer::createRenderInputState(isd, rcd);

        hRenderer::hPipelineStateDesc::hSamplerStateDesc ssd;
        ssd.filter_ = proto::renderstate::linear;
        rcd.vertex_ = vert2;
        rcd.fragment_ = frag2;
        rcd.setSampler(hStringID("tSampler"), ssd);
        rc[1] = hRenderer::createRenderPipelineState(rcd);
        isd.setUniformBuffer(hStringID("ViewportConstants"), ub1[0]);
        isd.setTextureSlot(hStringID("t_tex2D"), t2d);
        is[1] = hRenderer::createRenderInputState(isd, rcd);

        timer_.reset();
        timer_.setPause(hFalse);

        SetCanRender(hTrue);
    }
    ~RenderTarget() {
        for (const auto& i : rc) {
            hRenderer::destroyRenderPipelineState(i);
        }
        for (const auto& i : is) {
            hRenderer::destroyRenderInputState(i);
        }
        for (const auto& i : ub1) {
            hRenderer::destroyUniformBuffer(i);
        }
        hRenderer::destroyUniformBuffer(ub2);
        hRenderer::destroyVertexBuffer(vb);
    }

    const hChar* getHelpString() override {
        return "Render Target test";
    }

    virtual hUint32 RunUnitTest() override {

        if (timer_.elapsedMilliSec() > 10*1000 || getForceExitFlag()) {
            SetExitCode(UNIT_TEST_EXIT_CODE_OK);
        }

        return 0;
    }

    Heart::hRenderer::hCmdList* RenderUnitTest() override {
        auto* cl = hRenderer::createCmdList();
        if (fences[currentFence]) {
            hRenderer::wait(fences[currentFence]);
        }
        hMatrix v = hMatrix::lookAt(hPoint3(0.f, 0.f, 5.f), hPoint3(0.f, 0.f, 0.f), hVec3(0.f, 1.f, 0.f));
        hMatrix p1 = hRenderer::getClipspaceMatrix()*hMatrix::perspective(3.1415f/4.f, 1280.f/720.f, 0.01f, 100.f);
        hMatrix p2 = hRenderer::getClipspaceMatrix()*hMatrix::perspective(3.1415f/4.f, 512.f/512.f, 0.01f, 100.f);
        hMatrix w = hMatrix::identity();
        hFloat r = timer_.elapsedMilliSec()/2000.f;
        w = hMatrix::rotationZYX(hVec3(r, r*2.f, r*3.f));

        auto* ub1data1 = (ViewportConstants*) hRenderer::getMappingPtr(ub1[0]);
        auto* ub1data2 = (ViewportConstants*) hRenderer::getMappingPtr(ub1[1]);
        InstanceConstants ub2data;

        hMatrix v_inverse = inverse(v);
        hMatrix vp = p1*v;
        ub1data1->view                 = v;
        ub1data1->viewInverse          = v_inverse;
        ub1data1->viewInverseTranspose = transpose(v_inverse);
        ub1data1->projection           = p1;
        ub1data1->projectionInverse    = inverse(p1);
        ub1data1->viewProjection       = vp;
        ub1data1->viewProjectionInverse= inverse(vp);
        ub1data1->viewportSize         = hVec4(1280.f, 720.f, 1.f/1280.f, 1.f/720.f);

        vp = p2*v;
        ub1data2->view                 = v;
        ub1data2->viewInverse          = v_inverse;
        ub1data2->viewInverseTranspose = transpose(v_inverse);
        ub1data2->projection           = p2;
        ub1data2->projectionInverse    = inverse(p2);
        ub1data2->viewProjection       = vp;
        ub1data2->viewProjectionInverse= inverse(vp);
        ub1data2->viewportSize         = hVec4(rt_res->getWidth(), rt_res->getHeight(), 1.f/rt_res->getWidth(), 1.f/rt_res->getHeight());

        ub2data.world = w;

        hRenderer::flushUnibufferMemoryRange(cl, ub1[0], 0, sizeof(ViewportConstants));
        hRenderer::flushUnibufferMemoryRange(cl, ub1[1], 0, sizeof(ViewportConstants));
        hRenderer::flushUnibufferMemoryRangeUserPtr(cl, ub2, &ub2data, 0, sizeof(InstanceConstants));
        hRenderer::setRenderTargets(cl, &rt, 1);
        hRenderer::clear(cl, hColour(0.f, 0.f, 0.f, 1.f), 1.f);
        hRenderer::draw(cl, rc[0], is[0], hRenderer::Primative::Triangles, 12, 0);
        hRenderer::setRenderTargets(cl, nullptr, 0);
        hRenderer::clear(cl, hColour(0.f, 0.2f, 0.f, 1.f), 1.f);
        hRenderer::draw(cl, rc[1], is[1], hRenderer::Primative::Triangles, 12, 0);
        currentFence = (currentFence+1)%FENCE_COUNT;
        if (!fences[currentFence]) fences[currentFence] = hRenderer::createFence();
        hRenderer::fence(cl, fences[currentFence]);
        return cl;
    }
};

DEFINE_HEART_UNIT_TEST(RenderTarget);
