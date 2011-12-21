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
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hdDX11RenderDevice::hdDX11RenderDevice() 
        : kernel_(NULL)
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

    void hdDX11RenderDevice::Create( Device::Kernel* sysHandle, hUint32 width, hUint32 height, hUint32 bbp, hFloat shaderVersion, hBool fullscreen, hBool vsync )
    {
        HRESULT hr;

        kernel_ = sysHandle;
        width_ = width;
        height_ = height;

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
        sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        sd.BufferDesc.RefreshRate.Numerator = 60;
        sd.BufferDesc.RefreshRate.Denominator = 1;
        sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd.OutputWindow = sysHandle->GetSystemHandle()->hWnd_;
        sd.SampleDesc.Count = 1;
        sd.SampleDesc.Quality = 0;
        sd.Windowed = !fullscreen;

        if( D3D11CreateDeviceAndSwapChain( 
                NULL, 
                D3D_DRIVER_TYPE_HARDWARE, 
                NULL, 
                0, 
                featureLevels, 
                numFeatureLevels,
                D3D11_SDK_VERSION, 
                &sd, 
                &mainSwapChain_, 
                &d3d11Device_, 
                &featureLevel_, 
                &mainDeviceCtx_ ) )
        {
            hcAssertFailMsg( "Couldn't Create D3D11 context" );
        }

        //Grab the back buffer & depth buffer
        // Create a render target view
        ID3D11Texture2D* pBackBuffer = NULL;
        hr = mainSwapChain_->GetBuffer( 0, __uuidof( ID3D11Texture2D ), (LPVOID*)&pBackBuffer );
        hcAssert( SUCCEEDED( hr ) );

        hr = d3d11Device_->CreateRenderTargetView( pBackBuffer, NULL, &renderTargetView_ );
        pBackBuffer->Release();
        hcAssert( SUCCEEDED( hr ) );

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

        // Create the depth stencil view
        D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
        ZeroMemory( &descDSV, sizeof(descDSV) );
        descDSV.Format = descDepth.Format;
        descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
        descDSV.Texture2D.MipSlice = 0;
        hr = d3d11Device_->CreateDepthStencilView( depthStencil_, &descDSV, &depthStencilView_ );
        hcAssert( SUCCEEDED( hr ) );

        mainRenderCtx_.device_ = mainDeviceCtx_;
        mainRenderCtx_.renderTargetView_ = renderTargetView_;
        mainRenderCtx_.depthStencilView_ = depthStencilView_;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdDX11RenderDevice::Destroy()
    {
        if( mainDeviceCtx_ ) 
        {
            mainDeviceCtx_->ClearState();
        }
        if ( renderTargetView_ )
        {
            renderTargetView_->Release();
        }
        if ( depthStencilView_ )
        {
            depthStencilView_->Release();
        }
        if ( depthStencil_ )
        {
            depthStencil_->Release();
        }
        if ( mainSwapChain_ )
        {
            mainSwapChain_->Release();
        }
        if ( mainDeviceCtx_ )
        {
            mainDeviceCtx_->Release();
        }
        if ( d3d11Device_ )
        {
            d3d11Device_->Release();
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdDX11RenderDevice::BeginRender()
    {
        mainDeviceCtx_->OMSetRenderTargets( 1, &renderTargetView_, depthStencilView_ );
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdDX11RenderDevice::EndRender()
    {

    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdDX11RenderDevice::SwapBuffers()
    {
        mainSwapChain_->Present( 1, 0 );
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdDX11RenderDevice::RunSubmissionCtx( hdDX11RenderSubmissionCtx* subCtx )
    {
        //TODO:
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hdDX11ShaderProgram* hdDX11RenderDevice::CompileShader( const hChar* shaderProg, hUint32 len, ShaderType type )
    {
        HRESULT hr;
/*
        const hChar* profile;
        if ( type == ShaderType_VERTEXPROG )
        {
            switch ( featureLevel_ )
            {
            case D3D_FEATURE_LEVEL_11_0:
                profile = "vs_5_0"; break;
            case D3D_FEATURE_LEVEL_10_1:
                profile = "vs_4_0"; break;
            case D3D_FEATURE_LEVEL_10_0:
                profile = "vs_4_0"; break;
            case D3D_FEATURE_LEVEL_9_3:
            default:
                profile = "vs_3_0";//Won't work
            }
        }

        if ( type == ShaderType_FRAGMENTPROG )
        {
            switch ( featureLevel_ )
            {
            case D3D_FEATURE_LEVEL_11_0:
                profile = "ps_5_0"; break;
            case D3D_FEATURE_LEVEL_10_1:
                profile = "ps_4_0"; break;
            case D3D_FEATURE_LEVEL_10_0:
                profile = "ps_4_0"; break;
            case D3D_FEATURE_LEVEL_9_3:
            default:
                profile = "ps_3_0";//Won't work
            }
        }

        hUint32 compileFlags = D3DCOMPILE_ENABLE_BACKWARDS_COMPATIBILITY | D3DCOMPILE_DEBUG;
        ID3DBlob* result;
        ID3DBlob* errors;

        hr = D3DX11CompileFromMemory( 
            shaderProg, 
            len, 
            "Memory", 
            NULL, 
            NULL, 
            "mainPS", 
            profile, 
            compileFlags, 
            0, 
            NULL, 
            &result, 
            &errors, 
            NULL );

        if ( FAILED( hr ) )
        {
            hcAssertMsg( SUCCEEDED( hr ), "Shader Compile Failed: %s", errors->GetBufferPointer() );
            return NULL;
        }
*/
        hdDX11ShaderProgram* shader = hNEW ( hRendererHeap ) hdDX11ShaderProgram;

        if ( type == ShaderType_FRAGMENTPROG )
        {
            hr = d3d11Device_->CreatePixelShader( shaderProg, len, NULL, &shader->pixelShader_ );
            hcAssert( SUCCEEDED( hr ) );
            hr = D3DReflect( shaderProg, len, IID_ID3D11ShaderReflection, (void**)&shader->shaderInfo_ );
            hcAssert( SUCCEEDED( hr ) );
        }
        else if ( type == ShaderType_VERTEXPROG )
        {
            hr = d3d11Device_->CreateVertexShader( shaderProg, len, NULL, &shader->vertexShader_ );
            hcAssert( SUCCEEDED( hr ) );
            hr = D3DReflect( shaderProg, len, IID_ID3D11ShaderReflection, (void**)&shader->shaderInfo_ );
            hcAssert( SUCCEEDED( hr ) );
        }

//HOW TO:
//         D3D11_SHADER_DESC desc;
//         shader->shaderInfo_->GetDesc( &desc );
// 
//         //VERTEX/PIXEL INPUT
//         for ( hUint32 i = 0; i < desc.InputParameters; ++i )
//         {
//             D3D11_SIGNATURE_PARAMETER_DESC ipDesc;
//             shader->shaderInfo_->GetInputParameterDesc( i, &ipDesc );
// 
//             hcPrintf( ipDesc.SemanticName );
//         }
// 
//         //SAMPLERS/CONSTANT BUFFERS/ETC
//         for ( hUint32 i = 0; i < desc.BoundResources; ++i )
//         {
//             D3D11_SHADER_INPUT_BIND_DESC brDesc;
//             shader->shaderInfo_->GetResourceBindingDesc( i, &brDesc );
// 
//             hcPrintf( brDesc.Name );
//         }
// 
//         //CONSTANT BUFFERS
//         for ( hUint32 i = 0; i < desc.ConstantBuffers; ++i )
//         {
//             ID3D11ShaderReflectionConstantBuffer* constInfo = shader->shaderInfo_->GetConstantBufferByIndex( i );
//             D3D11_SHADER_BUFFER_DESC bufInfo;
//             constInfo->GetDesc( &bufInfo );
// 
//             //TODO: Get Parameters...
//             for ( hUint32 i = 0; i < bufInfo.Variables; ++i )
//             {
//                 ID3D11ShaderReflectionVariable* var = constInfo->GetVariableByIndex( i );
//                 D3D11_SHADER_VARIABLE_DESC varDesc;
//                 var->GetDesc(&varDesc);
// 
//                 hcPrintf( varDesc.Name );
//             }
//         }

        return shader;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hdDX11Texture* hdDX11RenderDevice::CreateTextrue( hUint32 width, hUint32 height, hUint32 levels, TextureFormat format, void* initialData, hUint32 initDataSize )
    {
        hdDX11Texture* texture = hNEW ( hRendererHeap ) hdDX11Texture;

        D3D11_TEXTURE2D_DESC desc;
        desc.Height             = height;
        desc.Width              = width;
        desc.MipLevels          = 0;//levels;
        desc.ArraySize          = 1;
        desc.SampleDesc.Count   = 1;
        switch ( format )
        {
        case TFORMAT_ARGB8:     desc.Format = DXGI_FORMAT_R8G8B8A8_UINT; break;
        case TFORMAT_XRGB8:     desc.Format = DXGI_FORMAT_R8G8B8A8_UINT; break;
        case TFORMAT_RGB8:      desc.Format = DXGI_FORMAT_R8G8B8A8_UINT; break;
        case TFORMAT_R16F:      desc.Format = DXGI_FORMAT_R16_FLOAT; break;
        case TFORMAT_GR16F:     desc.Format = DXGI_FORMAT_R16G16_FLOAT; break;
        case TFORMAT_ABGR16F:   desc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT; break;
        case TFORMAT_R32F:      desc.Format = DXGI_FORMAT_R32_FLOAT; break;
        case TFORMAT_D32F:      desc.Format = DXGI_FORMAT_D32_FLOAT; break;
        case TFORMAT_D24S8F:    desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT; break;
        case TFORMAT_L8:        desc.Format = DXGI_FORMAT_A8_UNORM; break;
        case TFORMAT_DXT5:      desc.Format = DXGI_FORMAT_BC5_UNORM; break;
        case TFORMAT_DXT3:      desc.Format = DXGI_FORMAT_BC3_UNORM; break;
        case TFORMAT_DXT1:      desc.Format = DXGI_FORMAT_BC1_UNORM; break; 
        }
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
        desc.CPUAccessFlags  = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
        desc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

        D3D11_SUBRESOURCE_DATA data;
        D3D11_SUBRESOURCE_DATA* dataptr = NULL;
        
        if ( initialData )
        {
            data.pSysMem          = initialData;
            data.SysMemPitch      = initDataSize / height;
            data.SysMemSlicePitch = 0;
            dataptr = &data;
        }

        d3d11Device_->CreateTexture2D( &desc, dataptr, &texture->dx11Texture_ );

        return texture;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdDX11RenderDevice::DestroyTexture( hdDX11Texture* texture )
    {
        texture->dx11Texture_->Release();
        texture->dx11Texture_ = NULL;
        delete texture;
    }

}