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

    void					create(hSystem* pSystem, hUint32 width, hUint32 height, hUint32 bpp, hFloat shaderVersion, hBool fullscreen, hBool vsync);
    void					destroy();
    hFORCEINLINE hFloat     getRatio() { return 1.f; }

    ///////////////////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////////

    const hRenderFrameStats* getRenderStats();
    
    hShaderStage* compileShaderStageFromSource(const hChar* shaderProg, hUint32 len, const hChar* entry, hShaderProfile profile);
    hShaderStage* createShaderStage(const hChar* shaderProg, hUint32 len, Heart::hShaderType type);
    void  destroyShader(hShaderProgram* prog);

    hTexture2D*  createTexture2D(hUint32 levels, hMipDesc* initialData, hTextureFormat format, hUint32 flags);
    void  destroyTexture2D(hTexture2D* t);

    hIndexBuffer* createIndexBuffer(const void* pIndices, hUint32 nIndices, hUint32 flags);
    void  destroyIndexBuffer(hIndexBuffer* ib);

    hVertexBuffer*  createVertexBuffer(const void* initData, hUint32 elementsize, hUint32 elementcount, hUint32 flags);
    void  destroyVertexBuffer(hVertexBuffer* vb);

    hUniformBuffer* createUniformBuffer(const void* initdata, hUint size, hUint32 flags);
    void destroyUniformBuffer(hUniformBuffer* ub);

    hRenderCall* createRenderCall(const hRenderCallDesc& rcd); 

    void* allocTempRenderMemory( hUint32 size );

    hCmdList* createCmdList();
    void      linkCmdLists(hCmdList* before, hCmdList* after, hCmdList* i);
    void      detachCmdLists(hCmdList* i);
    hCmdList* nextCmdList(hCmdList* i);


    void clear(hCmdList* cl, hColour colour, hFloat depth);
	void draw(hCmdList* cl, hRenderCall* rc, Primative t, hUint prims);
    void swapBuffers(hCmdList* cl);

    void submitFrame(hCmdList* cmds);

    /*
        Methods to be called between beginCmdList & endCmdList
    */

    void rendererFrameSubmit(hCmdList* cmdlists, hUint count);
    hFloat getLastGPUTime();
    hBool isRenderThread();

}
}

