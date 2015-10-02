/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#include "opengl/GLFallback.h"
#include "render/hUniformBufferFlags.h"
#include "opengl/GLTypes.h"
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#if HEART_USEOPENGL

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
    vb->alignedSize = elementsize*elementcount;
    vb->persistantMapping = mapped_ptr;
    return vb;
}

void flushVertexBuffer(hVertexBuffer* vb, hUint offset, hUint size) {
    glBindBuffer(GL_ARRAY_BUFFER, vb->name);
    glBufferSubData(GL_ARRAY_BUFFER, offset, size, ((hByte*)vb->persistantMapping)+offset);
}

void destroyVertexBuffer(hVertexBuffer* vb) {
    glDeleteBuffers(1, &vb->name);
    delete (hByte*)vb->persistantMapping;
    delete vb;
}

hUniformBuffer* createUniformBuffer(const void* initdata, hUint size, hUint32 flags) {
    hglEnsureTLSContext();
    hGLErrorScope();
/*
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
*/
    auto* ub = new hUniformBuffer();
    ub->name = 0/*ubname*/;
    ub->size_ = size;
    ub->createFlags_ = flags;
    ub->persistantMapping = new hByte[size];
    ub->mappedOffset_ = 0;
    ub->mappedSize_ = size;
    return ub;
}

void flushUniformBuffer(hUniformBuffer* ub) {
    //glBindBuffer(GL_UNIFORM_BUFFER, ub->name);
    //glBufferSubData(GL_UNIFORM_BUFFER, ub->mappedOffset_, ub->mappedSize_, ((hByte*)ub->persistantMapping)+ub->mappedOffset_);
}

void destroyUniformBuffer(hUniformBuffer* ub) {
    //glDeleteBuffers(1, &ub->name);
    delete (hByte*)ub->persistantMapping;
    delete ub;
}

struct GLESSamplerObject : hGLSampler {
    GLuint minFilter;
    GLuint magFilter;
    GLuint addressU;
    GLuint addressV;
};

hSize_t getSamplerObjectSize() {
    return sizeof(GLESSamplerObject);
}

void genSamplerObjectInplace(const hRenderCallDesc::hSamplerStateDesc& desc, hGLSampler* out, hSize_t osize) {
    hcAssert(osize >= getSamplerObjectSize());
    if (osize < getSamplerObjectSize()) {
        return;
    }
    static auto filtertogl = [](proto::renderstate::SamplerState a) -> GLint {
        switch (a) {
        case proto::renderstate::point: return GL_NEAREST;
        case proto::renderstate::linear: return GL_LINEAR;
        case proto::renderstate::anisotropic: return GL_LINEAR;
        }
        return GL_INVALID_VALUE;
    };
    static auto minfiltertogl = [](proto::renderstate::SamplerState a) -> GLint {
        switch (a) {
        case proto::renderstate::point: return GL_NEAREST_MIPMAP_NEAREST;
        case proto::renderstate::linear: return GL_LINEAR_MIPMAP_LINEAR;
        case proto::renderstate::anisotropic: return GL_LINEAR_MIPMAP_LINEAR;
        }
        return GL_INVALID_VALUE;
    };
    static auto bordertogl = [](proto::renderstate::SamplerBorder a) -> GLint {
        switch (a) {
        case proto::renderstate::wrap: return GL_REPEAT;
        case proto::renderstate::mirror: return GL_MIRRORED_REPEAT;
        case proto::renderstate::clamp: return GL_CLAMP_TO_EDGE;
        }
        return GL_INVALID_VALUE;
    };


    auto* s = (GLESSamplerObject*)out;
    s->minFilter = minfiltertogl(desc.filter_);
    s->magFilter = filtertogl(desc.filter_);
    s->addressU = bordertogl(desc.addressU_);
    s->addressV = bordertogl(desc.addressV_);
}

void destroySamplerObjectInplace(hGLSampler* data) {
}

void bindSamplerObject(hInt index, Heart::hRenderer::hGLSampler* so) {
    auto* s = (GLESSamplerObject*)so;
    // GL_TEXTURE_MIN_FILTER, GL_TEXTURE_MAG_FILTER, GL_TEXTURE_WRAP_S, or GL_TEXTURE_WRAP_T
    glTexParameteri(index, GL_TEXTURE_MIN_FILTER, s->minFilter);
    glTexParameteri(index, GL_TEXTURE_MAG_FILTER, s->magFilter);
    glTexParameteri(index, GL_TEXTURE_WRAP_S, s->addressU);
    glTexParameteri(index, GL_TEXTURE_WRAP_T, s->addressV);
}

}
}

#endif//HEART_USEOPENGL
