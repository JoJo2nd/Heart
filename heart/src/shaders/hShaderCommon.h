/********************************************************************
Written by James Moran
Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/
#ifndef SHADER_COMMON_CGH_
#define SHADER_COMMON_CGH_

// 
// HLSL funcs, macros & magic
// 
#ifdef __cplusplus

#include "base/hTypes.h"
#include "math/hMatrix.h"
#include "math/hVec2.h"
#include "math/hVec3.h"
#include "math/hVec4.h"
#include "math/hVector.h"

#   define HEART_DEFINE_SHADER_CONSTANT_LAYOUT (1)
#   define cbuffer struct

typedef hHalfFloat half;

struct half2 { half x, y; };
struct half3 { half x, y, z; };
struct half4 { half x, y, z, w; };
struct float2 { float x, y; };
struct float3 { float x, y, z; };

struct mat3x4 { float m[3][4]; };
struct mat2x4 { float m[2][4]; };
struct mat3x3 { float m[3][3]; };
struct mat3x2 { float m[3][2]; };
struct mat2x3 { float m[2][3]; };
struct mat4x3 { float m[4][3]; };
struct mat4x2 { float m[4][2]; };

typedef Heart::hMatrix mat4x4;
typedef float2 vec2;
typedef float3 vec3;
typedef Heart::hVec4 vec4;
typedef Heart::hMatrix float4x4;
typedef Heart::hVec4 float4;

// Cast operators for writing brevity ?
/*
float2& operator = (const hVec2& a) { return {a.getX(), a.getY()}; }
float3& operator = (const hVec3& a) { return {a.getX(), a.getY(), a.getZ()}; }
half2& operator = (const hVec2& a) { return {hFloatToHalf(a.getX()), hFloatToHalf(a.getY())}; }
*/

#else

#   define HEART_DEFINE_SHADER_CONSTANT_LAYOUT (0)

#   define  uniform   cbuffer   
#   define  mat4x4    float4x4  
#   define  mat3x4    float3x4  
#   define  mat2x4    float2x4  
#   define  mat4x4    float4x4  
#   define  mat4x3    float4x3  
#   define  mat4x2    float4x2  
#   define  mat3x3    float3x3  
#   define  mat3x2    float3x2  	
#   define  mat2x3    float2x3  
#   define  vec2      float2    
#   define  vec3      float3    
#   define  vec4      float4    

#   define layout(x) 
#   define in
#   define out
#   define var_semantic(x) : x

#endif

#endif
