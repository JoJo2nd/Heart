/********************************************************************
Written by James Moran
Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/
#ifndef TILE_RENDERER_2D_CONSTANTS_CGH_
#define TILE_RENDERER_2D_CONSTANTS_CGH_

#include "hShaderCommon.h" 
#if HEART_DEFINE_SHADER_CONSTANT_LAYOUT
#   include "render/hProgramReflectionInfo.h"
namespace Heart {
namespace hTileRenderer2D {
#endif

struct Vert2D {
    float2 pos;
    half2 uv;
};

cbuffer TilePlaneParameters {
    float4 planeSize; // Width, Height, -, -
};
#if HEART_DEFINE_SHADER_CONSTANT_LAYOUT
inline const hRenderer::hUniformLayoutDesc* getTilePlaneParametersLayout(hUint32* count) {
    static hRenderer::hUniformLayoutDesc layout[] = {
        { "planeSize", hRenderer::ShaderParamType::Float4, 0 },
    };
    *count = (hUint32)hStaticArraySize(layout);
    return layout;
}
#endif

#if HEART_DEFINE_SHADER_CONSTANT_LAYOUT
} // namespace hTileRenderer2D
} // namespace Heart
#endif

#endif //TILE_RENDERER_2D_CONSTANTS_CGH_
