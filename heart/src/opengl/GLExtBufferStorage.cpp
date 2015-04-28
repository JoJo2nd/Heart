/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#include "render/hUniformBufferFlags.h"
#include "opengl/GLTypes.h"
#include <GL/glew.h>
#include <GL/gl.h>

namespace GLExt {
namespace BufferStorage {

using namespace Heart::hRenderer;

hVertexBuffer* createVertexBuffer(const void* data, hUint32 elementsize, hUint32 elementcount, hUint32 flags) {
    hglEnsureTLSContext();
    hGLErrorScope();
    GLuint bname;
    GLenum gl_flags;
    hBool dynamic = (flags & (hUint32)hUniformBufferFlags::Dynamic) == (hUint32)hUniformBufferFlags::Dynamic;
    void* mapped_ptr = nullptr;
    if (dynamic) {
        gl_flags = GL_MAP_WRITE_BIT |
            GL_MAP_COHERENT_BIT |
            GL_MAP_PERSISTENT_BIT;
    }
    else {
        gl_flags = GL_STATIC_DRAW;
    }

    glGenBuffers(1, &bname);
    if (!bname) {
        return nullptr;
    }
    glBindBuffer(GL_ARRAY_BUFFER, bname);
    GLuint size = elementsize * elementcount;

    if (dynamic) {
        glBufferStorage(GL_ARRAY_BUFFER, size, data, gl_flags);
        mapped_ptr = glMapBufferRange(GL_ARRAY_BUFFER, 0, size, gl_flags);
    }
    else {
        glBufferData(GL_ARRAY_BUFFER, size, data, gl_flags);
    }
    hGLSyncFlush();

    auto* vb = new hVertexBuffer();
    vb->name = bname;
    vb->elementCount_ = elementcount;
    vb->elementSize_ = elementsize;
    vb->createFlags_ = flags;
    vb->alignedSize = elementcount*elementsize;
    vb->persistantMapping = mapped_ptr;
    return vb;
}

void destroyVertexBuffer(hVertexBuffer* vb) {
    glDeleteBuffers(1, &vb->name);
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
        gl_flags = GL_MAP_WRITE_BIT |
            GL_MAP_COHERENT_BIT |
            GL_MAP_PERSISTENT_BIT;
    }
    else {
        gl_flags = GL_STATIC_DRAW;
    }

    glGenBuffers(1, &ubname);
    glBindBuffer(GL_UNIFORM_BUFFER, ubname);
    if (dynamic) {
        glBufferStorage(GL_UNIFORM_BUFFER, size, initdata, gl_flags);
        mapped_ptr = glMapBufferRange(GL_UNIFORM_BUFFER, 0, size, gl_flags);
    }
    else {
        glBufferData(GL_UNIFORM_BUFFER, size, initdata, gl_flags);
    }
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

void destroyUniformBuffer(hUniformBuffer* ub) {
    glDeleteBuffers(1, &ub->name);
    delete ub;
}

}
}
