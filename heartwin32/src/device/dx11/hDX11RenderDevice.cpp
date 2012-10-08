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
        : sysWindow_(NULL)
    {
        vertexLayoutMap_.SetHeap(GetGlobalHeap()/*!heap*/);
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

    void hdDX11RenderDevice::Create( hdSystemWindow* sysHandle, hUint32 width, hUint32 height, hUint32 bbp, hFloat shaderVersion, hBool fullscreen, hBool vsync, hRenderDeviceSetup setup )
    {
        HRESULT hr;

        sysWindow_ = sysHandle;
        width_ = width;
        height_ = height;
        alloc_ = setup.alloc_;
        free_ = setup.free_;

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
        sd.Windowed = !fullscreen;

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

        mainRenderCtx_.SetDeviceCtx( mainDeviceCtx_, alloc_, free_ );

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
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdDX11RenderDevice::Destroy()
    {
        timerDisjoint_->Release();
        timerFrameStart_->Release();
        timerFrameEnd_->Release();

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
        
        hFloat clearcolour[] = { 1.f, 0.f, 1.f, 1.f };
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
        ctx->SetDefaultTargets( renderTargetView_, depthStencilView_ );
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
        ctx->SetDefaultTargets( renderTargetView_, depthStencilView_ );
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hdDX11ShaderProgram* hdDX11RenderDevice::CompileShader(const hChar* shaderProg, hUint32 len, hInputLayoutDesc* inputLayout, hUint32 layoutCount, ShaderType type)
    {
        HRESULT hr;
        hdDX11ShaderProgram* shader = hNEW ( GetGlobalHeap()/*!heap*/, hdDX11ShaderProgram );
        shader->type_ = type;

        if ( type == ShaderType_FRAGMENTPROG )
        {
            shader->inputLayoutFlags_ = 0;
            hr = d3d11Device_->CreatePixelShader( shaderProg, len, NULL, &shader->pixelShader_ );
            hcAssert( SUCCEEDED( hr ) );
            hr = D3DReflect( shaderProg, len, IID_ID3D11ShaderReflection, (void**)&shader->shaderInfo_ );
            hcAssert( SUCCEEDED( hr ) );
        }
        else if ( type == ShaderType_VERTEXPROG )
        {
            hdDX11VertexLayout* vl = CreateVertexLayout(inputLayout, layoutCount, shaderProg, len);

            shader->inputLayoutFlags_ = 0/*inputLayout*/;//TODO: grab layout id from createvertexlayout
            shader->inputLayout_ = vl;
            
            hr = d3d11Device_->CreateVertexShader( shaderProg, len, NULL, &shader->vertexShader_ );
            hcAssert( SUCCEEDED( hr ) );
            hr = D3DReflect( shaderProg, len, IID_ID3D11ShaderReflection, (void**)&shader->shaderInfo_ );
            hcAssert( SUCCEEDED( hr ) );
        }

        return shader;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdDX11RenderDevice::DestroyShader( hdDX11ShaderProgram* shaderProg )
    {
        hDELETE(GetGlobalHeap()/*!heap*/, shaderProg);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hdDX11Texture* hdDX11RenderDevice::CreateTextrue( hUint32 width, hUint32 height, hUint32 levels, hTextureFormat format, hMipDesc* initialData, hUint32 flags )
    {
        HRESULT hr;
        hdDX11Texture* texture = hNEW(GetGlobalHeap()/*!heap*/, hdDX11Texture);
        hBool compressedFormat = hFalse;

        D3D11_TEXTURE2D_DESC desc;
        hZeroMem( &desc, sizeof(desc) );
        desc.Height             = height;
        desc.Width              = width;
        desc.MipLevels          = levels;
        desc.ArraySize          = 1;
        desc.SampleDesc.Count   = 1;
        switch ( format )
        {
        case TFORMAT_ARGB8:         desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; break;
        case TFORMAT_ARGB8_sRGB:    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB; break;
        case TFORMAT_XRGB8:         desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; break;
        case TFORMAT_XRGB8_sRGB:    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB; break;
        case TFORMAT_R16F:          desc.Format = DXGI_FORMAT_R16_FLOAT; break;
        case TFORMAT_GR16F:         desc.Format = DXGI_FORMAT_R16G16_FLOAT; break;
        case TFORMAT_ABGR16F:       desc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT; break;
        case TFORMAT_R32F:          desc.Format = DXGI_FORMAT_R32_FLOAT; break;
        case TFORMAT_D32F:          desc.Format = DXGI_FORMAT_R32_FLOAT; break;
        case TFORMAT_D24S8F:        desc.Format = DXGI_FORMAT_R24G8_TYPELESS; break;
        case TFORMAT_L8:            desc.Format = DXGI_FORMAT_A8_UNORM; break;
        case TFORMAT_DXT5:          desc.Format = DXGI_FORMAT_BC3_UNORM; compressedFormat = hTrue; break;
        case TFORMAT_DXT3:          desc.Format = DXGI_FORMAT_BC2_UNORM; compressedFormat = hTrue; break;
        case TFORMAT_DXT1:          desc.Format = DXGI_FORMAT_BC1_UNORM; compressedFormat = hTrue; break; 
        case TFORMAT_DXT5_sRGB:     desc.Format = DXGI_FORMAT_BC3_UNORM_SRGB; compressedFormat = hTrue; break;
        case TFORMAT_DXT3_sRGB:     desc.Format = DXGI_FORMAT_BC2_UNORM_SRGB; compressedFormat = hTrue; break;
        case TFORMAT_DXT1_sRGB:     desc.Format = DXGI_FORMAT_BC1_UNORM_SRGB; compressedFormat = hTrue; break;
        }
        desc.Usage = (flags & RESOURCEFLAG_DYNAMIC) ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;
        desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        desc.BindFlags |= (flags & RESOURCEFLAG_RENDERTARGET) ? D3D11_BIND_RENDER_TARGET : 0; 
        desc.BindFlags |= (flags & RESOURCEFLAG_DEPTHTARGET) ? D3D11_BIND_DEPTH_STENCIL : 0;
        desc.CPUAccessFlags  = (flags & RESOURCEFLAG_DYNAMIC) ? (D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE) : 0;
        desc.MiscFlags = 0;//(flags & RESOURCEFLAG_DEPTHTARGET) ? 0 : D3D11_RESOURCE_MISC_GENERATE_MIPS;

        D3D11_SUBRESOURCE_DATA* dataptr = (D3D11_SUBRESOURCE_DATA*)hAlloca(sizeof(D3D11_SUBRESOURCE_DATA)*desc.MipLevels+2);
        
        if ( initialData )
        {
            for (hUint32 i = 0; i < desc.MipLevels; ++i)
            {
                D3D11_SUBRESOURCE_DATA& data = dataptr[i];
                data.pSysMem          = initialData[i].data;
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
        }

        hr = d3d11Device_->CreateTexture2D( &desc, dataptr, &texture->dx11Texture_ );
        hcAssert( SUCCEEDED( hr ) );

        if ( flags & RESOURCEFLAG_RENDERTARGET )
        {
            hr = d3d11Device_->CreateRenderTargetView( texture->dx11Texture_, NULL, &texture->renderTargetView_ );
            hcAssert( SUCCEEDED( hr ) );
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
        }
        else
        {
            hr = d3d11Device_->CreateShaderResourceView( texture->dx11Texture_, NULL, &texture->shaderResourceView_ );
            hcAssert( SUCCEEDED( hr ) );
        }
        

        return texture;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdDX11RenderDevice::DestroyTexture( hdDX11Texture* texture )
    {
        if (texture->dx11Texture_)
        {
            texture->dx11Texture_->Release();
            texture->dx11Texture_ = NULL;
        }
        hDELETE_SAFE(GetGlobalHeap()/*!heap*/, texture);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hdDX11IndexBuffer* hdDX11RenderDevice::CreateIndexBufferDevice( hUint32 sizeInBytes, void* initialDataPtr, hUint32 flags )
    {
        hdDX11IndexBuffer* idxBuf = hNEW( GetGlobalHeap()/*!heap*/, hdDX11IndexBuffer );
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
        return idxBuf;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdDX11RenderDevice::DestroyIndexBufferDevice( hdDX11IndexBuffer* indexBuffer )
    {
        indexBuffer->buffer_->Release();
        indexBuffer->buffer_ = NULL;
        hDELETE(GetGlobalHeap()/*!heap*/, indexBuffer);
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

        resourceMutex_.Lock();

        layout = vertexLayoutMap_.Find( inputLayoutId );

        if (!layout)
        {
            layout = hNEW( GetGlobalHeap()/*!heap*/, hdDX11VertexLayout );
            hr = d3d11Device_->CreateInputLayout( elements, elementCount, shaderProg, progLen, &layout->layout_ );
            hcAssert( SUCCEEDED( hr ) );

            vertexLayoutMap_.Insert( inputLayoutId, layout);
        }

        resourceMutex_.Unlock();

        return layout;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdDX11RenderDevice::DestroyVertexLayout( hdDX11VertexLayout* layout )
    {
        //TODO:
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hdDX11VertexBuffer* hdDX11RenderDevice::CreateVertexBufferDevice( hUint32 vertexLayout, hUint32 sizeInBytes, void* initialDataPtr, hUint32 flags )
    {
        hdDX11VertexBuffer* vtxBuf = hNEW( GetGlobalHeap()/*!heap*/, hdDX11VertexBuffer );
        HRESULT hr;
        D3D11_BUFFER_DESC desc;
        D3D11_SUBRESOURCE_DATA initData;

        hZeroMem( &desc, sizeof(desc) );
        desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        desc.ByteWidth = sizeInBytes;
        desc.Usage = (flags & RESOURCEFLAG_DYNAMIC) ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;
        desc.CPUAccessFlags = (flags & RESOURCEFLAG_DYNAMIC) ? D3D11_CPU_ACCESS_WRITE : 0;
        desc.MiscFlags = 0;

        hZeroMem( &initData, sizeof(initData) );
        initData.pSysMem = initialDataPtr;

        hr = d3d11Device_->CreateBuffer( &desc, initialDataPtr ? &initData : NULL, &vtxBuf->buffer_ );
        hcAssert( SUCCEEDED( hr ) );

        vtxBuf->flags_ = flags;
        vtxBuf->dataSize_ = sizeInBytes;
        vtxBuf->vertexLayoutFlags_ = vertexLayout;
        return vtxBuf;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdDX11RenderDevice::DestroyVertexBufferDevice( hdDX11VertexBuffer* vtxBuf )
    {
        vtxBuf->buffer_->Release();
        vtxBuf->buffer_ = NULL;
        hDELETE(GetGlobalHeap()/*!heap*/, vtxBuf);
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

            hDELETE(GetGlobalHeap()/*!heap*/, state);

            resourceMutex_.Unlock();
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hUint32 hdDX11RenderDevice::ComputeVertexLayoutStride(hInputLayoutDesc* desc, hUint32 desccount)
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

    hUint32 hdDX11RenderDevice::BuildVertexFormatArray(const hInputLayoutDesc* desc, hUint32 desccount, hUint32* stride, hUint32* fmtID, D3D11_INPUT_ELEMENT_DESC* elements)
    {
        hUint32 elementsadded = 0;
        WORD offset = 0;
        *stride = 0;

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

            elements[i].SemanticIndex = desc[i].semIndex_;
            elements[i].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
            elements[i].InputSlotClass = desc[i].instanceDataRepeat_ == 0 ? D3D11_INPUT_PER_VERTEX_DATA : D3D11_INPUT_PER_INSTANCE_DATA;
            elements[i].InstanceDataStepRate = desc[i].instanceDataRepeat_;

            if (i == 0)
            {
                hCRC32::StartCRC32(fmtID, (hChar*)&elements[i], sizeof(D3D11_INPUT_ELEMENT_DESC));
            }
            else
            {
                hCRC32::ContinueCRC32(fmtID, (hChar*)&elements[i], sizeof(D3D11_INPUT_ELEMENT_DESC));
            }

            ++elementsadded;
        }

        if (elementsadded)
        {
            hCRC32::FinishCRC32(fmtID);
        }

        return elementsadded;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hdDX11ParameterConstantBlock* hdDX11RenderDevice::CreateConstantBlocks( const hUint32* sizes, hUint32 count )
    {
        hdDX11ParameterConstantBlock* constBlocks = hNEW_ARRAY(GetGlobalHeap()/*!heap*/, hdDX11ParameterConstantBlock, count);
        for ( hUint32 i = 0; i < count; ++i )
        {
            HRESULT hr;
            D3D11_BUFFER_DESC desc;
            hZeroMem( &desc, sizeof(desc) );
            desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
            desc.Usage = D3D11_USAGE_DEFAULT;
            desc.ByteWidth = sizes[i];
            desc.CPUAccessFlags = 0;
            hr = d3d11Device_->CreateBuffer( &desc, NULL, &constBlocks[i].constBuffer_ );
            hcAssert( SUCCEEDED( hr ) );
            constBlocks[i].size_ = sizes[i];
            constBlocks[i].mapData_ = hHeapMalloc(GetGlobalHeap(), sizes[i]);
        }

        return constBlocks;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdDX11RenderDevice::UpdateConstantBlockParameters( hdDX11ParameterConstantBlock* /*constBlock*/, hShaderParameter* /*params*/, hUint32 /*parameters*/ )
    {
        //Does Nothing
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdDX11RenderDevice::DestroyConstantBlocks( hdDX11ParameterConstantBlock* constBlocks, hUint32 count )
    {
        for ( hUint32 i = 0; i < count; ++i )
        {
            constBlocks[i].constBuffer_->Release();
            hHeapFree(GetGlobalHeap(), constBlocks[i].mapData_);
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

}
