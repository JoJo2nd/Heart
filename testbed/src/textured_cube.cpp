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
#include "render/hTextureFlags.h"
#include "render/hProgramReflectionInfo.h"
#include "render/hRenderShaderProgram.h"
#include "render/hTextureResource.h"
#include "render/hMipDesc.h"
#include "render/hImGuiRenderer.h"
#include "math/hMatrix.h"
#include "math/hVector.h"
#include "UnitTestFactory.h"

namespace Heart {
class hHeartEngine;
}

using namespace Heart;

struct ViewportConstants {
    hMatrix view                 ;
    hMatrix viewInverse          ;
    hMatrix viewInverseTranspose ;
    hMatrix projection           ;
    hMatrix projectionInverse    ;
    hMatrix viewProjection       ;
    hMatrix viewProjectionInverse;
    hVec4   viewportSize         ;
};
static hRenderer::hUniformLayoutDesc ViewportConstants_layout[] = {
    {"g_View", hRenderer::ShaderParamType::Float44, 0},
    {"g_ViewInverse", hRenderer::ShaderParamType::Float44, 64},
    {"g_ViewInverseTranspose", hRenderer::ShaderParamType::Float44, 128},
    {"g_Projection", hRenderer::ShaderParamType::Float44, 192},
    {"g_ProjectionInverse", hRenderer::ShaderParamType::Float44, 256},
    {"g_ViewProjection", hRenderer::ShaderParamType::Float44, 320},
    {"g_ViewProjectionInverse", hRenderer::ShaderParamType::Float44, 384},
    {"g_ViewportSize", hRenderer::ShaderParamType::Float4, 448},
};

struct InstanceConstants {
    hMatrix world;
};
static hRenderer::hUniformLayoutDesc InstanceConstants_layout[] = {
    {"g_World", hRenderer::ShaderParamType::Float44, 0},
};


class TexturedCube : public IUnitTest {
    
    hTimer                              timer_;
    hShaderProgram*                     shaderProg;
    hRenderer::hShaderStage*            vert;
    hRenderer::hShaderStage*            frag;
    hRenderer::hVertexBuffer*           vb;
    hRenderer::hIndexBuffer*            ib;
    hRenderer::hPipelineState*          pls;
    hRenderer::hInputState*             is;
    hRenderer::hUniformBuffer*          ub1;
    hRenderer::hUniformBuffer*          ub2;
    hRenderer::hTexture2D*              t2d;
    hRenderer::hRenderTarget*           rt;

    static const hUint FENCE_COUNT = 3;
    hUint                               paramBlockSize;
    hRenderer::hRenderFence*            fences[FENCE_COUNT];
    hUint                               currentFence;

    hVec3 camPos;
    hFloat rotationSpeed;

public:
    TexturedCube( Heart::hHeartEngine* engine ) 
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

        shaderProg = hResourceManager::weakResource<hShaderProgram>(hStringID("/system/textured_cube"));

        vert = shaderProg->getShader(hRenderer::getActiveProfile(hShaderFrequency::Vertex));
        frag = shaderProg->getShader(hRenderer::getActiveProfile(hShaderFrequency::Pixel));
        vb = hRenderer::createVertexBuffer(cube_verts, sizeof(Vtx), hStaticArraySize(cube_verts), 0);
        ib = hRenderer::createIndexBuffer(cube_indices, hStaticArraySize(cube_indices), 0);
        t2d = hResourceManager::weakResource<hTextureResource>(hStringID("/textures/test_tile_set_bc1"))->getTexture2D();


        ub1 = hRenderer::createUniformBuffer(nullptr, ViewportConstants_layout, (hUint)hStaticArraySize(ViewportConstants_layout), (hUint)sizeof(ViewportConstants), 3, (hUint32)hRenderer::hUniformBufferFlags::Dynamic);
        ub2 = hRenderer::createUniformBuffer(nullptr, InstanceConstants_layout, (hUint)hStaticArraySize(InstanceConstants_layout), (hUint)sizeof(InstanceConstants), 3, (hUint32)hRenderer::hUniformBufferFlags::Dynamic);
        for (auto& i:fences) {
            i = nullptr;
        }
        currentFence = 0;

        hRenderer::hPipelineStateDesc::hSamplerStateDesc ssd;
        ssd.filter_ = proto::renderstate::linear;
        hRenderer::hPipelineStateDesc rcd;
        rcd.depthStencil_.depthEnable_ = hTrue;
        rcd.rasterizer_.cullMode_ = proto::renderstate::CullClockwise;
        rcd.vertex_ = vert;
        rcd.fragment_ = frag;
        rcd.vertexBuffer_ = vb;
        rcd.indexBuffer_ = ib;
        rcd.setSampler(hStringID("tSampler"), ssd);
        rcd.setVertexBufferLayout(lo, 3);
        pls = hRenderer::createRenderPipelineState(rcd);
        hRenderer::hInputStateDesc isd;
        isd.setUniformBuffer(hStringID("ViewportConstants"), ub1);
        isd.setUniformBuffer(hStringID("InstanceConstants"), ub2);
        isd.setTextureSlot(hStringID("t_tex2D"), t2d);
        is = hRenderer::createRenderInputState(isd, rcd);

        timer_.reset();
        timer_.setPause(hFalse);

        camPos = hVec3(0.f, 0.f, 3.f);
        rotationSpeed = 1.f;

        SetCanRender(hTrue);
    }
    ~TexturedCube() {
        hRenderer::destroyRenderPipelineState(pls);
        hRenderer::destroyRenderInputState(is);
        hRenderer::destroyUniformBuffer(ub1);
        hRenderer::destroyUniformBuffer(ub2);
        hRenderer::destroyTexture2D(t2d);
        hRenderer::destroyRenderTarget(rt);
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
            fences[currentFence] = nullptr;
        }

        hFloat p[3] = {camPos.getX(), camPos.getY(), camPos.getZ()};
        ImGui::SetNextWindowPos(ImVec2(10, 200));
        ImGui::Begin("Textured Cube View", nullptr, ImGuiWindowFlags_NoSavedSettings|ImGuiWindowFlags_NoResize|ImGuiWindowFlags_AlwaysAutoResize|ImGuiWindowFlags_ShowBorders);
        ImGui::Text("View Settings");
        ImGui::Separator();
        ImGui::SliderFloat("Camera X", p  , -5, 5);
        ImGui::SliderFloat("Camera Y", p+1, -5, 5);
        ImGui::SliderFloat("Camera Z", p+2, 0, 25);
        ImGui::Separator();
        ImGui::SliderFloat("Cube Rotation Factor", &rotationSpeed, 0, 10);
        ImGui::End();

        camPos = hVec3(p[0], p[1], p[2]);
        hMatrix v = hMatrix::lookAt((hPoint3)camPos, camPos+hPoint3(0.f, 0.f, -1.f), hVec3(0.f, 1.f, 0.f));
        hMatrix p1 = hRenderer::getClipspaceMatrix()*hMatrix::perspective(3.1415f/4.f, 1280.f/720.f, 0.01f, 100.f);
        hMatrix p2 = hRenderer::getClipspaceMatrix()*hMatrix::perspective(3.1415f/4.f, 512.f/512.f, 0.01f, 100.f);
        hMatrix w = hMatrix::identity();
        hFloat r = timer_.elapsedMilliSec()/2000.f;
        w = hMatrix::rotationZYX(hVec3(0, r*rotationSpeed, 0));

        auto* ub1data1 = (ViewportConstants*) (((hByte*)hRenderer::getMappingPtr(ub1)) + (currentFence*sizeof(ViewportConstants)));
        auto* ub2data = (InstanceConstants*) (((hByte*)hRenderer::getMappingPtr(ub2)) + (currentFence*sizeof(InstanceConstants)));

        ub1data1->view                 = v;
        ub1data1->viewInverse          = inverse(v);
        ub1data1->viewInverseTranspose = transpose(ub1data1->viewInverse);
        ub1data1->projection           = p1;
        ub1data1->projectionInverse    = inverse(p1);
        ub1data1->viewProjection       = p1*v;
        ub1data1->viewProjectionInverse= inverse(ub1data1->viewProjection);
        ub1data1->viewportSize         = hVec4(1280.f, 720.f, 1.f/1280.f, 1.f/720.f);

        ub2data->world = w;

        hRenderer::flushUnibufferMemoryRange(cl, ub1, (currentFence*sizeof(ViewportConstants)), sizeof(ViewportConstants));
        hRenderer::flushUnibufferMemoryRange(cl, ub2, (currentFence*sizeof(InstanceConstants)), sizeof(InstanceConstants));
        hRenderer::clear(cl, hColour(0.f, 0.f, 0.f, 1.f), 1.f);
        hRenderer::draw(cl, pls, is, hRenderer::Primative::Triangles, 12, 0);
        fences[currentFence] = hRenderer::fence(cl);
        currentFence = (currentFence+1)%FENCE_COUNT;
        return cl;
    }
};

DEFINE_HEART_UNIT_TEST(TexturedCube);
