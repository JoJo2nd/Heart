/********************************************************************
Written by James Moran
Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/
#ifndef GLOBAL_CONSTANTS_CGH_
#define GLOBAL_CONSTANTS_CGH_

#include "hShaderCommon.h" 
#if HEART_DEFINE_SHADER_CONSTANT_LAYOUT
#   include "render/hProgramReflectionInfo.h"
#endif

cbuffer FrameConstants {
    float4 g_time;
};
#if HEART_DEFINE_SHADER_CONSTANT_LAYOUT
namespace Heart {
inline const hRenderer::hUniformLayoutDesc* getFrameConstantsLayout(hUint32* count) {
    using namespace Heart::hRenderer;
    static hUniformLayoutDesc layout[] = {
        { "g_time", ShaderParamType::Float4, 0 },
    };
    *count = (hUint32)hStaticArraySize(layout);
    return layout;
}
}
#endif

cbuffer ViewportConstants {
    float4x4 g_View;
    float4x4 g_ViewInverse;
    float4x4 g_ViewInverseTranspose;
    float4x4 g_Projection;
    float4x4 g_ProjectionInverse;
    float4x4 g_ViewProjection;
    float4x4 g_ViewProjectionInverse;
    float4   g_viewportSize;
};
#if HEART_DEFINE_SHADER_CONSTANT_LAYOUT
namespace Heart {
inline const hRenderer::hUniformLayoutDesc* getViewportConstantsLayout(hUint32* count) {
    using namespace Heart::hRenderer;
    static hUniformLayoutDesc layout[] = {
        { "g_View", ShaderParamType::Float44, 0 },
        { "g_ViewInverse", ShaderParamType::Float44, 64 },
        { "g_ViewInverseTranspose", ShaderParamType::Float44, 128 },
        { "g_Projection", ShaderParamType::Float44, 192 },
        { "g_ProjectionInverse", ShaderParamType::Float44, 256 },
        { "g_ViewProjection", ShaderParamType::Float44, 320 },
        { "g_ViewProjectionInverse", ShaderParamType::Float44, 384 },
        { "g_ViewportSize", ShaderParamType::Float4, 448 },
    };
    *count = (hUint32)hStaticArraySize(layout);
    return layout;
}
}
#endif


cbuffer InstanceConstants {
    float4x4 g_World;
};
#if HEART_DEFINE_SHADER_CONSTANT_LAYOUT
namespace Heart {
inline const hRenderer::hUniformLayoutDesc* getInstanceConstantsLayout(hUint32* count) {
    using namespace Heart::hRenderer;
    static hUniformLayoutDesc layout[] = {
        { "g_World", ShaderParamType::Float44, 0 },
    };
    *count = (hUint32)hStaticArraySize(layout);
    return layout;
}
}
#endif

#endif //GLOBAL_CONSTANTS_CGH_
