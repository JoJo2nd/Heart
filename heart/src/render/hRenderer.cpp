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
    HEART_MODULE_API_ENTRY(hRenderCall*, createRenderCall)(const hRenderCallDescBase& rcd) = nullptr;
    HEART_MODULE_API_ENTRY(void, destroyRenderCall)(hRenderCall* rc) = nullptr;
    HEART_MODULE_API_ENTRY(void*, allocTempRenderMemory)( hUint32 size ) = nullptr;
    HEART_MODULE_API_ENTRY(hCmdList*, createCmdList)() = nullptr;
    HEART_MODULE_API_ENTRY(void, linkCmdLists)(hCmdList* before, hCmdList* after, hCmdList* i) = nullptr;
    HEART_MODULE_API_ENTRY(void, detachCmdLists)(hCmdList* i) = nullptr;
    HEART_MODULE_API_ENTRY(hCmdList*, nextCmdList)(hCmdList* i) = nullptr;
    HEART_MODULE_API_ENTRY(void, clear)(hCmdList* cl, hColour colour, hFloat depth) = nullptr;
    HEART_MODULE_API_ENTRY(void, setViewport)(hCmdList* cl, hUint x, hUint y, hUint width, hUint height, hFloat minz, hFloat maxz) = nullptr;
    HEART_MODULE_API_ENTRY(void, scissorRect)(hCmdList* cl, hUint left, hUint top, hUint right, hUint bottom) = nullptr;
    HEART_MODULE_API_ENTRY(void, draw)(hCmdList* cl, hRenderCall* rc, Primative t, hUint prims, hUint vtx_offset) = nullptr;
    HEART_MODULE_API_ENTRY(void, flushUnibufferMemoryRange)(hCmdList* cl, hUniformBuffer* ub, hUint offset, hUint size) = nullptr;
    HEART_MODULE_API_ENTRY(void, flushVertexBufferMemoryRange)(hCmdList* cl, hVertexBuffer* ub, hUint offset, hUint size) = nullptr;
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
    HEART_MODULE_API_ENTRY(hProgramReflectionInfo*, createProgramReflectionInfo)(hShaderStage* vertex, hShaderStage* pixel, hShaderStage* geom, hShaderStage* hull, hShaderStage* domain) = nullptr;
    HEART_MODULE_API_ENTRY(void, destroyProgramReflectionInfo)(hProgramReflectionInfo* p) = nullptr;
    HEART_MODULE_API_ENTRY(hShaderParamInfo, getParameterInfo)(hProgramReflectionInfo* p, const hChar* name) = nullptr;
    HEART_MODULE_API_ENTRY(hUint, getUniformBlockCount)(hProgramReflectionInfo* p) = nullptr;
    HEART_MODULE_API_ENTRY(hUniformBlockInfo, getUniformBlockInfo)(hProgramReflectionInfo* p, hUint i) = nullptr;
    HEART_MODULE_API_ENTRY(hUint, getParameterTypeByteSize)(ShaderParamType type) = nullptr;

    void loadRendererModule(const hChar* module_name, hIConfigurationVariables* cvars, hRendererInterfaceInitializer* out_funcs) {
        auto lib = hSysCall::openSharedLib(module_name);
        void (HEART_API *hrt_initialiseRenderFunc_ptr)(hIConfigurationVariables*, hRendererInterfaceInitializer*) = nullptr;
        auto success = true;
        success &= hSysCall::getFunctionAddress(lib, "hrt_initialiseRenderFunc", hrt_initialiseRenderFunc_ptr) != nullptr;
        hrt_initialiseRenderFunc_ptr(cvars, out_funcs);
    }

    void initialiseRenderFunc(const hRendererInterfaceInitializer& in_funcs) {
        hRenderer::create = in_funcs.create;
        hRenderer::destroy = in_funcs.destroy;
        hRenderer::isProfileSupported = in_funcs.isProfileSupported;
        hRenderer::getActiveProfile = in_funcs.getActiveProfile;
        hRenderer::getRenderStats = in_funcs.getRenderStats;
        hRenderer::compileShaderStageFromSource = in_funcs.compileShaderStageFromSource;
        hRenderer::destroyShader = in_funcs.destroyShader;
        hRenderer::createTexture2D = in_funcs.createTexture2D;
        hRenderer::destroyTexture2D = in_funcs.destroyTexture2D;
        hRenderer::createIndexBuffer = in_funcs.createIndexBuffer;
        hRenderer::getIndexBufferMappingPtr = in_funcs.getIndexBufferMappingPtr;
        hRenderer::destroyIndexBuffer = in_funcs.destroyIndexBuffer;
        hRenderer::createVertexBuffer = in_funcs.createVertexBuffer;
        hRenderer::getVertexBufferMappingPtr = in_funcs.getVertexBufferMappingPtr;
        hRenderer::destroyVertexBuffer = in_funcs.destroyVertexBuffer;
        hRenderer::createUniformBuffer = in_funcs.createUniformBuffer;
        hRenderer::getUniformBufferLayoutInfo = in_funcs.getUniformBufferLayoutInfo;
        hRenderer::getUniformBufferMappingPtr = in_funcs.getUniformBufferMappingPtr;
        hRenderer::destroyUniformBuffer = in_funcs.destroyUniformBuffer;
        hRenderer::createRenderCall = in_funcs.createRenderCall;
        hRenderer::destroyRenderCall = in_funcs.destroyRenderCall;
        hRenderer::allocTempRenderMemory = in_funcs.allocTempRenderMemory;
        hRenderer::createCmdList = in_funcs.createCmdList;
        hRenderer::linkCmdLists = in_funcs.linkCmdLists;
        hRenderer::detachCmdLists = in_funcs.detachCmdLists;
        hRenderer::nextCmdList = in_funcs.nextCmdList;
        hRenderer::clear = in_funcs.clear;
        hRenderer::setViewport = in_funcs.setViewport;
        hRenderer::scissorRect = in_funcs.scissorRect;
        hRenderer::draw = in_funcs.draw;
        hRenderer::flushUnibufferMemoryRange = in_funcs.flushUnibufferMemoryRange;
        hRenderer::flushVertexBufferMemoryRange = in_funcs.flushVertexBufferMemoryRange;
        hRenderer::fence = in_funcs.fence;
        hRenderer::wait = in_funcs.wait;
        hRenderer::flush = in_funcs.flush;
        hRenderer::finish = in_funcs.finish;
        hRenderer::call = in_funcs.call;
        hRenderer::endReturn = in_funcs.endReturn;
        hRenderer::swapBuffers = in_funcs.swapBuffers;
        hRenderer::submitFrame = in_funcs.submitFrame;
        hRenderer::rendererFrameSubmit = in_funcs.rendererFrameSubmit;
        hRenderer::getLastGPUTime = in_funcs.getLastGPUTime;
        hRenderer::isRenderThread = in_funcs.isRenderThread;
        hRenderer::getParameterTypeByteSize = in_funcs.getParameterTypeByteSize;
    }

#endif
}
}

