/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#include "opengl/GLFallback.h"
#include "render/hUniformBufferFlags.h"
#include "opengl/GLTypes.h"
#include <GL/glew.h>
#include <GL/gl.h>

namespace GLExt {
namespace Fallback {

using namespace Heart;
using namespace Heart::hRenderer;

hVertexBuffer* createVertexBuffer(const void* data, hUint32 elementsize, hUint32 elementcount, hUint32 flags) {
    hglEnsureTLSContext();
    hGLErrorScope();
    GLuint bname;
    GLenum gl_flags;
    hBool dynamic = (flags & (hUint32)hUniformBufferFlags::Dynamic) == (hUint32)hUniformBufferFlags::Dynamic;
    void* mapped_ptr = nullptr;
    if (dynamic) {
        gl_flags = GL_STREAM_DRAW;
    } else {
        gl_flags = GL_STATIC_DRAW;
    }

    glGenBuffers(1, &bname);
    if (!bname) {
        return nullptr;
    }
    glBindBuffer(GL_ARRAY_BUFFER, bname);
    GLuint size = elementsize * elementcount;
    if (dynamic) {
        mapped_ptr = new hByte[size];
    }
    glBufferData(GL_ARRAY_BUFFER, size, data, gl_flags);
    hGLSyncFlush();

    auto* vb = new hVertexBuffer();
    vb->name = bname;
    vb->elementCount_ = elementcount;
    vb->elementSize_ = elementsize;
    vb->createFlags_ = flags;
    vb->persistantMapping = mapped_ptr;
    return vb;
}

void destroyVertexBuffer(hVertexBuffer* vb) {
    glDeleteBuffers(1, &vb->name);
    delete (hByte*)vb->persistantMapping;
    delete vb;
}

hUniformBuffer* createUniformBuffer(const void* initdata, hUint size, hUint32 flags) {
    hglEnsureTLSContext();
    hGLErrorScope();
    GLuint ubname;
    GLenum gl_flags;
    hBool dynamic = (flags & (hUint32)hUniformBufferFlags::Dynamic) == (hUint32)hUniformBufferFlags::Dynamic;
    void* mapped_ptr = nullptr;
    if (dynamic) {
        gl_flags = GL_STREAM_DRAW;
    }
    else {
        gl_flags = GL_STATIC_DRAW;
    }

    glGenBuffers(1, &ubname);
    if (!ubname) {
        return nullptr;
    }
    glBindBuffer(GL_UNIFORM_BUFFER, ubname);
    if (dynamic) {
        mapped_ptr = new hByte[size];
    }
    glBufferData(GL_UNIFORM_BUFFER, size, initdata, gl_flags);
    hGLSyncFlush();

    auto* ub = new hUniformBuffer();
    ub->name = ubname;
    ub->size_ = size;
    ub->createFlags_ = flags;
    ub->persistantMapping = mapped_ptr;
    ub->mappedOffset_ = 0;
    ub->mappedSize_ = size;
    return ub;
}

void flushUniformBuffer(hUniformBuffer* ub) {
    glBufferSubData(GL_UNIFORM_BUFFER, ub->mappedOffset_, ub->mappedSize_, ((hByte*)ub->persistantMapping)+ub->mappedOffset_);
}

void destroyUniformBuffer(hUniformBuffer* ub) {
    glDeleteBuffers(1, &ub->name);
    delete (hByte*)ub->persistantMapping;
    delete ub;
}

hSize_t getSamplerObjectSize() {
    return 0;
}

void genSamplerObjectInplace(const hRenderCallDesc::hSamplerStateDesc& desc, hGLSampler* out, hSize_t osize) {

}

void destroySamplerObjectInplace(hGLSampler* data) {
}

void bindSamplerObject(hInt index, Heart::hRenderer::hGLSampler* so) {
}

}
}
