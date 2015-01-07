/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/
#include "opengl/GLCaps.h"
#include "base/hMemoryUtil.h"
#include "core/hConfigOptions.h"
#include "render/hMipDesc.h"
// Extensions...
#include "opengl/GLExtBufferStorage.h"
#include "opengl/GLExtSamplerObjects.h"

namespace Heart {
namespace hRenderer {

void texture_storage_create2D(hUint32 levels, hBool compressed, GLuint internal_fmt, GLuint fmt, GLuint type, hMipDesc* initdata);

hInt initialiseOpenGLCaps(GLCaps* out_caps) {
    hZeroMem(out_caps, sizeof(GLCaps));
    // !!JM TODO: handle failure better than just asserting...
    if (!GLEW_VERSION_3_3) {
        hcAssertFailMsg("OpenGL 3.3 is required but not supported.");
        return -1;
    }
    if (!GLEW_EXT_texture_compression_s3tc) {
        hcAssertFailMsg("GL_EXT_texture_compression_s3tc is required but not found.");
        return -1;
    }
    if (!GLEW_EXT_texture_sRGB) {
        hcAssertFailMsg("GL_EXT_texture_sRGB is required but not found.");
        return -1;
    }

    auto disableTexStorage = hConfigurationVariables::getCVarBool("renderer.gl.notexturestorage", hFalse);
    out_caps->ImmutableTextureStorage = (((GLEW_ARB_texture_storage && GLEW_ARB_texture_storage_multisample) || GLEW_VERSION_4_3) && !disableTexStorage);
    if (out_caps->ImmutableTextureStorage) {
        out_caps->ImmutableTextureStorageProc.create2D = texture_storage_create2D;
    }
    auto disableBufferStorage = hConfigurationVariables::getCVarBool("renderer.gl.nobufferstorage", hFalse);
    out_caps->BufferStorage = ((GLEW_ARB_buffer_storage || GLEW_VERSION_4_4) && !disableBufferStorage);
    if (out_caps->BufferStorage) {
        out_caps->BufferStorageProc.createVertexBuffer = GLExt::BufferStorage::createVertexBuffer;
        out_caps->BufferStorageProc.destroyVertexBuffer = GLExt::BufferStorage::destroyVertexBuffer;
        out_caps->BufferStorageProc.createUniformBuffer = GLExt::BufferStorage::createUniformBuffer;
        out_caps->BufferStorageProc.destroyUniformBuffer = GLExt::BufferStorage::destroyUniformBuffer;
    }
    auto disableSamplerObjects = hConfigurationVariables::getCVarBool("renderer.gl.nosamplerobjects", hFalse);
    out_caps->SamplerObjects = (GLEW_ARB_sampler_objects || GLEW_VERSION_3_3) && !disableSamplerObjects;
    if (out_caps->SamplerObjects) {
        out_caps->SamplerObjectsProc.getSamplerObjectSize = GLExt::SamplerObjects::getSamplerObjectSize;
        out_caps->SamplerObjectsProc.genSamplerObjectInplace = GLExt::SamplerObjects::genSamplerObjectInplace;
        out_caps->SamplerObjectsProc.bindSamplerObject = GLExt::SamplerObjects::bindSamplerObject;
        out_caps->SamplerObjectsProc.destroySamplerObjectInplace = GLExt::SamplerObjects::destroySamplerObjectInplace;
    }
    //GL_NV_fence is not ok, it doesn't support multiple context
//     out_caps->FenceSync = (GLEW_ARB_sync || GLEW_VERSION_3_2);
//     if (out_caps->FenceSync) {
//             out_caps->FenceSyncProc.FenceSync = glFenceSync;
//             out_caps->FenceSyncProc.IsSync = glIsSync;
//             out_caps->FenceSyncProc.DeleteSync = glDeleteSync;
//             out_caps->FenceSyncProc.ClientWaitSync = glClientWaitSync;
//             out_caps->FenceSyncProc.WaitSync = glWaitSync;
//     }

    glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &out_caps->MaxTextureUnits);
    glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &out_caps->UniformBufferOffsetAlignment);

    return 0;
}

void texture_storage_create2D(hUint32 levels, hBool compressed, GLuint internal_fmt, GLuint fmt, GLuint type, hMipDesc* initdata) {
    glTexStorage2D(GL_TEXTURE_2D, levels, internal_fmt, initdata[0].width, initdata[0].height);
    if (compressed) {
        for (auto i = 0u; i < levels; ++i) {
            glCompressedTexSubImage2D(GL_TEXTURE_2D, i, 0, 0, initdata[i].width, initdata[i].height, fmt, initdata[i].size, initdata[i].data);
        }
    }
    else {
        for (auto i = 0u; i < levels; ++i) {
            glTexSubImage2D(GL_TEXTURE_2D, i, 0, 0, initdata[i].width, initdata[i].height, fmt, type, initdata[i].data);
        }
    }
}

}
}