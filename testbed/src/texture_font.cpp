/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#include "base/hTypes.h"
#include "base/hBase64.h"
#include "base/hClock.h"
#include "base/hStringUtil.h"
#include "render/hFont.h"
#include "render/hRenderCallDesc.h"
#include "render/hRenderer.h"
#include "render/hVertexBufferLayout.h"
#include "render/hRenderPrim.h"
#include "render/hUniformBufferFlags.h"
#include "render/hProgramReflectionInfo.h"
#include "UnitTestFactory.h"
#include "render/hMipDesc.h"

struct TimerBlock { 
	float timeSecs;
	float pad[3];
};

namespace Heart {
class hHeartEngine;
}

using namespace Heart;
namespace {
const hChar vertSrc[] = 
"\
#version 330\n\
layout(location=0) in vec3 in_position;\n\
layout(location=1) in vec2 in_uv;\n\
out vec2 inout_uv;\n\
void main() {\n\
    inout_uv = in_uv;\n\
    gl_Position.xyz = in_position.xyz/vec3(1280*.3,720*.3,1);\n\
    gl_Position.w = 1;\n\
}\n\
";

const hChar vert2Src[] = 
"\
#version 330\n\
layout(location=0) in vec3 in_position;\n\
layout(location=1) in vec2 in_uv;\n\
out vec2 inout_uv;\n\
void main() {\n\
    inout_uv = in_uv;\n\
    gl_Position.xyz = in_position.xyz;\n\
    gl_Position.w = 1;\n\
}\n\
";

const hChar fragSrc[] = 
"\
#version 330\n\
in vec2 inout_uv;\n\
uniform sampler2D font_sampler;\n\
out vec4 fragColour;\n\
void main() {\n\
    fragColour.rgb = texture(font_sampler, inout_uv.xy).rrr;\n\
    //fragColour.rg = inout_uv.xy;\n\
    fragColour.a = 1;\n\
}\n\
";

const hChar testString[] = "Hello World!";
}

class TextureFont : public IUnitTest {
    
	hTimer								timer_;
	hRenderer::hShaderStage*			vert;
    hRenderer::hShaderStage*			vertatlas;
    hRenderer::hShaderStage*			frag;
    hRenderer::hVertexBuffer*			vb;
    hRenderer::hVertexBuffer*			vbatlas;
    hRenderer::hRenderCall*				rc;
    hRenderer::hRenderCall*				rcatlas;
	hRenderer::hTexture2D*			    t2d;
    hTTFFontFace*                       fontFace;
    hFontRenderCache                    fontCache;

public:
    TextureFont( Heart::hHeartEngine* engine ) 
        : IUnitTest( engine )
        , vert(nullptr)
        , frag(nullptr)
        , vb(nullptr)
        , rc(nullptr)
        , t2d(nullptr)
        , fontFace(nullptr) {
        hResourceManager::loadPackage("fonts");
    }

    void createResources() {
        fontCache.initialise();

        static const hUint element_count = hStaticArraySizeConstExpr(testString)*6;
        hFloat verts[element_count*5] = { .0f };
        hRenderer::hVertexBufferLayout lo[] = {
            {hStringID("in_position"), 3, hRenderer::hVertexInputType::Float,				 0, hFalse, sizeof(hFloat)*5},
            {hStringID("in_uv")      , 2, hRenderer::hVertexInputType::Float, sizeof(hFloat)*3, hFalse, sizeof(hFloat)*5},
        };

        // warm up the cache and get the data we need
        // this is not a test of the dynamic cache yet...
        auto cursor = 0.f;
        auto* vert_ptr = verts;
        fontFace->setPixelSize(48);
        for (const auto& i : testString) {
            auto* glyph = fontCache.getCachedGlyphBitmap(fontFace, i);
            hcAssert(glyph); // there should be no issues with cache running out of space
            // tri 1
            vert_ptr[0]  = cursor;                       vert_ptr[1]  = glyph->height; 
                vert_ptr[3] = glyph->uv_[0]; vert_ptr[4] = glyph->uv_[1]; 
            vert_ptr[5]  = cursor+glyph->width; vert_ptr[6]  = glyph->height; 
                vert_ptr[8] = glyph->uv_[2]; vert_ptr[9] = glyph->uv_[1]; 
            vert_ptr[10] = cursor;                       vert_ptr[11] = 0.f;
                vert_ptr[13] = glyph->uv_[0]; vert_ptr[14] = glyph->uv_[3]; 
            // tri 2
            vert_ptr[15] = cursor;                          vert_ptr[16] = 0.f;
                vert_ptr[18] = glyph->uv_[0]; vert_ptr[19] = glyph->uv_[3]; 
            vert_ptr[20] = cursor + glyph->width;  vert_ptr[21] = (hFloat)glyph->height;
                vert_ptr[23] = glyph->uv_[2]; vert_ptr[24] = glyph->uv_[1]; 
            vert_ptr[25] = cursor + glyph->width;  vert_ptr[26] = 0.f;
                vert_ptr[28] = glyph->uv_[2]; vert_ptr[29] = glyph->uv_[3]; 
            vert_ptr += 30;
            cursor += (hFloat)(glyph->advanceX);
        }
        hRenderer::hMipDesc fnt_cache_mips;
        hUint cw, ch;
        fnt_cache_mips.data = (const hByte*)fontCache.getTextureData(&cw, &ch);
        fnt_cache_mips.width = cw;
        fnt_cache_mips.height = ch;
        fnt_cache_mips.size = cw*ch;

        vert = hRenderer::compileShaderStageFromSource(vertSrc, hStrLen(vertSrc), "main", hShaderProfile::ES2_vs);
        frag = hRenderer::compileShaderStageFromSource(fragSrc, hStrLen(fragSrc), "main", hShaderProfile::ES2_ps);
        vb   = hRenderer::createVertexBuffer(verts, sizeof(hFloat)*5, element_count, 0);
        t2d  = hRenderer::createTexture2D(1, &fnt_cache_mips, hTextureFormat::R8_unorm, 0);

        hRenderer::hRenderCallDesc::hSamplerStateDesc font_sampler_desc;

        hRenderer::hRenderCallDesc rcd;
        rcd.vertex_ = vert;
        rcd.fragment_ = frag;
        rcd.vertexBuffer_ = vb;
        rcd.setSampler(hStringID("font_sampler"), font_sampler_desc);
        rcd.setTextureSlot(hStringID("font_sampler"), t2d);
        rcd.setVertexBufferLayout(lo, 2);
        rc = hRenderer::createRenderCall(rcd);

        hFloat altas_verts[] = {
            -1.f, 1.f, 0.f, /*uv*/0.f, 0.f,
             0.f, 1.f, 0.f, /*uv*/1.f, 0.f,
            -1.f, 0.f, 0.f, /*uv*/0.f, 1.f,

            -1.f, 0.f, 0.f, /*uv*/0.f, 1.f,
             0.f, 1.f, 0.f, /*uv*/1.f, 0.f,
             0.f, 0.f, 0.f, /*uv*/1.f, 1.f,
        };
        vertatlas = hRenderer::compileShaderStageFromSource(vert2Src, hStrLen(vert2Src), "main", hShaderProfile::ES2_vs);
        vbatlas = hRenderer::createVertexBuffer(altas_verts, sizeof(hFloat)*5, 6, 0);

        hUint8 test_tex_data[] = {
            0xFF, 0x0, 0x0, 0xFF, 0x0, 0xFF, 0x0, 0xFF, 
            0x0, 0x0, 0xFF, 0xFF, 0xFF,0xFF,0xFF, 0xFF,
        };
        fnt_cache_mips.data = test_tex_data;
        fnt_cache_mips.width = 2;
        fnt_cache_mips.height = 2;
        fnt_cache_mips.size = 2*2*4;

        rcd.vertex_ = vertatlas;
        rcd.vertexBuffer_ = vbatlas;
        rcatlas = hRenderer::createRenderCall(rcd);

		timer_.reset();
		timer_.setPause(hFalse);

        SetCanRender(hTrue);
    }
    ~TextureFont() {
        if (fontFace) {
            fontFace = nullptr;
            hRenderer::destroyRenderCall(rcatlas);
            hRenderer::destroyVertexBuffer(vbatlas);
            hRenderer::destroyShader(vertatlas);
            hRenderer::destroyRenderCall(rc);
            hRenderer::destroyTexture2D(t2d);
            hRenderer::destroyVertexBuffer(vb);
            hRenderer::destroyShader(frag);
            hRenderer::destroyShader(vert);

            hResourceManager::unloadPackage("fonts");
        }
    }

    virtual hUint32 RunUnitTest() override {

        if (!fontFace && hResourceManager::getIsPackageLoaded("fonts")) {
            fontFace = hResourceManager::weakResource<hTTFFontFace>(hStringID("/fonts/cour"));
            createResources();
            timer_.reset();
        }
        if (fontFace) {
             if (timer_.elapsedMilliSec() > 10*1000 || getForceExitFlag()) {
                 SetExitCode(UNIT_TEST_EXIT_CODE_OK);
             }
        }

        return 0;
    }

    void RenderUnitTest() override {
        auto* cl = hRenderer::createCmdList();
        hRenderer::clear(cl, hColour(0.f, 0.f, 0.f, 1.f), 1.f);
        hRenderer::draw(cl, rc, hRenderer::Primative::Triangles, hStaticArraySizeConstExpr(testString)*2);
        hRenderer::draw(cl, rcatlas, hRenderer::Primative::Triangles, 2);
        hRenderer::swapBuffers(cl);
        
        hRenderer::submitFrame(cl);
    }
};

DEFINE_HEART_UNIT_TEST(TextureFont);