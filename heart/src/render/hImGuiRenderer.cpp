/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#include "base/hTypes.h"
#include "core/hSystem.h"
#include "core/hResourceManager.h"
#include "render/hPipelineStateDesc.h"
#include "render/hMipDesc.h"
#include "render/hRenderer.h"
#include "render/hVertexBufferLayout.h"
#include "render/hRenderPrim.h"
#include "render/hUniformBufferFlags.h"
#include "render/hProgramReflectionInfo.h"
#include "render/hImGuiRenderer.h"
#include "render/hMaterial.h"
#include "render/hUniformBufferResource.h"
#include "input/hActionManager.h"
#include "imgui.h"

#include "shaders/ImguiConstants.hpp"

namespace Heart {
namespace {
struct ImGuiCtx {
    static hUint VERTEX_BUFFER_SIZE; // From ImGui Samples
    static hUint FENCE_COUNT;
    hMaterial* material;
    hRenderer::hVertexBuffer* vb;
    hRenderer::hUniformBuffer* ub;
    hRenderer::hPipelineState* pls;
    hRenderer::hInputState* is;
    hRenderer::hTexture2DUniquePtr fonttex;
    std::vector<hRenderer::hRenderFence*> fences;
    hUint currentFence;
    hRenderer::hCmdList* lastCmdList;
} g_imguiCtx;

hUint ImGuiCtx::VERTEX_BUFFER_SIZE; // From ImGui Samples
hUint ImGuiCtx::FENCE_COUNT;


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
    vtx_start += (ImGuiCtx::VERTEX_BUFFER_SIZE*imguiCtx.currentFence);
    auto* vtx_dst = vtx_start;
    for (int n = 0; n < cmd_lists_count; n++)
    {
        const ImDrawList* cmd_list = cmd_lists[n];
        memcpy(vtx_dst, &cmd_list->vtx_buffer[0], cmd_list->vtx_buffer.size() * sizeof(ImDrawVert));
        vtx_dst += cmd_list->vtx_buffer.size();
    }
    auto vtx_bytes = (hPtrdiff_t)vtx_dst-(hPtrdiff_t)vtx_start;
    hRenderer::flushVertexBufferMemoryRange(gfx_cmd_list, imguiCtx.vb, (ImGuiCtx::VERTEX_BUFFER_SIZE*imguiCtx.currentFence)*sizeof(ImDrawVert), (hUint32)vtx_bytes);

    auto ubsize = (hUint)sizeof(ImguiConstants);
    auto* ubptr = (hByte*)hRenderer::getMappingPtr(imguiCtx.ub);
    auto* ubdata = (ImguiConstants*) (ubptr);
    const float L = 0.0f;
    const float R = ImGui::GetIO().DisplaySize.x;
    const float B = ImGui::GetIO().DisplaySize.y;
    const float T = 0.0f;
    ubdata->projection = hRenderer::getClipspaceMatrix()*hMatrix::orthographic(L, R, B, T, 0.f, 1.f);
    hRenderer::flushUnibufferMemoryRange(gfx_cmd_list, imguiCtx.ub, 0, ubsize);

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
                hRenderer::scissorRect(gfx_cmd_list, (hUint)pcmd->clip_rect.x, (hUint)(pcmd->clip_rect.w), (hUint)pcmd->clip_rect.z, (hUint)(pcmd->clip_rect.y));
                hRenderer::draw(gfx_cmd_list, imguiCtx.pls, imguiCtx.is, hRenderer::Primative::Triangles, pcmd->vtx_count/3, vtx_offset);
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
    ImGuiCtx::VERTEX_BUFFER_SIZE = hConfigurationVariables::getCVarUint("imgui.vertex_buffer_size", 30000);
    ImGuiCtx::FENCE_COUNT = g_RenderFenceCount;
    g_imguiCtx.fences.resize(ImGuiCtx::FENCE_COUNT);
    auto* io = &ImGui::GetIO();
    io->MemAllocFn = [](size_t x) {
        return hMalloc(x);
    };
    io->MemFreeFn = hFree;
    unsigned char* pixels;
    int width, height, bbp;
    io->Fonts->GetTexDataAsRGBA32(&pixels, &width, &height, &bbp);
    //imguiCtx.
    hRenderer::hMipDesc mip = {(hUint32)width, (hUint32)height, pixels, (hUint32)(width*height*bbp)};
    imguiCtx.fonttex.reset(hRenderer::createTexture2D(1, &mip, hTextureFormat::RGBA8_unorm, 0));

    hRenderer::hVertexBufferLayout lo[] = {
        { hStringID("in_position"), hRenderer::hSemantic::Position, 0, 2, hRenderer::hVertexInputType::Float,                  0, hFalse, sizeof(ImDrawVert) },
        { hStringID("in_uv")      , hRenderer::hSemantic::Texcoord, 1, 2, hRenderer::hVertexInputType::Float, sizeof(hFloat) * 2, hFalse, sizeof(ImDrawVert) },
        { hStringID("in_colour")  , hRenderer::hSemantic::Texcoord, 0, 4, hRenderer::hVertexInputType::UByte, sizeof(hFloat) * 4, hTrue , sizeof(ImDrawVert) },
    };
    imguiCtx.material = hResourceManager::weakResource<hMaterial>(hStringID("/system/imgui_material"));
    imguiCtx.vb = hRenderer::createVertexBuffer(nullptr, sizeof(ImDrawVert), ImGuiCtx::VERTEX_BUFFER_SIZE*ImGuiCtx::FENCE_COUNT, (hUint32)hRenderer::hUniformBufferFlags::Dynamic);
    imguiCtx.ub = hResourceManager::weakResource<hUniformBufferResource>(hStringID("/system/ImguiConstants"))->getSharedUniformBuffer();
    imguiCtx.currentFence = 0;

    hcAssertMsg(imguiCtx.material->getTechniqueCount() == 1 && imguiCtx.material->getTechniquePassCount(0) == 1, "Material \"/system/imgui_material\" should have one technique with one pass");
    hRenderer::hPipelineStateDesc plsd = imguiCtx.material->getTechniquePassPipelineStateDesc(0, 0);
    hRenderer::hInputStateDesc isd = imguiCtx.material->getTechniquePassInputStateDesc(0, 0);
    plsd.vertexBuffer_ = imguiCtx.vb;
    plsd.setVertexBufferLayout(lo, (hUint)hStaticArraySize(lo));
    imguiCtx.pls = hRenderer::createRenderPipelineState(plsd);
    isd.setTextureSlotWithOverride(hStringID("t_tex2D"), imguiCtx.fonttex.get(), 0);
    imguiCtx.is = hRenderer::createRenderInputState(isd, plsd);

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
