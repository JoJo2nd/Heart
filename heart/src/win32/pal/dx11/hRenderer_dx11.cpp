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

//#define HEART_DO_FRAMETIMES
//#define DO_RESIZE

#include "hShaderStage_dx11.h"
#include <d3d11.h>
#include <D3Dcompiler.h>

namespace Heart {    

#if 1// defined (HEART_DEBUG)
#   define HEART_D3D_OBJECT_REPORT(device) {\
        ID3D11Debug* debuglayer;\
        device->QueryInterface(__uuidof(ID3D11Debug), (void**)&debuglayer);\
        if (debuglayer) { \
            debuglayer->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);\
            debuglayer->Release();\
        }\
    }
#   define HEART_D3D_DEBUG_NAME_OBJECT(obj, name) obj->SetPrivateData( WKPDID_D3DDebugObjectName, hStrLen(name), name)
#else
#   define HEART_D3D_OBJECT_REPORT(device)
#   define HEART_D3D_DEBUG_NAME_OBJECT(obj, name)
#endif

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

class hSystem;

namespace hRenderer {
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

    hUint32 renderThreadMain(void* param) {
        while (!dx11.renderKill_.wait(&dx11.renderMtx_)) {

        }
        return 0;
    }

    void create(hSystem* system, hUint32 width, hUint32 height, hUint32 bpp, hFloat shaderVersion, hBool fullscreen, hBool vsync) {
        HRESULT hr;

        dx11.system_ = system;
        dx11.width_ = width;
        dx11.height_ = height;
        //depthBufferTex_ = setup.depthBufferTex_;
        //hcAssert(depthBufferTex_);

        D3D_FEATURE_LEVEL featureLevels[] =
        {
            D3D_FEATURE_LEVEL_11_0,
            D3D_FEATURE_LEVEL_10_1,
            D3D_FEATURE_LEVEL_10_0,
            D3D_FEATURE_LEVEL_9_3,
        };
        UINT numFeatureLevels = ARRAYSIZE( featureLevels );

        //Create a DX11 Swap chain
        DXGI_SWAP_CHAIN_DESC sd;
        ZeroMemory( &sd, sizeof(sd) );
        sd.BufferCount = 2;
        sd.BufferDesc.Width = width;
        sd.BufferDesc.Height = height;
        sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
        sd.BufferDesc.RefreshRate.Numerator = 60;
        sd.BufferDesc.RefreshRate.Denominator = 1;
        sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd.OutputWindow = NULL;//dx11.system_->GetSystemHandle()->hWnd_;
        sd.SampleDesc.Count = 1;
        sd.SampleDesc.Quality = 0;
        sd.Windowed = dx11.system_->getOwnWindow() ? !fullscreen : TRUE;
        sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

        hcPrintf("Creating DirectX 11 Device");
        if( hr = D3D11CreateDeviceAndSwapChain( 
                NULL, 
                D3D_DRIVER_TYPE_HARDWARE, 
                NULL, 
#if !defined (HEART_DEBUG)
                0, 
#else
                D3D11_CREATE_DEVICE_DEBUG,
#endif
                featureLevels, 
                numFeatureLevels,
                D3D11_SDK_VERSION, 
                &sd, 
                &dx11.mainSwapChain_, 
                &dx11.d3d11Device_, 
                &dx11.featureLevel_, 
                &dx11.mainDeviceCtx_ ) != S_OK )
        {
            hcAssertFailMsg( "Couldn't Create D3D11 context" );
        }
        HEART_D3D_DEBUG_NAME_OBJECT(dx11.mainDeviceCtx_, "main context");

        //Grab the back buffer & depth buffer
        // Create a render target view
        hr = dx11.mainSwapChain_->GetBuffer( 0, __uuidof( ID3D11Texture2D ), (LPVOID*)&dx11.backBuffer );
        hcAssert( SUCCEEDED( hr ) );
        HEART_D3D_DEBUG_NAME_OBJECT(dx11.backBuffer, "back buffer");

        hr = dx11.d3d11Device_->CreateRenderTargetView( dx11.backBuffer, NULL, &dx11.renderTargetView_ );
        hcAssert( SUCCEEDED( hr ) );
        HEART_D3D_DEBUG_NAME_OBJECT(dx11.renderTargetView_, "back buffer Target View");

        D3D11_QUERY_DESC qdesc;
        qdesc.MiscFlags = 0;

#ifdef HEART_DO_FRAMETIMES
        qdesc.Query = D3D11_QUERY_TIMESTAMP_DISJOINT;
        hr = d3d11Device_->CreateQuery(&qdesc, &dx11.timerDisjoint_);
        hcAssert(SUCCEEDED(hr));

        qdesc.Query = D3D11_QUERY_TIMESTAMP;
        hr = d3d11Device_->CreateQuery(&qdesc, &dx11.timerFrameStart_);
        hcAssert(SUCCEEDED(hr));

        qdesc.Query = D3D11_QUERY_TIMESTAMP;
        hr = d3d11Device_->CreateQuery(&qdesc, &dx11.timerFrameEnd_);
        hcAssert(SUCCEEDED(hr));
#endif

        dx11.frameCounter_ = 0;
        dx11.renderThread_.create("RenderThread", hThread::PRIORITY_NORMAL, hFUNCTOR_BINDSTATIC(hThreadFunc, renderThreadMain), nullptr);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void destroy() {
        // !!JM todo: a better method may be to push a kill command and 
        // let the thread wind down normally
        dx11.renderKill_.signal();
        dx11.renderThread_.join();
#ifdef HEART_DO_FRAMETIMES
        dx11.timerDisjoint_->Release();
        dx11.timerFrameStart_->Release();
        dx11.timerFrameEnd_->Release();
#endif
        
        if( dx11.mainDeviceCtx_ ) {
            dx11.mainDeviceCtx_->ClearState();
            dx11.mainDeviceCtx_->Flush();
            dx11.mainDeviceCtx_->Release();
        }
        if ( dx11.renderTargetView_ ) {
            dx11.renderTargetView_->Release();
        }
        if (dx11.backBuffer) {
            dx11.backBuffer->Release();
        }
//         if ( depthStencilView_ ) {
//             depthStencilView_->Release();
//         }
//         if ( depthStencil_ ) {
//             depthStencil_->Release();
//         }
        if ( dx11.mainSwapChain_ ) {
            dx11.mainSwapChain_->Release();
        }
        if ( dx11.d3d11Device_ ) {
            ULONG ref=dx11.d3d11Device_->Release();
            hcPrintf("D3DDevice Ref %u", ref);
            if (ref != 0) {
                HEART_D3D_OBJECT_REPORT(dx11.d3d11Device_);
            }
        }
    }


hShaderStage* createShaderStage(const hChar* shaderProg, hUint32 len, hShaderType type) {
    HRESULT hr;
    ID3D11Device* dx11_device = dx11.d3d11Device_;
    hShaderStage* shader = new hShaderStage(type);

    if ( type == ShaderType_FRAGMENTPROG ) {
        hr = dx11_device->CreatePixelShader( shaderProg, len, NULL, &shader->pixelShader_ );
        hcAssert( SUCCEEDED( hr ) );
    } else if ( type == ShaderType_VERTEXPROG ) {
        hr = dx11_device->CreateVertexShader( shaderProg, len, NULL, &shader->vertexShader_ );
        hcAssert( SUCCEEDED( hr ) );
    } else if (type==ShaderType_GEOMETRYPROG) {
        hr=dx11_device->CreateGeometryShader(shaderProg, len, NULL, &shader->geomShader_);
        hcAssert( SUCCEEDED( hr ) );
    } else if (type==ShaderType_DOMAINPROG) {
        hr=dx11_device->CreateDomainShader(shaderProg, len, NULL, &shader->domainShader_);
        hcAssert( SUCCEEDED( hr ) );
    } else if (type==ShaderType_HULLPROG) {
        hr=dx11_device->CreateHullShader(shaderProg, len, NULL, &shader->hullShader_);
        hcAssert( SUCCEEDED( hr ) );
    } else if (type==ShaderType_COMPUTEPROG) {
        hr=dx11_device->CreateComputeShader(shaderProg, len, NULL, &shader->computeShader_);
        hcAssert( SUCCEEDED( hr ) );
    }

    if (shader->shaderPtr_ == nullptr) {
        delete shader;
        shader = nullptr;
    }

    return shader;
}

hShaderStage* compileShaderStageFromSource(const hChar* shaderProg, hUint32 len, const hChar* entry, hShaderProfile profile) {
    HRESULT hr;
    const hChar* profileStr=s_shaderProfileNames[profile];
    ID3DBlob* codeBlob;
    ID3DBlob* errorBlob;
    hShaderType type=ShaderType_MAX;
    DWORD compileflags = 0;
    D3D_SHADER_MACRO* macros=hNullptr;

#ifdef HEART_DEBUG
    compileflags |= D3DCOMPILE_DEBUG;
    compileflags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    if (profile >= eShaderProfile_vs4_0 && profile <= eShaderProfile_vs5_0) {
        type=ShaderType_VERTEXPROG;
    } else if (profile >= eShaderProfile_ps4_0 && profile <= eShaderProfile_ps5_0) {
        type=ShaderType_FRAGMENTPROG;
    } else if (profile >= eShaderProfile_gs4_0 && profile <= eShaderProfile_gs5_0) {
        type=ShaderType_GEOMETRYPROG;
    } else if (profile >= eShaderProfile_cs4_0 && profile <= eShaderProfile_cs5_0) {
        type=ShaderType_COMPUTEPROG;
    } else if (profile == eShaderProfile_hs5_0) {
        type=ShaderType_HULLPROG;
    } else if (profile == eShaderProfile_ds5_0) {
        type=ShaderType_DOMAINPROG;
    }

    
    hr=D3DCompile(shaderProg, len, nullptr, nullptr, nullptr, entry, profileStr, compileflags, 0, &codeBlob, &errorBlob);
    if (errorBlob) {
        hcPrintf("Shader Output:\n%s", errorBlob->GetBufferPointer());
        errorBlob->Release();
    }
    if (FAILED(hr)) {
        return NULL;
    }
    hShaderStage* out = createShaderStage((hChar*)codeBlob->GetBufferPointer(), (hUint32)codeBlob->GetBufferSize(), type);
    codeBlob->Release();
    return out;
}

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

static hShaderProfile getProfileFromString(const hChar* str) {
    for (hUint i=0; i<hStaticArraySize(s_shaderProfileNames); ++i) {
        if (hStrICmp(s_shaderProfileNames[i],str) == 0) {
            return (hShaderProfile)i;
        }
    }
    return eShaderProfile_Max;
}

void swapBuffers() {
#if 0
    HRESULT hr;
    if (buffer) {
        mainDeviceCtx_->CopyResource(pBackBuffer, buffer->dx11Texture_);
    }
    mainDeviceCtx_->OMSetRenderTargets(1, &renderTargetView_, hNullptr);
    hr=mainSwapChain_->Present( 0, 0 );
#ifdef HEART_DO_FRAMETIMES
    if (frameCounter_ > 0)
    {
        mainDeviceCtx_->End(timerFrameEnd_);
        mainDeviceCtx_->End(timerDisjoint_);
    }
#endif

    if (width_ != sysWindow_->getWindowWidth() || height_ != sysWindow_->getWindowHeight()) {
        if (sysWindow_->getWindowWidth() > 0 && sysWindow_->getWindowHeight() > 0) {
            width_ =sysWindow_->getWindowWidth();
            height_=sysWindow_->getWindowHeight();
            //Resize the back buffers
            // Release all outstanding references to the swap chain's buffers. 
            // Otherwise ResizeBuffers will fail!
            hUint32 refc;
            mainDeviceCtx_->OMSetRenderTargets(0, 0, 0);
            refc=pBackBuffer->Release();
            refc=renderTargetView_->Release();
            renderTargetView_=hNullptr;
            pBackBuffer=hNullptr;
            // Preserve the existing buffer count and format.
            // Automatically choose the width and height to match the client rect for HWNDs.
            hr = mainSwapChain_->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH);
            hcAssertMsg(SUCCEEDED(hr), "mainSwapChain::ResizeBuffers failed 0x%x", hr);
            // TODO:Perform error handling here!
            // Get buffer and create a render-target-view.
            hr = mainSwapChain_->GetBuffer(0, __uuidof( ID3D11Texture2D),(void**)&pBackBuffer);
            hcAssertMsg(SUCCEEDED(hr), "mainSwapChain::GetBuffer failed 0x%x", hr);
            HEART_D3D_DEBUG_NAME_OBJECT(pBackBuffer, "back buffer");
            // TODO: Perform error handling here!
            hr = d3d11Device_->CreateRenderTargetView(pBackBuffer, NULL, &renderTargetView_);
            hcAssertMsg(SUCCEEDED(hr), "d3d11Device::CreateRenderTargetView failed 0x%x", hr);
            HEART_D3D_DEBUG_NAME_OBJECT(renderTargetView_, "back buffer target view");

            if (resizeCallback_.isValid()) {
                resizeCallback_(width_, height_);
            }
        }
    }
#endif
}
   
}
}
