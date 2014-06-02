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

namespace Heart
{
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

    const hChar* hdDX11RenderDevice::s_shaderProfileNames[] = {
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

    class hSourceIncludeHandler : public ID3DInclude
    {
    public:
        hSourceIncludeHandler(hIIncludeHandler* impl)
            : impl_(impl)
        {}
        STDMETHOD(Open)(THIS_ D3D_INCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID *ppData, UINT *pBytes) {
            if (!impl_) {
                return E_FAIL;
            }
            impl_->findInclude(pFileName, ppData, pBytes);
            if (*ppData==hNullptr) {
                return E_FAIL;
            }
            return S_OK;
        }
        STDMETHOD(Close)(THIS_ LPCVOID pData) {
            return S_OK;
        }
    private:
        hIIncludeHandler* impl_;
    };

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hdDX11RenderDevice::hdDX11RenderDevice() 
        : sysWindow_(NULL)
    {
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hdDX11RenderDevice::~hdDX11RenderDevice()
    {

    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdDX11RenderDevice::Create(hdSystemWindow* sysHandle, hUint32 width, hUint32 height, hBool fullscreen, hBool vsync, hRenderDeviceSetup setup)
    {
        HRESULT hr;

        sysWindow_ = sysHandle;
        width_ = width;
        height_ = height;
        alloc_ = setup.alloc_;
        free_ = setup.free_;
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
        sd.OutputWindow = sysHandle->GetSystemHandle()->hWnd_;
        sd.SampleDesc.Count = 1;
        sd.SampleDesc.Quality = 0;
        sd.Windowed = sysWindow_->getOwnWindow() ? !fullscreen : TRUE;
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
                &mainSwapChain_, 
                &d3d11Device_, 
                &featureLevel_, 
                &mainDeviceCtx_ ) != S_OK )
        {
            hcAssertFailMsg( "Couldn't Create D3D11 context" );
        }
        HEART_D3D_DEBUG_NAME_OBJECT(mainDeviceCtx_, "main context");

        //Grab the back buffer & depth buffer
        // Create a render target view
        hr = mainSwapChain_->GetBuffer( 0, __uuidof( ID3D11Texture2D ), (LPVOID*)&pBackBuffer );
        hcAssert( SUCCEEDED( hr ) );
        HEART_D3D_DEBUG_NAME_OBJECT(pBackBuffer, "back buffer");

        hr = d3d11Device_->CreateRenderTargetView( pBackBuffer, NULL, &renderTargetView_ );
        hcAssert( SUCCEEDED( hr ) );
        HEART_D3D_DEBUG_NAME_OBJECT(renderTargetView_, "back buffer Target View");

        // Create depth stencil texture
//         D3D11_TEXTURE2D_DESC descDepth;
//         ZeroMemory( &descDepth, sizeof(descDepth) );
//         descDepth.Width = width;
//         descDepth.Height = height;
//         descDepth.MipLevels = 1;
//         descDepth.ArraySize = 1;
//         descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
//         descDepth.SampleDesc.Count = 1;
//         descDepth.SampleDesc.Quality = 0;
//         descDepth.Usage = D3D11_USAGE_DEFAULT;
//         descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
//         descDepth.CPUAccessFlags = 0;
//         descDepth.MiscFlags = 0;
//         hr = d3d11Device_->CreateTexture2D( &descDepth, NULL, &depthStencil_ );
//         hcAssert( SUCCEEDED( hr ) );
//         HEART_D3D_DEBUG_NAME_OBJECT(depthStencil_, "Depth Stencil");
// 
//         // Create the depth stencil view
//         D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
//         ZeroMemory( &descDSV, sizeof(descDSV) );
//         descDSV.Format = descDepth.Format;
//         descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
//         descDSV.Texture2D.MipSlice = 0;
//         hr = d3d11Device_->CreateDepthStencilView( depthStencil_, &descDSV, &depthStencilView_ );
//         hcAssert( SUCCEEDED( hr ) );
//         HEART_D3D_DEBUG_NAME_OBJECT(depthStencilView_, "Depth Stencil View");

        mainRenderCtx_.SetDeviceCtx( mainDeviceCtx_, alloc_, free_ );

        //update textures
        //depthBufferTex_->dx11Texture_=depthStencil_;

        D3D11_QUERY_DESC qdesc;
        qdesc.MiscFlags = 0;

#ifdef HEART_DO_FRAMETIMES
        qdesc.Query = D3D11_QUERY_TIMESTAMP_DISJOINT;
        hr = d3d11Device_->CreateQuery(&qdesc, &timerDisjoint_);
        hcAssert(SUCCEEDED(hr));

        qdesc.Query = D3D11_QUERY_TIMESTAMP;
        hr = d3d11Device_->CreateQuery(&qdesc, &timerFrameStart_);
        hcAssert(SUCCEEDED(hr));

        qdesc.Query = D3D11_QUERY_TIMESTAMP;
        hr = d3d11Device_->CreateQuery(&qdesc, &timerFrameEnd_);
        hcAssert(SUCCEEDED(hr));
#endif

        frameCounter_ = 0;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdDX11RenderDevice::Destroy()
    {
#ifdef HEART_DO_FRAMETIMES
        timerDisjoint_->Release();
        timerFrameStart_->Release();
        timerFrameEnd_->Release();
#endif
        
        if( mainDeviceCtx_ ) {
            mainDeviceCtx_->ClearState();
            mainDeviceCtx_->Flush();
            mainDeviceCtx_->Release();
        }
        if ( renderTargetView_ ) {
            renderTargetView_->Release();
        }
        if (pBackBuffer) {
            pBackBuffer->Release();
        }
//         if ( depthStencilView_ ) {
//             depthStencilView_->Release();
//         }
//         if ( depthStencil_ ) {
//             depthStencil_->Release();
//         }
        if ( mainSwapChain_ ) {
            mainSwapChain_->Release();
        }
        if ( d3d11Device_ ) {
            ULONG ref=d3d11Device_->Release();
            hcPrintf("D3DDevice Ref %u", ref);
            if (ref != 0) {
                HEART_D3D_OBJECT_REPORT(d3d11Device_);
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    DXGI_FORMAT hdDX11RenderDevice::toDXGIFormat(hTextureFormat format, hBool* compressed) {
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

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdDX11RenderDevice::BeginRender(hFloat* gpuTime)
    {
        //get prev GPU timer
        *gpuTime = -1.f;
#ifdef HEART_DO_FRAMETIMES
        if ( frameCounter_ > 0)
        {
            D3D11_QUERY_DATA_TIMESTAMP_DISJOINT gpustats;
            UINT64 fstart, fend;

            while(mainDeviceCtx_->GetData(timerDisjoint_, &gpustats, sizeof(gpustats), 0) != S_OK) {}
            while(mainDeviceCtx_->GetData(timerFrameStart_, &fstart, sizeof(fstart), 0) != S_OK) {}
            while(mainDeviceCtx_->GetData(timerFrameEnd_, &fend, sizeof(fend), 0) != S_OK) {}

            UINT64 delta = fend - fstart;
            *gpuTime = (hFloat)((hDouble)delta / ((hDouble)gpustats.Frequency/1000.0));
            if (gpustats.Disjoint) *gpuTime = -2.f;
        }

        //Start new timer
        mainDeviceCtx_->Begin(timerDisjoint_);
        mainDeviceCtx_->End(timerFrameStart_);
#endif

        mainDeviceCtx_->OMSetRenderTargets(0, 0, 0);
        
        //hFloat clearcolour[] = { 0.f, 0.f, 0.f, 1.f };
        //mainDeviceCtx_->ClearRenderTargetView( renderTargetView_, clearcolour );
        //mainDeviceCtx_->ClearDepthStencilView( depthStencilView_, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0 );
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdDX11RenderDevice::EndRender()
    {
        ++frameCounter_;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdDX11RenderDevice::SwapBuffers(hdDX11Texture* buffer)
    {
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

// It seems that resizing in DX11 is broken...?
#ifdef DO_RESIZE
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
#   ifdef USE_SWAPCHAIN_RESIZE
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
#   else
                BOOL fullscreen;
                IDXGIOutput* dxgioutput;
                mainSwapChain_->GetFullscreenState(&fullscreen, &dxgioutput);
                // Cannot release in fullscreen because it may cause a deadlock
                mainSwapChain_->SetFullscreenState(FALSE, NULL);
                mainSwapChain_->Release();
                mainSwapChain_=hNullptr;
                //Create a DX11 Swap chain
                DXGI_SWAP_CHAIN_DESC sd;
                ZeroMemory( &sd, sizeof(sd) );
                sd.BufferCount = 2;
                sd.BufferDesc.Width = width_;
                sd.BufferDesc.Height = height_;
                sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
                sd.BufferDesc.RefreshRate.Numerator = 60;
                sd.BufferDesc.RefreshRate.Denominator = 1;
                sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
                sd.OutputWindow = sysWindow_->GetSystemHandle()->hWnd_;
                sd.SampleDesc.Count = 1;
                sd.SampleDesc.Quality = 0;
                sd.Windowed = sysWindow_->getOwnWindow() ? !fullscreen : TRUE;
                sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
                IDXGIFactory1 * factory;
                hr = CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**)(&factory) );
                hcAssertMsg(SUCCEEDED(hr), "CreateDXGIFactory1 Failed %u", hr);
                hr = factory->CreateSwapChain(d3d11Device_, &sd, &mainSwapChain_);
                hcAssertMsg(SUCCEEDED(hr), "CreateSwapChain Failed %u", hr);
                // Get buffer and create a render-target-view.
                hr = mainSwapChain_->GetBuffer(0, __uuidof( ID3D11Texture2D),(void**)&pBackBuffer);
                hcAssertMsg(SUCCEEDED(hr), "mainSwapChain::GetBuffer failed 0x%x", hr);
                HEART_D3D_DEBUG_NAME_OBJECT(pBackBuffer, "back buffer");
                // TODO: Perform error handling here!
                hr = d3d11Device_->CreateRenderTargetView(pBackBuffer, NULL, &renderTargetView_);
                hcAssertMsg(SUCCEEDED(hr), "d3d11Device::CreateRenderTargetView failed 0x%x", hr);
                HEART_D3D_DEBUG_NAME_OBJECT(renderTargetView_, "back buffer target view");
#   endif

                if (resizeCallback_.isValid()) {
                    resizeCallback_(width_, height_);
                }
            }
        }
#endif
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdDX11RenderDevice::InitialiseRenderSubmissionCtx( hdDX11RenderSubmissionCtx* ctx )
    {
        HRESULT hr;
        ID3D11DeviceContext* rsc;
        hr = d3d11Device_->CreateDeferredContext( 0, &rsc );
        hcAssert( SUCCEEDED( hr ) );
        ctx->SetDeviceCtx( rsc, alloc_, free_ );
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdDX11RenderDevice::DestroyRenderSubmissionCtx( hdDX11RenderSubmissionCtx* ctx )
    {
        ctx->GetDeviceCtx()->Release();
        ctx->SetDeviceCtx( NULL, NULL, NULL );
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdDX11RenderDevice::InitialiseMainRenderSubmissionCtx( hdDX11RenderSubmissionCtx* ctx )
    {
        ctx->SetDeviceCtx( mainDeviceCtx_, alloc_, free_ );
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hShaderProfile hdDX11RenderDevice::getProfileFromString(const hChar* str) {
        for (hUint i=0; i<hStaticArraySize(s_shaderProfileNames); ++i) {
            if (hStrICmp(s_shaderProfileNames[i],str) == 0) {
                return (hShaderProfile)i;
            }
        }
        return eShaderProfile_Max;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hdDX11ShaderProgram* hdDX11RenderDevice::compileShaderFromSourceDevice(
        const hChar* shaderProg, hSize_t len, const hChar* entry, 
        hShaderProfile profile, hIIncludeHandler* includesimpl, 
        hShaderDefine* defines, hUint ndefines, hdDX11ShaderProgram* out) {
        HRESULT hr;
        hSourceIncludeHandler includes(includesimpl);
        hdDX11ShaderProgram* shader=out;
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

        if (ndefines > 0) {
            macros=(D3D_SHADER_MACRO*)hAlloca((ndefines+1)*sizeof(D3D_SHADER_MACRO));
            for (hUint di=0; di<ndefines; ++di) {
                macros[di].Name=defines[di].define_;
                macros[di].Definition=defines[di].value_;
            }
            macros[ndefines].Name=hNullptr;
            macros[ndefines].Definition=hNullptr;
        }
        
        hr=D3DCompile(shaderProg, len, NULL, macros, &includes, entry, profileStr, compileflags, 0, &codeBlob, &errorBlob);
        if (errorBlob) {
            hcPrintf("Shader Output:\n%s", errorBlob->GetBufferPointer());
            errorBlob->Release();
        }
        if (FAILED(hr)) {
            return NULL;
        }
        out=compileShaderDevice((hChar*)codeBlob->GetBufferPointer(), codeBlob->GetBufferSize(), type, out);
        codeBlob->Release();
        return out;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hdDX11ShaderProgram* hdDX11RenderDevice::compileShaderDevice(const hChar* shaderProg, 
    hSize_t len, hShaderType type, hdDX11ShaderProgram* out) {
        HRESULT hr;
        hdDX11ShaderProgram* shader = out;
        shader->type_ = type;
        shader->blobLen_= 0;
        shader->shaderBlob_= hNullptr;

        if ( type == ShaderType_FRAGMENTPROG ) {
            hr = d3d11Device_->CreatePixelShader( shaderProg, len, NULL, &shader->pixelShader_ );
            hcAssert( SUCCEEDED( hr ) );
            hr = D3DReflect( shaderProg, len, IID_ID3D11ShaderReflection, (void**)&shader->shaderInfo_ );
            hcAssert( SUCCEEDED( hr ) );
        } else if ( type == ShaderType_VERTEXPROG ) {
            ID3DBlob* inputBlob;
            hr = d3d11Device_->CreateVertexShader( shaderProg, len, NULL, &shader->vertexShader_ );
            hcAssert( SUCCEEDED( hr ) );
            hr = D3DReflect( shaderProg, len, IID_ID3D11ShaderReflection, (void**)&shader->shaderInfo_ );
            hcAssert( SUCCEEDED( hr ) );

            hr = D3DGetInputSignatureBlob(shaderProg, len, &inputBlob);
            hcAssert(SUCCEEDED(hr));
            shader->blobLen_= (hUint)inputBlob->GetBufferSize();
            hcAssert(shader->blobLen_ == inputBlob->GetBufferSize());
            shader->shaderBlob_= new hUint8[shader->blobLen_];
            hMemCpy(shader->shaderBlob_, inputBlob->GetBufferPointer(), shader->blobLen_);
            inputBlob->Release();
        } else if (type==ShaderType_GEOMETRYPROG) {
            hr=d3d11Device_->CreateGeometryShader(shaderProg, len, NULL, &shader->geomShader_);
            hcAssert( SUCCEEDED( hr ) );
            hr = D3DReflect( shaderProg, len, IID_ID3D11ShaderReflection, (void**)&shader->shaderInfo_ );
            hcAssert( SUCCEEDED( hr ) );
        } else if (type==ShaderType_DOMAINPROG) {
            hr=d3d11Device_->CreateDomainShader(shaderProg, len, NULL, &shader->domainShader_);
            hcAssert( SUCCEEDED( hr ) );
            hr = D3DReflect( shaderProg, len, IID_ID3D11ShaderReflection, (void**)&shader->shaderInfo_ );
            hcAssert( SUCCEEDED( hr ) );
        } else if (type==ShaderType_HULLPROG) {
            hr=d3d11Device_->CreateHullShader(shaderProg, len, NULL, &shader->hullShader_);
            hcAssert( SUCCEEDED( hr ) );
            hr = D3DReflect( shaderProg, len, IID_ID3D11ShaderReflection, (void**)&shader->shaderInfo_ );
            hcAssert( SUCCEEDED( hr ) );
        } else if (type==ShaderType_COMPUTEPROG) {
            hr=d3d11Device_->CreateComputeShader(shaderProg, len, NULL, &shader->computeShader_);
            hcAssert( SUCCEEDED( hr ) );
            hr = D3DReflect( shaderProg, len, IID_ID3D11ShaderReflection, (void**)&shader->shaderInfo_ );
            hcAssert( SUCCEEDED( hr ) );
        }

        return shader;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdDX11RenderDevice::destroyShaderDevice(hdDX11ShaderProgram* prog)
    {
        if (!prog) return;
        if (prog->shaderInfo_) {
            prog->shaderInfo_->Release();
            prog->shaderInfo_ = NULL;
        }
        if (prog->vertexShader_ && prog->type_ == ShaderType_VERTEXPROG) {
            prog->vertexShader_->Release();
            prog->vertexShader_ = NULL;
        }
        if (prog->pixelShader_ && prog->type_ == ShaderType_FRAGMENTPROG) {
            prog->pixelShader_->Release();
            prog->pixelShader_ = NULL;
        }
        if (prog->geomShader_ && prog->type_ == ShaderType_GEOMETRYPROG) {
            prog->geomShader_->Release();
            prog->geomShader_ = NULL;
        }
        if (prog->hullShader_ && prog->type_ == ShaderType_HULLPROG) {
            prog->hullShader_->Release();
            prog->hullShader_ = NULL;
        }
        if (prog->domainShader_ && prog->type_ == ShaderType_DOMAINPROG) {
            prog->domainShader_->Release();
            prog->domainShader_ = NULL;
        }
        if (prog->computeShader_ && prog->type_ == ShaderType_COMPUTEPROG) {
            prog->computeShader_->Release();
            prog->computeShader_ = NULL;
        }
        delete[] prog->shaderBlob_;
        prog->shaderBlob_ = nullptr;
        prog->blobLen_=0;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hdDX11Texture* hdDX11RenderDevice::createTextureDevice(hUint32 levels, hTextureFormat format, hMipDesc* initialData, hUint32 flags, hdDX11Texture* texture)
    {
        hcAssert(levels > 0 && initialData);
        HRESULT hr;
        hBool compressedFormat = hFalse;

        D3D11_TEXTURE2D_DESC desc;
        hZeroMem( &desc, sizeof(desc) );
        desc.Height             = initialData[0].height;
        desc.Width              = initialData[0].width;
        desc.MipLevels          = levels;
        desc.ArraySize          = 1;
        desc.SampleDesc.Count   = 1;
        desc.Format = toDXGIFormat(format, &compressedFormat);
        desc.Usage = (flags & RESOURCEFLAG_DYNAMIC) ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;
        desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        desc.BindFlags |= (flags & RESOURCEFLAG_RENDERTARGET) ? D3D11_BIND_RENDER_TARGET : 0; 
        desc.BindFlags |= (flags & RESOURCEFLAG_DEPTHTARGET) ? D3D11_BIND_DEPTH_STENCIL : 0;
        desc.BindFlags |= (flags & RESOURCEFLAG_UNORDEREDACCESS) ? D3D11_BIND_UNORDERED_ACCESS : 0;
        desc.CPUAccessFlags  = (flags & RESOURCEFLAG_DYNAMIC) ? (D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE) : 0;
        desc.MiscFlags = 0;//(flags & RESOURCEFLAG_DEPTHTARGET) ? 0 : D3D11_RESOURCE_MISC_GENERATE_MIPS;

        D3D11_SUBRESOURCE_DATA* dataptr = (D3D11_SUBRESOURCE_DATA*)hAlloca(sizeof(D3D11_SUBRESOURCE_DATA)*desc.MipLevels+2);
        if ( initialData ) {
            for (hUint32 i = 0; i < desc.MipLevels; ++i)
            {
                D3D11_SUBRESOURCE_DATA& data = dataptr[i];
                data.pSysMem          = initialData[i].data;
                if (!data.pSysMem) {
                    dataptr=NULL;
                    break;
                }
                if ( compressedFormat )
                {
                    //D3D want the number of 4x4 blocks in the first row of the texture
                    if ( desc.Format == DXGI_FORMAT_BC1_UNORM || desc.Format == DXGI_FORMAT_BC1_UNORM_SRGB )
                    {
                        data.SysMemPitch = (( initialData[i].width + 3 )/4 )*8;
                    }
                    else
                    {
                        data.SysMemPitch = (( initialData[i].width + 3 )/4 )*16;
                    }
                }
                else
                {
                    data.SysMemPitch      = initialData[i].size / initialData[i].height;
                }
                data.SysMemSlicePitch = 0;
            }
        } else {
            dataptr=NULL;
        }

        hr = d3d11Device_->CreateTexture2D( &desc, dataptr, &texture->dx11Texture_ );
        hcAssert( SUCCEEDED( hr ) );

        return texture;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdDX11RenderDevice::destroyTextureDevice(hdDX11Texture* texture)
    {
        if (texture->dx11Texture_) {
            texture->dx11Texture_->Release();
            texture->dx11Texture_ = NULL;
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdDX11RenderDevice::createIndexBufferDevice( hUint32 sizeInBytes, const void* initialDataPtr, hUint32 flags, hdDX11IndexBuffer* idxBuf )
    {
        HRESULT hr;
        D3D11_BUFFER_DESC desc;
        D3D11_SUBRESOURCE_DATA initData;

        hZeroMem( &desc, sizeof(desc) );
        desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
        desc.ByteWidth = sizeInBytes;
        desc.Usage = (flags & RESOURCEFLAG_DYNAMIC) ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;
        desc.CPUAccessFlags = (flags & RESOURCEFLAG_DYNAMIC) ? D3D11_CPU_ACCESS_WRITE : 0;
        desc.MiscFlags = 0;

        hZeroMem( &initData, sizeof(initData) );
        initData.pSysMem = initialDataPtr;

        hr = d3d11Device_->CreateBuffer( &desc, initialDataPtr ? &initData : NULL, &idxBuf->buffer_ );
        hcAssert( SUCCEEDED( hr ) );

        idxBuf->flags_ = flags;
        idxBuf->dataSize_ = sizeInBytes;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdDX11RenderDevice::destroyIndexBufferDevice(hdDX11IndexBuffer* indexBuffer)
    {
        indexBuffer->buffer_->Release();
        indexBuffer->buffer_ = NULL;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hdDX11VertexLayout* hdDX11RenderDevice::CreateVertexLayout(const hInputLayoutDesc* inputdesc, hUint32 desccount, const void* shaderProg, hUint32 progLen)
    {
        HRESULT hr;
        hdDX11VertexLayout* layout = NULL;
        D3D11_INPUT_ELEMENT_DESC* elements = (D3D11_INPUT_ELEMENT_DESC*)hAlloca(sizeof(D3D11_INPUT_ELEMENT_DESC)*desccount);
        hUint32 stride;
        hUint32 inputLayoutId;
        hUint32 elementCount = BuildVertexFormatArray(inputdesc, desccount, &stride, &inputLayoutId, elements);
#pragma message ("TODO: Hash map this call (hash of input streams and vertex prog input), will probably a good chunk of memory")
        hr = d3d11Device_->CreateInputLayout( elements, elementCount, shaderProg, progLen, &layout );
        hcAssert( SUCCEEDED( hr ) );

        return layout;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdDX11RenderDevice::DestroyVertexLayout( hdDX11VertexLayout* layout )
    {
    
        layout->Release();
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdDX11RenderDevice::createVertexBufferDevice(hInputLayoutDesc* desc, hUint32 desccount, hUint stride, hUint32 sizeInBytes, const void* initialDataPtr, hUint32 flags, hdDX11VertexBuffer* vtxBuf)
    {
        HRESULT hr;
        D3D11_BUFFER_DESC bufdesc;
        D3D11_SUBRESOURCE_DATA initData;

        hZeroMem(vtxBuf->streamLayoutDesc_, sizeof(vtxBuf->streamLayoutDesc_));
        hZeroMem(&bufdesc, sizeof(bufdesc));
        bufdesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        bufdesc.ByteWidth = sizeInBytes;
        bufdesc.Usage = (flags & RESOURCEFLAG_DYNAMIC) ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;
        bufdesc.CPUAccessFlags = (flags & RESOURCEFLAG_DYNAMIC) ? D3D11_CPU_ACCESS_WRITE : 0;
        bufdesc.MiscFlags = 0;

        hZeroMem( &initData, sizeof(initData) );
        initData.pSysMem = initialDataPtr;

        hr = d3d11Device_->CreateBuffer( &bufdesc, initialDataPtr ? &initData : NULL, &vtxBuf->buffer_ );
        hcAssert( SUCCEEDED( hr ) );

        vtxBuf->flags_=flags;
        vtxBuf->stride_=stride;
        vtxBuf->dataSize_=sizeInBytes;
        hMemCpy(vtxBuf->streamLayoutDesc_, desc, sizeof(hInputLayoutDesc)*desccount);
        vtxBuf->streamDescCount_=desccount;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdDX11RenderDevice::destroyVertexBufferDevice(hdDX11VertexBuffer* vtxBuf)
    {
        vtxBuf->buffer_->Release();
        vtxBuf->buffer_ = NULL;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdDX11RenderDevice::createBlendStateDevice(const hBlendStateDesc& desc, hdDX11BlendState* out)
    {
        HRESULT hr;
        D3D11_BLEND_DESC blendDesc;

        //Build device state desc
        hZeroMem( &blendDesc, sizeof(blendDesc) );
        blendDesc.IndependentBlendEnable = FALSE;
            
        switch( desc.blendEnable_ )
        {
        case RSV_ENABLE:    blendDesc.RenderTarget[0].BlendEnable = TRUE;   break;
        case RSV_DISABLE:   blendDesc.RenderTarget[0].BlendEnable = FALSE;  break;
        default:            hcAssert( hFalse );
        }

        switch( desc.srcBlend_ )
        {
        case RSV_BLEND_OP_ZERO:           blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ZERO            ;   break;
        case RSV_BLEND_OP_ONE:            blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE             ;   break;
        case RSV_BLEND_OP_SRC_COLOUR:     blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_COLOR       ;   break;
        case RSV_BLEND_OP_INVSRC_COLOUR:  blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_INV_SRC_COLOR   ;   break;
        case RSV_BLEND_OP_SRC_ALPHA:      blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA       ;   break;
        case RSV_BLEND_OP_INVSRC_ALPHA:   blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_INV_SRC_ALPHA   ;   break;
        case RSV_BLEND_OP_DEST_ALPHA:     blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_DEST_ALPHA      ;   break;
        case RSV_BLEND_OP_INVDEST_ALPHA:  blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_INV_DEST_ALPHA  ;   break;
        case RSV_BLEND_OP_DEST_COLOUR:    blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_DEST_COLOR      ;   break;
        case RSV_BLEND_OP_INVDEST_COLOUR: blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_INV_DEST_COLOR  ;   break;
        default: hcAssert( false );
        }

        switch( desc.destBlend_ )
        {
        case RSV_BLEND_OP_ZERO:           blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ZERO            ;   break;
        case RSV_BLEND_OP_ONE:            blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE             ;   break;
        case RSV_BLEND_OP_SRC_COLOUR:     blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_SRC_COLOR       ;   break;
        case RSV_BLEND_OP_INVSRC_COLOUR:  blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_COLOR   ;   break;
        case RSV_BLEND_OP_SRC_ALPHA:      blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_SRC_ALPHA       ;   break;
        case RSV_BLEND_OP_INVSRC_ALPHA:   blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA   ;   break;
        case RSV_BLEND_OP_DEST_ALPHA:     blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_DEST_ALPHA      ;   break;
        case RSV_BLEND_OP_INVDEST_ALPHA:  blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_DEST_ALPHA  ;   break;
        case RSV_BLEND_OP_DEST_COLOUR:    blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_DEST_COLOR      ;   break;
        case RSV_BLEND_OP_INVDEST_COLOUR: blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_DEST_COLOR  ;   break;
        default: hcAssert( false );
        }


        switch( desc.srcBlendAlpha_ )
        {
        case RSV_BLEND_OP_ZERO:           blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ZERO            ;   break;
        case RSV_BLEND_OP_ONE:            blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE             ;   break;
        case RSV_BLEND_OP_SRC_COLOUR:     blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_SRC_COLOR       ;   break;
        case RSV_BLEND_OP_INVSRC_COLOUR:  blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_INV_SRC_COLOR   ;   break;
        case RSV_BLEND_OP_SRC_ALPHA:      blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_SRC_ALPHA       ;   break;
        case RSV_BLEND_OP_INVSRC_ALPHA:   blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA   ;   break;
        case RSV_BLEND_OP_DEST_ALPHA:     blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_DEST_ALPHA      ;   break;
        case RSV_BLEND_OP_INVDEST_ALPHA:  blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_INV_DEST_ALPHA  ;   break;
        case RSV_BLEND_OP_DEST_COLOUR:    blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_DEST_COLOR      ;   break;
        case RSV_BLEND_OP_INVDEST_COLOUR: blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_INV_DEST_COLOR  ;   break;
        default: hcAssert( false );
        }

        switch( desc.destBlendAlpha_ )
        {
        case RSV_BLEND_OP_ZERO:           blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO            ;   break;
        case RSV_BLEND_OP_ONE:            blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE             ;   break;
        case RSV_BLEND_OP_SRC_COLOUR:     blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_SRC_COLOR       ;   break;
        case RSV_BLEND_OP_INVSRC_COLOUR:  blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_COLOR   ;   break;
        case RSV_BLEND_OP_SRC_ALPHA:      blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_SRC_ALPHA       ;   break;
        case RSV_BLEND_OP_INVSRC_ALPHA:   blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA   ;   break;
        case RSV_BLEND_OP_DEST_ALPHA:     blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_DEST_ALPHA      ;   break;
        case RSV_BLEND_OP_INVDEST_ALPHA:  blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_DEST_ALPHA  ;   break;
        case RSV_BLEND_OP_DEST_COLOUR:    blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_DEST_COLOR      ;   break;
        case RSV_BLEND_OP_INVDEST_COLOUR: blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_DEST_COLOR  ;   break;
        default: hcAssert( false );
        }

        switch( desc.blendOp_ )
        {
        case RSV_BLEND_FUNC_ADD:   blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;          break;
        case RSV_BLEND_FUNC_SUB:   blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_REV_SUBTRACT; break;
        case RSV_BLEND_FUNC_MIN:   blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_MIN;          break;
        case RSV_BLEND_FUNC_MAX:   blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_MAX;          break;
        default:            hcAssert( hFalse );
        }

        switch( desc.blendOpAlpha_ )
        {
        case RSV_BLEND_FUNC_ADD:   blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;          break;
        case RSV_BLEND_FUNC_SUB:   blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_REV_SUBTRACT; break;
        case RSV_BLEND_FUNC_MIN:   blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_MIN;          break;
        case RSV_BLEND_FUNC_MAX:   blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_MAX;          break;
        default:            hcAssert( hFalse );
        }

        blendDesc.RenderTarget[0].RenderTargetWriteMask |= (desc.renderTargetWriteMask_ & RSV_COLOUR_WRITE_RED)   ? D3D11_COLOR_WRITE_ENABLE_RED   : 0;
        blendDesc.RenderTarget[0].RenderTargetWriteMask |= (desc.renderTargetWriteMask_ & RSV_COLOUR_WRITE_BLUE)  ? D3D11_COLOR_WRITE_ENABLE_BLUE  : 0;
        blendDesc.RenderTarget[0].RenderTargetWriteMask |= (desc.renderTargetWriteMask_ & RSV_COLOUR_WRITE_GREEN) ? D3D11_COLOR_WRITE_ENABLE_GREEN : 0;
        blendDesc.RenderTarget[0].RenderTargetWriteMask |= (desc.renderTargetWriteMask_ & RSV_COLOUR_WRITE_ALPHA) ? D3D11_COLOR_WRITE_ENABLE_ALPHA : 0;

        hr = d3d11Device_->CreateBlendState( &blendDesc, &out->stateObj_ );
        hcAssert( SUCCEEDED( hr ) );
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdDX11RenderDevice::destroyBlendStateDevice( hdDX11BlendState* state )
    {
        hcAssert( state );
        state->stateObj_->Release();
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdDX11RenderDevice::createRasterizerStateDevice( const hRasterizerStateDesc& desc, hdDX11RasterizerState* out)
    {
        HRESULT hr;
        D3D11_RASTERIZER_DESC rasDesc;

        rasDesc.FrontCounterClockwise = FALSE;
        rasDesc.AntialiasedLineEnable = TRUE;
        rasDesc.MultisampleEnable = FALSE;

        switch ( desc.fillMode_ )
        {
        case RSV_FILL_MODE_SOLID:       rasDesc.FillMode = D3D11_FILL_SOLID;    break;
        case RSV_FILL_MODE_WIREFRAME:   rasDesc.FillMode = D3D11_FILL_WIREFRAME;break;
        default: hcAssert( hFalse );
        }

        switch ( desc.cullMode_ )
        {
        case RSV_CULL_MODE_NONE:        rasDesc.CullMode = D3D11_CULL_NONE;     break;
        case RSV_CULL_MODE_CW:          rasDesc.CullMode = D3D11_CULL_FRONT;    break;
        case RSV_CULL_MODE_CCW:         rasDesc.CullMode = D3D11_CULL_BACK;     break;
        default: hcAssert( hFalse );
        }

        switch ( desc.scissorEnable_ )
        {
        case RSV_ENABLE:    rasDesc.ScissorEnable = TRUE;  break;
        case RSV_DISABLE:   rasDesc.ScissorEnable = FALSE; break;
        default: hcAssert( hFalse );
        }

        switch ( desc.depthClipEnable_ )
        {
        case RSV_ENABLE:    rasDesc.DepthClipEnable = TRUE;  break;
        case RSV_DISABLE:   rasDesc.DepthClipEnable = FALSE; break;
        default: hcAssert( hFalse );
        }

        rasDesc.DepthBias            = (INT)desc.depthBias_;
        rasDesc.DepthBiasClamp       = desc.depthBiasClamp_;
        rasDesc.SlopeScaledDepthBias = desc.slopeScaledDepthBias_;

        hr = d3d11Device_->CreateRasterizerState( &rasDesc, &out->stateObj_ );
        hcAssert( SUCCEEDED( hr ) );
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdDX11RenderDevice::destroyRasterizerStateDevice( hdDX11RasterizerState* state )
    {
        hcAssert( state );
        state->stateObj_->Release();
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdDX11RenderDevice::createDepthStencilStateDevice( const hDepthStencilStateDesc& desc, hdDX11DepthStencilState* out)
    {
        HRESULT hr;
        D3D11_DEPTH_STENCIL_DESC dsDesc;

        switch ( desc.depthEnable_ )
        {
        case RSV_ENABLE:        dsDesc.DepthEnable = TRUE;  break;
        case RSV_DISABLE:       dsDesc.DepthEnable = FALSE; break;
        default: hcAssert( hFalse );
        }

        switch ( desc.depthFunc_ )
        {
        case RSV_Z_CMP_NEVER:           dsDesc.DepthFunc = D3D11_COMPARISON_NEVER;          break;
        case RSV_Z_CMP_LESS:            dsDesc.DepthFunc = D3D11_COMPARISON_LESS;           break;
        case RSV_Z_CMP_EQUAL:           dsDesc.DepthFunc = D3D11_COMPARISON_EQUAL;          break;
        case RSV_Z_CMP_LESSEQUAL:       dsDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;     break;
        case RSV_Z_CMP_GREATER:         dsDesc.DepthFunc = D3D11_COMPARISON_GREATER;        break;
        case RSV_Z_CMP_NOT_EQUAL:       dsDesc.DepthFunc = D3D11_COMPARISON_NOT_EQUAL;      break;
        case RSV_Z_CMP_GREATER_EQUAL:   dsDesc.DepthFunc = D3D11_COMPARISON_GREATER_EQUAL;  break;
        case RSV_Z_CMP_ALWAYS:          dsDesc.DepthFunc = D3D11_COMPARISON_ALWAYS;         break;
        default: hcAssert( hFalse );
        }

        switch ( desc.depthWriteMask_ )
        {
        case RSV_ENABLE:        dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;  break;
        case RSV_DISABLE:       dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO; break;
        default: hcAssert( hFalse );
        }

        switch ( desc.stencilEnable_ )
        {
        case RSV_ENABLE:        dsDesc.StencilEnable = TRUE;  break;
        case RSV_DISABLE:       dsDesc.StencilEnable = FALSE; break;
        default: hcAssert( hFalse );
        }

        dsDesc.StencilReadMask = (UINT8)desc.stencilReadMask_;
        dsDesc.StencilWriteMask = (UINT8)desc.stencilWriteMask_;

        switch ( desc.stencilFunc_ )
        {
        case RSV_SF_CMP_NEVER:          dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_NEVER;          break;
        case RSV_SF_CMP_LESS:           dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_LESS;           break;
        case RSV_SF_CMP_EQUAL:          dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_EQUAL;          break;
        case RSV_SF_CMP_LESSEQUAL:      dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_LESS_EQUAL;     break;
        case RSV_SF_CMP_GREATER:        dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_GREATER;        break;
        case RSV_SF_CMP_NOT_EQUAL:      dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_NOT_EQUAL;      break;     
        case RSV_SF_CMP_GREATER_EQUAL:  dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_GREATER_EQUAL;  break;
        case RSV_SF_CMP_ALWAYS:         dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;         break;
        default: hcAssert( hFalse );
        }

        switch ( desc.stencilPassOp_ )
        {
        case RSV_SO_KEEP:    dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;     break;
        case RSV_SO_ZERO:    dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_ZERO;     break;
        case RSV_SO_REPLACE: dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;  break;
        case RSV_SO_INCRSAT: dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_INCR_SAT; break;
        case RSV_SO_DECRSAT: dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_DECR_SAT; break;
        case RSV_SO_INVERT:  dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_INVERT;   break;
        case RSV_SO_INCR:    dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_INCR;     break;
        case RSV_SO_DECR:    dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_DECR;     break;
        }

        switch ( desc.stencilFailOp_ )
        {
        case RSV_SO_KEEP:    dsDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;     break;
        case RSV_SO_ZERO:    dsDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_ZERO;     break;
        case RSV_SO_REPLACE: dsDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_REPLACE;  break;
        case RSV_SO_INCRSAT: dsDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_INCR_SAT; break;
        case RSV_SO_DECRSAT: dsDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_DECR_SAT; break;
        case RSV_SO_INVERT:  dsDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_INVERT;   break;
        case RSV_SO_INCR:    dsDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_INCR;     break;
        case RSV_SO_DECR:    dsDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_DECR;     break;
        }

        switch ( desc.stencilDepthFailOp_ )
        {
        case RSV_SO_KEEP:    dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;     break;
        case RSV_SO_ZERO:    dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_ZERO;     break;
        case RSV_SO_REPLACE: dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_REPLACE;  break;
        case RSV_SO_INCRSAT: dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR_SAT; break;
        case RSV_SO_DECRSAT: dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR_SAT; break;
        case RSV_SO_INVERT:  dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INVERT;   break;
        case RSV_SO_INCR:    dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;     break;
        case RSV_SO_DECR:    dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;     break;
        }

        switch ( desc.stencilFunc_ )
        {
        case RSV_SF_CMP_NEVER:          dsDesc.BackFace.StencilFunc = D3D11_COMPARISON_NEVER;          break;
        case RSV_SF_CMP_LESS:           dsDesc.BackFace.StencilFunc = D3D11_COMPARISON_LESS;           break;
        case RSV_SF_CMP_EQUAL:          dsDesc.BackFace.StencilFunc = D3D11_COMPARISON_EQUAL;          break;
        case RSV_SF_CMP_LESSEQUAL:      dsDesc.BackFace.StencilFunc = D3D11_COMPARISON_LESS_EQUAL;     break;
        case RSV_SF_CMP_GREATER:        dsDesc.BackFace.StencilFunc = D3D11_COMPARISON_GREATER;        break;
        case RSV_SF_CMP_NOT_EQUAL:      dsDesc.BackFace.StencilFunc = D3D11_COMPARISON_NOT_EQUAL;      break;     
        case RSV_SF_CMP_GREATER_EQUAL:  dsDesc.BackFace.StencilFunc = D3D11_COMPARISON_GREATER_EQUAL;  break;
        case RSV_SF_CMP_ALWAYS:         dsDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;         break;
        default: hcAssert( hFalse );
        }

        switch ( desc.stencilPassOp_ )
        {
        case RSV_SO_KEEP:    dsDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;     break;
        case RSV_SO_ZERO:    dsDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_ZERO;     break;
        case RSV_SO_REPLACE: dsDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;  break;
        case RSV_SO_INCRSAT: dsDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_INCR_SAT; break;
        case RSV_SO_DECRSAT: dsDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_DECR_SAT; break;
        case RSV_SO_INVERT:  dsDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_INVERT;   break;
        case RSV_SO_INCR:    dsDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_INCR;     break;
        case RSV_SO_DECR:    dsDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_DECR;     break;
        }

        switch ( desc.stencilFailOp_ )
        {
        case RSV_SO_KEEP:    dsDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;     break;
        case RSV_SO_ZERO:    dsDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_ZERO;     break;
        case RSV_SO_REPLACE: dsDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_REPLACE;  break;
        case RSV_SO_INCRSAT: dsDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_INCR_SAT; break;
        case RSV_SO_DECRSAT: dsDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_DECR_SAT; break;
        case RSV_SO_INVERT:  dsDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_INVERT;   break;
        case RSV_SO_INCR:    dsDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_INCR;     break;
        case RSV_SO_DECR:    dsDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_DECR;     break;
        }

        switch ( desc.stencilDepthFailOp_ )
        {
        case RSV_SO_KEEP:    dsDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;     break;
        case RSV_SO_ZERO:    dsDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_ZERO;     break;
        case RSV_SO_REPLACE: dsDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_REPLACE;  break;
        case RSV_SO_INCRSAT: dsDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR_SAT; break;
        case RSV_SO_DECRSAT: dsDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR_SAT; break;
        case RSV_SO_INVERT:  dsDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_INVERT;   break;
        case RSV_SO_INCR:    dsDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;     break;
        case RSV_SO_DECR:    dsDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;     break;
        }

        hr = d3d11Device_->CreateDepthStencilState( &dsDesc, &out->stateObj_ );
        hcAssert( SUCCEEDED( hr ) );
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdDX11RenderDevice::destroyDepthStencilStateDevice( hdDX11DepthStencilState* state )
    {
        hcAssert( state );
        state->stateObj_->Release();
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdDX11RenderDevice::createSamplerStateDevice(const hSamplerStateDesc& desc, hdDX11SamplerState* out)
    {
        HRESULT hr;
        D3D11_SAMPLER_DESC samDesc;

        switch ( desc.filter_ )
        {
        case SSV_POINT:             samDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT; break;
        case SSV_LINEAR:            samDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;  break;
        case SSV_ANISOTROPIC:       samDesc.Filter = D3D11_FILTER_ANISOTROPIC;        break;
        default: hcAssert( hFalse );
        }

        switch ( desc.addressU_ )
        {
        case SSV_WRAP:      samDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;   break;
        case SSV_MIRROR:    samDesc.AddressU = D3D11_TEXTURE_ADDRESS_MIRROR; break;
        case SSV_CLAMP:     samDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;  break;
        case SSV_BORDER:    samDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER; break;
        default: hcAssert( hFalse );
        }

        switch ( desc.addressV_ )
        {
        case SSV_WRAP:      samDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;   break;
        case SSV_MIRROR:    samDesc.AddressV = D3D11_TEXTURE_ADDRESS_MIRROR; break;
        case SSV_CLAMP:     samDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;  break;
        case SSV_BORDER:    samDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER; break;
        default: hcAssert( hFalse );
        }

        switch ( desc.addressW_ )
        {
        case SSV_WRAP:      samDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;   break;
        case SSV_MIRROR:    samDesc.AddressW = D3D11_TEXTURE_ADDRESS_MIRROR; break;
        case SSV_CLAMP:     samDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;  break;
        case SSV_BORDER:    samDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER; break;
        default: hcAssert( hFalse );
        }

        samDesc.BorderColor[0] = desc.borderColour_.r_;
        samDesc.BorderColor[1] = desc.borderColour_.g_;
        samDesc.BorderColor[2] = desc.borderColour_.b_;
        samDesc.BorderColor[3] = desc.borderColour_.a_;

        samDesc.MaxAnisotropy   = desc.maxAnisotropy_;
        samDesc.MinLOD          = desc.minLOD_;
        samDesc.MaxLOD          = desc.maxLOD_;
        samDesc.MipLODBias      = desc.mipLODBias_;

        samDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;

        hr = d3d11Device_->CreateSamplerState( &samDesc, &out->stateObj_ );
        hcAssert( SUCCEEDED( hr ) );
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdDX11RenderDevice::destroySamplerStateDevice( hdDX11SamplerState* state )
    {
        hcAssert( state );
        state->stateObj_->Release();
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hUint32 hdDX11RenderDevice::computeVertexLayoutStride(hInputLayoutDesc* desc, hUint32 desccount)
    {
        hUint32 stride = 0;

        for (hUint32 i = 0; i < desccount; ++i)
        {
            switch(desc[i].getInputFormat())
            {
            case eIF_FLOAT4: stride += 4*sizeof(hFloat); break;
            case eIF_FLOAT3: stride += 3*sizeof(hFloat); break;
            case eIF_FLOAT2: stride += 2*sizeof(hFloat); break;
            case eIF_FLOAT1: stride += sizeof(hFloat);   break;
            case eIF_UBYTE4_UNORM:
            case eIF_UBYTE4_SNORM: stride += 4*sizeof(hByte); break;
            default: hcAssertFailMsg("Invalid size format");
            }
        }

        return stride;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hUint32 hdDX11RenderDevice::BuildVertexFormatArray(const hInputLayoutDesc* desc, hUint32 desccount, 
    hUint32* stride, hUint32* fmtID, D3D11_INPUT_ELEMENT_DESC* elements)
    {
        hUint32 elementsadded = 0;
        WORD offset = 0;
        *stride = 0;
        *fmtID=0;

        for (hUint32 i = 0; i < desccount; ++i)
        {
            hZeroMem(&elements[i], sizeof(D3D11_INPUT_ELEMENT_DESC));

            elements[i].SemanticName=desc[i].getSemanticName();
            /*switch(desc[i].semantic_)
            {
            case eIS_POSITION:  elements[i].SemanticName = "POSITION";  break;
            case eIS_TEXCOORD:  elements[i].SemanticName = "TEXCOORD";  break;
            case eIS_NORMAL:    elements[i].SemanticName = "NORMAL";    break;
            case eIS_TANGENT:   elements[i].SemanticName = "TANGENT";   break;
            case eIS_BITANGENT: elements[i].SemanticName = "BITANGENT"; break;
            case eIS_COLOUR:    elements[i].SemanticName = "COLOR";     break;
            case eIS_INSTANCE:  elements[i].SemanticName = "INSTANCE";  break;
            }*/

            switch(desc[i].getInputFormat())
            {
            case eIF_FLOAT4: *stride += 4*sizeof(hFloat); elements[i].Format = DXGI_FORMAT_R32G32B32A32_FLOAT; break;
            case eIF_FLOAT3: *stride += 3*sizeof(hFloat); elements[i].Format = DXGI_FORMAT_R32G32B32_FLOAT; break;
            case eIF_FLOAT2: *stride += 2*sizeof(hFloat); elements[i].Format = DXGI_FORMAT_R32G32_FLOAT; break;
            case eIF_FLOAT1: *stride += sizeof(hFloat);   elements[i].Format = DXGI_FORMAT_R32_FLOAT; break;
            case eIF_UBYTE4_UNORM: *stride += 4*sizeof(hByte); elements[i].Format = DXGI_FORMAT_R8G8B8A8_UNORM; break;
            case eIF_UBYTE4_SNORM: *stride += 4*sizeof(hByte); elements[i].Format = DXGI_FORMAT_R8G8B8A8_SNORM; break;
            }

            elements[i].InputSlot=desc[i].getInputStream();
            elements[i].SemanticIndex = desc[i].getSemanticIndex();
            elements[i].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
            elements[i].InputSlotClass = desc[i].getInstanceRepeat() == 0 ? D3D11_INPUT_PER_VERTEX_DATA : D3D11_INPUT_PER_INSTANCE_DATA;
            elements[i].InstanceDataStepRate = desc[i].getInstanceRepeat();

            ++elementsadded;
        }

        if (elementsadded)
        {
            *fmtID=hCRC32::FullCRC((hChar*)elements, sizeof(D3D11_INPUT_ELEMENT_DESC)*elementsadded);
        }

        return elementsadded;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdDX11RenderDevice::createBufferDevice(hUint size, void* initdata, hUint flags, hUint stride, hdRenderBuffer* block) {
        hcAssert(block);
        HRESULT hr;
        D3D11_BUFFER_DESC desc={0};
        D3D11_SUBRESOURCE_DATA resdata={0};
        D3D11_SUBRESOURCE_DATA* subresdata=initdata ? &resdata : NULL;
        //hZeroMem( &desc, sizeof(desc) );
        resdata.pSysMem=initdata;
        desc.BindFlags = 0;
        desc.BindFlags |= flags & eResourceFlag_ConstantBuffer ? D3D11_BIND_CONSTANT_BUFFER : 0;
        desc.BindFlags |= flags & eResourceFlag_ShaderResource ? D3D11_BIND_SHADER_RESOURCE : 0;
        desc.BindFlags |= flags & eResourceFlag_StreamOut ? D3D11_BIND_STREAM_OUTPUT : 0;
        desc.Usage = D3D11_USAGE_DYNAMIC;
        desc.ByteWidth = hMax(size, 16);
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        desc.MiscFlags = 0;
        desc.MiscFlags |= flags & eResourceFlag_StructuredBuffer ? D3D11_RESOURCE_MISC_BUFFER_STRUCTURED : 0;
        desc.StructureByteStride=stride;
        hr = d3d11Device_->CreateBuffer( &desc, subresdata, &block->buffer_ );
        hcAssert( SUCCEEDED( hr ) );
        block->size_ = size;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdDX11RenderDevice::destroyConstantBlockDevice(hdDX11Buffer* constBlock)
    {
        constBlock->buffer_->Release();
        constBlock->buffer_ = NULL;
        constBlock->size_ = 0;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdDX11RenderDevice::ReleaseCommandBuffer( hdDX11CommandBuffer cmdBuf )
    {
        cmdBuf->Release();
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    const hChar* hdDX11RenderDevice::getDebugShaderSource(hDebugShaderID shaderid) {
        hcAssert(shaderid < eDebugShaderMax); 
        return s_debugSrcs[shaderid];
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdDX11RenderDevice::createComputeUAV(hdDX11Texture* res, hTextureFormat viewformat, hUint mip, hdDX11ComputeUAV* outres) {
        hcAssert(res && outres);
        D3D11_UNORDERED_ACCESS_VIEW_DESC desc;
        HRESULT hr;
        hZeroMem(&desc, sizeof(desc));
        desc.Format=toDXGIFormat(viewformat, NULL);
        desc.ViewDimension=D3D11_UAV_DIMENSION_TEXTURE2D;
        desc.Texture2D.MipSlice=mip;
        hr = d3d11Device_->CreateUnorderedAccessView(res->dx11Texture_, &desc, &outres->uav_);
        hcAssertMsg(SUCCEEDED(hr), "Failed to create Unordered Access View");
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdDX11RenderDevice::destroyComputeUAV(hdDX11ComputeUAV* uav) {
        hcAssert(uav);
        if (uav->uav_) {
            uav->uav_->Release();
            uav->uav_=NULL;
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdDX11RenderDevice::createShaderResourseViewDevice(hdDX11Texture* texture, const hShaderResourceViewDesc& desc, hdDX11ShaderResourceView* outsrv) {
        createShaderResourseViewDevice(texture->dx11Texture_, desc, outsrv);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdDX11RenderDevice::createShaderResourseViewDevice(hdDX11Buffer* cb, const hShaderResourceViewDesc& desc, hdDX11ShaderResourceView* outsrv) {
        createShaderResourseViewDevice(cb->buffer_, desc, outsrv);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdDX11RenderDevice::createShaderResourseViewDevice(ID3D11Resource* res, const hShaderResourceViewDesc& desc, hdDX11ShaderResourceView* outsrv) {
        hcAssert(outsrv);
        HRESULT hr;
        D3D11_SHADER_RESOURCE_VIEW_DESC ddesc;
        hZeroMem(&ddesc, sizeof(ddesc));

        switch (desc.resourceType_) {
        case eRenderResourceType_Buffer      : ddesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;           break;
        case eRenderResourceType_Tex1D       : ddesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE1D;        break;
        case eRenderResourceType_Tex2D       : ddesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;        break;
        case eRenderResourceType_Tex3D       : ddesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE3D;        break;
        case eRenderResourceType_TexCube     : ddesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;      break;
        case eRenderResourceType_Tex1DArray  : ddesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE1DARRAY;   break;
        case eRenderResourceType_Tex2DArray  : ddesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;   break;
        case eRenderResourceType_TexCubeArray: ddesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBEARRAY; break;
        default: hcAssertFailMsg("Invalid resource type for shader resource view");
        }
        ddesc.Format = toDXGIFormat(desc.format_, NULL);
        switch (desc.resourceType_) {
        case eRenderResourceType_Buffer      : {
            ddesc.Buffer.FirstElement = desc.buffer_.firstElement_;
            ddesc.Buffer.NumElements  = desc.buffer_.numElements_;
        } break;
        case eRenderResourceType_Tex1D       : {
            ddesc.Texture1D.MostDetailedMip = desc.tex1D_.topMip_;
            ddesc.Texture1D.MipLevels       = desc.tex1D_.mipLevels_;
        } break;
        case eRenderResourceType_Tex2D       : {
            ddesc.Texture2D.MostDetailedMip = desc.tex2D_.topMip_;
            ddesc.Texture2D.MipLevels       = desc.tex2D_.mipLevels_;
        } break;
        case eRenderResourceType_Tex3D       : {

        } break;
        case eRenderResourceType_TexCube     : {

        } break;
        case eRenderResourceType_Tex1DArray  : {
            ddesc.Texture1DArray.MostDetailedMip = desc.tex1DArray_.topMip_;
            ddesc.Texture1DArray.MipLevels       = desc.tex1DArray_.mipLevels_;
            ddesc.Texture1DArray.FirstArraySlice = desc.tex1DArray_.arrayStart_;
            ddesc.Texture1DArray.ArraySize       = desc.tex1DArray_.arraySize_;
        } break;
        case eRenderResourceType_Tex2DArray  : {
            ddesc.Texture2DArray.MostDetailedMip = desc.tex2DArray_.topMip_;
            ddesc.Texture2DArray.MipLevels       = desc.tex2DArray_.mipLevels_;
            ddesc.Texture2DArray.FirstArraySlice = desc.tex2DArray_.arrayStart_;
            ddesc.Texture2DArray.ArraySize       = desc.tex2DArray_.arraySize_;
        } break;
        case eRenderResourceType_TexCubeArray: {

        } break;
        }

        hr = d3d11Device_->CreateShaderResourceView(res, &ddesc, &outsrv->srv_);
        hcAssert(SUCCEEDED(hr));
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdDX11RenderDevice::destroyShaderResourceViewDevice(hdDX11ShaderResourceView* srv) {
        if (srv && srv->srv_) {
            srv->srv_->Release();
            srv->srv_=NULL;
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdDX11RenderDevice::createRenderTargetViewDevice(hdDX11Texture* texture, const hRenderTargetViewDesc& desc, hdDX11RenderTargetView* outrtv) {
        hcAssert(outrtv);
        HRESULT hr;
        D3D11_RENDER_TARGET_VIEW_DESC rtvd;
        hZeroMem(&rtvd, sizeof(rtvd));
        switch (desc.resourceType_) {
        case eRenderResourceType_Buffer      : rtvd.ViewDimension = D3D11_RTV_DIMENSION_BUFFER;           break;
        case eRenderResourceType_Tex1D       : rtvd.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE1D;        break;
        case eRenderResourceType_Tex2D       : rtvd.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;        break;
        case eRenderResourceType_Tex3D       : rtvd.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE3D;        break;
        case eRenderResourceType_Tex1DArray  : rtvd.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE1DARRAY;   break;
        case eRenderResourceType_Tex2DArray  : rtvd.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;   break;
        default: hcAssertFailMsg("Invalid resource type for shader resource view");
        }
        rtvd.Format = toDXGIFormat(desc.format_, NULL);
        switch (desc.resourceType_) {
        case eRenderResourceType_Buffer      : {
            rtvd.Buffer.FirstElement = desc.buffer_.firstElement_;
            rtvd.Buffer.NumElements  = desc.buffer_.numElements_;
        } break;
        case eRenderResourceType_Tex1D       : {
            rtvd.Texture1D.MipSlice = desc.tex1D_.topMip_;
        } break;
        case eRenderResourceType_Tex2D       : {
            rtvd.Texture2D.MipSlice = desc.tex2D_.topMip_;
        } break;
        case eRenderResourceType_Tex3D       : {

        } break;
        case eRenderResourceType_Tex1DArray  : {
            rtvd.Texture1DArray.MipSlice = desc.tex1DArray_.topMip_;
            rtvd.Texture1DArray.FirstArraySlice = desc.tex1DArray_.arrayStart_;
            rtvd.Texture1DArray.ArraySize       = desc.tex1DArray_.arraySize_;
        } break;
        case eRenderResourceType_Tex2DArray  : {
            rtvd.Texture2DArray.MipSlice = desc.tex2DArray_.topMip_;
            rtvd.Texture2DArray.FirstArraySlice = desc.tex2DArray_.arrayStart_;
            rtvd.Texture2DArray.ArraySize       = desc.tex2DArray_.arraySize_;
        } break;
        }
        hr = d3d11Device_->CreateRenderTargetView(texture->dx11Texture_, &rtvd, &outrtv->rtv_);
        hcAssert(SUCCEEDED(hr));
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdDX11RenderDevice::destroyRenderTargetViewDevice(hdDX11RenderTargetView* rtv) {
        if (rtv && rtv->rtv_) {
            rtv->rtv_->Release();
            rtv->rtv_=NULL;
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdDX11RenderDevice::createDepthStencilViewDevice(hdDX11Texture* texture, const hDepthStencilViewDesc& desc, hdDX11DepthStencilView* outdsv) {
        hcAssert(outdsv);
        HRESULT hr;
        D3D11_DEPTH_STENCIL_VIEW_DESC rtvd;
        hZeroMem(&rtvd, sizeof(rtvd));
        switch (desc.resourceType_) {
        case eRenderResourceType_Tex1D       : rtvd.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE1D;        break;
        case eRenderResourceType_Tex2D       : rtvd.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;        break;
        case eRenderResourceType_Tex1DArray  : rtvd.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE1DARRAY;   break;
        case eRenderResourceType_Tex2DArray  : rtvd.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;   break;
        default: hcAssertFailMsg("Invalid resource type for shader resource view");
        }
        rtvd.Format = toDXGIFormat(desc.format_, NULL);
        switch (desc.resourceType_) {
        case eRenderResourceType_Tex1D       : {
            rtvd.Texture1D.MipSlice = desc.tex1D_.topMip_;
        } break;
        case eRenderResourceType_Tex2D       : {
            rtvd.Texture2D.MipSlice = desc.tex2D_.topMip_;
        } break;
        case eRenderResourceType_Tex1DArray  : {
            rtvd.Texture1DArray.MipSlice = desc.tex1DArray_.topMip_;
            rtvd.Texture1DArray.FirstArraySlice = desc.tex1DArray_.arrayStart_;
            rtvd.Texture1DArray.ArraySize       = desc.tex1DArray_.arraySize_;
        } break;
        case eRenderResourceType_Tex2DArray  : {
            rtvd.Texture2DArray.MipSlice = desc.tex2DArray_.topMip_;
            rtvd.Texture2DArray.FirstArraySlice = desc.tex2DArray_.arrayStart_;
            rtvd.Texture2DArray.ArraySize       = desc.tex2DArray_.arraySize_;
        } break;
        }
        hr = d3d11Device_->CreateDepthStencilView(texture->dx11Texture_, &rtvd, &outdsv->dsv_);
        hcAssert(SUCCEEDED(hr));
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdDX11RenderDevice::destroyDepthStencilViewDevice(hdDX11DepthStencilView* dsv) {
        if (dsv && dsv->dsv_) {
            dsv->dsv_->Release();
            dsv->dsv_=NULL;
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdDX11RenderDevice::clearDeviceInputs(ID3D11DeviceContext* device) {
        void* nullout[HEART_MAX_RESOURCE_INPUTS] = {0};
        device->PSSetConstantBuffers(0, HEART_MAX_CONSTANT_BLOCKS, (ID3D11Buffer**)&nullout);
        device->PSSetSamplers(0, HEART_MAX_RESOURCE_INPUTS, (ID3D11SamplerState**)&nullout);
        device->PSSetShaderResources(0, HEART_MAX_RESOURCE_INPUTS, (ID3D11ShaderResourceView**)&nullout);
        device->VSSetConstantBuffers(0, HEART_MAX_CONSTANT_BLOCKS, (ID3D11Buffer**)&nullout);
        device->VSSetSamplers(0, HEART_MAX_RESOURCE_INPUTS, (ID3D11SamplerState**)&nullout);
        device->VSSetShaderResources(0, HEART_MAX_RESOURCE_INPUTS, (ID3D11ShaderResourceView**)&nullout);
        device->GSSetConstantBuffers(0, HEART_MAX_CONSTANT_BLOCKS, (ID3D11Buffer**)&nullout);
        device->GSSetSamplers(0, HEART_MAX_RESOURCE_INPUTS, (ID3D11SamplerState**)&nullout);
        device->GSSetShaderResources(0, HEART_MAX_RESOURCE_INPUTS, (ID3D11ShaderResourceView**)&nullout);
        device->DSSetConstantBuffers(0, HEART_MAX_CONSTANT_BLOCKS, (ID3D11Buffer**)&nullout);
        device->DSSetSamplers(0, HEART_MAX_RESOURCE_INPUTS, (ID3D11SamplerState**)&nullout);
        device->DSSetShaderResources(0, HEART_MAX_RESOURCE_INPUTS, (ID3D11ShaderResourceView**)&nullout);
        device->HSSetConstantBuffers(0, HEART_MAX_CONSTANT_BLOCKS, (ID3D11Buffer**)&nullout);
        device->HSSetSamplers(0, HEART_MAX_RESOURCE_INPUTS, (ID3D11SamplerState**)&nullout);
        device->HSSetShaderResources(0, HEART_MAX_RESOURCE_INPUTS, (ID3D11ShaderResourceView**)&nullout);
        device->CSSetConstantBuffers(0, HEART_MAX_CONSTANT_BLOCKS, (ID3D11Buffer**)&nullout);
        device->CSSetSamplers(0, HEART_MAX_RESOURCE_INPUTS, (ID3D11SamplerState**)&nullout);
        device->CSSetShaderResources(0, HEART_MAX_RESOURCE_INPUTS, (ID3D11ShaderResourceView**)&nullout);
        device->CSSetUnorderedAccessViews(0, HEART_MAX_UAV_INPUTS, (ID3D11UnorderedAccessView**)&nullout, NULL);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hUint hdDX11RenderCommandGenerator::setRenderStates(hdDX11BlendState* bs, hdDX11RasterizerState* rs, hdDX11DepthStencilState* dss) {
        hRCmdSetStates cmd(bs->stateObj_, rs->stateObj_, dss->stateObj_, dss->stencilRef_);
        return appendCmd(&cmd);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hUint hdDX11RenderCommandGenerator::setShader(hdDX11ShaderProgram* shader, hShaderType type) {
        switch(type) {
            case ShaderType_VERTEXPROG: {
                hRCmdSetVertexShader cmd(!shader ? hNullptr : shader->vertexShader_);
                return appendCmd(&cmd);
            } break;
            case ShaderType_FRAGMENTPROG: {
                hRCmdSetPixelShader cmd(!shader ? hNullptr : shader->pixelShader_);
                return appendCmd(&cmd);
            } break;
            case ShaderType_GEOMETRYPROG: {
                hRCmdSetGeometryShader cmd(!shader ? hNullptr : shader->geomShader_);
                return appendCmd(&cmd);
            } break;
            case ShaderType_HULLPROG: {
                hRCmdSetHullShader cmd(!shader ? hNullptr : shader->hullShader_);
                return appendCmd(&cmd);
            } break;
            case ShaderType_DOMAINPROG: {
                hRCmdSetDomainShader cmd(!shader ? hNullptr : shader->domainShader_);
                return appendCmd(&cmd);
            } break;
            default:
                hcAssertFailMsg("Invalid shader type");
                return 0;
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hUint hdDX11RenderCommandGenerator::setVertexInputs(hdDX11SamplerState** samplers, hUint nsamplers, hdDX11ShaderResourceView** srv, hUint nsrv, hdDX11Buffer** cb, hUint ncb) {
        hUint cmdsize=sizeof(ID3D11ShaderResourceView*)*nsrv+sizeof(ID3D11SamplerState*)*nsamplers+sizeof(ID3D11Buffer*)*ncb;
        cmdsize+=sizeof(hRCmdSetVertexInputs);
        hRCmdSetVertexInputs* cmd=(hRCmdSetVertexInputs*)hAlloca(cmdsize);
        ID3D11ShaderResourceView** cmdsrv=(ID3D11ShaderResourceView**)(cmd+1);
        ID3D11SamplerState** cmdsamp=(ID3D11SamplerState**)(cmdsrv+nsrv);
        ID3D11Buffer** cmdpcb=(ID3D11Buffer**)(cmdsamp+nsamplers);
        hPLACEMENT_NEW(cmd) hRCmdSetVertexInputs(nsrv, nsamplers, ncb);
        cmd->size_=cmdsize;
        for (hUint i=0, n=nsrv; i<n; ++i) {
            cmdsrv[i]=srv[i] ? srv[i]->srv_ : hNullptr;
        }
        for (hUint i=0, n=nsamplers; i<n; ++i) {
            cmdsamp[i]=samplers[i] ? samplers[i]->stateObj_ : hNullptr;
        }
        for (hUint i=0, n=ncb; i<n; ++i) {
            cmdpcb[i]=cb[i] ? cb[i]->buffer_ : hNullptr;
        }
        return appendCmd(cmd);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hUint hdDX11RenderCommandGenerator::updateVertexInputs(hRCmd* oldcmd, hdDX11SamplerState** samplers, hUint nsamplers, hdDX11ShaderResourceView** srv, hUint nsrv, hdDX11Buffer** cb, hUint ncb) {
        hUint cmdsize=sizeof(ID3D11ShaderResourceView*)*nsrv+sizeof(ID3D11SamplerState*)*nsamplers+sizeof(ID3D11Buffer*)*ncb;
        cmdsize+=sizeof(hRCmdSetVertexInputs);
        hRCmdSetVertexInputs* cmd=(hRCmdSetVertexInputs*)hAlloca(cmdsize);
        ID3D11ShaderResourceView** cmdsrv=(ID3D11ShaderResourceView**)(cmd+1);
        ID3D11SamplerState** cmdsamp=(ID3D11SamplerState**)(cmdsrv+nsrv);
        ID3D11Buffer** cmdpcb=(ID3D11Buffer**)(cmdsamp+nsamplers);
        hPLACEMENT_NEW(cmd) hRCmdSetVertexInputs(nsrv, nsamplers, ncb);
        cmd->size_=cmdsize;
        for (hUint i=0, n=nsrv; i<n; ++i) {
            cmdsrv[i]=srv[i] ? srv[i]->srv_ : hNullptr;
        }
        for (hUint i=0, n=nsamplers; i<n; ++i) {
            cmdsamp[i]=samplers[i] ? samplers[i]->stateObj_ : hNullptr;
        }
        for (hUint i=0, n=ncb; i<n; ++i) {
            cmdpcb[i]=cb[i] ? cb[i]->buffer_ : hNullptr;
        }
        hcAssert(cmd->size_ == oldcmd->size_);
        return overwriteCmd(oldcmd, cmd);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hUint hdDX11RenderCommandGenerator::setPixelInputs(hdDX11SamplerState** samplers, hUint nsamplers, hdDX11ShaderResourceView** srv, hUint nsrv, hdDX11Buffer** cb, hUint ncb) {
        hUint cmdsize=sizeof(ID3D11ShaderResourceView*)*nsrv+sizeof(ID3D11SamplerState*)*nsamplers+sizeof(ID3D11Buffer*)*ncb;
        cmdsize+=sizeof(hRCmdSetPixelInputs);
        hRCmdSetPixelInputs* cmd=(hRCmdSetPixelInputs*)hAlloca(cmdsize);
        ID3D11ShaderResourceView** cmdsrv=(ID3D11ShaderResourceView**)(cmd+1);
        ID3D11SamplerState** cmdsamp=(ID3D11SamplerState**)(cmdsrv+nsrv);
        ID3D11Buffer** cmdpcb=(ID3D11Buffer**)(cmdsamp+nsamplers);
        hPLACEMENT_NEW(cmd) hRCmdSetPixelInputs(nsrv, nsamplers, ncb);
        cmd->size_=cmdsize;
        for (hUint i=0, n=nsrv; i<n; ++i) {
            cmdsrv[i]=srv[i] ? srv[i]->srv_ : hNullptr;
        }
        for (hUint i=0, n=nsamplers; i<n; ++i) {
            cmdsamp[i]=samplers[i] ? samplers[i]->stateObj_ : hNullptr;
        }
        for (hUint i=0, n=ncb; i<n; ++i) {
            cmdpcb[i]=cb[i] ? cb[i]->buffer_ : hNullptr;
        }
        return appendCmd(cmd);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hUint hdDX11RenderCommandGenerator::updatePixelInputs(hRCmd* oldcmd, hdDX11SamplerState** samplers, hUint nsamplers, hdDX11ShaderResourceView** srv, hUint nsrv, hdDX11Buffer** cb, hUint ncb) {
        hUint cmdsize=sizeof(ID3D11ShaderResourceView*)*nsrv+sizeof(ID3D11SamplerState*)*nsamplers+sizeof(ID3D11Buffer*)*ncb;
        cmdsize+=sizeof(hRCmdSetPixelInputs);
        hRCmdSetPixelInputs* cmd=(hRCmdSetPixelInputs*)hAlloca(cmdsize);
        ID3D11ShaderResourceView** cmdsrv=(ID3D11ShaderResourceView**)(cmd+1);
        ID3D11SamplerState** cmdsamp=(ID3D11SamplerState**)(cmdsrv+nsrv);
        ID3D11Buffer** cmdpcb=(ID3D11Buffer**)(cmdsamp+nsamplers);
        hPLACEMENT_NEW(cmd) hRCmdSetPixelInputs(nsrv, nsamplers, ncb);
        cmd->size_=cmdsize;
        for (hUint i=0, n=nsrv; i<n; ++i) {
            cmdsrv[i]=srv[i] ? srv[i]->srv_ : hNullptr;
        }
        for (hUint i=0, n=nsamplers; i<n; ++i) {
            cmdsamp[i]=samplers[i] ? samplers[i]->stateObj_ : hNullptr;
        }
        for (hUint i=0, n=ncb; i<n; ++i) {
            cmdpcb[i]=cb[i] ? cb[i]->buffer_ : hNullptr;
        }
        hcAssert(cmd->size_ == oldcmd->size_);
        return overwriteCmd(oldcmd, cmd);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hUint hdDX11RenderCommandGenerator::setGeometryInputs(hdDX11SamplerState** samplers, hUint nsamplers, hdDX11ShaderResourceView** srv, hUint nsrv, hdDX11Buffer** cb, hUint ncb) {
        hUint cmdsize=sizeof(ID3D11ShaderResourceView*)*nsrv+sizeof(ID3D11SamplerState*)*nsamplers+sizeof(ID3D11Buffer*)*ncb;
        cmdsize+=sizeof(hRCmdSetGeometryInputs);
        hRCmdSetGeometryInputs* cmd=(hRCmdSetGeometryInputs*)hAlloca(cmdsize);
        ID3D11ShaderResourceView** cmdsrv=(ID3D11ShaderResourceView**)(cmd+1);
        ID3D11SamplerState** cmdsamp=(ID3D11SamplerState**)(cmdsrv+nsrv);
        ID3D11Buffer** cmdpcb=(ID3D11Buffer**)(cmdsamp+nsamplers);
        hPLACEMENT_NEW(cmd) hRCmdSetGeometryInputs(nsrv, nsamplers, ncb);
        cmd->size_=cmdsize;
        for (hUint i=0, n=nsrv; i<n; ++i) {
            cmdsrv[i]=srv[i] ? srv[i]->srv_ : hNullptr;
        }
        for (hUint i=0, n=nsamplers; i<n; ++i) {
            cmdsamp[i]=samplers[i] ? samplers[i]->stateObj_ : hNullptr;
        }
        for (hUint i=0, n=ncb; i<n; ++i) {
            cmdpcb[i]=cb[i] ? cb[i]->buffer_ : hNullptr;
        }
        return appendCmd(cmd);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hUint hdDX11RenderCommandGenerator::setHullInputs(hdDX11SamplerState** samplers, hUint nsamplers, hdDX11ShaderResourceView** srv, hUint nsrv, hdDX11Buffer** cb, hUint ncb) {
        hUint cmdsize=sizeof(ID3D11ShaderResourceView*)*nsrv+sizeof(ID3D11SamplerState*)*nsamplers+sizeof(ID3D11Buffer*)*ncb;
        cmdsize+=sizeof(hRCmdSetHullInputs);
        hRCmdSetHullInputs* cmd=(hRCmdSetHullInputs*)hAlloca(cmdsize);
        ID3D11ShaderResourceView** cmdsrv=(ID3D11ShaderResourceView**)(cmd+1);
        ID3D11SamplerState** cmdsamp=(ID3D11SamplerState**)(cmdsrv+nsrv);
        ID3D11Buffer** cmdpcb=(ID3D11Buffer**)(cmdsamp+nsamplers);
        hPLACEMENT_NEW(cmd) hRCmdSetHullInputs(nsrv, nsamplers, ncb);
        cmd->size_=cmdsize;
        for (hUint i=0, n=nsrv; i<n; ++i) {
            cmdsrv[i]=srv[i] ? srv[i]->srv_ : hNullptr;
        }
        for (hUint i=0, n=nsamplers; i<n; ++i) {
            cmdsamp[i]=samplers[i] ? samplers[i]->stateObj_ : hNullptr;
        }
        for (hUint i=0, n=ncb; i<n; ++i) {
            cmdpcb[i]=cb[i] ? cb[i]->buffer_ : hNullptr;
        }
        return appendCmd(cmd);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hUint hdDX11RenderCommandGenerator::setDomainInputs(hdDX11SamplerState** samplers, hUint nsamplers, hdDX11ShaderResourceView** srv, hUint nsrv, hdDX11Buffer** cb, hUint ncb) {
        hUint cmdsize=sizeof(ID3D11ShaderResourceView*)*nsrv+sizeof(ID3D11SamplerState*)*nsamplers+sizeof(ID3D11Buffer*)*ncb;
        cmdsize+=sizeof(hRCmdSetDomainInputs);
        hRCmdSetDomainInputs* cmd=(hRCmdSetDomainInputs*)hAlloca(cmdsize);
        ID3D11ShaderResourceView** cmdsrv=(ID3D11ShaderResourceView**)(cmd+1);
        ID3D11SamplerState** cmdsamp=(ID3D11SamplerState**)(cmdsrv+nsrv);
        ID3D11Buffer** cmdpcb=(ID3D11Buffer**)(cmdsamp+nsamplers);
        hPLACEMENT_NEW(cmd) hRCmdSetDomainInputs(nsrv, nsamplers, ncb);
        cmd->size_=cmdsize;
        for (hUint i=0, n=nsrv; i<n; ++i) {
            cmdsrv[i]=srv[i] ? srv[i]->srv_ : hNullptr;
        }
        for (hUint i=0, n=nsamplers; i<n; ++i) {
            cmdsamp[i]=samplers[i] ? samplers[i]->stateObj_ : hNullptr;
        }
        for (hUint i=0, n=ncb; i<n; ++i) {
            cmdpcb[i]=cb[i] ? cb[i]->buffer_ : hNullptr;
        }
        return appendCmd(cmd);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hUint hdDX11RenderCommandGenerator::setStreamInputs(PrimitiveType primType, hdDX11IndexBuffer* index, hIndexBufferType format, hdDX11VertexLayout* layout, hdDX11VertexBuffer** vtx, hUint firstStream, hUint streamCount) {
        hUint cmdsize=sizeof(ID3D11Buffer*)*streamCount;
        cmdsize+=sizeof(hUint)*streamCount;
        cmdsize+=sizeof(hRCmdSetInputStreams);
        D3D11_PRIMITIVE_TOPOLOGY top;
        if ( primType == PRIMITIVETYPE_LINELIST ) top = D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
        else if ( primType == PRIMITIVETYPE_TRILIST) top = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
        else if ( primType == PRIMITIVETYPE_TRISTRIP) top = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
        hRCmdSetInputStreams* cmd=(hRCmdSetInputStreams*)hAlloca(cmdsize);
        ID3D11Buffer** cmdvb=(ID3D11Buffer**)(cmd+1);
        hUint* cmdstrides=(hUint*)(&cmdvb[streamCount]);
        hPLACEMENT_NEW(cmd) hRCmdSetInputStreams(top, format == hIndexBufferType_Index16 ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT, layout, firstStream, firstStream+streamCount, index ? index->buffer_ : hNullptr);
        cmd->size_=cmdsize;
        for (hUint i=0, n=streamCount; i<n; ++i) {
            cmdvb[i]=vtx[i]->buffer_;
            cmdstrides[i]=vtx[i]->stride_;
        }
        return appendCmd(cmd);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hUint hdDX11RenderCommandGenerator::updateShaderInputBuffer(hRCmd* cmd, hUint reg, hdDX11Buffer* cb) {
        hRCmdSetInputsBase* inputcmd=static_cast<hRCmdSetInputsBase*>(cmd);
        hcAssert(reg < inputcmd->bufferCount_);
        ID3D11ShaderResourceView** cmdsrv=(ID3D11ShaderResourceView**)(inputcmd+1);
        ID3D11SamplerState** cmdsamp=(ID3D11SamplerState**)(cmdsrv+inputcmd->resourceViewCount_);
        ID3D11Buffer** cmdpcb=(ID3D11Buffer**)(cmdsamp+inputcmd->samplerCount_);
        cmdpcb[reg]=cb ? cb->buffer_ : hNullptr;
        return 0;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hUint hdDX11RenderCommandGenerator::updateShaderInputSampler(hRCmd* cmd, hUint reg, hdDX11SamplerState* ss) {
        hRCmdSetInputsBase* inputcmd=static_cast<hRCmdSetInputsBase*>(cmd);
        hcAssert(reg < inputcmd->samplerCount_);
        ID3D11ShaderResourceView** cmdsrv=(ID3D11ShaderResourceView**)(inputcmd+1);
        ID3D11SamplerState** cmdsamp=(ID3D11SamplerState**)(cmdsrv+inputcmd->resourceViewCount_);
        ID3D11Buffer** cmdpcb=(ID3D11Buffer**)(cmdsamp+inputcmd->samplerCount_);
        cmdsamp[reg]=ss ? ss->stateObj_ : hNullptr;
        return 0;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hUint hdDX11RenderCommandGenerator::updateShaderInputView(hRCmd* cmd, hUint reg, hdDX11ShaderResourceView* srv) {
        hRCmdSetInputsBase* inputcmd=static_cast<hRCmdSetInputsBase*>(cmd);
        hcAssert(reg < inputcmd->resourceViewCount_);
        ID3D11ShaderResourceView** cmdsrv=(ID3D11ShaderResourceView**)(inputcmd+1);
        ID3D11SamplerState** cmdsamp=(ID3D11SamplerState**)(cmdsrv+inputcmd->resourceViewCount_);
        ID3D11Buffer** cmdpcb=(ID3D11Buffer**)(cmdsamp+inputcmd->samplerCount_);
        cmdsrv[reg]=srv ? srv->srv_ : hNullptr;
        return 0;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hUint hdDX11RenderCommandGenerator::updateStreamInputs(hRCmd* origcmd, PrimitiveType primType, hdDX11IndexBuffer* index, hIndexBufferType format, hdDX11VertexLayout* layout, hdDX11VertexBuffer** vtx, hUint firstStream, hUint streamCount) {
        hUint cmdsize=sizeof(ID3D11Buffer*)*streamCount;
        cmdsize+=sizeof(hRCmdSetInputStreams);
        D3D11_PRIMITIVE_TOPOLOGY top;
        if ( primType == PRIMITIVETYPE_LINELIST ) top = D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
        else if ( primType == PRIMITIVETYPE_TRILIST) top = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
        else if ( primType == PRIMITIVETYPE_TRISTRIP) top = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
        hRCmdSetInputStreams* cmd=(hRCmdSetInputStreams*)hAlloca(cmdsize);
        ID3D11Buffer** cmdib=(ID3D11Buffer**)(cmd+1);
        hPLACEMENT_NEW(cmd) hRCmdSetInputStreams(top, format == hIndexBufferType_Index16 ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT, layout, firstStream, firstStream+streamCount, index->buffer_);
        cmd->size_=cmdsize;
        for (hUint i=0, n=streamCount; i<n; ++i) {
            cmdib[i]=vtx[i]->buffer_;
        }
        return overwriteCmd(origcmd, cmd);
    }

}
