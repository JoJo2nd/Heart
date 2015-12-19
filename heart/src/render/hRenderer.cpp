/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/
#include "render/hRenderer.h"
#include "base/hSysCalls.h"

#if !HEART_STATIC_RENDER_API
#   define HEART_MODULE_API_ENTRY(r, fn_name) r (HEART_API *fn_name)
#endif

namespace Heart {
namespace hRenderer {

#if !HEART_STATIC_RENDER_API
    
    HEART_MODULE_API_ENTRY(void, create)(hSystem* pSystem, hUint32 width, hUint32 height, hUint32 bpp, hFloat shaderVersion, hBool fullscreen, hBool vsync) = nullptr;
    HEART_MODULE_API_ENTRY(void, destroy)() = nullptr;
    HEART_MODULE_API_ENTRY(hFloat, getRatio)() = nullptr;
    HEART_MODULE_API_ENTRY(bool, isProfileSupported)(hShaderProfile profile) = nullptr;
    HEART_MODULE_API_ENTRY(hShaderProfile, getActiveProfile)(hShaderFrequency freq) = nullptr;
    HEART_MODULE_API_ENTRY(const hRenderFrameStats*, getRenderStats)() = nullptr;
    HEART_MODULE_API_ENTRY(hShaderStage*, compileShaderStageFromSource)(const hChar* shaderProg, hUint32 len, const hChar* entry, hShaderProfile profile) = nullptr;
    HEART_MODULE_API_ENTRY(void, destroyShader)(hShaderStage* prog) = nullptr;
    HEART_MODULE_API_ENTRY(hTexture2D*, createTexture2D)(hUint32 levels, hMipDesc* initialData, hTextureFormat format, hUint32 flags) = nullptr;
    HEART_MODULE_API_ENTRY(void*, getTexture2DMappingPtr)(hTexture2D* t, hUint mip_level) = nullptr;
    HEART_MODULE_API_ENTRY(void, destroyTexture2D)(hTexture2D* t) = nullptr;
    HEART_MODULE_API_ENTRY(hIndexBuffer*, createIndexBuffer)(const void* pIndices, hUint32 nIndices, hUint32 flags) = nullptr;
    HEART_MODULE_API_ENTRY(void*, getIndexBufferMappingPtr)(hIndexBuffer* vb) = nullptr;
    HEART_MODULE_API_ENTRY(void, destroyIndexBuffer)(hIndexBuffer* ib) = nullptr;
    HEART_MODULE_API_ENTRY(hVertexBuffer*,  createVertexBuffer)(const void* initData, hUint32 elementsize, hUint32 elementcount, hUint32 flags) = nullptr;
    HEART_MODULE_API_ENTRY(void*, getVertexBufferMappingPtr)(hVertexBuffer* vb, hUint32* size) = nullptr;
    HEART_MODULE_API_ENTRY(void,  destroyVertexBuffer)(hVertexBuffer* vb) = nullptr;
    HEART_MODULE_API_ENTRY(hUniformBuffer*, createUniformBuffer)(const void* initdata, const hUniformLayoutDesc* layout, hUint layout_count, hUint structSize, hUint bufferCount, hUint32 flags) = nullptr;
    HEART_MODULE_API_ENTRY(const hUniformLayoutDesc*, getUniformBufferLayoutInfo)(const hUniformBuffer* ub, hUint* out_count) = nullptr;
    HEART_MODULE_API_ENTRY(void*, getUniformBufferMappingPtr)(hUniformBuffer* ub) = nullptr;
    HEART_MODULE_API_ENTRY(void, destroyUniformBuffer)(hUniformBuffer* ub) = nullptr;
    HEART_MODULE_API_ENTRY(hPipelineState*, createRenderPipelineState)(const hPipelineStateDescBase& plsd) = nullptr;
    HEART_MODULE_API_ENTRY(void, destroyRenderPipelineState)(hPipelineState* pls) = nullptr;
    HEART_MODULE_API_ENTRY(hInputState*, createRenderInputState)(const hInputStateDescBase& isd, const hPipelineStateDescBase& plsd) = nullptr;
    HEART_MODULE_API_ENTRY(void, destroyRenderInputState)(hInputState* is) = nullptr;
    HEART_MODULE_API_ENTRY(hRenderTarget*, createRenderTarget)(hTexture2D* target, hUint mip) = nullptr;
    HEART_MODULE_API_ENTRY(void, destroyRenderTarget)(hRenderTarget*) = nullptr;
    HEART_MODULE_API_ENTRY(void*, allocTempRenderMemory)( hUint32 size ) = nullptr;
    HEART_MODULE_API_ENTRY(hCmdList*, createCmdList)() = nullptr;
    HEART_MODULE_API_ENTRY(void, linkCmdLists)(hCmdList* before, hCmdList* after, hCmdList* i) = nullptr;
    HEART_MODULE_API_ENTRY(void, detachCmdLists)(hCmdList* i) = nullptr;
    HEART_MODULE_API_ENTRY(hCmdList*, nextCmdList)(hCmdList* i) = nullptr;
    HEART_MODULE_API_ENTRY(void, clear)(hCmdList* cl, hColour colour, hFloat depth) = nullptr;
    HEART_MODULE_API_ENTRY(void, setViewport)(hCmdList* cl, hUint x, hUint y, hUint width, hUint height, hFloat minz, hFloat maxz) = nullptr;
    HEART_MODULE_API_ENTRY(void, setRenderTargets)(hCmdList* cl, hRenderTarget** targets, hUint count) = nullptr;
    HEART_MODULE_API_ENTRY(void, scissorRect)(hCmdList* cl, hUint left, hUint top, hUint right, hUint bottom) = nullptr;
    HEART_MODULE_API_ENTRY(void, setTextureOverride)(hCmdList* cl, hInputState* is, hUint32 slot, hTexture2D* tex) = nullptr;
    HEART_MODULE_API_ENTRY(void, draw)(hCmdList* cl, hPipelineState* pls, hInputState* is, Primative t, hUint prims, hUint vtx_offset) = nullptr;
    HEART_MODULE_API_ENTRY(void, flushUnibufferMemoryRange)(hCmdList* cl, hUniformBuffer* ub, hUint offset, hUint size) = nullptr;
    HEART_MODULE_API_ENTRY(void, flushVertexBufferMemoryRange)(hCmdList* cl, hVertexBuffer* ub, hUint offset, hUint size) = nullptr;
    HEART_MODULE_API_ENTRY(void, flushTexture2DMemoryRange)(hCmdList* cl, hTexture2D* ub, hUint mip_level, hUint offset, hUint size) = nullptr;
    HEART_MODULE_API_ENTRY(hRenderFence*, fence)(hCmdList* cl) = nullptr;
    HEART_MODULE_API_ENTRY(void, wait)(hRenderFence* fence) = nullptr;
    HEART_MODULE_API_ENTRY(void, flush)(hCmdList* cl) = nullptr;
    HEART_MODULE_API_ENTRY(void, finish)() = nullptr;
    HEART_MODULE_API_ENTRY(void, call)(hCmdList* cl, hCmdList* tocall) = nullptr;
    HEART_MODULE_API_ENTRY(void, endReturn)(hCmdList* cl) = nullptr;
    HEART_MODULE_API_ENTRY(void, swapBuffers)(hCmdList* cl) = nullptr;
    HEART_MODULE_API_ENTRY(void, submitFrame)(hCmdList* cmds) = nullptr;
    HEART_MODULE_API_ENTRY(void, rendererFrameSubmit)(hCmdList* cmdlists, hUint count) = nullptr;
    HEART_MODULE_API_ENTRY(hFloat, getLastGPUTime)() = nullptr;
    HEART_MODULE_API_ENTRY(hBool, isRenderThread)() = nullptr;
    HEART_MODULE_API_ENTRY(void, getPlatformClipMatrix)(hFloat*) = nullptr;
    HEART_MODULE_API_ENTRY(hProgramReflectionInfo*, createProgramReflectionInfo)(hShaderStage* vertex, hShaderStage* pixel, hShaderStage* geom, hShaderStage* hull, hShaderStage* domain) = nullptr;
    HEART_MODULE_API_ENTRY(void, destroyProgramReflectionInfo)(hProgramReflectionInfo* p) = nullptr;
    HEART_MODULE_API_ENTRY(hShaderParamInfo, getParameterInfo)(hProgramReflectionInfo* p, const hChar* name) = nullptr;
    HEART_MODULE_API_ENTRY(hUint, getUniformBlockCount)(hProgramReflectionInfo* p) = nullptr;
    HEART_MODULE_API_ENTRY(hUniformBlockInfo, getUniformBlockInfo)(hProgramReflectionInfo* p, hUint i) = nullptr;
    HEART_MODULE_API_ENTRY(hUint, getParameterTypeByteSize)(ShaderParamType type) = nullptr;
    HEART_MODULE_API_ENTRY(hUint, getTextureFormatBytesPerPixel)(hTextureFormat) = nullptr;

    void loadRendererModule(const hChar* module_name, hIConfigurationVariables* cvars) {
        auto lib = hSysCall::openSharedLib(module_name);
        auto success = true;
        success &= hSysCall::getFunctionAddress(lib, "create", hRenderer::create) != nullptr;
        success &= hSysCall::getFunctionAddress(lib, "destroy", hRenderer::destroy) != nullptr;
        success &= hSysCall::getFunctionAddress(lib, "isProfileSupported", hRenderer::isProfileSupported) != nullptr;
        success &= hSysCall::getFunctionAddress(lib, "getActiveProfile", hRenderer::getActiveProfile) != nullptr;
        success &= hSysCall::getFunctionAddress(lib, "getRenderStats", hRenderer::getRenderStats) != nullptr;
        success &= hSysCall::getFunctionAddress(lib, "compileShaderStageFromSource", hRenderer::compileShaderStageFromSource) != nullptr;
        success &= hSysCall::getFunctionAddress(lib, "destroyShader", hRenderer::destroyShader) != nullptr;
        success &= hSysCall::getFunctionAddress(lib, "createTexture2D", hRenderer::createTexture2D) != nullptr;
        success &= hSysCall::getFunctionAddress(lib, "getTexture2DMappingPtr", hRenderer::getTexture2DMappingPtr) != nullptr;
        success &= hSysCall::getFunctionAddress(lib, "destroyTexture2D", hRenderer::destroyTexture2D) != nullptr;
        success &= hSysCall::getFunctionAddress(lib, "createIndexBuffer", hRenderer::createIndexBuffer) != nullptr;
        success &= hSysCall::getFunctionAddress(lib, "getIndexBufferMappingPtr", hRenderer::getIndexBufferMappingPtr) != nullptr;
        success &= hSysCall::getFunctionAddress(lib, "destroyIndexBuffer", hRenderer::destroyIndexBuffer) != nullptr;
        success &= hSysCall::getFunctionAddress(lib, "createVertexBuffer", hRenderer::createVertexBuffer) != nullptr;
        success &= hSysCall::getFunctionAddress(lib, "getVertexBufferMappingPtr", hRenderer::getVertexBufferMappingPtr) != nullptr;
        success &= hSysCall::getFunctionAddress(lib, "destroyVertexBuffer", hRenderer::destroyVertexBuffer) != nullptr;
        success &= hSysCall::getFunctionAddress(lib, "createUniformBuffer", hRenderer::createUniformBuffer) != nullptr;
        success &= hSysCall::getFunctionAddress(lib, "getUniformBufferLayoutInfo", hRenderer::getUniformBufferLayoutInfo) != nullptr;
        success &= hSysCall::getFunctionAddress(lib, "getUniformBufferMappingPtr", hRenderer::getUniformBufferMappingPtr) != nullptr;
        success &= hSysCall::getFunctionAddress(lib, "destroyUniformBuffer", hRenderer::destroyUniformBuffer) != nullptr;
        success &= hSysCall::getFunctionAddress(lib, "createRenderPipelineState", hRenderer::createRenderPipelineState) != nullptr;
        success &= hSysCall::getFunctionAddress(lib, "destroyRenderPipelineState", hRenderer::destroyRenderPipelineState) != nullptr;
        success &= hSysCall::getFunctionAddress(lib, "createRenderInputState", hRenderer::createRenderInputState) != nullptr;
        success &= hSysCall::getFunctionAddress(lib, "destroyRenderInputState", hRenderer::destroyRenderInputState) != nullptr;
        success &= hSysCall::getFunctionAddress(lib, "createRenderTarget", hRenderer::createRenderTarget) != nullptr;
        success &= hSysCall::getFunctionAddress(lib, "destroyRenderTarget", hRenderer::destroyRenderTarget) != nullptr;
        success &= hSysCall::getFunctionAddress(lib, "allocTempRenderMemory", hRenderer::allocTempRenderMemory) != nullptr;
        success &= hSysCall::getFunctionAddress(lib, "createCmdList", hRenderer::createCmdList) != nullptr;
        success &= hSysCall::getFunctionAddress(lib, "linkCmdLists", hRenderer::linkCmdLists) != nullptr;
        success &= hSysCall::getFunctionAddress(lib, "detachCmdLists", hRenderer::detachCmdLists) != nullptr;
        success &= hSysCall::getFunctionAddress(lib, "nextCmdList", hRenderer::nextCmdList) != nullptr;
        success &= hSysCall::getFunctionAddress(lib, "clear", hRenderer::clear) != nullptr;
        success &= hSysCall::getFunctionAddress(lib, "setViewport", hRenderer::setViewport) != nullptr;
        success &= hSysCall::getFunctionAddress(lib, "setRenderTargets", hRenderer::setRenderTargets) != nullptr;
        success &= hSysCall::getFunctionAddress(lib, "scissorRect", hRenderer::scissorRect) != nullptr;
        success &= hSysCall::getFunctionAddress(lib, "setTextureOverride", hRenderer::setTextureOverride) != nullptr;
        success &= hSysCall::getFunctionAddress(lib, "draw", hRenderer::draw) != nullptr;
        success &= hSysCall::getFunctionAddress(lib, "flushUnibufferMemoryRange", hRenderer::flushUnibufferMemoryRange) != nullptr;
        success &= hSysCall::getFunctionAddress(lib, "flushVertexBufferMemoryRange", hRenderer::flushVertexBufferMemoryRange) != nullptr;
        success &= hSysCall::getFunctionAddress(lib, "flushTexture2DMemoryRange", hRenderer::flushTexture2DMemoryRange) != nullptr;
        success &= hSysCall::getFunctionAddress(lib, "fence", hRenderer::fence) != nullptr;
        success &= hSysCall::getFunctionAddress(lib, "wait", hRenderer::wait) != nullptr;
        success &= hSysCall::getFunctionAddress(lib, "flush", hRenderer::flush) != nullptr;
        success &= hSysCall::getFunctionAddress(lib, "finish", hRenderer::finish) != nullptr;
        success &= hSysCall::getFunctionAddress(lib, "call", hRenderer::call) != nullptr;
        success &= hSysCall::getFunctionAddress(lib, "endReturn", hRenderer::endReturn) != nullptr;
        success &= hSysCall::getFunctionAddress(lib, "swapBuffers", hRenderer::swapBuffers) != nullptr;
        success &= hSysCall::getFunctionAddress(lib, "submitFrame", hRenderer::submitFrame) != nullptr;
        success &= hSysCall::getFunctionAddress(lib, "rendererFrameSubmit", hRenderer::rendererFrameSubmit) != nullptr;
        success &= hSysCall::getFunctionAddress(lib, "getLastGPUTime", hRenderer::getLastGPUTime) != nullptr;
        success &= hSysCall::getFunctionAddress(lib, "isRenderThread", hRenderer::isRenderThread) != nullptr;
        success &= hSysCall::getFunctionAddress(lib, "getPlatformClipMatrix", hRenderer::getPlatformClipMatrix) != nullptr;
        success &= hSysCall::getFunctionAddress(lib, "getTextureFormatBytesPerPixel", hRenderer::getTextureFormatBytesPerPixel) != nullptr;
        success &= hSysCall::getFunctionAddress(lib, "getParameterTypeByteSize", hRenderer::getParameterTypeByteSize) != nullptr;
        void (*initialiseRendererPlugin)(hIConfigurationVariables*);
        success &= hSysCall::getFunctionAddress(lib, "initialiseRendererPlugin", initialiseRendererPlugin) != nullptr;
        hcAssertMsg(success, "Failed to load renderer plug-in.");
        initialiseRendererPlugin(cvars);
    }

#endif
}
}

