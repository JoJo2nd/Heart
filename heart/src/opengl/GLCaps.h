/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/
#pragma once

#include "base/hTypes.h"
#include "render/hRenderCallDesc.h"
#include "opengl/GLFallback.h"
#include <GL/glew.h>    
#include <GL/gl.h>

namespace Heart {
namespace hRenderer {

struct hMipDesc;
struct hVertexBuffer;
struct hUniformBuffer;
struct hGLSampler;

struct GLCaps {
    hInt MaxTextureUnits;
    hInt UniformBufferOffsetAlignment;
    hBool ImmutableTextureStorage;
    struct {
        void (*create2D)(hUint32 levels, hBool compressed, GLuint internal_fmt, GLuint fmt, GLuint type, hMipDesc* initdata);
    } ImmutableTextureStorageProc;
    hBool BufferStorage;
    struct {
        hVertexBuffer* (*createVertexBuffer)(const void* data, hUint32 elementsize, hUint32 elementcount, hUint32 flags);
        void (*destroyVertexBuffer)(hVertexBuffer*);
        hUniformBuffer* (*createUniformBuffer)(const void* initdata, hUint size, hUint32 flags);
        void (*destroyUniformBuffer)(hUniformBuffer* ub);
    } BufferStorageProc;
    hBool SamplerObjects;
    struct {
        hSize_t (*getSamplerObjectSize)();
        void    (*genSamplerObjectInplace)(const hRenderCallDesc::hSamplerStateDesc& desc, hGLSampler* out, hSize_t osize);
        void    (*destroySamplerObjectInplace)(hGLSampler* data);
        void    (*bindSamplerObject)(hInt index, hGLSampler* so);
    } SamplerObjectsProc;
    hBool FenceSync;
    struct {
        GLsync (*FenceSync)(GLenum condition, GLbitfield flags);
        GLboolean (*IsSync)(GLsync sync);
        void (*DeleteSync)(GLsync sync);
        GLenum (*ClientWaitSync)(GLsync sync, GLbitfield flags, GLuint64 timeout);
        void (*WaitSync)(GLsync sync, GLbitfield flags, GLuint64 timeout);
    } FenceSyncProc;
};

hInt initialiseOpenGLCaps(GLCaps* out_caps);
}
}