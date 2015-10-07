/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#pragma once

#include "base/hTypes.h"
#include "base/hFunctor.h"
#include "base/hClock.h"
#include "base/hLinkedList.h"
#include "base/hArray.h"
#include "base/hRendererConstants.h"
#include "core/hResource.h"
#include "math/hMatrix.h"
#include "math/hVec3.h"
#include "math/hVec4.h"
#include "render/hRenderStateBlock.h"
#include "render/hRenderSubmissionContext.h"
#include "render/hRendererCamera.h"
#include "render/hTextureFormat.h"
#include "render/hRenderPrim.h"
#include "render/hProgramReflectionInfo.h"

#ifndef HEART_STATIC_RENDER_API
#   define HEART_STATIC_RENDER_API (0)
#endif

#if !HEART_STATIC_RENDER_API
#   define HEART_MODULE_API(r, fn_name) extern r (HEART_API *fn_name)
#else
#   define HEART_MODULE_API(r, fn_name) r fn_name
#endif

namespace Heart {

class hShaderProgram;


#define HEART_DEBUG_CAMERA_ID (13)
#define HEART_DEBUGUI_CAMERA_ID (14)

hFUNCTOR_TYPEDEF(void (*)(const hChar*, void**, hUint32*), hShaderIncludeCallback);
hFUNCTOR_TYPEDEF(void (*)(hRenderSubmissionCtx*), hCustomRenderCallback);

class hSystem;

namespace hRenderer {

    struct hCmdList;
    struct hRenderCall;
    struct hRenderCallDesc;
    struct hMipDesc;
    struct hTexture1D;
    struct hTexture2D;
    struct hTexture3D;
    struct hShaderStage;
    struct hIndexBuffer;
    struct hVertexBuffer;
    struct hUniformBuffer;
	struct hRenderFence;

    HEART_MODULE_API(void, create)(hSystem* pSystem, hUint32 width, hUint32 height, hUint32 bpp, hFloat shaderVersion, hBool fullscreen, hBool vsync);
    HEART_MODULE_API(void, destroy)();
    HEART_MODULE_API(hFloat, getRatio)();
    HEART_MODULE_API(bool, isProfileSupported)(hShaderProfile profile);
    HEART_MODULE_API(hShaderProfile, getActiveProfile)(hShaderFrequency);
    HEART_MODULE_API(const hRenderFrameStats*, getRenderStats)();
    HEART_MODULE_API(hShaderStage*, compileShaderStageFromSource)(const hChar* shaderProg, hUint32 len, const hChar* entry, hShaderProfile profile);
    HEART_MODULE_API(void, destroyShader)(hShaderStage* prog);
    HEART_MODULE_API(hTexture2D*, createTexture2D)(hUint32 levels, hMipDesc* initialData, hTextureFormat format, hUint32 flags);
    HEART_MODULE_API(void, destroyTexture2D)(hTexture2D* t);
    HEART_MODULE_API(hIndexBuffer*, createIndexBuffer)(const void* pIndices, hUint32 nIndices, hUint32 flags);
    HEART_MODULE_API(void*, getIndexBufferMappingPtr)(hIndexBuffer* vb);
    HEART_MODULE_API(void, destroyIndexBuffer)(hIndexBuffer* ib);
    HEART_MODULE_API(hVertexBuffer*,  createVertexBuffer)(const void* initData, hUint32 elementsize, hUint32 elementcount, hUint32 flags);
    HEART_MODULE_API(void*, getVertexBufferMappingPtr)(hVertexBuffer* vb, hUint32* size);
    HEART_MODULE_API(void,  destroyVertexBuffer)(hVertexBuffer* vb);
    HEART_MODULE_API(hUniformBuffer*, createUniformBuffer)(const void* initdata, const hUniformLayoutDesc* layout, hUint layout_count, hUint structSize, hUint bufferCount, hUint32 flags);
    HEART_MODULE_API(const hUniformLayoutDesc*, getUniformBufferLayoutInfo)(const hUniformBuffer* ub, hUint* out_count);
    HEART_MODULE_API(void*, getUniformBufferMappingPtr)(hUniformBuffer* ub);
    HEART_MODULE_API(void, destroyUniformBuffer)(hUniformBuffer* ub);
    HEART_MODULE_API(hRenderCall*, createRenderCall)(const hRenderCallDesc& rcd);
    HEART_MODULE_API(void, destroyRenderCall)(hRenderCall* rc);
    HEART_MODULE_API(void*, allocTempRenderMemory)( hUint32 size );
    HEART_MODULE_API(hCmdList*, createCmdList)();
    HEART_MODULE_API(void, linkCmdLists)(hCmdList* before, hCmdList* after, hCmdList* i);
    HEART_MODULE_API(void, detachCmdLists)(hCmdList* i);
    HEART_MODULE_API(hCmdList*, nextCmdList)(hCmdList* i);
    HEART_MODULE_API(void, clear)(hCmdList* cl, hColour colour, hFloat depth);
    HEART_MODULE_API(void, setViewport)(hCmdList* cl, hUint x, hUint y, hUint width, hUint height, hFloat minz, hFloat maxz);
    HEART_MODULE_API(void, scissorRect)(hCmdList* cl, hUint left, hUint top, hUint right, hUint bottom);
    HEART_MODULE_API(void, draw)(hCmdList* cl, hRenderCall* rc, Primative t, hUint prims, hUint vtx_offset);
    HEART_MODULE_API(void, flushUnibufferMemoryRange)(hCmdList* cl, hUniformBuffer* ub, hUint offset, hUint size);
    HEART_MODULE_API(void, flushVertexBufferMemoryRange)(hCmdList* cl, hVertexBuffer* ub, hUint offset, hUint size);
    HEART_MODULE_API(hRenderFence*, fence)(hCmdList* cl);
    HEART_MODULE_API(void, wait)(hRenderFence* fence);
    HEART_MODULE_API(void, flush)(hCmdList* cl);
    HEART_MODULE_API(void, finish)();
    HEART_MODULE_API(void, call)(hCmdList* cl, hCmdList* tocall);
    HEART_MODULE_API(void, endReturn)(hCmdList* cl);
    HEART_MODULE_API(void, swapBuffers)(hCmdList* cl);
    HEART_MODULE_API(void, submitFrame)(hCmdList* cmds);
    HEART_MODULE_API(void, rendererFrameSubmit)(hCmdList* cmdlists, hUint count);
    HEART_MODULE_API(hFloat, getLastGPUTime)();
    HEART_MODULE_API(hBool, isRenderThread)();

    inline void* getMappingPtr(hIndexBuffer* ib) {
        return getIndexBufferMappingPtr(ib);
    }
    inline void* getMappingPtr(hVertexBuffer* vb, hUint32* size) {
        return getVertexBufferMappingPtr(vb, size);
    }
    inline void* getMappingPtr(hUniformBuffer* ub) {
        return getUniformBufferMappingPtr(ub);
    }

#if !HEART_STATIC_RENDER_API
    void initialiseRenderFunc();
#endif
}
}

