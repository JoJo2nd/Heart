/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/
#pragma once

#include "base/hTypes.h"
#include "render/hRenderCallDesc.h"

namespace Heart {
namespace hRenderer {
    struct hVertexBuffer;
    struct hUniformBuffer;
    struct hGLSampler;
}
}

namespace GLExt {
namespace Fallback {

Heart::hRenderer::hVertexBuffer* createVertexBuffer(const void* data, hUint32 elementsize, hUint32 elementcount, hUint32 flags);
void flushVertexBuffer(Heart::hRenderer::hVertexBuffer* vb, hUint offset, hUint size);
void destroyVertexBuffer(Heart::hRenderer::hVertexBuffer* vb);
Heart::hRenderer::hUniformBuffer* createUniformBuffer(const void* initdata, hUint size, hUint32 flags);
void flushUniformBuffer(Heart::hRenderer::hUniformBuffer* ub);
void destroyUniformBuffer(Heart::hRenderer::hUniformBuffer* ub);
hSize_t getSamplerObjectSize();
void genSamplerObjectInplace(const Heart::hRenderer::hRenderCallDesc::hSamplerStateDesc& desc, Heart::hRenderer::hGLSampler* out, hSize_t osize);
void destroySamplerObjectInplace(Heart::hRenderer::hGLSampler* data);
void bindSamplerObject(hInt index, Heart::hRenderer::hGLSampler* so);

}
}