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
#include "render/hUniformBufferResource.h"
#include "render/hTextureFlags.h"
#include "render/hProgramReflectionInfo.h"
#include "render/hRenderShaderProgram.h"
#include "render/hTextureResource.h"
#include "render/hMipDesc.h"
#include "render/hImGuiRenderer.h"
#include "render/hMaterial.h"
#include "math/hMatrix.h"
#include "math/hVector.h"
#include "UnitTestFactory.h"

#include "shaders/ViewportConstants.hpp"
#include "shaders/InstanceConstants.hpp"

namespace Heart {
class hHeartEngine;
}

#define PI (3.1415926535897932384626433832795f)
#define TWOPI (2.f*PI)
#define PIOVER2 (PI*.5f)
#define FOV_Y (PI/2.f)
#define SCREEN_SIZE_PERCENT (0.75f)

using namespace Heart;

static const char* test_texture_names[] = {
    "/system/default_tex_rgba",
    "/system/default_tex_bc1",
    "/system/default_tex_bc1a",
    "/system/default_tex_bc3",
    "/system/default_tex_bc3n",
    "/textures/test_tile_set_bc1",
    "/textures/test_tile_set_alpha_bc1a",
    "/textures/test_tile_set_alpha_bc3",
    "/textures/Testtileset"
};

float ComputeBoundsScreenSize(const hVec3& Origin, const float SphereRadius, float ViewWidth, float ViewHeight, const hVec3& viewCentre, const hVec3& viewDir, hMatrix proj, float& out_m, float& out_r)
{
    // Only need one component from a view transformation; just calculate the one we're interested in.
    float Divisor = dot(Origin - viewCentre, viewDir);

    // Get projection multiple accounting for view scaling.
    ViewWidth = ViewHeight = 1;
    float ScreenMultiple = hMax(ViewWidth / 2.0f * proj[0][0], ViewHeight / 2.0f * proj[1][1]);

    float ScreenRadius = ScreenMultiple * SphereRadius / Divisor;
    float ScreenArea = PI * ScreenRadius * ScreenRadius;
    float t = PIOVER2 - ((PIOVER2 / 4.f));
    float r = SphereRadius*tanf(t);
    out_r = r / Divisor;
    out_m = (PI*out_r*out_r) / 4;
    return ScreenArea / (ViewWidth*ViewHeight);
}

class TexturedCube : public IUnitTest {
    
    struct TextureFormat {
        const hChar* name;
        hRenderer::hTexture2D* t;
    };

    hTimer                              timer_;

    hRenderer::hVertexBufferUniquePtr   vb;
    hRenderer::hIndexBufferUniquePtr    ib;
    hRenderer::hUniformBuffer*          ub1;
    hRenderer::hUniformBufferUniquePtr  ub2;

    hMaterial*                          material;
    hRenderer::hPipelineStateUniquePtr  mat_pls;
    hRenderer::hInputStateUniquePtr     mat_is;

    static const hUint FENCE_COUNT = 3;
    hUint                               paramBlockSize;
    hRenderer::hRenderFence*            fences[FENCE_COUNT];
    hUint                               currentFence;

    std::vector< TextureFormat >        testTextures;
    hVec3 camPos;
    hFloat rotationSpeed;
    hFloat cubeRotation;
    hInt currentTex;

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

        material = hResourceManager::weakResource<hMaterial>(hStringID("/system/test_material_1"));
        auto tech_i = material->getTechniqueByName(hStringID("main"));
        hcAssertMsg(material->getTechniquePassCount(tech_i) == 1, "/system/test_material_1 'main' is expected to only have one pass");
        hRenderer::hPipelineStateDesc rcd = material->getTechniquePassPipelineStateDesc(tech_i, 0);
        hRenderer::hInputStateDesc isd = material->getTechniquePassInputStateDesc(tech_i, 0);

        vb.reset(hRenderer::createVertexBuffer(cube_verts, sizeof(Vtx), hStaticArraySize(cube_verts), 0));
        ib.reset(hRenderer::createIndexBuffer(cube_indices, hStaticArraySize(cube_indices), 0));

        ub1 = hResourceManager::weakResource<hUniformBufferResource>(hStringID("/system/ViewportConstants"))->getSharedUniformBuffer();
        ub2.reset(hRenderer::createUniformBuffer(nullptr, InstanceConstants::getLayout(), InstanceConstants::getLayoutCount(), (hUint)sizeof(InstanceConstants), (hUint32)hRenderer::hUniformBufferFlags::Dynamic));
        for (auto& i:fences) {
            i = nullptr;
        }
        currentFence = 0;

        rcd.vertexBuffer_ = vb.get();
        rcd.indexBuffer_ = ib.get();
        rcd.setVertexBufferLayout(lo, 3);
        mat_pls.reset(hRenderer::createRenderPipelineState(rcd));

        isd.setUniformBuffer(hStringID("InstanceConstants"), ub2.get());
        isd.setTextureSlotWithOverride(hStringID("t_tex2D"), hResourceManager::weakResource<hTextureResource>(hStringID("/system/default_tex_rgba"))->getTexture2D(), 0);
        mat_is.reset(hRenderer::createRenderInputState(isd, rcd));
        

        testTextures.resize(hStaticArraySize(test_texture_names));
        for (hSize_t i = 0, n = hStaticArraySize(test_texture_names)-1; i < n; ++i) {
            auto* tt = hResourceManager::weakResource<hTextureResource>(hStringID(test_texture_names[i]));
            testTextures[i].name = test_texture_names[i];
            testTextures[i].t = tt->getTexture2D();
        }
        
        {
            int i = hStaticArraySize(test_texture_names)-1;
            auto* tt = hResourceManager::weakResource<hTextureAtlasResource>(hStringID("/textures/Testtileset"));
            testTextures[i].name = test_texture_names[i];
            testTextures[i].t = tt->getTextureResource()->getTexture2D();
        }

        timer_.reset();
        timer_.setPause(hFalse);

        camPos = hVec3(0.f, 0.f, 3.f);
        rotationSpeed = 0.f;
        cubeRotation = 0.f;
        currentTex = 0;

        SetCanRender(hTrue);
    }
    ~TexturedCube() {
    }

    const hChar* getHelpString() override {
        return "Test of a number of texture formats.\nAlso tests texture override slots for renderer input states.";
    }

    virtual hUint32 RunUnitTest() override {

        float r = length(hVec3(1.f,1.f,0.f));
        float n = 1.f;
        hVec3 d(0.f, 0.f, -1.f);
        hVec3 v = camPos;
        hVec3 c(0.f, 0.f, 0.f);
        //p = nr/d.(c-v)
        float dp = (n*r)/dot(d, c-v);
        float guess_p = sqrt((4*SCREEN_SIZE_PERCENT)/PI)/tanf(PIOVER2 - (FOV_Y*.5f));

        hMatrix p1 = hMatrix::perspective(FOV_Y, 1280.f / 720.f, 1.f, 100.f);
        float m, roverd;
        float screenSize = ComputeBoundsScreenSize(c, r, 1280.f, 720.f, v, d, p1, m, roverd);


        hFloat p[3] = { camPos.getX(), camPos.getY(), camPos.getZ() };
        //ImGui::SetNextWindowPos(ImVec2(10, 200));
        ImGui::Begin("Textured Cube View", nullptr, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_ShowBorders);
        ImGui::Text("View Settings");
        ImGui::Separator();
        ImGui::SliderFloat("Camera X", p, -15, 15);
        ImGui::SliderFloat("Camera Y", p + 1, -15, 15);
        ImGui::SliderFloat("Camera Z", p + 2, 0, 50);
        ImGui::Separator();
        ImGui::SliderFloat("Cube Rotation Factor", &rotationSpeed, 0, 10);
        ImGui::Separator();
        ImGui::Combo("", &currentTex, test_texture_names, hStaticArraySize(test_texture_names));
        ImGui::Separator();
        ImGui::Text("Camera Area d=%f screensize=%f m=%f r/d=%f 4PIa=%f", dp, screenSize, m, roverd, guess_p);
        ImGui::End();
        camPos = hVec3(p[0], p[1], p[2]);

        return 0;
    }

    Heart::hRenderer::hCmdList* RenderUnitTest() override {
        auto* cl = hRenderer::createCmdList();
        if (fences[currentFence]) {
            hRenderer::wait(fences[currentFence]);
            fences[currentFence] = nullptr;
        }

        hMatrix v = hMatrix::lookAt((hPoint3)camPos, camPos+hPoint3(0.f, 0.f, -1.f), hVec3(0.f, 1.f, 0.f));
        hMatrix p1 = hRenderer::getClipspaceMatrix()*hMatrix::perspective(FOV_Y, 1280.f/720.f, 0.01f, 100.f);
        hMatrix w = hMatrix::identity();
        hFloat r = timer_.elapsedMilliSec()/1000.f;
        cubeRotation += r*rotationSpeed;
        w = hMatrix::rotationZYX(hVec3(0, cubeRotation, 0));
        if (cubeRotation > 6.2831f) {
            cubeRotation -= 6.2831f;
        }
        timer_.reset();

        auto* ub1data1 = (ViewportConstants*) (hByte*)hRenderer::getMappingPtr(ub1);
        auto* ub2data = (InstanceConstants*) (hByte*)hRenderer::getMappingPtr(ub2.get());

        hMatrix v_inverse = inverse(v);
        hMatrix vp = p1*v;
        ub1data1->view                 = v;
        ub1data1->viewInverse          = inverse(v);
        ub1data1->viewInverseTranspose = transpose(v_inverse);
        ub1data1->projection           = p1;
        ub1data1->projectionInverse    = inverse(p1);
        ub1data1->viewProjection       = p1*v;
        ub1data1->viewProjectionInverse= inverse(vp);
        ub1data1->viewportSize         = hVec4(1280.f, 720.f, 1.f/1280.f, 1.f/720.f);

        ub2data->world = w;

        hRenderer::flushUnibufferMemoryRange(cl, ub1, 0, sizeof(ViewportConstants));
        hRenderer::flushUnibufferMemoryRange(cl, ub2.get(), 0, sizeof(InstanceConstants));
        hRenderer::clear(cl, hColour(0.f, 0.f, 0.f, 1.f), 1.f);
        hRenderer::setTextureOverride(cl, mat_is.get(), 0, testTextures[currentTex].t);
        hRenderer::draw(cl, mat_pls.get(), mat_is.get(), hRenderer::Primative::Triangles, 12, 0);
        fences[currentFence] = hRenderer::fence(cl);
        currentFence = (currentFence+1)%FENCE_COUNT;
        return cl;
    }
};

DEFINE_HEART_UNIT_TEST(TexturedCube);
