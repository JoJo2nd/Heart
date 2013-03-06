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

namespace Heart
{
#if defined (HEART_DEBUG)
#   define HEART_D3D_OBJECT_REPORT(device) {\
        ID3D11Debug* debuglayer;\
        device->QueryInterface(__uuidof(ID3D11Debug), (void**)&debuglayer);\
        debuglayer->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);\
        debuglayer->Release();\
    }
#   define HEART_D3D_DEBUG_NAME_OBJECT(obj, name) obj->SetPrivateData( WKPDID_D3DDebugObjectName, strlen(name), name)
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
        cbuffer FontParams                                             \n\
        {                                                              \n\
            float4 fontColour;                                         \n\
            float4 dropOffset;                                         \n\
        }                                                              \n\
                                                                       \n\
        Texture2D   SignedDistanceField;                               \n\
                                                                       \n\
        SamplerState fontSampler= sampler_state{};                     \n\
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
            pos.xyz += dropOffset.xyz;                                 \n\
            output.position = mul(mul(g_ViewProjection,g_World), pos); \n\
            output.colour = input.colour;	                           \n\
            output.uv = input.uv;                                      \n\
            return output;                                             \n\
        }                                                                     \n\
                                                                              \n\
        float4 mainFP( PSInput input ) : SV_TARGET0                           \n\
        {                                                                     \n\
            float a   = SignedDistanceField.Sample(fontSampler, input.uv).a;  \n\
            return float4(fontColour.rgb,a);                                  \n\
        }                                                                     \n\
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
            float2 uv 		: TEXCOORD0;                               \n\
        };                                                             \n\
                                                                       \n\
        struct PSInput                                                 \n\
        {                                                              \n\
            float4 position : SV_POSITION;                             \n\
            float2 uv 		: TEXCOORD0;                               \n\
        };                                                             \n\
                                                                       \n\
        PSInput mainVP( VSInput input )                                \n\
        {                                                              \n\
            PSInput output;                                            \n\
            float4 pos = float4(input.position.xyz,1);                 \n\
            output.position = mul(mul(g_ViewProjection,g_World), pos); \n\
            output.uv = input.uv;                                      \n\
            return output;                                             \n\
        }                                                              \n\
                                                                       \n\
        float4 mainFP( PSInput input ) : SV_TARGET0                    \n\
        {                                                              \n\
            float4 c = g_texture.Sample(g_sampler, input.uv).rgba;     \n\
            return float4(c.rgb,c.a);                                  \n\
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
        backBufferTex_ = setup.backBufferTex_;
        depthBufferTex_= setup.depthBufferTex_;

        hcAssert(backBufferTex_ && depthBufferTex_);

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
        ID3D11Texture2D* pBackBuffer = NULL;
        hr = mainSwapChain_->GetBuffer( 0, __uuidof( ID3D11Texture2D ), (LPVOID*)&pBackBuffer );
        hcAssert( SUCCEEDED( hr ) );

        hr = d3d11Device_->CreateRenderTargetView( pBackBuffer, NULL, &renderTargetView_ );
        hcAssert( SUCCEEDED( hr ) );
        HEART_D3D_DEBUG_NAME_OBJECT(renderTargetView_, "Render Target View");

        // Create depth stencil texture
        D3D11_TEXTURE2D_DESC descDepth;
        ZeroMemory( &descDepth, sizeof(descDepth) );
        descDepth.Width = width;
        descDepth.Height = height;
        descDepth.MipLevels = 1;
        descDepth.ArraySize = 1;
        descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        descDepth.SampleDesc.Count = 1;
        descDepth.SampleDesc.Quality = 0;
        descDepth.Usage = D3D11_USAGE_DEFAULT;
        descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
        descDepth.CPUAccessFlags = 0;
        descDepth.MiscFlags = 0;
        hr = d3d11Device_->CreateTexture2D( &descDepth, NULL, &depthStencil_ );
        hcAssert( SUCCEEDED( hr ) );
        HEART_D3D_DEBUG_NAME_OBJECT(depthStencil_, "Depth Stencil");

        // Create the depth stencil view
        D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
        ZeroMemory( &descDSV, sizeof(descDSV) );
        descDSV.Format = descDepth.Format;
        descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
        descDSV.Texture2D.MipSlice = 0;
        hr = d3d11Device_->CreateDepthStencilView( depthStencil_, &descDSV, &depthStencilView_ );
        hcAssert( SUCCEEDED( hr ) );
        HEART_D3D_DEBUG_NAME_OBJECT(depthStencilView_, "Depth Stencil View");

        mainRenderCtx_.SetDeviceCtx( mainDeviceCtx_, alloc_, free_ );

        //update textures
        hTRACK_CUSTOM_ADDRESS_ALLOC("DirectX", pBackBuffer);
        HEART_D3D_DEBUG_NAME_OBJECT(pBackBuffer, "backbuffer");
        backBufferTex_->shaderResourceView_=NULL;
        backBufferTex_->depthStencilView_=NULL;
        backBufferTex_->dx11Texture_=pBackBuffer;
        backBufferTex_->renderTargetView_=renderTargetView_;

        depthBufferTex_->shaderResourceView_=NULL;
        depthBufferTex_->depthStencilView_=depthStencilView_;
        depthBufferTex_->dx11Texture_=depthStencil_;
        depthBufferTex_->renderTargetView_=NULL;

        D3D11_QUERY_DESC qdesc;
        qdesc.MiscFlags = 0;

        qdesc.Query = D3D11_QUERY_TIMESTAMP_DISJOINT;
        hr = d3d11Device_->CreateQuery(&qdesc, &timerDisjoint_);
        hcAssert(SUCCEEDED(hr));

        qdesc.Query = D3D11_QUERY_TIMESTAMP;
        hr = d3d11Device_->CreateQuery(&qdesc, &timerFrameStart_);
        hcAssert(SUCCEEDED(hr));

        qdesc.Query = D3D11_QUERY_TIMESTAMP;
        hr = d3d11Device_->CreateQuery(&qdesc, &timerFrameEnd_);
        hcAssert(SUCCEEDED(hr));

        frameCounter_ = 0;

        HEART_D3D_OBJECT_REPORT(d3d11Device_);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdDX11RenderDevice::Destroy()
    {
        HEART_D3D_OBJECT_REPORT(d3d11Device_);
        timerDisjoint_->Release();
        timerFrameStart_->Release();
        timerFrameEnd_->Release();

        if (backBufferTex_->dx11Texture_) {
            hTRACK_CUSTOM_ADDRESS_FREE("DirectX", backBufferTex_->dx11Texture_);
            backBufferTex_->dx11Texture_->Release();
        }
        
        if( mainDeviceCtx_ ) {
            mainDeviceCtx_->ClearState();
            mainDeviceCtx_->Flush();
            mainDeviceCtx_->Release();
        }
        if ( renderTargetView_ ) {
            renderTargetView_->Release();
        }
        if ( depthStencilView_ ) {
            depthStencilView_->Release();
        }
        if ( depthStencil_ ) {
            depthStencil_->Release();
        }
        if ( mainSwapChain_ ) {
            mainSwapChain_->Release();
        }
        if ( d3d11Device_ ) {
            ULONG ref=d3d11Device_->Release();
            hcPrintf("D3DDevice Ref %u", ref);
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    DXGI_FORMAT hdDX11RenderDevice::toDXGIFormat(hTextureFormat format, hBool* compressed) {
        DXGI_FORMAT fmt = DXGI_FORMAT_UNKNOWN;
        hBool compressedFormat=hFalse;
        switch ( format )
        {
        case TFORMAT_ARGB8:         fmt = DXGI_FORMAT_R8G8B8A8_UNORM; break;
        case TFORMAT_ARGB8_sRGB:    fmt = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB; break;
        case TFORMAT_XRGB8:         fmt = DXGI_FORMAT_R8G8B8A8_UNORM; break;
        case TFORMAT_XRGB8_sRGB:    fmt = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB; break;
        case TFORMAT_R16F:          fmt = DXGI_FORMAT_R16_FLOAT; break;
        case TFORMAT_GR16F:         fmt = DXGI_FORMAT_R16G16_FLOAT; break;
        case TFORMAT_ABGR16F:       fmt = DXGI_FORMAT_R16G16B16A16_FLOAT; break;
        case TFORMAT_R32F:          fmt = DXGI_FORMAT_R32_FLOAT; break;
        case TFORMAT_D32F:          fmt = DXGI_FORMAT_R32_TYPELESS; break;
        case TFORMAT_D24S8F:        fmt = DXGI_FORMAT_R24G8_TYPELESS; break;
        case TFORMAT_L8:            fmt = DXGI_FORMAT_A8_UNORM; break;
        case TFORMAT_DXT5:          fmt = DXGI_FORMAT_BC3_UNORM; compressedFormat = hTrue; break;
        case TFORMAT_DXT3:          fmt = DXGI_FORMAT_BC2_UNORM; compressedFormat = hTrue; break;
        case TFORMAT_DXT1:          fmt = DXGI_FORMAT_BC1_UNORM; compressedFormat = hTrue; break; 
        case TFORMAT_DXT5_sRGB:     fmt = DXGI_FORMAT_BC3_UNORM_SRGB; compressedFormat = hTrue; break;
        case TFORMAT_DXT3_sRGB:     fmt = DXGI_FORMAT_BC2_UNORM_SRGB; compressedFormat = hTrue; break;
        case TFORMAT_DXT1_sRGB:     fmt = DXGI_FORMAT_BC1_UNORM_SRGB; compressedFormat = hTrue; break;
        }
        if(compressed) {
            *compressed=compressedFormat;
        }
        return fmt;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdDX11RenderDevice::BeginRender(hFloat* gpuTime)
    {
        //get prev GPU timer
        *gpuTime = -1.f;
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

        mainDeviceCtx_->OMSetRenderTargets( 1, &renderTargetView_, depthStencilView_ );
        
        hFloat clearcolour[] = { 0.f, 0.f, 0.f, 1.f };
        mainDeviceCtx_->ClearRenderTargetView( renderTargetView_, clearcolour );
        mainDeviceCtx_->ClearDepthStencilView( depthStencilView_, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0 );
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

    void hdDX11RenderDevice::SwapBuffers()
    {
        mainSwapChain_->Present( 0, 0 );
        if (frameCounter_ > 0)
        {
            mainDeviceCtx_->End(timerFrameEnd_);
            mainDeviceCtx_->End(timerDisjoint_);
        }

        if (width_ != sysWindow_->getWindowWidth() || height_ != sysWindow_->getWindowHeight()) {
            if (sysWindow_->getWindowWidth() > 0 && sysWindow_->getWindowHeight() > 0) {
                HRESULT hr;
                ID3D11Texture2D* backBufferTex;

                width_ =sysWindow_->getWindowWidth();
                height_=sysWindow_->getWindowHeight();
                //Resize the back buffers
                // Release all outstanding references to the swap chain's buffers. 
                // Otherwise ResizeBuffers will fail!
                mainDeviceCtx_->OMSetRenderTargets(0, 0, 0);
                renderTargetView_->Release();
                if (backBufferTex_->dx11Texture_) {
                    hTRACK_CUSTOM_ADDRESS_FREE("DirectX", backBufferTex_->dx11Texture_);
                    backBufferTex_->dx11Texture_->Release();
                    backBufferTex_->dx11Texture_=NULL;
                }
                // Preserve the existing buffer count and format.
                // Automatically choose the width and height to match the client rect for HWNDs.
                hr = mainSwapChain_->ResizeBuffers(0, width_, height_, DXGI_FORMAT_UNKNOWN, 0);
                // TODO:Perform error handling here!
                // Get buffer and create a render-target-view.
                hr = mainSwapChain_->GetBuffer(0, __uuidof( ID3D11Texture2D),(void**)&backBufferTex);
                // TODO: Perform error handling here!
                hr = d3d11Device_->CreateRenderTargetView(backBufferTex, NULL, &renderTargetView_);

                if ( depthStencilView_ ) {
                    depthStencilView_->Release();
                    depthStencilView_=NULL;
                }
                if ( depthStencil_ ) {
                    depthStencil_->Release();
                    depthStencil_=NULL;
                }
                // Create depth stencil texture
                D3D11_TEXTURE2D_DESC descDepth;
                ZeroMemory( &descDepth, sizeof(descDepth) );
                descDepth.Width = width_;
                descDepth.Height = height_;
                descDepth.MipLevels = 1;
                descDepth.ArraySize = 1;
                descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
                descDepth.SampleDesc.Count = 1;
                descDepth.SampleDesc.Quality = 0;
                descDepth.Usage = D3D11_USAGE_DEFAULT;
                descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
                descDepth.CPUAccessFlags = 0;
                descDepth.MiscFlags = 0;
                hr = d3d11Device_->CreateTexture2D(&descDepth, NULL, &depthStencil_);
                hcAssert( SUCCEEDED( hr ) );
                HEART_D3D_DEBUG_NAME_OBJECT(depthStencil_, "depth Stencil");

                // Create the depth stencil view
                D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
                ZeroMemory( &descDSV, sizeof(descDSV) );
                descDSV.Format = descDepth.Format;
                descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
                descDSV.Texture2D.MipSlice = 0;
                hr = d3d11Device_->CreateDepthStencilView(depthStencil_, &descDSV, &depthStencilView_);
                hcAssert( SUCCEEDED( hr ) );
                HEART_D3D_DEBUG_NAME_OBJECT(depthStencilView_, "Depth Stencil View");

                //update textures
                hTRACK_CUSTOM_ADDRESS_ALLOC("DirectX", backBufferTex);
                HEART_D3D_DEBUG_NAME_OBJECT(backBufferTex, "backbuffer");
                backBufferTex_->shaderResourceView_=NULL;
                backBufferTex_->depthStencilView_=NULL;
                backBufferTex_->dx11Texture_=backBufferTex;
                backBufferTex_->renderTargetView_=renderTargetView_;
               
                depthBufferTex_->shaderResourceView_=NULL;
                depthBufferTex_->depthStencilView_=depthStencilView_;
                depthBufferTex_->dx11Texture_=depthStencil_;
                depthBufferTex_->renderTargetView_=NULL;

                mainDeviceCtx_->OMSetRenderTargets(1, &renderTargetView_, depthStencilView_);

                if (resizeCallback_.isValid()) {
                    resizeCallback_(width_, height_);
                }
            }
        }
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
        hTRACK_CUSTOM_ADDRESS_ALLOC("DirectX", rsc);
        ctx->SetDeviceCtx( rsc, alloc_, free_ );
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdDX11RenderDevice::DestroyRenderSubmissionCtx( hdDX11RenderSubmissionCtx* ctx )
    {
        hTRACK_CUSTOM_ADDRESS_FREE("DirectX", ctx->GetDeviceCtx());
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

    hdDX11ShaderProgram* hdDX11RenderDevice::compileShaderFromSource(hMemoryHeapBase* heap, 
    const hChar* shaderProg, hUint32 len, const hChar* entry, 
    hShaderProfile profile, hdDX11ShaderProgram* out) {
        HRESULT hr;
        hdDX11ShaderProgram* shader=out;
        const hChar* profileStr=s_shaderProfileNames[profile];
        ID3DBlob* codeBlob;
        ID3DBlob* errorBlob;
        hShaderType type=ShaderType_MAX;

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

        hr=D3DCompile(shaderProg, len, "memory", NULL, NULL, entry, profileStr, 0, 0, &codeBlob, &errorBlob);
        if (errorBlob) {
            hcPrintf("Shader Output:\n%s", errorBlob->GetBufferPointer());
            errorBlob->Release();
        }
        if (FAILED(hr)) {
            return NULL;
        }
        out=CompileShader(heap, (hChar*)codeBlob->GetBufferPointer(), codeBlob->GetBufferSize(), type, out);
        codeBlob->Release();
        return out;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hdDX11ShaderProgram* hdDX11RenderDevice::CompileShader(
        hMemoryHeapBase* heap, const hChar* shaderProg, 
        hUint32 len, hShaderType type, hdDX11ShaderProgram* out)
    {
        HRESULT hr;
        hdDX11ShaderProgram* shader = out;
        shader->type_ = type;
        shader->blobLen_= 0;
        shader->shaderBlob_= NULL;

        if ( type == ShaderType_FRAGMENTPROG ) {
            hr = d3d11Device_->CreatePixelShader( shaderProg, len, NULL, &shader->pixelShader_ );
            hcAssert( SUCCEEDED( hr ) );
            hTRACK_CUSTOM_ADDRESS_ALLOC("DirectX", shader->pixelShader_);
            hr = D3DReflect( shaderProg, len, IID_ID3D11ShaderReflection, (void**)&shader->shaderInfo_ );
            hcAssert( SUCCEEDED( hr ) );
            hTRACK_CUSTOM_ADDRESS_ALLOC("DirectX", shader->shaderInfo_);
        } else if ( type == ShaderType_VERTEXPROG ) {
            ID3DBlob* inputBlob;
            hr = d3d11Device_->CreateVertexShader( shaderProg, len, NULL, &shader->vertexShader_ );
            hcAssert( SUCCEEDED( hr ) );
            hTRACK_CUSTOM_ADDRESS_ALLOC("DirectX", shader->vertexShader_);
            hr = D3DReflect( shaderProg, len, IID_ID3D11ShaderReflection, (void**)&shader->shaderInfo_ );
            hcAssert( SUCCEEDED( hr ) );
            hTRACK_CUSTOM_ADDRESS_ALLOC("DirectX", shader->shaderInfo_);

            hr = D3DGetInputSignatureBlob(shaderProg, len, &inputBlob);
            hcAssert(SUCCEEDED(hr));
            shader->blobLen_= inputBlob->GetBufferSize();
            shader->shaderBlob_= hNEW_ARRAY(heap, hUint8, inputBlob->GetBufferSize());
            hMemCpy(shader->shaderBlob_, inputBlob->GetBufferPointer(), inputBlob->GetBufferSize());
            inputBlob->Release();
        } else if (type==ShaderType_GEOMETRYPROG) {
            hr=d3d11Device_->CreateGeometryShader(shaderProg, len, NULL, &shader->geomShader_);
            hcAssert( SUCCEEDED( hr ) );
            hTRACK_CUSTOM_ADDRESS_ALLOC("DirectX", shader->geomShader_);
            hr = D3DReflect( shaderProg, len, IID_ID3D11ShaderReflection, (void**)&shader->shaderInfo_ );
            hcAssert( SUCCEEDED( hr ) );
            hTRACK_CUSTOM_ADDRESS_ALLOC("DirectX", shader->shaderInfo_);
        } else if (type==ShaderType_DOMAINPROG) {
            hr=d3d11Device_->CreateDomainShader(shaderProg, len, NULL, &shader->domainShader_);
            hcAssert( SUCCEEDED( hr ) );
            hTRACK_CUSTOM_ADDRESS_ALLOC("DirectX", shader->geomShader_);
            hr = D3DReflect( shaderProg, len, IID_ID3D11ShaderReflection, (void**)&shader->shaderInfo_ );
            hcAssert( SUCCEEDED( hr ) );
            hTRACK_CUSTOM_ADDRESS_ALLOC("DirectX", shader->shaderInfo_);
        } else if (type==ShaderType_HULLPROG) {
            hr=d3d11Device_->CreateHullShader(shaderProg, len, NULL, &shader->hullShader_);
            hcAssert( SUCCEEDED( hr ) );
            hTRACK_CUSTOM_ADDRESS_ALLOC("DirectX", shader->geomShader_);
            hr = D3DReflect( shaderProg, len, IID_ID3D11ShaderReflection, (void**)&shader->shaderInfo_ );
            hcAssert( SUCCEEDED( hr ) );
            hTRACK_CUSTOM_ADDRESS_ALLOC("DirectX", shader->shaderInfo_);
        } else if (type==ShaderType_COMPUTEPROG) {
            hr=d3d11Device_->CreateComputeShader(shaderProg, len, NULL, &shader->computeShader_);
            hcAssert( SUCCEEDED( hr ) );
            hTRACK_CUSTOM_ADDRESS_ALLOC("DirectX", shader->geomShader_);
            hr = D3DReflect( shaderProg, len, IID_ID3D11ShaderReflection, (void**)&shader->shaderInfo_ );
            hcAssert( SUCCEEDED( hr ) );
            hTRACK_CUSTOM_ADDRESS_ALLOC("DirectX", shader->shaderInfo_);
        }

        return shader;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdDX11RenderDevice::DestroyShader(hMemoryHeapBase* heap, hdDX11ShaderProgram* prog)
    {
        if (!prog) return;
        if (prog->shaderInfo_) {
            hTRACK_CUSTOM_ADDRESS_FREE("DirectX", prog->shaderInfo_);
            prog->shaderInfo_->Release();
            prog->shaderInfo_ = NULL;
        }
        if (prog->vertexShader_ && prog->type_ == ShaderType_VERTEXPROG) {
            hTRACK_CUSTOM_ADDRESS_FREE("DirectX", prog->vertexShader_);
            prog->vertexShader_->Release();
            prog->vertexShader_ = NULL;
        }
        if (prog->pixelShader_ && prog->type_ == ShaderType_FRAGMENTPROG) {
            hTRACK_CUSTOM_ADDRESS_FREE("DirectX", prog->pixelShader_);
            prog->pixelShader_->Release();
            prog->pixelShader_ = NULL;
        }
        if (prog->geomShader_ && prog->type_ == ShaderType_GEOMETRYPROG) {
            hTRACK_CUSTOM_ADDRESS_FREE("DirectX", prog->geomShader_);
            prog->geomShader_->Release();
            prog->geomShader_ = NULL;
        }
        if (prog->hullShader_ && prog->type_ == ShaderType_HULLPROG) {
            hTRACK_CUSTOM_ADDRESS_FREE("DirectX", prog->hullShader_);
            prog->hullShader_->Release();
            prog->hullShader_ = NULL;
        }
        if (prog->domainShader_ && prog->type_ == ShaderType_DOMAINPROG) {
            hTRACK_CUSTOM_ADDRESS_FREE("DirectX", prog->domainShader_);
            prog->domainShader_->Release();
            prog->domainShader_ = NULL;
        }
        if (prog->computeShader_ && prog->type_ == ShaderType_COMPUTEPROG) {
            hTRACK_CUSTOM_ADDRESS_FREE("DirectX", prog->computeShader_);
            prog->computeShader_->Release();
            prog->computeShader_ = NULL;
        }
        hDELETE_ARRAY_SAFE(heap, prog->shaderBlob_);
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
        hTRACK_CUSTOM_ADDRESS_ALLOC("DirectX", texture->dx11Texture_);

        if ( flags & RESOURCEFLAG_RENDERTARGET )
        {
            hr = d3d11Device_->CreateRenderTargetView( texture->dx11Texture_, NULL, &texture->renderTargetView_ );
            hcAssert( SUCCEEDED( hr ) );
            hTRACK_CUSTOM_ADDRESS_ALLOC("DirectX", texture->renderTargetView_);
        }

        if ( flags & RESOURCEFLAG_DEPTHTARGET )
        {
            D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
            ZeroMemory( &descDSV, sizeof(descDSV) );
            if ( format == TFORMAT_D24S8F )
                descDSV.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
            else if ( format == TFORMAT_D32F )
                descDSV.Format = DXGI_FORMAT_D32_FLOAT;
            else
                hcAssertFailMsg( "Invalid Depth format" );
            descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
            descDSV.Texture2D.MipSlice = 0;
            hr = d3d11Device_->CreateDepthStencilView( texture->dx11Texture_, &descDSV, &texture->depthStencilView_ );
            hcAssert( SUCCEEDED( hr ) );
            hTRACK_CUSTOM_ADDRESS_ALLOC("DirectX", texture->depthStencilView_);

            D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
            hZeroMem( &srvDesc, sizeof(srvDesc) );
            srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
            if ( format == TFORMAT_D24S8F )
                srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
            else if ( format == TFORMAT_D32F )
                srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
            else
                hcAssertFailMsg( "Invalid Depth format" );
            srvDesc.Texture2D.MipLevels = 1;
            srvDesc.Texture2D.MostDetailedMip = 0;

            hr = d3d11Device_->CreateShaderResourceView( texture->dx11Texture_, &srvDesc, &texture->shaderResourceView_ );
            hcAssert( SUCCEEDED( hr ) );
            hTRACK_CUSTOM_ADDRESS_ALLOC("DirectX", texture->shaderResourceView_);
        }
        else
        {
            hr = d3d11Device_->CreateShaderResourceView( texture->dx11Texture_, NULL, &texture->shaderResourceView_ );
            hcAssert( SUCCEEDED( hr ) );
            hTRACK_CUSTOM_ADDRESS_ALLOC("DirectX", texture->shaderResourceView_);
        }
        

        return texture;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdDX11RenderDevice::destroyTextureDevice(hdDX11Texture* texture)
    {
        if (texture->dx11Texture_) {
            hTRACK_CUSTOM_ADDRESS_FREE("DirectX", texture->dx11Texture_);
            texture->dx11Texture_->Release();
            texture->dx11Texture_ = NULL;
        }
        if (texture->depthStencilView_) {
            hTRACK_CUSTOM_ADDRESS_FREE("DirectX", texture->depthStencilView_);
            texture->depthStencilView_->Release();
            texture->depthStencilView_ = NULL;
        }
        if (texture->shaderResourceView_) {
            hTRACK_CUSTOM_ADDRESS_FREE("DirectX", texture->shaderResourceView_);
            texture->shaderResourceView_->Release();
            texture->shaderResourceView_ = NULL;
        }
        if (texture->renderTargetView_) {
            hTRACK_CUSTOM_ADDRESS_FREE("DirectX", texture->renderTargetView_);
            texture->renderTargetView_->Release();
            texture->renderTargetView_ = NULL;
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hdDX11IndexBuffer* hdDX11RenderDevice::createIndexBufferDevice( hUint32 sizeInBytes, void* initialDataPtr, hUint32 flags, hdDX11IndexBuffer* idxBuf )
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
        hTRACK_CUSTOM_ADDRESS_ALLOC("DirectX", idxBuf->buffer_);

        idxBuf->flags_ = flags;
        idxBuf->dataSize_ = sizeInBytes;
        return idxBuf;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdDX11RenderDevice::destroyIndexBufferDevice(hdDX11IndexBuffer* indexBuffer)
    {
        hTRACK_CUSTOM_ADDRESS_FREE("DirectX", indexBuffer->buffer_);
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

        hr = d3d11Device_->CreateInputLayout( elements, elementCount, shaderProg, progLen, &layout );
        hcAssert( SUCCEEDED( hr ) );
        hTRACK_CUSTOM_ADDRESS_ALLOC("DirectX", layout);

        return layout;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdDX11RenderDevice::DestroyVertexLayout( hdDX11VertexLayout* layout )
    {
        hTRACK_CUSTOM_ADDRESS_FREE("DirectX", layout);
        layout->Release();
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hdDX11VertexBuffer* hdDX11RenderDevice::createVertexBufferDevice(hInputLayoutDesc* desc, hUint32 desccount, hUint32 sizeInBytes, void* initialDataPtr, hUint32 flags, hdDX11VertexBuffer* vtxBuf)
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
        hTRACK_CUSTOM_ADDRESS_ALLOC("DirectX", vtxBuf->buffer_);

        vtxBuf->flags_ = flags;
        vtxBuf->dataSize_ = sizeInBytes;
        hMemCpy(vtxBuf->streamLayoutDesc_, desc, sizeof(hInputLayoutDesc)*desccount);
        vtxBuf->streamDescCount_=desccount;
        return vtxBuf;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdDX11RenderDevice::destroyVertexBufferDevice(hdDX11VertexBuffer* vtxBuf)
    {
        hTRACK_CUSTOM_ADDRESS_FREE("DirectX", vtxBuf->buffer_);
        vtxBuf->buffer_->Release();
        vtxBuf->buffer_ = NULL;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hdDX11BlendState* hdDX11RenderDevice::CreateBlendState( const hBlendStateDesc& desc )
    {
        HRESULT hr;
        //Build state key
        hUint32 stateKey = hCRC32::FullCRC( (const hChar*)&desc, sizeof(desc) );

        resourceMutex_.Lock();

        hdDX11BlendState* outBlendState = blendStates_.Find( stateKey );

        if ( !outBlendState )
        {
            //
            outBlendState = hNEW( GetGlobalHeap()/*!heap*/, hdDX11BlendState );
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

            hr = d3d11Device_->CreateBlendState( &blendDesc, &outBlendState->stateObj_ );
            hcAssert( SUCCEEDED( hr ) );
            hTRACK_CUSTOM_ADDRESS_ALLOC("DirectX", outBlendState->stateObj_);

            blendStates_.Insert( stateKey, outBlendState );
        }

        outBlendState->AddRef();

        resourceMutex_.Unlock();

        return outBlendState;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdDX11RenderDevice::DestroyBlendState( hdDX11BlendState* state )
    {
        hcAssert( state );
        state->DecRef();

        if ( state->GetRefCount() == 0 )
        {
            resourceMutex_.Lock();

            blendStates_.Remove( state->GetKey() );

            hTRACK_CUSTOM_ADDRESS_FREE("DirectX", state->stateObj_);
            state->stateObj_->Release();

            hDELETE(GetGlobalHeap()/*!heap*/, state);

            resourceMutex_.Unlock();
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hdDX11RasterizerState* hdDX11RenderDevice::CreateRasterizerState( const hRasterizerStateDesc& desc )
    {
        HRESULT hr;
        //Build state key
        hUint32 stateKey = hCRC32::FullCRC( (const hChar*)&desc, sizeof(desc) );

        resourceMutex_.Lock();

        hdDX11RasterizerState* outRasState = rasterizerStates_.Find( stateKey );

        if ( !outRasState )
        {
            outRasState = hNEW( GetGlobalHeap()/*!heap*/, hdDX11RasterizerState );
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

            hr = d3d11Device_->CreateRasterizerState( &rasDesc, &outRasState->stateObj_ );
            hcAssert( SUCCEEDED( hr ) );
            hTRACK_CUSTOM_ADDRESS_ALLOC("DirectX", outRasState->stateObj_);

            rasterizerStates_.Insert( stateKey, outRasState );
        }

        outRasState->AddRef();

        resourceMutex_.Unlock();

        return outRasState;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdDX11RenderDevice::DestoryRasterizerState( hdDX11RasterizerState* state )
    {
        hcAssert( state );
        state->DecRef();

        if ( state->GetRefCount() == 0 )
        {
            resourceMutex_.Lock();

            rasterizerStates_.Remove( state->GetKey() );

            hTRACK_CUSTOM_ADDRESS_FREE("DirectX", state->stateObj_);
            state->stateObj_->Release();

            hDELETE(GetGlobalHeap()/*!heap*/, state);

            resourceMutex_.Unlock();
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hdDX11DepthStencilState* hdDX11RenderDevice::CreateDepthStencilState( const hDepthStencilStateDesc& desc )
    {
        HRESULT hr;
        //Build state key
        hUint32 stateKey = hCRC32::FullCRC( (const hChar*)&desc, sizeof(desc) );

        resourceMutex_.Lock();

        hdDX11DepthStencilState* outDSState = depthStencilStates_.Find( stateKey );

        if ( !outDSState )
        {
            outDSState = hNEW(GetGlobalHeap()/*!heap*/, hdDX11DepthStencilState);
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

            hr = d3d11Device_->CreateDepthStencilState( &dsDesc, &outDSState->stateObj_ );
            hcAssert( SUCCEEDED( hr ) );
            hTRACK_CUSTOM_ADDRESS_ALLOC("DirectX", outDSState->stateObj_);

            depthStencilStates_.Insert( stateKey, outDSState );
        }

        outDSState->AddRef();

        resourceMutex_.Unlock();

        return outDSState;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdDX11RenderDevice::DestoryDepthStencilState( hdDX11DepthStencilState* state )
    {
        hcAssert( state );
        state->DecRef();

        if ( state->GetRefCount() == 0 )
        {
            resourceMutex_.Lock();

            depthStencilStates_.Remove( state->GetKey() );

            hTRACK_CUSTOM_ADDRESS_FREE("DirectX", state->stateObj_);
            state->stateObj_->Release();

            hDELETE(GetGlobalHeap()/*!heap*/, state);

            resourceMutex_.Unlock();
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hdDX11SamplerState* hdDX11RenderDevice::CreateSamplerState( const hSamplerStateDesc& desc )
    {
        HRESULT hr;
        //Build state key
        hUint32 stateKey = hCRC32::FullCRC( (const hChar*)&desc, sizeof(desc) );

        resourceMutex_.Lock();

        hdDX11SamplerState* outSamState = samplerStateMap_.Find( stateKey );

        if ( !outSamState )
        {
            outSamState = hNEW(GetGlobalHeap()/*!heap*/, hdDX11SamplerState);
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

            hr = d3d11Device_->CreateSamplerState( &samDesc, &outSamState->stateObj_ );
            hcAssert( SUCCEEDED( hr ) );
            hTRACK_CUSTOM_ADDRESS_ALLOC("DirectX", outSamState->stateObj_);

            samplerStateMap_.Insert( stateKey, outSamState );
        }

        outSamState->AddRef();

        resourceMutex_.Unlock();

        return outSamState;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdDX11RenderDevice::DestroySamplerState( hdDX11SamplerState* state )
    {
        hcAssert( state );
        state->DecRef();

        if ( state->GetRefCount() == 0 )
        {
            resourceMutex_.Lock();

            samplerStateMap_.Remove( state->GetKey() );

            hTRACK_CUSTOM_ADDRESS_FREE("DirectX", state->stateObj_);
            state->stateObj_->Release();

            hDELETE(GetGlobalHeap()/*!heap*/, state);

            resourceMutex_.Unlock();
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hUint32 hdDX11RenderDevice::computeVertexLayoutStride(hInputLayoutDesc* desc, hUint32 desccount)
    {
        hUint32 stride = 0;

        for (hUint32 i = 0; i < desccount; ++i)
        {
            switch(desc[i].typeFormat_)
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

            switch(desc[i].semantic_)
            {
            case eIS_POSITION:  elements[i].SemanticName = "POSITION";  break;
            case eIS_TEXCOORD:  elements[i].SemanticName = "TEXCOORD";  break;
            case eIS_NORMAL:    elements[i].SemanticName = "NORMAL";    break;
            case eIS_TANGENT:   elements[i].SemanticName = "TANGENT";   break;
            case eIS_BITANGENT: elements[i].SemanticName = "BITANGENT"; break;
            case eIS_COLOUR:    elements[i].SemanticName = "COLOR";     break;
            case eIS_INSTANCE:  elements[i].SemanticName = "INSTANCE";  break;
            }

            switch(desc[i].typeFormat_)
            {
            case eIF_FLOAT4: *stride += 4*sizeof(hFloat); elements[i].Format = DXGI_FORMAT_R32G32B32A32_FLOAT; break;
            case eIF_FLOAT3: *stride += 3*sizeof(hFloat); elements[i].Format = DXGI_FORMAT_R32G32B32_FLOAT; break;
            case eIF_FLOAT2: *stride += 2*sizeof(hFloat); elements[i].Format = DXGI_FORMAT_R32G32_FLOAT; break;
            case eIF_FLOAT1: *stride += sizeof(hFloat);   elements[i].Format = DXGI_FORMAT_R32_FLOAT; break;
            case eIF_UBYTE4_UNORM: *stride += 4*sizeof(hByte); elements[i].Format = DXGI_FORMAT_R8G8B8A8_UNORM; break;
            case eIF_UBYTE4_SNORM: *stride += 4*sizeof(hByte); elements[i].Format = DXGI_FORMAT_R8G8B8A8_SNORM; break;
            }

            elements[i].InputSlot=desc[i].inputStream_;
            elements[i].SemanticIndex = desc[i].semIndex_;
            elements[i].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
            elements[i].InputSlotClass = desc[i].instanceDataRepeat_ == 0 ? D3D11_INPUT_PER_VERTEX_DATA : D3D11_INPUT_PER_INSTANCE_DATA;
            elements[i].InstanceDataStepRate = desc[i].instanceDataRepeat_;

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

    hdDX11ParameterConstantBlock* hdDX11RenderDevice::CreateConstantBlocks(const hUint32* sizes, void** initdatas, hUint32 count) {
        hdDX11ParameterConstantBlock* constBlocks = hNEW_ARRAY(GetGlobalHeap()/*!heap*/, hdDX11ParameterConstantBlock, count);
        for ( hUint32 i = 0; i < count; ++i ) {
            HRESULT hr;
            D3D11_BUFFER_DESC desc={0};
            D3D11_SUBRESOURCE_DATA resdata={0};
            D3D11_SUBRESOURCE_DATA* initdata=initdatas ? &resdata : NULL;
            //hZeroMem( &desc, sizeof(desc) );
            resdata.pSysMem=initdatas ? initdatas[i] : NULL;
            desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
            desc.Usage = D3D11_USAGE_DYNAMIC;
            desc.ByteWidth = sizes[i];
            desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
            hr = d3d11Device_->CreateBuffer( &desc, initdata, &constBlocks[i].constBuffer_ );
            hTRACK_CUSTOM_ADDRESS_ALLOC("DirectX", constBlocks[i].constBuffer_);
            hcAssert( SUCCEEDED( hr ) );
            constBlocks[i].size_ = sizes[i];
        }

        return constBlocks;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdDX11RenderDevice::DestroyConstantBlocks(hdDX11ParameterConstantBlock* constBlocks, hUint32 count, hBool inPlace)
    {
        for ( hUint32 i = 0; i < count; ++i )
        {
            hTRACK_CUSTOM_ADDRESS_FREE("DirectX", constBlocks[i].constBuffer_);
            constBlocks[i].constBuffer_->Release();
            constBlocks[i].constBuffer_ = NULL;
            constBlocks[i].size_ = 0;
        }

        hDELETE_ARRAY_SAFE(GetGlobalHeap()/*!heap*/, constBlocks);
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
        hcAssertMsg(SUCCEEDED(hr), "Failed to create Unorder Access View");
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
}
