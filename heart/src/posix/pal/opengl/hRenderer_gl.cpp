/********************************************************************

    filename: 	DeviceDX11RenderDevice.cpp	
    
    Copyright (c) 18:12:2011 James Moran
    
    This software is provided 'as-is', without any express or implied
    warranty. In no event will the authors be held liable for any damages
    arising from the use of this software.
    
    Permission is granted to anyone to use this software for any purpose,
    including commercial applications, and to alter it and redistribute it
    freely, subject to the following restrictions:
    
    1. The origin of this software must not be misrepresented; you must not
    claim that you wrote the original software. If you use this software
    in a product, an acknowledgment in the product documentation would be
    appreciated but is not required.
    
    2. Altered source versions must be plainly marked as such, and must not be
    misrepresented as being the original software.
    
    3. This notice may not be removed or altered from any source
    distribution.

*********************************************************************/

#include "base/hTypes.h"
#include "base/hRendererConstants.h"

namespace Heart {    

#if 0

#define COMMON_CONST_BLOCK() "           \
    cbuffer ViewportConstants            \
    {                                    \
    float4x4 g_View					;    \
    float4x4 g_ViewInverse			;    \
    float4x4 g_ViewInverseTranspose ;    \
    float4x4 g_Projection			;    \
    float4x4 g_ProjectionInverse	;    \
    float4x4 g_ViewProjection       ;    \
    float4x4 g_ViewProjectionInverse;    \
    float4   g_viewportSize         ;    \
    };                                   \
    cbuffer InstanceConstants            \
    {                                    \
        float4x4 g_World;                \
    };                                   \
    "

    static const hChar* s_shaderProfileNames[] = {
        "vs_4_0",   //eShaderProfile_vs4_0,
        "vs_4_1",   //eShaderProfile_vs4_1,
        "vs_5_0",   //eShaderProfile_vs5_0,
                    //
        "ps_4_0",   //eShaderProfile_ps4_0,
        "ps_4_1",   //eShaderProfile_ps4_1,
        "ps_5_0",   //eShaderProfile_ps5_0,
                    //
        "gs_4_0",   //eShaderProfile_gs4_0,
        "gs_4_1",   //eShaderProfile_gs4_1,
        "gs_5_0",   //eShaderProfile_gs5_0,
                    //
        "cs_4_0",   //eShaderProfile_cs4_0,
        "cs_4_1",   //eShaderProfile_cs4_1,
        "cs_5_0",   //eShaderProfile_cs5_0,
                    //
        "hs_5_0",   //eShaderProfile_hs5_0,
        "ds_5_0",   //eShaderProfile_ds5_0,
    };

    static const hChar s_debugVertex[] ={
    COMMON_CONST_BLOCK()
    "                                                                     \n\
    struct VSInput                                                        \n\
    {                                                                     \n\
        float3 position : POSITION;                                       \n\
    };                                                                    \n\
                                                                          \n\
    void mainVP( VSInput input, out float4 position : SV_POSITION )       \n\
    {                                                                     \n\
        position = float4(input.position.xyz,1);                          \n\
        position = mul(mul(g_ViewProjection, g_World), position);         \n\
    }                                                                     \n\
                                                                          \n\
    float4 mainFP() : SV_Target0                                          \n\
    {                                                                     \n\
        return float4(1,1,1,1);                                           \n\
    }                                                                     \n\
    "
    };

    static const hChar s_debugWSVertexCol[] ={
        COMMON_CONST_BLOCK()
        "                                                                     \n\
        struct VSInput                                                        \n\
        {                                                                     \n\
            float4 position : POSITION;                                       \n\
            float4 colour : COLOR0;                                           \n\
        };                                                                    \n\
        struct PSInput { \n\
            float4 colour : TEXCOORD0; \n\
        }; \n\
        \n\
        PSInput mainVP( VSInput input, out float4 position : SV_POSITION )       \n\
        {\n\
        PSInput output=(PSInput)0;\n\
        position = float4(input.position.xyz,1);                              \n\
        position = mul(mul(g_ViewProjection,  g_World), position);            \n\
        output.colour = input.colour;                                                \n\
        return output; \n\
        }                                                                     \n\
        \n\
        float4 mainFP(PSInput input) : SV_Target0                                          \n\
        {                                                                     \n\
        return input.colour;                                           \n\
        }                                                                     \n\
        "
    };

    static const hChar s_debugVertexLit[] ={
    COMMON_CONST_BLOCK()
    "                                                                     \n\
    struct VSInput                                                        \n\
    {                                                                     \n\
        float3 position : POSITION;                                       \n\
        float3 normal   : NORMAL;                                         \n\
    };                                                                    \n\
                                                                          \n\
    struct PSInput {                                                      \n\
        float3 normal : TEXCOORD0;                                        \n\
    };                                                                    \n\
                                                                          \n\
    PSInput mainVP( VSInput input, out float4 position : SV_POSITION )    \n\
    {                                                                     \n\
        PSInput output=(PSInput)0;                                        \n\
        position = float4(input.position.xyz,1);                          \n\
        position = mul(mul(g_ViewProjection,  g_World), position);         \n\
        output.normal = mul((float3x3)g_World, input.normal);             \n\
        return output;                                                    \n\
    }                                                                     \n\
                                                                          \n\
    float4 mainFP(PSInput input) : SV_Target0                               \n\
    {                                                                       \n\
        float NdotL=dot(normalize(input.normal),-normalize(g_View[2].xyz)); \n\
        return float4(NdotL,NdotL,NdotL,1);                                 \n\
    }                                                                       \n\
    "
    };

    static const hChar s_debugInstVertex[] ={
        COMMON_CONST_BLOCK()
        "                                                                    \n\
        struct VSInput                                                       \n\
        {                                                                    \n\
            float3 position : POSITION;                                      \n\
            float4 mmtx0 : INSTANCE0;                                        \n\
            float4 mmtx1 : INSTANCE1;                                        \n\
            float4 mmtx2 : INSTANCE2;                                        \n\
            float4 mmtx3 : INSTANCE3;                                        \n\
            float4 colour : INSTANCE4;                                       \n\
        };                                                                   \n\
                                                                             \n\
        struct PSInput {                                                     \n\
            float4 colour;                                                   \n\
        };                                                                   \n\
                                                                             \n\
        PSInput mainVP( VSInput input, out float4 position : SV_POSITION )   \n\
        {                                                                    \n\
            PSInput output={0};                                              \n\
            position = float4(input.position.xyz,1);                         \n\
            position = mul(g_ViewProjection, position);                      \n\
            output.colour=input.colour;                                      \n\
            return output;                                                   \n\
        }                                                                    \n\
                                                                             \n\
        float4 mainFP(PSInput input) : SV_Target0                            \n\
        {                                                                    \n\
            return input.colour;                                             \n\
        }                                                                    \n\
    "
    };

    static const hChar s_debugConsole[] = {
    COMMON_CONST_BLOCK()
    "                                                                       \n\
    struct VSInput                                                          \n\
    {                                                                       \n\
        float3 position : POSITION;                                         \n\
        float4 colour   : COLOR0;                                           \n\
    };                                                                      \n\
                                                                            \n\
    struct PSInput {                                                        \n\
        float4 colour   : COLOR0;                                           \n\
    };                                                                      \n\
                                                                            \n\
    PSInput mainVP( VSInput input, out float4 position : SV_POSITION )      \n\
    {                                                                       \n\
        PSInput output = (PSInput)0;                                        \n\
        output.colour = input.colour;                                       \n\
                                                                            \n\
        position = float4(input.position.xyz,1);                            \n\
        position.xy *= g_viewportSize.xy;                                   \n\
        position = mul( mul(g_ViewProjection,g_World), position );          \n\
                                                                            \n\
        return output;                                                      \n\
    }                                                                       \n\
                                                                            \n\
    float4 mainFP( PSInput input ) : SV_Target0                             \n\
    {                                                                       \n\
        return input.colour;                                                \n\
    }                                                                       \n\
    "
    };

    const hChar s_debugFont[] = {
        COMMON_CONST_BLOCK()
        "                                                              \n\
                                                                       \n\
        Texture2D   g_texture;                               \n\
        SamplerState g_sampler= sampler_state{};                     \n\
                                                                       \n\
        struct VSInput                                                 \n\
        {                                                              \n\
            float3 position : POSITION;                                \n\
            float4 colour 	: COLOR0;                                  \n\
            float2 uv 		: TEXCOORD0;                               \n\
        };                                                             \n\
                                                                       \n\
        struct PSInput                                                 \n\
        {                                                              \n\
            float4 position : SV_POSITION;                             \n\
            float4 colour 	: COLOR0;                                  \n\
            float2 uv 		: TEXCOORD0;                               \n\
        };                                                             \n\
                                                                       \n\
        PSInput mainVP( VSInput input )                                \n\
        {                                                              \n\
            PSInput output;                                            \n\
            float4 pos = float4(input.position.xyz,1);                 \n\
            output.position = mul(mul(g_ViewProjection,g_World), pos); \n\
            output.colour = input.colour;	                           \n\
            output.uv = input.uv;                                      \n\
            return output;                                             \n\
        }                                                                     \n\
                                                                              \n\
        float4 mainFP( PSInput input ) : SV_TARGET0                           \n\
        {                                                                     \n\
            float a   = g_texture.Sample(g_sampler, input.uv).a; \n\
            return float4(input.colour.rgb,a); \n\
        } \n\
        "
    };

    const hChar s_debugTexture[] = {
        COMMON_CONST_BLOCK()
        "                                                              \n\
        Texture2D   g_texture;                                         \n\
        SamplerState g_sampler= sampler_state{};                       \n\
                                                                       \n\
        struct VSInput                                                 \n\
        {                                                              \n\
            float3 position : POSITION;                                \n\
            float4 colour   : COLOR0; \n\
            float2 uv 		: TEXCOORD0;                               \n\
        };                                                             \n\
                                                                       \n\
        struct PSInput                                                 \n\
        {                                                              \n\
            float4 position : SV_POSITION;                             \n\
            float2 uv 		: TEXCOORD0;                               \n\
            float4 colour   : TEXCOORD1; \n\
        };                                                             \n\
                                                                       \n\
        PSInput mainVP( VSInput input )                                \n\
        {                                                              \n\
            PSInput output;                                            \n\
            float4 pos = float4(input.position.xyz,1);                 \n\
            output.position = mul(mul(g_ViewProjection,g_World), pos); \n\
            output.uv = input.uv;                                      \n\
            output.colour = input.colour; \n\
            return output;                                             \n\
        }                                                              \n\
                                                                       \n\
        float4 mainFP( PSInput input ) : SV_TARGET0                    \n\
        {                                                              \n\
            float4 c = g_texture.Sample(g_sampler, input.uv).rgba;     \n\
            return c.rgba*input.colour.rgba;                 \n\
        }                                                              \n\
        "
    };

    static const hChar* s_debugSrcs[eDebugShaderMax] = {
        s_debugVertex,
        s_debugVertex,
        s_debugConsole,
        s_debugConsole,
        s_debugFont,
        s_debugFont,
        s_debugVertexLit,
        s_debugVertexLit,
        s_debugTexture, 
        s_debugTexture,
        s_debugWSVertexCol,
        s_debugWSVertexCol,
    };
#endif
class hSystem;

namespace hRenderer {
#if 0
struct hDX11System {
    hSystem*                  system_;
    hUint64                   frameCounter_;
    hUint32                   width_;
    hUint32                   height_;
    IDXGISwapChain*           mainSwapChain_;
    ID3D11Device*             d3d11Device_;
    ID3D11DeviceContext*      mainDeviceCtx_;
    D3D_FEATURE_LEVEL         featureLevel_;
    ID3D11RenderTargetView*   renderTargetView_;
    ID3D11Query*              timerDisjoint_;
    ID3D11Query*              timerFrameStart_;
    ID3D11Query*              timerFrameEnd_;
    //hDeviceResizeCallback     resizeCallback_; !!JM todo
    ID3D11Texture2D*          backBuffer;
    hThread                   renderThread_;
    hMutex                    renderMtx_;
    hConditionVariable        renderKill_;
};
static hDX11System dx11;
#endif

struct hShaderStage {
    // !!JM placeholder class
};

hUint32 renderThreadMain(void* param) {
    return 0;
}

void create(hSystem* system, hUint32 width, hUint32 height, hUint32 bpp, hFloat shaderVersion, hBool fullscreen, hBool vsync) {
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void destroy() {
}


hShaderStage* createShaderStage(const hChar* shaderProg, hUint32 len, hShaderType type) {
    return nullptr;
}

hShaderStage* compileShaderStageFromSource(const hChar* shaderProg, hUint32 len, const hChar* entry, hShaderProfile profile) {
    return nullptr;
}

#if 0
DXGI_FORMAT toDXGIFormat(hTextureFormat format, hBool* compressed) {
    DXGI_FORMAT fmt = DXGI_FORMAT_FORCE_UINT;

    hBool compressedFormat=hFalse;
    switch ( format )
    {
    case eTextureFormat_Unknown: fmt = DXGI_FORMAT_UNKNOWN; break;
    case eTextureFormat_RGBA32_typeless: fmt = DXGI_FORMAT_R32G32B32A32_TYPELESS; break;
    case eTextureFormat_RGBA32_float: fmt = DXGI_FORMAT_R32G32B32A32_FLOAT; break;
    case eTextureFormat_RGBA32_uint: fmt = DXGI_FORMAT_R32G32B32A32_UINT; break;
    case eTextureFormat_RGBA32_sint: fmt = DXGI_FORMAT_R32G32B32A32_SINT; break;
    case eTextureFormat_RGB32_typeless: fmt = DXGI_FORMAT_R32G32B32_TYPELESS; break;
    case eTextureFormat_RGB32_float: fmt = DXGI_FORMAT_R32G32B32_FLOAT; break;
    case eTextureFormat_RGB32_uint: fmt = DXGI_FORMAT_R32G32B32_UINT; break;
    case eTextureFormat_RGB32_sint: fmt = DXGI_FORMAT_R32G32B32_SINT; break;
    case eTextureFormat_RGBA16_typeless: fmt = DXGI_FORMAT_R16G16B16A16_TYPELESS; break;
    case eTextureFormat_RGBA16_float: fmt = DXGI_FORMAT_R16G16B16A16_FLOAT; break;
    case eTextureFormat_RGBA16_unorm: fmt = DXGI_FORMAT_R16G16B16A16_UNORM; break;
    case eTextureFormat_RGBA16_uint: fmt = DXGI_FORMAT_R16G16B16A16_UINT; break;
    case eTextureFormat_RGBA16_snorm: fmt = DXGI_FORMAT_R16G16B16A16_SNORM; break;
    case eTextureFormat_RGBA16_sint: fmt = DXGI_FORMAT_R16G16B16A16_SINT; break;
    case eTextureFormat_RG32_typeless: fmt = DXGI_FORMAT_R32G32_TYPELESS; break;
    case eTextureFormat_RG32_float: fmt = DXGI_FORMAT_R32G32_FLOAT; break;
    case eTextureFormat_RG32_uint: fmt = DXGI_FORMAT_R32G32_UINT; break;
    case eTextureFormat_RG32_sint: fmt = DXGI_FORMAT_R32G32_SINT; break;
    case eTextureFormat_RG16_typeless: fmt = DXGI_FORMAT_R16G16_TYPELESS; break;
    case eTextureFormat_RG16_float: fmt = DXGI_FORMAT_R16G16_FLOAT; break;
    case eTextureFormat_RG16_uint: fmt = DXGI_FORMAT_R16G16_UINT; break;
    case eTextureFormat_RG16_sint: fmt = DXGI_FORMAT_R16G16_SINT; break;
    case eTextureFormat_R32_typeless: fmt = DXGI_FORMAT_R32_TYPELESS; break;
    case eTextureFormat_R32_float: fmt = DXGI_FORMAT_R32_FLOAT; break;
    case eTextureFormat_R32_uint: fmt = DXGI_FORMAT_R32_UINT; break;
    case eTextureFormat_R32_sint: fmt = DXGI_FORMAT_R32_SINT; break;
    case eTextureFormat_R16_typeless: fmt = DXGI_FORMAT_R16_TYPELESS; break;
    case eTextureFormat_R16_float: fmt = DXGI_FORMAT_R16_FLOAT; break;
    case eTextureFormat_R16_uint: fmt = DXGI_FORMAT_R16_UINT; break;
    case eTextureFormat_R16_sint: fmt = DXGI_FORMAT_R16_SINT; break;
    case eTextureFormat_RGB10A2_typeless: fmt = DXGI_FORMAT_R10G10B10A2_TYPELESS; break;
    case eTextureFormat_RGB10A2_unorm: fmt = DXGI_FORMAT_R10G10B10A2_UNORM; break;
    case eTextureFormat_RGB10A2_uint: fmt = DXGI_FORMAT_R10G10B10A2_UINT; break;
    case eTextureFormat_RGBA8_unorm: fmt = DXGI_FORMAT_R8G8B8A8_UNORM; break;
    case eTextureFormat_RGBA8_typeless: fmt = DXGI_FORMAT_R8G8B8A8_TYPELESS; break;
    case eTextureFormat_D32_float: fmt = DXGI_FORMAT_D32_FLOAT; break;
    case eTextureFormat_D24S8_float: fmt = DXGI_FORMAT_D24_UNORM_S8_UINT; break;
    case eTextureFormat_R8_unorm: fmt = DXGI_FORMAT_A8_UNORM; break;

    case eTextureFormat_RGBA8_sRGB_unorm:   fmt = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB; break;
    case eTextureFormat_BC3_unorm:          fmt = DXGI_FORMAT_BC3_UNORM; compressedFormat = hTrue; break;
    case eTextureFormat_BC2_unorm:          fmt = DXGI_FORMAT_BC2_UNORM; compressedFormat = hTrue; break;
    case eTextureFormat_BC1_unorm:          fmt = DXGI_FORMAT_BC1_UNORM; compressedFormat = hTrue; break; 
    case eTextureFormat_BC3_sRGB_unorm:     fmt = DXGI_FORMAT_BC3_UNORM_SRGB; compressedFormat = hTrue; break;
    case eTextureFormat_BC2_sRGB_unorm:     fmt = DXGI_FORMAT_BC2_UNORM_SRGB; compressedFormat = hTrue; break;
    case eTextureFormat_BC1_sRGB_unorm:     fmt = DXGI_FORMAT_BC1_UNORM_SRGB; compressedFormat = hTrue; break;
    }
    if(compressed) {
        *compressed=compressedFormat;
    }
    hcAssert(fmt!=DXGI_FORMAT_FORCE_UINT);
    return fmt;
}
#endif

static hShaderProfile getProfileFromString(const hChar* str) {
#if 0
    for (hUint i=0; i<hStaticArraySize(s_shaderProfileNames); ++i) {
        if (hStrICmp(s_shaderProfileNames[i],str) == 0) {
            return (hShaderProfile)i;
        }
    }
#endif
    return eShaderProfile_Max;
}

void swapBuffers() {
}
   
}
}
