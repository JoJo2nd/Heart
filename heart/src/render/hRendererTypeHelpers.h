/********************************************************************
Written by James Moran
Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#pragma once

#if !HEART_STATIC_RENDER_API
#   ifndef HEART_MODULE_API
#      define HEART_MODULE_API(r, fn_name) extern r (HEART_API *fn_name)
#   endif
#   ifndef HEART_MODULE_API_INIT
#      define HEART_MODULE_API_INIT(r, fn_name) r (HEART_API *fn_name)
#   endif
#else
#   ifndef HEART_MODULE_API
#      define HEART_MODULE_API(r, fn_name) r fn_name
#   endif
#endif

#include <memory>

namespace Heart {
namespace hRenderer {

struct hCmdList;
struct hPipelineState;
struct hPipelineStateDescBase;
struct hInputState;
struct hInputStateDescBase;
struct hTexture1D;
struct hTexture2D;
struct hTexture3D;
struct hRenderTarget;
struct hShaderStage;
struct hIndexBuffer;
struct hVertexBuffer;
struct hUniformBuffer;
struct hRenderFence;

HEART_MODULE_API(void, destroyShader)(hShaderStage* prog);
HEART_MODULE_API(void, destroyTexture2D)(hTexture2D* t);
HEART_MODULE_API(void, destroyIndexBuffer)(hIndexBuffer* ib);
HEART_MODULE_API(void, destroyVertexBuffer)(hVertexBuffer* vb);
HEART_MODULE_API(void, destroyUniformBuffer)(hUniformBuffer* ub);
HEART_MODULE_API(void, destroyRenderPipelineState)(hPipelineState* pls);
HEART_MODULE_API(void, destroyRenderInputState)(hInputState* is);
HEART_MODULE_API(void, destroyRenderTarget)(hRenderTarget*);

struct hIndexBufferDeleter { // deleter
    void operator()(hIndexBuffer* p) const {
        destroyIndexBuffer(p);
    };
};

struct hVertexBufferDeleter { // deleter
    void operator()(hVertexBuffer* p) const {
        destroyVertexBuffer(p);
    };
};

struct hShaderStageDeleter { // deleter
    void operator()(hShaderStage* p) const {
        destroyShader(p);
    };
};

struct hUniformBufferDeleter { // deleter
    void operator()(hUniformBuffer* p) const {
        destroyUniformBuffer(p);
    };
};

struct hPipelineStateDeleter { // deleter
    void operator()(hPipelineState* p) const {
        destroyRenderPipelineState(p);
    };
};

struct hInputStateDeleter { // deleter
    void operator()(hInputState* p) const {
        destroyRenderInputState(p);
    };
};

struct hTexture2DDeleter { // deleter
    void operator()(hTexture2D* p) const {
        destroyTexture2D(p);
    };
};

struct hRenderTargetDeleter { // deleter
    void operator()(hRenderTarget* p) const {
        destroyRenderTarget(p);
    };
};

typedef std::unique_ptr<hIndexBuffer, hIndexBufferDeleter> hIndexBufferUniquePtr;
typedef std::unique_ptr<hVertexBuffer, hVertexBufferDeleter> hVertexBufferUniquePtr;
typedef std::unique_ptr<hUniformBuffer, hUniformBufferDeleter> hUniformBufferUniquePtr;
typedef std::unique_ptr<hPipelineState, hPipelineStateDeleter> hPipelineStateUniquePtr;
typedef std::unique_ptr<hInputState, hInputStateDeleter> hInputStateUniquePtr;
typedef std::unique_ptr<hTexture2D, hTexture2DDeleter> hTexture2DUniquePtr;
typedef std::unique_ptr<hRenderTarget, hRenderTargetDeleter> hRenderTargetUniquePtr;

}
}