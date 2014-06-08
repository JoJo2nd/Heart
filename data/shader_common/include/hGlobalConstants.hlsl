
#ifndef GLOBAL_CONSTANTS_CGH_
#define GLOBAL_CONSTANTS_CGH_

#include "hlsl2glsl.glsl"

cbuffer FrameConstants {
    float g_time;
    float g_fracTime;
};

cbuffer ViewportConstants {
    float4x4 g_View                 ;
    float4x4 g_ViewInverse          ;
    float4x4 g_ViewInverseTranspose ;
    float4x4 g_Projection           ;
    float4x4 g_ProjectionInverse    ;
    float4x4 g_ViewProjection       ;
    float4x4 g_ViewProjectionInverse;
    float4   g_viewportSize         ;
};

cbuffer InstanceConstants {
    float4x4 g_World;
};

#endif //GLOBAL_CONSTANTS_CGH_
