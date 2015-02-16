/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/
#include "opengl/GLExtSamplerObjects.h"
#include "opengl/GLTypes.h"

namespace GLExt {
namespace SamplerObjects {
using namespace Heart;
using namespace Heart::hRenderer;

struct ExtSamplerObject : hGLSampler {
    GLuint samplerObj;
};

hSize_t getSamplerObjectSize() {
    return sizeof(ExtSamplerObject);
}

void    genSamplerObjectInplace(const hRenderCallDesc::hSamplerStateDesc& desc, hGLSampler* out, hSize_t osize) {
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
        case proto::renderstate::border: return GL_CLAMP_TO_BORDER;
        }
        return GL_INVALID_VALUE;
    };


    auto* s = (ExtSamplerObject*)out;
    glGenSamplers(1, &s->samplerObj);
    auto minfilter = minfiltertogl(desc.filter_);
    auto filter = filtertogl(desc.filter_);
    auto wraps = bordertogl(desc.addressU_);
    auto wrapt = bordertogl(desc.addressV_);
    auto wrapr = bordertogl(desc.addressW_);
    glSamplerParameteriv(s->samplerObj, GL_TEXTURE_WRAP_S, &wraps);
    glSamplerParameteriv(s->samplerObj, GL_TEXTURE_WRAP_T, &wrapt);
    glSamplerParameteriv(s->samplerObj, GL_TEXTURE_WRAP_R, &wrapr);
    glSamplerParameteriv(s->samplerObj, GL_TEXTURE_MIN_FILTER, &minfilter);
    glSamplerParameteriv(s->samplerObj, GL_TEXTURE_MAG_FILTER, &filter);
    glSamplerParameterfv(s->samplerObj, GL_TEXTURE_MIN_LOD, &desc.minLOD_);
    glSamplerParameterfv(s->samplerObj, GL_TEXTURE_MAX_LOD, &desc.maxLOD_);
    glSamplerParameterfv(s->samplerObj, GL_TEXTURE_BORDER_COLOR, (hFloat*)&desc.borderColour_);
}

void    destroySamplerObjectInplace(hGLSampler* data) {
    auto* s = (ExtSamplerObject*)data;
    glDeleteSamplers(1, &s->samplerObj);
}

void    bindSamplerObject(hInt index, hGLSampler* so) {
    auto* s = (ExtSamplerObject*)so;
    glBindSampler(index, s->samplerObj);
}

}
}