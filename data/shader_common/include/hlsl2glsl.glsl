// 
// HLSL to GLSL funcs, macros & magic
// 

#if HEART_IS_GLSL

#   define cbuffer      uniform
#   define float4x4     mat4x4
#   define float3x4     mat3x4
#   define float2x4     mat2x4
#   define float4x4     mat4x4
#   define float4x3     mat4x3
#   define float4x2     mat4x2
#   define float3x3     mat3x3
#   define float3x2     mat3x2
#   define float2x3     mat2x3
#   define float2       vec2
#   define float3       vec3
#   define float4       vec4

#   define var_semantic(x)

#elif HEART_IS_HLSL

#   define layout(x) 
#   define in
#   define out
#   define uniform
#   define var_semantic(x) : x

#endif


/** use regex - 
    glsl_(in|out)_struct\((?!__)(.+?)(?!__)\)

*/
