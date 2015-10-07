/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/
#pragma once

#include "base/hTypes.h"
#include "render/hRenderCallDesc.h"

namespace Heart {
namespace hRenderer {
    struct hGLSampler;

}
}

namespace GLExt {
namespace SamplerObjects {

hSize_t getSamplerObjectSize();
void    genSamplerObjectInplace(const Heart::hRenderer::hRenderCallDesc::hSamplerStateDesc& desc, Heart::hRenderer::hGLSampler* out, hSize_t osize);
void    destroySamplerObjectInplace(Heart::hRenderer::hGLSampler* data);
void    bindSamplerObject(hInt index, Heart::hRenderer::hGLSampler* so);

}
}