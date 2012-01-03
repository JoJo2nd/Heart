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
            outBlendState = hNEW( hGeneralHeap ) hdDX11BlendState();
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

            blendStates_.Insert( stateKey, outBlendState );
        }

        hAtomic::Increment( &outBlendState->refCount_ );

        resourceMutex_.Unlock();

        return outBlendState;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdDX11RenderDevice::DestroyBlendState( hdDX11BlendState* state )
    {
        hcAssert( state );
        hAtomic::Decrement( &state->refCount_ );

        if ( state->refCount_ == 0 )
        {
            resourceMutex_.Lock();

            blendStates_.Remove( state->GetKey() );

            delete state; 

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
            outRasState = hNEW( hGeneralHeap ) hdDX11RasterizerState();
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

            rasterizerStates_.Insert( stateKey, outRasState );
        }

        hAtomic::Increment( &outRasState->refCount_ );

        resourceMutex_.Unlock();

        return outRasState;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdDX11RenderDevice::DestoryRasterizerState( hdDX11RasterizerState* state )
    {

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
            outDSState = hNEW( hGeneralHeap ) hdDX11DepthStencilState();
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

            depthStencilStates_.Insert( stateKey, outDSState );
        }

        hAtomic::Increment( &outDSState->refCount_ );

        resourceMutex_.Unlock();

        return outDSState;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdDX11RenderDevice::DestoryDepthStencilState( hdDX11DepthStencilState* state )
    {

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
            outSamState = hNEW( hGeneralHeap ) hdDX11SamplerState();
            D3D11_SAMPLER_DESC samDesc;

            switch ( desc.filter_ )
            {
            case SSV_POINT:             samDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR; break;
            case SSV_LINEAR:            samDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_POINT;  break;
            case SSV_ANISOTROPIC:       samDesc.Filter = D3D11_FILTER_COMPARISON_ANISOTROPIC;        break;
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

            samplerStateMap_.Insert( stateKey, outSamState );
        }

        hAtomic::Increment( &outSamState->refCount_ );

        resourceMutex_.Unlock();

        return outSamState;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdDX11RenderDevice::DestroySamplerState( hdDX11SamplerState* state )
    {

    }
}
