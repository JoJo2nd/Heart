/********************************************************************
	created:	2010/07/02
	created:	2:7:2010   19:58
	filename: 	DeviceD3D9Renderer.cpp	
	author:		James
	
	purpose:	
*********************************************************************/

namespace Heart
{
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	hdD3D9Renderer::hdD3D9Renderer() :
		pCurrentVertexStream_( NULL )
		,pBackBufferSurface_( NULL )
		,pDepthSurface_( NULL )		
		,system_( NULL )
	{
		//Setup our functions
		dwRenderStateSet_[ RS_CULL_MODE ] = 
			SetRenderStateDWord::bind< hdD3D9Renderer, &hdD3D9Renderer::SetCullMode >( this );
		dwRenderStateSet_[ RS_FILL_MODE ] = 
			SetRenderStateDWord::bind< hdD3D9Renderer, &hdD3D9Renderer::SetFillMode >( this );
		dwRenderStateSet_[ RS_SCISSOR_TEST ] = 
			SetRenderStateDWord::bind< hdD3D9Renderer, &hdD3D9Renderer::SetScissorTest >( this );
		dwRenderStateSet_[ RS_Z_TEST ] = 
			SetRenderStateDWord::bind< hdD3D9Renderer, &hdD3D9Renderer::SetZTest >( this );
		dwRenderStateSet_[ RS_Z_WRITE ] = 
			SetRenderStateDWord::bind< hdD3D9Renderer, &hdD3D9Renderer::SetZWrite >( this );
		dwRenderStateSet_[ RS_Z_COMPARE_FUNCTION ] = 
			SetRenderStateDWord::bind< hdD3D9Renderer, &hdD3D9Renderer::SetZCompareFunction >( this );
		dwRenderStateSet_[ RS_STENCIL_TEST ] = 
			SetRenderStateDWord::bind< hdD3D9Renderer, &hdD3D9Renderer::SetStencilTest >( this );
		dwRenderStateSet_[ RS_STENCIL_FAIL ] = 
			SetRenderStateDWord::bind< hdD3D9Renderer, &hdD3D9Renderer::SetStencilFail >( this );
		dwRenderStateSet_[ RS_STENCIL_Z_FAIL ] = 
			SetRenderStateDWord::bind< hdD3D9Renderer, &hdD3D9Renderer::SetStencilZFail >( this );
		dwRenderStateSet_[ RS_STENCIL_PASS ] = 
			SetRenderStateDWord::bind< hdD3D9Renderer, &hdD3D9Renderer::SetStencilPass >( this );
		dwRenderStateSet_[ RS_STENCIL_FUNCTION ] = 
			SetRenderStateDWord::bind< hdD3D9Renderer, &hdD3D9Renderer::SetStencilFunction >( this );
		dwRenderStateSet_[ RS_STENCIL_REF ] = 
			SetRenderStateDWord::bind< hdD3D9Renderer, &hdD3D9Renderer::SetStencilRef >( this );
		dwRenderStateSet_[ RS_STENCIL_WRITE_MASK ] = 
			SetRenderStateDWord::bind< hdD3D9Renderer, &hdD3D9Renderer::SetStencilWriteMask >( this );
		dwRenderStateSet_[ RS_STENCIL_MASK ] = 
			SetRenderStateDWord::bind< hdD3D9Renderer, &hdD3D9Renderer::SetStencilMask >( this );
		dwRenderStateSet_[ RS_ALPHA_BLEND ] = 
			SetRenderStateDWord::bind< hdD3D9Renderer, &hdD3D9Renderer::SetAlphaBlend >( this );
		dwRenderStateSet_[ RS_SRC_ALPHA_BLEND ] = 
			SetRenderStateDWord::bind< hdD3D9Renderer, &hdD3D9Renderer::SetSrcAlphaBlend >( this );
		dwRenderStateSet_[ RS_DST_ALPHA_BLEND ] = 
			SetRenderStateDWord::bind< hdD3D9Renderer, &hdD3D9Renderer::SetDstAlphaBlend >( this );
		dwRenderStateSet_[ RS_ALPHA_BLEND_FUNCTION ] = 
			SetRenderStateDWord::bind< hdD3D9Renderer, &hdD3D9Renderer::SetAlphaBlendFunction >( this );
		dwRenderStateSet_[ RS_COLOUR_WRITE_1 ] = 
			SetRenderStateDWord::bind< hdD3D9Renderer, &hdD3D9Renderer::SetColourWrite1 >( this );
		dwRenderStateSet_[ RS_COLOUR_WRITE_2 ] = 
			SetRenderStateDWord::bind< hdD3D9Renderer, &hdD3D9Renderer::SetColourWrite2 >( this );
		dwRenderStateSet_[ RS_COLOUR_WRITE_3 ] = 
			SetRenderStateDWord::bind< hdD3D9Renderer, &hdD3D9Renderer::SetColourWrite3 >( this );
		dwRenderStateSet_[ RS_COLOUR_WRITE_4 ] = 
			SetRenderStateDWord::bind< hdD3D9Renderer, &hdD3D9Renderer::SetColourWrite4 >( this );

		//Float versions
		fRenderStateSet_[ RS_DEPTH_BIAS ] = 
			SetRenderStateFloat::bind< hdD3D9Renderer, &hdD3D9Renderer::SetDepthBias >( this );

	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	hdD3D9Renderer::~hdD3D9Renderer()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hdD3D9Renderer::Create( Device::Kernel* system, hUint32 width, hUint32 height, hUint32 bpp, hFloat shaderVersion, hBool fullscreen, hBool vsync )
	{
		Direct3D::D3DInitParams params;

		system_ = system;

		params.ExactMatch = hFalse;
		params.AntialiasedQuality_ = 0;
		if ( fullscreen )
		{
			params.Format = D3DFMT_X8R8G8B8;
		}
		else
		{
			params.Format = D3DFMT_UNKNOWN;
		}
		params.pResolution.Height_ = height;
		params.pResolution.Width_ = width;
		params.hWnd = system->GetSystemHandle()->hWnd_;
		params.VSync_ = vsync;
		params.Windowed = !fullscreen;
		params.minShaderVersion_ = shaderVersion;


		direct3D9_.open( params );
		pD3D9Device_ = direct3D9_.pDevice();

		pD3D9Device_->GetRenderTarget( 0, &pBackBufferSurface_ );
		//pD3D9Device_->GetDepthStencilSurface( &pDepthSurface_ );

		pDepthTarget_ = hNEW ( hRendererHeap ) hdD3D9Texture();
		CreateRenderTarget( pDepthTarget_, params.pResolution.Width_, params.pResolution.Height_, TFORMAT_D24S8F );
		pDepthSurface_ = pDepthTarget_->pRenderTargetSurface_;

		pD3D9Device_->SetDepthStencilSurface( pDepthSurface_ );
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hdD3D9Renderer::Destroy()
	{
		if ( pBackBufferSurface_ )
		{
			pBackBufferSurface_->Release();
			pBackBufferSurface_ = NULL;
		}
		delete pDepthTarget_;
		if ( pDepthSurface_ )
		{
			pDepthSurface_->Release();
			pDepthSurface_ = NULL;
		}
		direct3D9_.close();
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hdD3D9Renderer::ClearTarget( hBool clearColour, hColour& colour, hBool clearZ, hFloat z )
	{
		DWORD flags = clearColour ? D3DCLEAR_TARGET : 0;
		flags |= clearZ ? D3DCLEAR_ZBUFFER : 0;
		D3DCOLOR d3dcol = D3DCOLOR_COLORVALUE( colour.r_, colour.g_, colour.b_, colour.a_ );

		pD3D9Device_->Clear( 0, NULL, flags, d3dcol, z, 0 );
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hdD3D9Renderer::BeginRender()
	{
		pD3D9Device_->BeginScene();
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hdD3D9Renderer::EndRender()
	{
		pD3D9Device_->EndScene();
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hdD3D9Renderer::SwapBuffers()
	{
		direct3D9_.flipBuffers();
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hdD3D9Renderer::DrawPrimitive( hUint32 nPrimatives )
	{
		hcAssertMsg( pCurrentVertexStream_, "No Vertex Stream Set" );

		HRESULT hr = pD3D9Device_->DrawIndexedPrimitive( pCurrentIndexStream_->primType_, 0, 0, pCurrentVertexStream_->nVertex_, 0, nPrimatives );
		
		hcAssertMsg( hr == D3D_OK, "DrawIndexedPrimitive failed 0x%08X\n", hr );
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hdD3D9Renderer::DrawVertexStream( PrimitiveType primType )
	{
		hUint32 primCount = 0;
		D3DPRIMITIVETYPE pt;

		if ( primType == PRIMITIVETYPE_TRILIST )
		{
			primCount = pCurrentVertexStream_->VertexCount()/3;
			pt = D3DPT_TRIANGLELIST;
		}
		else if ( primType == PRIMITIVETYPE_TRIFAN )
		{
			primCount = pCurrentVertexStream_->VertexCount()-2;
			pt = D3DPT_TRIANGLEFAN;
		}
		else if ( primType == PRIMITIVETYPE_TRISTRIP )
		{
			primCount = pCurrentVertexStream_->VertexCount()-2;
			pt = D3DPT_TRIANGLESTRIP;
		}
		else if ( primType == PRIMITIVETYPE_LINELIST )	
		{
			primCount = pCurrentVertexStream_->VertexCount()/2;
			pt = D3DPT_LINELIST;
		}

		hcAssert( primCount > 0 );

		HRESULT hr = pD3D9Device_->DrawPrimitive( pt, 0, primCount );

		hcAssertMsg( hr == D3D_OK, "DrawIndexedPrimitive failed 0x%08X\n", hr );
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hdD3D9Renderer::SetIndexStream( hdD3D9IndexBuffer* pIBuf )
	{
		pD3D9Device_->SetIndices( pIBuf->buffer_ );

		pCurrentIndexStream_ = pIBuf;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hdD3D9Renderer::SetVertexStream( hdD3D9VtxBuffer* pVBuf, hdD3D9VtxDecl* pDecl )
	{
		pD3D9Device_->SetStreamSource( 0, pVBuf->pBuffer_, 0, pDecl->stride_ );

		pCurrentVertexStream_ = pVBuf;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hdD3D9Renderer::SetRenderState( RENDER_STATE rs, RENDER_STATE_VALUE rsval )
	{
		if ( dwRenderStateSet_[ rs ].isValid() )
		{
			dwRenderStateSet_[ rs ]( rsval );
		}
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hdD3D9Renderer::SetRenderState( RENDER_STATE rs, hFloat rsval )
	{
		if ( fRenderStateSet_[ rs ].isValid() )
		{
			fRenderStateSet_[ rs ]( rsval );
		}
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hdD3D9Renderer::SetVertexFormat( hdD3D9VtxDecl* pVDecl )
	{
		pD3D9Device_->SetVertexDeclaration( pVDecl->pVtxDecl_ );
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hdD3D9Renderer::SetTexture( hdD3D9Texture* pTex, hUint32 idx )
	{
		hcAssert( pTex->renderTarget_ & hdD3D9Texture::SF_TEXTURE );
		pD3D9Device_->SetTexture( idx, pTex->pD3DTexture_ );
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hdD3D9Renderer::SetRenderTarget( hUint32 idx , hdD3D9Texture* pTar )
	{
		if ( pTar )
		{
			hcAssertMsg( pTar->renderTarget_ & hdD3D9Texture::SF_RENDERTARGET, "Not a Valid Render Target!" );
			pD3D9Device_->SetRenderTarget( idx, pTar->pRenderTargetSurface_ );
		}
		else
		{
			if ( idx == 0 )
			{
				pD3D9Device_->SetRenderTarget( idx, pBackBufferSurface_ );
			}
			else
			{
				pD3D9Device_->SetRenderTarget( idx, NULL );
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hdD3D9Renderer::SetDepthSurface( hdD3D9Texture* pSur )
	{
		HRESULT hr;
		if ( pSur )
		{
			hcAssert( pSur->renderTarget_ & hdD3D9Texture::SF_DEPTH );
			hr = pD3D9Device_->SetDepthStencilSurface( pSur->pRenderTargetSurface_ );
		}
		else
		{
			hr = pD3D9Device_->SetDepthStencilSurface( pDepthSurface_ );
		}

		hcAssertMsg( hr == D3D_OK, "D3D9::SetDepthStencilSurface failed 0x%08X", hr );
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hdD3D9Renderer::SetViewport( const hViewport& viewport )
	{
		D3DVIEWPORT9 vp;
		vp.X = viewport.x_;
		vp.Y = viewport.y_;
		vp.Width = viewport.width_;
		vp.Height = viewport.height_;
		vp.MinZ = 0.0f;
		vp.MaxZ = 1.0f;

		pD3D9Device_->SetViewport( &vp );
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hdD3D9Renderer::SetScissorRect( const ScissorRect& scissor )
	{
		RECT r;
		r.top = scissor.top_;
		r.bottom = scissor.bottom_;
		r.left = scissor.left_;
		r.right = scissor.right_;

		pD3D9Device_->SetScissorRect( &r );
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hdD3D9Renderer::SetCullMode( RENDER_STATE_VALUE val )
	{
		switch( val )
		{
		case RSV_CULL_MODE_NONE:
			pD3D9Device_->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );
			break;
		case RSV_CULL_MODE_CW:
			pD3D9Device_->SetRenderState( D3DRS_CULLMODE, D3DCULL_CW );
			break;
		case RSV_CULL_MODE_CCW:
			pD3D9Device_->SetRenderState( D3DRS_CULLMODE, D3DCULL_CCW );
			break;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hdD3D9Renderer::SetFillMode( RENDER_STATE_VALUE val )
	{
		switch ( val )
		{
		case RSV_FILL_MODE_SOLID:
			pD3D9Device_->SetRenderState( D3DRS_FILLMODE, D3DFILL_SOLID );
			break;
		case RSV_FILL_MODE_WIREFRAME:
			pD3D9Device_->SetRenderState( D3DRS_FILLMODE, D3DFILL_WIREFRAME );
			break;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hdD3D9Renderer::SetDepthBias( hFloat val )
	{
		pD3D9Device_->SetRenderState( D3DRS_DEPTHBIAS, (DWORD) val );
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hdD3D9Renderer::SetScissorTest( RENDER_STATE_VALUE val )
	{
		switch ( val )
		{
		case RSV_ENABLE:
			pD3D9Device_->SetRenderState( D3DRS_SCISSORTESTENABLE, TRUE );
			break;
		case RSV_DISABLE:
			pD3D9Device_->SetRenderState( D3DRS_SCISSORTESTENABLE, FALSE );
			break;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hdD3D9Renderer::SetZTest( RENDER_STATE_VALUE val )
	{
		switch( val )
		{
		case RSV_ENABLE:
			pD3D9Device_->SetRenderState( D3DRS_ZENABLE, TRUE );
			break;
		case RSV_DISABLE:
			pD3D9Device_->SetRenderState( D3DRS_ZENABLE, FALSE );
			break;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hdD3D9Renderer::SetZWrite( RENDER_STATE_VALUE val )
	{
		switch( val )
		{
		case RSV_ENABLE:
			pD3D9Device_->SetRenderState( D3DRS_ZWRITEENABLE, TRUE );
			break;
		case RSV_DISABLE:
			pD3D9Device_->SetRenderState( D3DRS_ZWRITEENABLE, FALSE );
			break;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hdD3D9Renderer::SetZCompareFunction( RENDER_STATE_VALUE val )
	{
		switch(val)
		{
		case RSV_Z_CMP_NEVER:
			pD3D9Device_->SetRenderState( D3DRS_ZFUNC, D3DCMP_NEVER );
			break;
		case RSV_Z_CMP_LESS:
			pD3D9Device_->SetRenderState( D3DRS_ZFUNC, D3DCMP_LESS );
			break;
		case RSV_Z_CMP_EQUAL:
			pD3D9Device_->SetRenderState( D3DRS_ZFUNC, D3DCMP_EQUAL );
			break;
		case RSV_Z_CMP_LESSEQUAL:
			pD3D9Device_->SetRenderState( D3DRS_ZFUNC, D3DCMP_LESSEQUAL );
			break;
		case RSV_Z_CMP_GREATER:
			pD3D9Device_->SetRenderState( D3DRS_ZFUNC, D3DCMP_GREATER );
			break;
		case RSV_Z_CMP_NOT_EQUAL:
			pD3D9Device_->SetRenderState( D3DRS_ZFUNC, D3DCMP_NOTEQUAL );
			break;
		case RSV_Z_CMP_GREATER_EQUAL:
			pD3D9Device_->SetRenderState( D3DRS_ZFUNC, D3DCMP_GREATEREQUAL );
			break;
		case RSV_Z_CMP_ALWAYS:
			pD3D9Device_->SetRenderState( D3DRS_ZFUNC, D3DCMP_ALWAYS );
			break;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hdD3D9Renderer::SetStencilTest( RENDER_STATE_VALUE val )
	{
		switch( val )
		{
		case RSV_ENABLE:
			pD3D9Device_->SetRenderState( D3DRS_STENCILENABLE, TRUE );
			//pD3D9Device_->SetRenderState( D3DRS_TWOSIDEDSTENCILMODE, TRUE );
			break;
		case RSV_DISABLE:
			pD3D9Device_->SetRenderState( D3DRS_STENCILENABLE, FALSE );
			//pD3D9Device_->SetRenderState( D3DRS_TWOSIDEDSTENCILMODE, FALSE );
			break;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hdD3D9Renderer::SetStencilFail( RENDER_STATE_VALUE val )
	{
		switch( val )
		{
		case RSV_SO_KEEP:
			pD3D9Device_->SetRenderState( D3DRS_STENCILFAIL, D3DSTENCILOP_KEEP );
			//pD3D9Device_->SetRenderState( D3DRS_CCW_STENCILFAIL, D3DSTENCILOP_KEEP );
			break;
		case RSV_SO_ZERO:
			pD3D9Device_->SetRenderState( D3DRS_STENCILFAIL, D3DSTENCILOP_ZERO );
			//pD3D9Device_->SetRenderState( D3DRS_CCW_STENCILFAIL, D3DSTENCILOP_ZERO );
			break;
		case RSV_SO_REPLACE:
			pD3D9Device_->SetRenderState( D3DRS_STENCILFAIL, D3DSTENCILOP_REPLACE );
			//pD3D9Device_->SetRenderState( D3DRS_CCW_STENCILFAIL, D3DSTENCILOP_REPLACE );
			break;
		case RSV_SO_INCRSAT:
			pD3D9Device_->SetRenderState( D3DRS_STENCILFAIL, D3DSTENCILOP_INCRSAT );
			//pD3D9Device_->SetRenderState( D3DRS_CCW_STENCILFAIL, D3DSTENCILOP_INCRSAT );
			break;
		case RSV_SO_DECRSAT:
			pD3D9Device_->SetRenderState( D3DRS_STENCILFAIL, D3DSTENCILOP_DECRSAT );
			//pD3D9Device_->SetRenderState( D3DRS_CCW_STENCILFAIL, D3DSTENCILOP_DECRSAT );
			break;
		case RSV_SO_INVERT:
			pD3D9Device_->SetRenderState( D3DRS_STENCILFAIL, D3DSTENCILOP_INVERT );
			//pD3D9Device_->SetRenderState( D3DRS_CCW_STENCILFAIL, D3DSTENCILOP_INVERT );
			break;
		case RSV_SO_INCR:
			pD3D9Device_->SetRenderState( D3DRS_STENCILFAIL, D3DSTENCILOP_INCR );
			//pD3D9Device_->SetRenderState( D3DRS_CCW_STENCILFAIL, D3DSTENCILOP_INCR );
			break;
		case RSV_SO_DECR:
			pD3D9Device_->SetRenderState( D3DRS_STENCILFAIL, D3DSTENCILOP_DECR );
			//pD3D9Device_->SetRenderState( D3DRS_CCW_STENCILFAIL, D3DSTENCILOP_DECR );
			break;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hdD3D9Renderer::SetStencilZFail( RENDER_STATE_VALUE val )
	{
		switch( val )
		{
		case RSV_SO_KEEP:
			pD3D9Device_->SetRenderState( D3DRS_STENCILZFAIL, D3DSTENCILOP_KEEP );
			//pD3D9Device_->SetRenderState( D3DRS_CCW_STENCILZFAIL, D3DSTENCILOP_KEEP );
			break;
		case RSV_SO_ZERO:
			pD3D9Device_->SetRenderState( D3DRS_STENCILZFAIL, D3DSTENCILOP_ZERO );
			//pD3D9Device_->SetRenderState( D3DRS_CCW_STENCILZFAIL, D3DSTENCILOP_ZERO );
			break;
		case RSV_SO_REPLACE:
			pD3D9Device_->SetRenderState( D3DRS_STENCILZFAIL, D3DSTENCILOP_REPLACE );
			//pD3D9Device_->SetRenderState( D3DRS_CCW_STENCILZFAIL, D3DSTENCILOP_REPLACE );
			break;
		case RSV_SO_INCRSAT:
			pD3D9Device_->SetRenderState( D3DRS_STENCILZFAIL, D3DSTENCILOP_INCRSAT );
			//pD3D9Device_->SetRenderState( D3DRS_CCW_STENCILZFAIL, D3DSTENCILOP_INCRSAT );
			break;
		case RSV_SO_DECRSAT:
			pD3D9Device_->SetRenderState( D3DRS_STENCILZFAIL, D3DSTENCILOP_DECRSAT );
			//pD3D9Device_->SetRenderState( D3DRS_CCW_STENCILZFAIL, D3DSTENCILOP_DECRSAT );
			break;
		case RSV_SO_INVERT:
			pD3D9Device_->SetRenderState( D3DRS_STENCILZFAIL, D3DSTENCILOP_INVERT );
			//pD3D9Device_->SetRenderState( D3DRS_CCW_STENCILZFAIL, D3DSTENCILOP_INVERT );
			break;
		case RSV_SO_INCR:
			pD3D9Device_->SetRenderState( D3DRS_STENCILZFAIL, D3DSTENCILOP_INCR );
			//pD3D9Device_->SetRenderState( D3DRS_CCW_STENCILZFAIL, D3DSTENCILOP_INCR );
			break;
		case RSV_SO_DECR:
			pD3D9Device_->SetRenderState( D3DRS_STENCILZFAIL, D3DSTENCILOP_DECR );
			//pD3D9Device_->SetRenderState( D3DRS_CCW_STENCILZFAIL, D3DSTENCILOP_DECR );
			break;
		}		
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hdD3D9Renderer::SetStencilPass( RENDER_STATE_VALUE val )
	{
		switch( val )
		{
		case RSV_SO_KEEP:
			pD3D9Device_->SetRenderState( D3DRS_STENCILPASS, D3DSTENCILOP_KEEP );
			//pD3D9Device_->SetRenderState( D3DRS_CCW_STENCILPASS, D3DSTENCILOP_KEEP );
			break;
		case RSV_SO_ZERO:
			pD3D9Device_->SetRenderState( D3DRS_STENCILPASS, D3DSTENCILOP_ZERO );
			//pD3D9Device_->SetRenderState( D3DRS_CCW_STENCILPASS, D3DSTENCILOP_ZERO );
			break;
		case RSV_SO_REPLACE:
			pD3D9Device_->SetRenderState( D3DRS_STENCILPASS, D3DSTENCILOP_REPLACE );
			//pD3D9Device_->SetRenderState( D3DRS_CCW_STENCILPASS, D3DSTENCILOP_REPLACE );
			break;
		case RSV_SO_INCRSAT:
			pD3D9Device_->SetRenderState( D3DRS_STENCILPASS, D3DSTENCILOP_INCRSAT );
			//pD3D9Device_->SetRenderState( D3DRS_CCW_STENCILPASS, D3DSTENCILOP_INCRSAT );
			break;
		case RSV_SO_DECRSAT:
			pD3D9Device_->SetRenderState( D3DRS_STENCILPASS, D3DSTENCILOP_DECRSAT );
			//pD3D9Device_->SetRenderState( D3DRS_CCW_STENCILPASS, D3DSTENCILOP_DECRSAT );
			break;
		case RSV_SO_INVERT:
			pD3D9Device_->SetRenderState( D3DRS_STENCILPASS, D3DSTENCILOP_INVERT );
			//pD3D9Device_->SetRenderState( D3DRS_CCW_STENCILPASS, D3DSTENCILOP_INVERT );
			break;
		case RSV_SO_INCR:
			pD3D9Device_->SetRenderState( D3DRS_STENCILPASS, D3DSTENCILOP_INCR );
			//pD3D9Device_->SetRenderState( D3DRS_CCW_STENCILPASS, D3DSTENCILOP_INCR );
			break;
		case RSV_SO_DECR:
			pD3D9Device_->SetRenderState( D3DRS_STENCILPASS, D3DSTENCILOP_DECR );
			//pD3D9Device_->SetRenderState( D3DRS_CCW_STENCILPASS, D3DSTENCILOP_DECR );
			break;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hdD3D9Renderer::SetStencilFunction( RENDER_STATE_VALUE val )
	{
		switch ( val )
		{
		case RSV_SF_CMP_NEVER:
			pD3D9Device_->SetRenderState( D3DRS_STENCILFUNC, D3DCMP_NEVER );
			//pD3D9Device_->SetRenderState( D3DRS_CCW_STENCILFUNC, D3DCMP_NEVER );
			break;
		case RSV_SF_CMP_LESS:
			pD3D9Device_->SetRenderState( D3DRS_STENCILFUNC, D3DCMP_LESS );
			//pD3D9Device_->SetRenderState( D3DRS_CCW_STENCILFUNC, D3DCMP_LESS );
			break;
		case RSV_SF_CMP_EQUAL:
			pD3D9Device_->SetRenderState( D3DRS_STENCILFUNC, D3DCMP_EQUAL );
			//pD3D9Device_->SetRenderState( D3DRS_CCW_STENCILFUNC, D3DCMP_EQUAL );
			break;
		case RSV_SF_CMP_LESSEQUAL:
			pD3D9Device_->SetRenderState( D3DRS_STENCILFUNC, D3DCMP_LESSEQUAL );
			//pD3D9Device_->SetRenderState( D3DRS_CCW_STENCILFUNC, D3DCMP_LESSEQUAL );
			break;
		case RSV_SF_CMP_GREATER:
			pD3D9Device_->SetRenderState( D3DRS_STENCILFUNC, D3DCMP_GREATER );
			//pD3D9Device_->SetRenderState( D3DRS_CCW_STENCILFUNC, D3DCMP_GREATER );
			break;
		case RSV_SF_CMP_NOT_EQUAL:
			pD3D9Device_->SetRenderState( D3DRS_STENCILFUNC, D3DCMP_NOTEQUAL );
			//pD3D9Device_->SetRenderState( D3DRS_CCW_STENCILFUNC, D3DCMP_NOTEQUAL );
			break;
		case RSV_SF_CMP_GREATER_EQUAL:
			pD3D9Device_->SetRenderState( D3DRS_STENCILFUNC, D3DCMP_GREATER );
			//pD3D9Device_->SetRenderState( D3DRS_CCW_STENCILFUNC, D3DCMP_GREATER );
			break;
		case RSV_SF_CMP_ALWAYS:
			pD3D9Device_->SetRenderState( D3DRS_STENCILFUNC, D3DCMP_ALWAYS );
			//pD3D9Device_->SetRenderState( D3DRS_CCW_STENCILFUNC, D3DCMP_ALWAYS );
			break;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hdD3D9Renderer::SetStencilRef( RENDER_STATE_VALUE val )
	{
		pD3D9Device_->SetRenderState( D3DRS_STENCILREF, (DWORD) val );
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hdD3D9Renderer::SetStencilWriteMask( RENDER_STATE_VALUE val )
	{
		pD3D9Device_->SetRenderState( D3DRS_STENCILWRITEMASK, (DWORD)val );
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hdD3D9Renderer::SetStencilMask( RENDER_STATE_VALUE val )
	{
		pD3D9Device_->SetRenderState( D3DRS_STENCILMASK, (DWORD)val );
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hdD3D9Renderer::SetAlphaBlend( RENDER_STATE_VALUE val )
	{
		switch( val )
		{
		case RSV_ENABLE:
			pD3D9Device_->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
			break;
		case RSV_DISABLE:
			pD3D9Device_->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
			break;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hdD3D9Renderer::SetSrcAlphaBlend( RENDER_STATE_VALUE val )
	{
		switch( val )
		{
		case RSV_BLEND_OP_ZERO:
			pD3D9Device_->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_ZERO );
			break;
		case RSV_BLEND_OP_ONE:
			pD3D9Device_->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_ONE );
			break;
		case RSV_BLEND_OP_SRC_COLOUR:
			pD3D9Device_->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCCOLOR );
			break;
		case RSV_BLEND_OP_INVSRC_COLOUR:
			pD3D9Device_->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_INVSRCALPHA );
			break;
		case RSV_BLEND_OP_DEST_COLOUR:
			pD3D9Device_->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_DESTCOLOR );
			break;
		case RSV_BLEND_OP_INVDEST_COLOUR:
			pD3D9Device_->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_INVDESTCOLOR );
			break;
		case RSV_BLEND_OP_SRC_ALPHA:
			pD3D9Device_->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
			break;
		case RSV_BLEND_OP_INVSRC_ALPHA:
			pD3D9Device_->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_INVSRCALPHA );
			break;
		case RSV_BLEND_OP_DEST_ALPHA:
			pD3D9Device_->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_DESTALPHA );
			break;
		case RSV_BLEND_OP_INVDEST_ALPHA:
			pD3D9Device_->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_INVDESTALPHA );
			break;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hdD3D9Renderer::SetDstAlphaBlend( RENDER_STATE_VALUE val )
	{
		switch( val )
		{
		case RSV_BLEND_OP_ZERO:
			pD3D9Device_->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ZERO );
			break;
		case RSV_BLEND_OP_ONE:
			pD3D9Device_->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ONE );
			break;
		case RSV_BLEND_OP_SRC_COLOUR:
			pD3D9Device_->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_SRCCOLOR );
			break;
		case RSV_BLEND_OP_INVSRC_COLOUR:
			pD3D9Device_->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
			break;
		case RSV_BLEND_OP_DEST_COLOUR:
			pD3D9Device_->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_DESTCOLOR );
			break;
		case RSV_BLEND_OP_INVDEST_COLOUR:
			pD3D9Device_->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVDESTCOLOR );
			break;
		case RSV_BLEND_OP_SRC_ALPHA:
			pD3D9Device_->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_SRCALPHA );
			break;
		case RSV_BLEND_OP_INVSRC_ALPHA:
			pD3D9Device_->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
			break;
		case RSV_BLEND_OP_DEST_ALPHA:
			pD3D9Device_->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_DESTALPHA );
			break;
		case RSV_BLEND_OP_INVDEST_ALPHA:
			pD3D9Device_->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVDESTALPHA );
			break;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hdD3D9Renderer::SetAlphaBlendFunction( RENDER_STATE_VALUE val )
	{
		switch( val )
		{
		case RSV_BLEND_FUNC_ADD:
			pD3D9Device_->SetRenderState( D3DRS_BLENDOP, D3DBLENDOP_ADD );
			break;
		case RSV_BLEND_FUNC_SUB:
			pD3D9Device_->SetRenderState( D3DRS_BLENDOP, D3DBLENDOP_SUBTRACT );
			break;
		case RSV_BLEND_FUNC_MIN:
			pD3D9Device_->SetRenderState( D3DRS_BLENDOP, D3DBLENDOP_MIN );
			break;
		case RSV_BLEND_FUNC_MAX:
			pD3D9Device_->SetRenderState( D3DRS_BLENDOP, D3DBLENDOP_MAX );
			break;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hdD3D9Renderer::SetColourWrite1( RENDER_STATE_VALUE val )
	{
		//These match nicely, just pass through.
		// D3DCOLORWRITEENABLE_ALPHA
		pD3D9Device_->SetRenderState( D3DRS_COLORWRITEENABLE, val );
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hdD3D9Renderer::SetColourWrite2( RENDER_STATE_VALUE val )
	{
		//These match nicely, just pass through.
		pD3D9Device_->SetRenderState( D3DRS_COLORWRITEENABLE1, val );
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hdD3D9Renderer::SetColourWrite3( RENDER_STATE_VALUE val )
	{
		//These match nicely, just pass through.
		pD3D9Device_->SetRenderState( D3DRS_COLORWRITEENABLE2, val );
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hdD3D9Renderer::SetColourWrite4( RENDER_STATE_VALUE val )
	{
		//These match nicely, just pass through.
		pD3D9Device_->SetRenderState( D3DRS_COLORWRITEENABLE3, val );
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////

	void hdD3D9Renderer::BuildVertexDecl( D3DVERTEXELEMENT9* elements, hUint32 vtxFlags, hUint32* stride, hUint32* pOffsets )
	{
		D3DVERTEXELEMENT9 endmarker[] = { D3DDECL_END() };
		hUint32 elementsadded = 0;
		WORD offset = 0;
		const hUint32 flags = vtxFlags;
		// declaration doesn't exist so create it [11/23/2008 James]

		//default the offsets table
		for ( hUint32 i = 0; i < hrVE_MAX; ++i )
		{
			pOffsets[ i ] = hErrorCode;
		}

		if ( flags & hrVF_XYZ )
		{
			elements[ elementsadded ].Stream = 0;
			elements[ elementsadded ].Offset = offset;
			elements[ elementsadded ].Type = D3DDECLTYPE_FLOAT3;
			elements[ elementsadded ].Method = D3DDECLMETHOD_DEFAULT;
			elements[ elementsadded ].Usage = D3DDECLUSAGE_POSITION;
			elements[ elementsadded ].UsageIndex = 0;
			pOffsets[ hrVE_XYZ ] = offset;

			++elementsadded;
			offset += sizeof( hFloat ) * 3;
		}
		if ( flags & hrVF_XYZW )
		{
			elements[ elementsadded ].Stream = 0;
			elements[ elementsadded ].Offset = offset;
			elements[ elementsadded ].Type = D3DDECLTYPE_FLOAT4;
			elements[ elementsadded ].Method = D3DDECLMETHOD_DEFAULT;
			elements[ elementsadded ].Usage = D3DDECLUSAGE_POSITION;
			elements[ elementsadded ].UsageIndex = 0;
			pOffsets[ hrVE_XYZW ] = offset;

			++elementsadded;
			offset += sizeof( hFloat ) * 4;
		}
		if ( flags & hrVF_NORMAL )
		{
			elements[ elementsadded ].Stream = 0;
			elements[ elementsadded ].Offset = offset;
			elements[ elementsadded ].Type = D3DDECLTYPE_FLOAT3;
			elements[ elementsadded ].Method = D3DDECLMETHOD_DEFAULT;
			elements[ elementsadded ].Usage = D3DDECLUSAGE_NORMAL;
			elements[ elementsadded ].UsageIndex = 0;
			pOffsets[ hrVE_NORMAL ] = offset;

			++elementsadded;
			offset += sizeof( hFloat ) * 3;
		}
		if ( flags & hrVF_TANGENT )
		{
			elements[ elementsadded ].Stream = 0;
			elements[ elementsadded ].Offset = offset;
			elements[ elementsadded ].Type = D3DDECLTYPE_FLOAT3;
			elements[ elementsadded ].Method = D3DDECLMETHOD_DEFAULT;
			elements[ elementsadded ].Usage = D3DDECLUSAGE_TANGENT;
			elements[ elementsadded ].UsageIndex = 0;
			pOffsets[ hrVE_TANGENT ] = offset;

			++elementsadded;
			offset += sizeof( hFloat ) * 3;
		}
		if ( flags & hrVF_BINORMAL )
		{
			elements[ elementsadded ].Stream = 0;
			elements[ elementsadded ].Offset = offset;
			elements[ elementsadded ].Type = D3DDECLTYPE_FLOAT3;
			elements[ elementsadded ].Method = D3DDECLMETHOD_DEFAULT;
			elements[ elementsadded ].Usage = D3DDECLUSAGE_BINORMAL;
			elements[ elementsadded ].UsageIndex = 0;
			pOffsets[ hrVE_BINORMAL ] = offset;

			++elementsadded;
			offset += sizeof( hFloat ) * 3;
		}
		if ( flags & hrVF_COLOR )
		{
			elements[ elementsadded ].Stream = 0;
			elements[ elementsadded ].Offset = offset;
			elements[ elementsadded ].Type = D3DDECLTYPE_D3DCOLOR;
			elements[ elementsadded ].Method = D3DDECLMETHOD_DEFAULT;
			elements[ elementsadded ].Usage = D3DDECLUSAGE_COLOR;
			elements[ elementsadded ].UsageIndex = 0;
			pOffsets[ hrVE_COLOR ] = offset;

			++elementsadded;
			offset += sizeof( hByte ) * 4;
		}

		hChar usageoffset = -1;
		switch( flags & hrVF_UVMASK )
		{
		case hrVF_8UV: usageoffset = 8; break;
		case hrVF_7UV: usageoffset = 7; break;
		case hrVF_6UV: usageoffset = 6; break;
		case hrVF_5UV: usageoffset = 5; break;
		case hrVF_4UV: usageoffset = 4; break;
		case hrVF_3UV: usageoffset = 3; break;
		case hrVF_2UV: usageoffset = 2; break;
		case hrVF_1UV: usageoffset = 1; break;
		}

		for ( hChar i = 0; i < usageoffset; ++i )
		{
			elements[ elementsadded ].Stream = 0;
			elements[ elementsadded ].Offset = offset;
			elements[ elementsadded ].Type = D3DDECLTYPE_FLOAT2;
			elements[ elementsadded ].Method = D3DDECLMETHOD_DEFAULT;
			elements[ elementsadded ].Usage = D3DDECLUSAGE_TEXCOORD;
			elements[ elementsadded ].UsageIndex = i;
			pOffsets[ hrVE_1UV + i ] = offset;

			++elementsadded;
			offset += sizeof( hFloat ) * 2;
		}

		switch( flags & hrVF_BLENDMASK )
		{
		case hrVF_BLEND4: usageoffset = 4; break;
		case hrVF_BLEND3: usageoffset = 3; break;
		case hrVF_BLEND2: usageoffset = 2; break;
		case hrVF_BLEND1: usageoffset = 1; break;
		}

		if ( flags & hrVF_BLENDMASK )
		{
			elements[ elementsadded ].Stream = 0;
			elements[ elementsadded ].Offset = offset;
			elements[ elementsadded ].Type = D3DDECLTYPE_UBYTE4;
			elements[ elementsadded ].Method = D3DDECLMETHOD_DEFAULT;
			elements[ elementsadded ].Usage = D3DDECLUSAGE_BLENDINDICES;
			elements[ elementsadded ].UsageIndex = 0;

			++elementsadded;
			offset += sizeof( hByte ) * 4;

			for ( BYTE i = 0; i < usageoffset; ++i )
			{
				elements[ elementsadded ].Stream = 0;
				elements[ elementsadded ].Offset = offset;
				elements[ elementsadded ].Type = D3DDECLTYPE_FLOAT1;
				elements[ elementsadded ].Method = D3DDECLMETHOD_DEFAULT;
				elements[ elementsadded ].Usage = D3DDECLUSAGE_BLENDWEIGHT;
				elements[ elementsadded ].UsageIndex = i;
				pOffsets[ hrVE_BLEND1 + i ] = offset;

				++elementsadded;
				offset += sizeof( hFloat );
			}
		}

		elements[ elementsadded ] = endmarker[ 0 ];

		//set the stride
		*stride = offset;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////

	void hdD3D9Renderer::GetVertexStrideOffset( hUint32 vtxFlags, hUint32& stride, hUint32* pOffsets )
	{
		D3DVERTEXELEMENT9 elements[ hrVF_MAX ];

		BuildVertexDecl( elements, vtxFlags, &stride, pOffsets );
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hdD3D9Renderer::CreateVertexDeclaration( hdD3D9VtxDecl* pOut, hUint32 vtxFlags, hUint32& stride, hUint32* pOffsets )
	{
		D3DVERTEXELEMENT9 elements[ hrVF_MAX ];

		BuildVertexDecl( elements, vtxFlags, &stride, pOffsets );

		//set the output data
		pOut->vertexFlags_ = vtxFlags;
		pOut->stride_ = stride;
 
 		HRESULT hr = pD3D9Device_->CreateVertexDeclaration( elements, &pOut->pVtxDecl_ );
 		hcAssert( hr == D3D_OK );
	}


	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hdD3D9Renderer::DestroyVertexDeclaration( hdD3D9VtxDecl* pVD )
	{
		if ( pVD->Bound() )
		{
			pVD->pVtxDecl_->Release();
			pVD->pVtxDecl_ = NULL;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	void hdD3D9Renderer::CompileShader( hdD3D9Shader* out, const hChar* shaderProg, hUint32 len, ShaderType type, const hChar* entry )
	{
		LPCTSTR profile;
		LPD3DXBUFFER obj, err;
		LPD3DXCONSTANTTABLE constTable;
		HRESULT hr;

		if ( type == ShaderType_VERTEXPROG )
		{
			profile = D3DXGetVertexShaderProfile( pD3D9Device_ );
		}
		else
		{
			profile = D3DXGetPixelShaderProfile( pD3D9Device_ );
		}

		hr = D3DXCompileShader( shaderProg, len, NULL, NULL, entry, profile, 0, &obj, &err, &constTable );

		if ( FAILED( hr ) )
		{
			hcPrintf( "Shader Compile Error: ", err->GetBufferPointer() );
			err->Release();
			return;
		}

		if ( type == ShaderType_VERTEXPROG )
		{
			hr = pD3D9Device_->CreateVertexShader( (DWORD*)obj->GetBufferPointer(), &out->vShader_ );
		}
		else
		{
			hr = pD3D9Device_->CreatePixelShader( (DWORD*)obj->GetBufferPointer(), &out->fShader_ );
		}

		out->constTable_ = constTable;
		out->type_ = type;
	
		obj->Release();
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

// 	void hdD3D9Renderer::ReleaseCompiledShader( CompiledShaderResource* shader )
// 	{
// 		if ( shader->pRuntimeCompileData_ )
// 		{
// 			shader->pRuntimeCompileData_->Release();
// 		}
// 	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

// 	void hdD3D9Renderer::CreateMaterial( hdD3D9Material* pOut, CompiledShaderResource* shader, hColour diffuse, hColour ambient, hColour emissive, hColour specular, hFloat power )
// 	{
// 		LPD3DXBUFFER pErrors;
// 		HRESULT hr;
// 		void* pShaderData = NULL;
// 		hUint32 dataLen = 0;
// 
// 		pOut->effectData_ = *shader;
// 
// 		if ( shader->pRuntimeCompileData_ )
// 		{
// 			//use runtime data
// 			pShaderData = shader->pRuntimeCompileData_->GetBufferPointer();
// 			dataLen = shader->pRuntimeCompileData_->GetBufferSize();
// 		}
// 		else
// 		{
// 			pShaderData = shader->pOfflineCompileData_;
// 			dataLen = shader->offlineSize_;
// 		}
// 
// 
// 		hr = D3DXCreateEffect(	
// 							pD3D9Device_, 
// 							pShaderData,
// 							dataLen,
// 							NULL,
// 							NULL,
// 							D3DXSHADER_DEBUG | D3DXSHADER_SKIPOPTIMIZATION,
// 							NULL,
// 							&pOut->pEffect_,
// 							&pErrors );
// 		hcAssertMsg( hr == D3D_OK, "D3DXCreateEffect() failed ( 0x%08X ) Error( %s )", hr, pErrors->GetBufferPointer() );
// 
// 		if ( pErrors )
// 		{
// 			pErrors->Release();
// 		}
// 
// 		// ambient
// 		pOut->materialParams_.Ambient.r = ambient.r_;
// 		pOut->materialParams_.Ambient.g = ambient.g_;
// 		pOut->materialParams_.Ambient.b = ambient.b_;
// 		pOut->materialParams_.Ambient.a = ambient.a_;
// 		// diffuse
// 		pOut->materialParams_.Diffuse.r = diffuse.r_;
// 		pOut->materialParams_.Diffuse.g = diffuse.g_;
// 		pOut->materialParams_.Diffuse.b = diffuse.b_;
// 		pOut->materialParams_.Diffuse.a = diffuse.a_;
// 		// emissive
// 		pOut->materialParams_.Emissive.r = emissive.r_;
// 		pOut->materialParams_.Emissive.g = emissive.g_;
// 		pOut->materialParams_.Emissive.b = emissive.b_;
// 		pOut->materialParams_.Emissive.a = emissive.a_;
// 		// specular
// 		pOut->materialParams_.Specular.r = specular.r_;
// 		pOut->materialParams_.Specular.g = specular.g_;
// 		pOut->materialParams_.Specular.b = specular.b_;
// 		pOut->materialParams_.Specular.a = specular.a_;
// 		//
// 		pOut->materialParams_.Power = power;
// 	
// 		if ( shader->pRuntimeCompileData_ )	
// 		{
// 			//shader was built @ runtime need to generate its parameter data
// 			pOut->pParameters_ = NULL;
// 			hUint32 paramCount = 0;
// 			while ( pOut->pEffect_->GetParameter( NULL, paramCount ) != NULL ) 
// 			{
// 				++paramCount;
// 			}
// 			
// 			if ( paramCount > 0 )
// 			{
// 				pOut->pParameters_ = NEW ( hRendererHeap ) hdMaterialParam[ paramCount ];
// 			}
// 
// 			pOut->InitialiseEffectParameters( paramCount );
// 		}
// 	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hdD3D9Renderer::OnMaterialLoad( hdD3D9Material* pOut, void* pStart )
	{
// 		hByte* ptr = (hByte*)pStart;
// 		LPD3DXBUFFER pErrors;
// 		HRESULT hr;
// 
// 		//pointer fix up
// 		pOut->effectData_.pOfflineCompileData_ = (void*)(ptr + (hUint32)pOut->effectData_.pOfflineCompileData_);
// 		pOut->pParameters_ = (hdMaterialParam*)(ptr + (hUint32)pOut->pParameters_);
// 
// 		//create extra data
// 		hcAssert( pOut->effectData_.pOfflineCompileData_ );
// 
// 		hr = D3DXCreateEffect(	
// 			pD3D9Device_, 
// 			pOut->effectData_.pOfflineCompileData_,
// 			pOut->effectData_.offlineSize_,
// 			NULL,
// 			NULL,
// 			D3DXSHADER_DEBUG,
// 			NULL,
// 			&pOut->pEffect_,
// 			&pErrors );
// 		hcAssertMsg( hr == D3D_OK, "D3DXCreateEffect() failed ( 0x%08X ) Error( %s )", hr, pErrors ? pErrors->GetBufferPointer() : "No Error Info" );
// 
// 		if ( pErrors )
// 		{
// 			pErrors->Release();
// 		}
// 
// 		pOut->InitialiseEffectParameters( pOut->nParameters_ );

	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hdD3D9Renderer::DestoryMaterial( hdD3D9Material* pMat )
	{
		//pMat->pEffect_->Release();
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hdD3D9Renderer::CreateTexture( hdD3D9Texture* pOut, hUint32 width, hUint32 height, hUint32 levels, TextureFormat format )
	{
		D3DFORMAT d3dFormat = GetCompatibleFormat( format );


		HRESULT hr = pD3D9Device_->CreateTexture(
						width,
						height,
						levels,
						0/*Usage*/,
						d3dFormat,
						D3DPOOL_MANAGED /*pool*/,
						&pOut->pD3DTexture_,
						NULL );

		hcAssertMsg( hr == D3D_OK, "D3D9::CreateTexture failed 0x%08X", hr );

		//for each level lock the texture and copy in
#if 0
		for ( hUint32 i = 0; i < levels; ++i )
		{
			D3DLOCKED_RECT region;
			hr = pOut->pD3DTexture_->LockRect( i, &region, NULL, 0 );
			hcAssertMsg( hr == D3D_OK, "D3D::LockRect failed 0x%08X", hr );

			hByte* pSrc = pLevels[ i ];
			hByte* pDst = (hByte*)region.pBits;
			hUint32 bpp = GetFormatBPP( d3dFormat );

			for ( hUint32 i2 = 0; i2 < height; ++i2 )
			{
				memcpy( pDst, pSrc, width * bpp );
				pSrc += width * bpp;
				pDst += region.Pitch;
			}

			pOut->pD3DTexture_->UnlockRect( i );
			width >>= 1;
			height >>= 1;
		}
#endif

		pOut->renderTarget_ = hdD3D9Texture::SF_TEXTURE;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hdD3D9Renderer::DestoryTexture( hdD3D9Texture* pBuf )
	{
		pBuf->pD3DTexture_->Release();
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hdD3D9Renderer::CreateRenderTarget( hdD3D9Texture* pOut, hUint32 width, hUint32 height, TextureFormat format )
	{
		D3DFORMAT d3dFormat = GetCompatibleFormat( format );
		DWORD usage;

		switch( d3dFormat )
		{
		case D3DFMT_D32:
		case D3DFMT_D24S8:
			usage = D3DUSAGE_DEPTHSTENCIL;
			pOut->renderTarget_ = hdD3D9Texture::SF_DEPTH;
			break;
		default:
			usage = D3DUSAGE_RENDERTARGET;
			pOut->renderTarget_ = hdD3D9Texture::SF_RENDERTARGET;
			break;
		}

		if ( usage == D3DUSAGE_RENDERTARGET )
		{
			HRESULT hr = pD3D9Device_->CreateTexture( 
				width,
				height,
				1,
				usage/*Usage*/,
				d3dFormat,
				D3DPOOL_DEFAULT /*pool*/,
				&pOut->pD3DTexture_,
				NULL );

			hcAssertMsg( hr == D3D_OK, "D3D9::CreateRenderTarget failed 0x%08X", hr );

			hr = pOut->pD3DTexture_->GetSurfaceLevel( 0, &pOut->pRenderTargetSurface_ );

			hcAssertMsg( hr == D3D_OK, "D3D9::GetSurfaceLevel failed 0x%08X", hr );

			pOut->renderTarget_ |= hdD3D9Texture::SF_TEXTURE;
		}
		else
		{
			HRESULT hr;

			hr = pD3D9Device_->CreateDepthStencilSurface(
				width,
				height, 
				d3dFormat,
				D3DMULTISAMPLE_NONE,
				0,
				FALSE, 
				&pOut->pRenderTargetSurface_, 
				NULL );

			hcAssertMsg( hr == D3D_OK, "D3D9::CreateRenderTarget failed 0x%08X", hr );
		}
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hdD3D9Renderer::DestroyRenderTarget( hdD3D9Texture* pBuf )
	{
		if ( pBuf->renderTarget_ & hdD3D9Texture::SF_TEXTURE )
		{
			pBuf->pD3DTexture_->Release();
		}
		pBuf->pRenderTargetSurface_->Release();
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hdD3D9Renderer::CreateIndexBuffer( hdD3D9IndexBuffer* pBuf, hUint16* pIndices, hUint16 nIndices, PrimitiveType primType, hUint32 flags )
	{
		DWORD d3dflags = 0;
		HRESULT hr;

		if ( direct3D9_.isSoftwareDevice() )
		{
			d3dflags |= D3DUSAGE_SOFTWAREPROCESSING;
		}

		hr = pD3D9Device_->CreateIndexBuffer( 
						nIndices * sizeof( hUint16 ), 
						d3dflags, 
						D3DFMT_INDEX16, 
						D3DPOOL_MANAGED,
						&pBuf->buffer_, 
						NULL );

		hcAssertMsg( hr == D3D_OK, "CreateIndexBuffer failed 0x%08X", hr );

		pBuf->nIndices_ = nIndices;

		switch( primType )
		{
		case PRIMITIVETYPE_TRILIST:		pBuf->primType_ = D3DPT_TRIANGLELIST;	break;
		case PRIMITIVETYPE_TRISTRIP:	pBuf->primType_ = D3DPT_TRIANGLESTRIP;	break;
		case PRIMITIVETYPE_TRIFAN:		pBuf->primType_ = D3DPT_TRIANGLEFAN;	break;
		}

		pBuf->mode_ = flags;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hdD3D9Renderer::DestoryIndexBuffer( hdD3D9IndexBuffer* pBuf )
	{
		if ( pBuf->buffer_ )
		{
			//hcAssertMsg( pBuf-> == NULL, "Freeing a locked Index buffer" );
			pBuf->buffer_->Release();
		}
	}


	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hdD3D9Renderer::CreateVertexBuffer( hdD3D9VtxBuffer* pBuf, hUint32 count, hUint32 formatStride, hUint32 flags )
	{
		DWORD d3dflags = 0;
		HRESULT hr;

		if ( direct3D9_.isSoftwareDevice() )
		{
			d3dflags |= D3DUSAGE_SOFTWAREPROCESSING;
		}

		hr = pD3D9Device_->CreateVertexBuffer( 
						formatStride * count,
						d3dflags,
						0, 
						D3DPOOL_MANAGED,
						&pBuf->pBuffer_, 
						NULL );

		hcAssertMsg( hr == D3D_OK, "CreateVertexBuffer failed 0x%08X", hr );

		pBuf->mode_ = flags;

		if ( flags & DYNAMIC )
		{
			pBuf->nVertex_ = count;
			pBuf->maxVertex_ = count;
		}
		else
		{
			pBuf->maxVertex_ = count;
			pBuf->nVertex_ = count;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hdD3D9Renderer::DestoryVertexBuffer( hdD3D9VtxBuffer* pBuf )
	{
		if ( pBuf->Bound() )
		{
			pBuf->pBuffer_->Release();
			pBuf->pBuffer_ = NULL;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	D3DFORMAT hdD3D9Renderer::GetCompatibleFormat( const TextureFormat format )//compatible 
	{
		D3DFORMAT d3dFormat;

		switch( format )
		{
		case TFORMAT_ARGB8:
			d3dFormat = D3DFMT_A8R8G8B8;
			break;
		case TFORMAT_XRGB8:
			d3dFormat = D3DFMT_X8R8G8B8;
			break;
		case TFORMAT_RGB8:
			d3dFormat = D3DFMT_R8G8B8;
			break;
		case TFORMAT_DXT5:
			d3dFormat = D3DFMT_DXT5;
			break;
		case TFORMAT_L8:
			d3dFormat = D3DFMT_L8;
			break;
		case TFORMAT_R32F:
			d3dFormat = D3DFMT_R32F;
			break;
		case TFORMAT_R16F:
			d3dFormat = D3DFMT_R16F;
			break;
		case TFORMAT_GR16F:
			d3dFormat = D3DFMT_G16R16F;
			break;
		case TFORMAT_ABGR16F:
			d3dFormat = D3DFMT_A16B16G16R16F;
			break;
		case TFORMAT_D24S8F:
			d3dFormat = D3DFMT_D24S8;
			break;
		case TFORMAT_D32F:
			d3dFormat = D3DFMT_D32;
			break;
		default:
			hcAssertFailMsg( "Unkown texture format %d", format );
			break;
		}	
		return d3dFormat;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	hUint32 hdD3D9Renderer::GetFormatBPP( const D3DFORMAT d3dFormat )
	{
		hUint32 bpp;

		switch( d3dFormat )
		{
		case D3DFMT_A8R8G8B8:
		case D3DFMT_X8R8G8B8:
			bpp = 4;
			break;
		case D3DFMT_R8G8B8:
			bpp = 3;
			break;
		case D3DFMT_L8:
		default:
			bpp = 1;
			break;
		}	
		return bpp;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hdD3D9Renderer::BeginDebuggerEvent( const hChar* name )
	{
		D3DCOLOR c = D3DCOLOR_ARGB(255,255,255,255);
		wchar_t wname[256];
		hUint32 s = strlen( name );
		hUint32 i = 0;
		for ( ; i < s && i < 255; ++i )
		{
			wname[i] = name[i];
		}
		wname[i] = 0;
		D3DPERF_BeginEvent( c, wname );
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hdD3D9Renderer::EndDebuggerEvent()
	{
		D3DPERF_EndEvent();
	}

}