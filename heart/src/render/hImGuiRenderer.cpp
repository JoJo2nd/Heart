/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#include "base/hTypes.h"
#include "core/hSystem.h"
#include "render/hRenderCallDesc.h"
#include "render/hMipDesc.h"
#include "render/hRenderer.h"
#include "render/hVertexBufferLayout.h"
#include "render/hRenderPrim.h"
#include "render/hUniformBufferFlags.h"
#include "render/hProgramReflectionInfo.h"
#include "render/hRenderShaderProgram.h"
#include "render/hImGuiRenderer.h"
#include "imgui.h"
#include "input/hActionManager.h"

namespace Heart {
namespace {
struct ImGuiCtx {
    static const hUint VERTEX_BUFFER_SIZE = 30000; // From ImGui Samples
    static const hUint FENCE_COUNT = 3;
    hShaderProgram* shaderProg;
    hRenderer::hShaderStage* vert;
    hRenderer::hShaderStage* pixel;
    hRenderer::hVertexBuffer* vb;
    hRenderer::hUniformBuffer* ub;
    hRenderer::hRenderCall* rc;
    hRenderer::hTexture2D* fonttex;
    hRenderer::hRenderFence* fences[FENCE_COUNT];
    hUint currentFence;
    hRenderer::hCmdList* lastCmdList;
} g_imguiCtx;

struct ImGuiConstBlock {
    hMatrix projection;
};

static const hStringID UISelectActionName("UI Select");
static const hStringID UIXAxisActionName("UI X Axis");
static const hStringID UIYAxisActionName("UI Y Axis");

void ImGuiRenderDrawLists(ImDrawList** const cmd_lists, int cmd_lists_count) {
    auto& imguiCtx = g_imguiCtx; // So the debugger can see it
    hcAssert(imguiCtx.lastCmdList); // Called ImGuiNewFrame
    auto* gfx_cmd_list = imguiCtx.lastCmdList;//hRenderer::createCmdList();

    if (imguiCtx.fences[imguiCtx.currentFence]) {
        hRenderer::wait(imguiCtx.fences[imguiCtx.currentFence]);
        imguiCtx.fences[imguiCtx.currentFence] = nullptr;
    }
    auto* vtx_start = (ImDrawVert*)hRenderer::getMappingPtr(imguiCtx.vb, nullptr);
    auto* vtx_dst = vtx_start;
    vtx_dst += (ImGuiCtx::VERTEX_BUFFER_SIZE*imguiCtx.currentFence);
    for (int n = 0; n < cmd_lists_count; n++)
    {
        const ImDrawList* cmd_list = cmd_lists[n];
        memcpy(vtx_dst, &cmd_list->vtx_buffer[0], cmd_list->vtx_buffer.size() * sizeof(ImDrawVert));
        vtx_dst += cmd_list->vtx_buffer.size();
    }
    auto vtx_bytes = (hPtrdiff_t)vtx_dst-(hPtrdiff_t)vtx_start;
    hRenderer::flushVertexBufferMemoryRange(gfx_cmd_list, imguiCtx.vb, (ImGuiCtx::VERTEX_BUFFER_SIZE*imguiCtx.currentFence)*sizeof(ImDrawVert), (hUint32)vtx_bytes);

    auto ubsize = 0u;
    auto* ubptr = (hByte*)hRenderer::getMappingPtr(imguiCtx.ub);
    auto* ubdata = (ImGuiConstBlock*) (ubptr + (imguiCtx.currentFence*ubsize));
    const float L = 0.0f;
    const float R = ImGui::GetIO().DisplaySize.x;
    const float B = ImGui::GetIO().DisplaySize.y;
    const float T = 0.0f;
    ubdata->projection = hMatrix::orthographic(L, R, B, T, 0.f, 1.f);
    hRenderer::flushUnibufferMemoryRange(gfx_cmd_list, imguiCtx.ub, (imguiCtx.currentFence*ubsize), ubsize);

    // Render command lists
    int vtx_offset = ImGuiCtx::VERTEX_BUFFER_SIZE*imguiCtx.currentFence;
    for (int n = 0; n < cmd_lists_count; n++)
    {
        const ImDrawList* cmd_list = cmd_lists[n];
        for (size_t cmd_i = 0; cmd_i < cmd_list->commands.size(); cmd_i++)
        {
            const ImDrawCmd* pcmd = &cmd_list->commands[cmd_i];
            if (pcmd->user_callback)
            {
                pcmd->user_callback(cmd_list, pcmd);
            }
            else
            {
                //hRenderer::overrideSampler(slot, sampler);
                //hRenderer::overrideTexture(slot, texture);
                hRenderer::scissorRect(gfx_cmd_list, (hUint)pcmd->clip_rect.x, (hUint)(B-pcmd->clip_rect.w), (hUint)pcmd->clip_rect.z, (hUint)(B-pcmd->clip_rect.y));
                hRenderer::draw(gfx_cmd_list, imguiCtx.rc, hRenderer::Primative::Triangles, pcmd->vtx_count/3, vtx_offset);
            }
            vtx_offset += pcmd->vtx_count;
        }
    }
    hRenderer::scissorRect(gfx_cmd_list, 0, 0, (hUint)ImGui::GetIO().DisplaySize.x, (hUint)ImGui::GetIO().DisplaySize.y);
    //update the fence
    imguiCtx.fences[imguiCtx.currentFence] = hRenderer::fence(gfx_cmd_list);
    imguiCtx.currentFence = (imguiCtx.currentFence + 1) % ImGuiCtx::FENCE_COUNT;

    imguiCtx.lastCmdList = nullptr;
}
}

hBool ImGuiInit() {
    auto& imguiCtx = g_imguiCtx; // So the debugger can see it
    auto* io = &ImGui::GetIO();

    unsigned char* pixels;
    int width, height, bbp;
    io->Fonts->GetTexDataAsRGBA32(&pixels, &width, &height, &bbp);
    //imguiCtx.
    hRenderer::hMipDesc mip = {width, height, pixels, width*height*bbp};
    imguiCtx.fonttex = hRenderer::createTexture2D(1, &mip, hTextureFormat::RGBA8_unorm, 0);

    hRenderer::hVertexBufferLayout lo[] = {
        { hStringID("in_position")  , 2, hRenderer::hVertexInputType::Float ,                  0, hFalse, 20 },
        { hStringID("in_uv")        , 2, hRenderer::hVertexInputType::Float , sizeof(hFloat) * 2, hFalse, 20 },
        { hStringID("in_colour")    , 4, hRenderer::hVertexInputType::UByte , sizeof(hFloat) * 4, hTrue , 20 },
    };
    imguiCtx.shaderProg = hResourceManager::weakResource<hShaderProgram>(hStringID("/system/imgui"));
    imguiCtx.vert = imguiCtx.shaderProg->getShader(hShaderProfile::ES2_vs);
    imguiCtx.pixel = imguiCtx.shaderProg->getShader(hShaderProfile::ES2_ps);
    imguiCtx.vb = hRenderer::createVertexBuffer(nullptr, sizeof(hFloat) * 9, ImGuiCtx::VERTEX_BUFFER_SIZE*ImGuiCtx::FENCE_COUNT, (hUint32)hRenderer::hUniformBufferFlags::Dynamic);

    hRenderer::hUniformLayoutDesc ublo[] = {
        { "projection", hRenderer::ShaderParamType::Float44, 0 },
    };

    imguiCtx.ub = hRenderer::createUniformBuffer(nullptr, ublo, (hUint)hStaticArraySize(ublo), (hUint)sizeof(ImGuiConstBlock), ImGuiCtx::FENCE_COUNT, (hUint32)hRenderer::hUniformBufferFlags::Dynamic);
    for (auto& i : imguiCtx.fences) {
        i = nullptr;
    }
    imguiCtx.currentFence = 0;


    hRenderer::hRenderCallDesc rcd;
    rcd.vertex_ = imguiCtx.vert;
    rcd.fragment_ = imguiCtx.pixel;
    rcd.vertexBuffer_ = imguiCtx.vb;
    rcd.blend_.blendEnable_ = hTrue;
    rcd.blend_.srcBlend_ = proto::renderstate::BlendSrcAlpha;
    rcd.blend_.destBlend_ = proto::renderstate::BlendInverseSrcAlpha;
    rcd.blend_.blendOp_ = proto::renderstate::Add;
    rcd.blend_.srcBlendAlpha_ = proto::renderstate::BlendInverseSrcAlpha;
    rcd.blend_.destBlendAlpha_ = proto::renderstate::BlendZero;
    rcd.blend_.blendOpAlpha_ = proto::renderstate::Add;
    rcd.rasterizer_.scissorEnable_ = 1;
    hRenderer::hRenderCallDesc::hSamplerStateDesc ssd;
    ssd.filter_ = proto::renderstate::linear;
    rcd.setSampler(hStringID("t_tex2D"), ssd);
    rcd.setTextureSlot(hStringID("t_tex2D"), imguiCtx.fonttex);
    rcd.setUniformBuffer(hStringID("ParamBlock"), imguiCtx.ub);
    rcd.setVertexBufferLayout(lo, (hUint)hStaticArraySize(lo));
    imguiCtx.rc = hRenderer::createRenderCall(rcd);

    io->RenderDrawListsFn = ImGuiRenderDrawLists;

    return hTrue;
}

void ImGuiShutdown() {
}

hRenderer::hCmdList* ImGuiNewFrame(hSystem* system, hActionManager* action_mgr) {
    auto& imguiCtx = g_imguiCtx; // So the debugger can see it
    auto* io = &ImGui::GetIO();
    io->DisplaySize = ImVec2((hFloat)system->getWindowWidth(), (hFloat)system->getWindowHeight());

    //io->
    hInputAction in_action;
    if (action_mgr->queryAction(0, UISelectActionName, &in_action)) {
        io->MouseDown[0] = in_action.value_ > 0;
    }
    if (action_mgr->queryAction(0, UIXAxisActionName, &in_action)) {
        io->MousePos.x = (hFloat)in_action.absValue;
    }
    if (action_mgr->queryAction(0, UIYAxisActionName, &in_action)) {
        io->MousePos.y = (hFloat)in_action.absValue;
    }

    imguiCtx.lastCmdList = hRenderer::createCmdList();
    ImGui::NewFrame();
    return imguiCtx.lastCmdList;
}

hRenderer::hCmdList* ImGuiCurrentCommandList() {
    auto& imguiCtx = g_imguiCtx;
    return imguiCtx.lastCmdList;
}

}