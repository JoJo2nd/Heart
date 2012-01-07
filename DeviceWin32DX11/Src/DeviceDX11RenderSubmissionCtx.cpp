/********************************************************************

	filename: 	DeviceDX11RenderSubmissionCtx.cpp	
	
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

    void hdDX11RenderSubmissionCtx::SetIndexStream( hdDX11IndexBuffer* idxBuf )
    {
        device_->IASetIndexBuffer( idxBuf->buffer_, DXGI_FORMAT_R16_UINT, 0 );
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdDX11RenderSubmissionCtx::SetVertexStream( hUint32 stream, hdDX11VertexBuffer* vtxBuf, hUint32 stride )
    {
        UINT offsets = 0;
        UINT strideui = stride;
        device_->IASetVertexBuffers( stream, 1, &vtxBuf->buffer_, &strideui, &offsets );
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdDX11RenderSubmissionCtx::SetRenderStateBlock( hdDX11BlendState& st )
    {
        hFloat factors[4] = { 1.f, 1.f, 1.f, 1.f };
        device_->OMSetBlendState( st.stateObj_, factors, ~0U );
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdDX11RenderSubmissionCtx::SetRenderStateBlock( hdDX11DepthStencilState& st )
    {
        device_->OMSetDepthStencilState( st.stateObj_, st.stencilRef_ );
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdDX11RenderSubmissionCtx::SetRenderStateBlock( hdDX11RasterizerState& st )
    {
        device_->RSSetState( st.stateObj_ );
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdDX11RenderSubmissionCtx::SetRenderStateBlock( hUint32 samplerIdx, hdDX11SamplerState& st )
    {
        device_->VSSetSamplers( samplerIdx, 1, &st.stateObj_ );
        device_->PSSetSamplers( samplerIdx, 1, &st.stateObj_ );
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdDX11RenderSubmissionCtx::SetVertexFormat( hdDX11VertexLayout* vf )
    {
        device_->IASetInputLayout( vf->layout_ );
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdDX11RenderSubmissionCtx::SetPixelShader( hdDX11ShaderProgram* prog )
    {
        hcAssert( prog->type_ == ShaderType_FRAGMENTPROG );
        device_->PSSetShader( prog->pixelShader_, NULL, 0 );
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdDX11RenderSubmissionCtx::SetVertexShader( hdDX11ShaderProgram* prog )
    {
        hcAssert( prog->type_ == ShaderType_VERTEXPROG );
        device_->VSSetShader( prog->vertexShader_, NULL, 0 );
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdDX11RenderSubmissionCtx::SetRenderTarget( hUint32 idx , hdDX11Texture* target )
    {
        if ( !target )
        {
            if ( renderTargetViews_[idx] )
            {
                renderTargetViews_[idx] = NULL;
                device_->OMSetRenderTargets( 4, renderTargetViews_, depthStencilView_ );
            }
        }
        else
        {
            hcAssertMsg( target->renderTargetView_, "Texture not created with RESOURCE_RENDERTARGET flag" );
            if ( renderTargetViews_[idx] != target->renderTargetView_ )
            {
                renderTargetViews_[idx] = target->renderTargetView_;
                device_->OMSetRenderTargets( 4, renderTargetViews_, depthStencilView_ );
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdDX11RenderSubmissionCtx::SetViewport( const hViewport& viewport )
    {
        D3D11_VIEWPORT vp;
        vp.TopLeftX = (FLOAT)viewport.x_;
        vp.TopLeftY = (FLOAT)viewport.y_;
        vp.Width = (FLOAT)viewport.width_;
        vp.Height = (FLOAT)viewport.height_;
        vp.MinDepth = 0.f;
        vp.MaxDepth = 1.f;
        device_->RSSetViewports( 1, &vp );
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdDX11RenderSubmissionCtx::SetScissorRect( const ScissorRect& scissor )
    {
        D3D11_RECT s;
        s.left = scissor.left_;
        s.right = scissor.right_;
        s.top = scissor.top_;
        s.bottom = scissor.bottom_;
        device_->RSSetScissorRects( 1, &s );
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdDX11RenderSubmissionCtx::ClearTarget( hBool clearColour, hColour& colour, hBool clearZ, hFloat z )    
    {
        if ( clearColour )
        {
            for ( hUint32 i = 0; i < MAX_RENDERTARGE_VIEWS; ++i )
            {
                if ( renderTargetViews_[i] )
                    device_->ClearRenderTargetView( renderTargetViews_[i], (FLOAT*)&colour );
            }
        }
        if ( clearZ )
            device_->ClearDepthStencilView( depthStencilView_, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, z, 0 );
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdDX11RenderSubmissionCtx::SetPrimitiveType( PrimitiveType type )
    {
        if ( type != primType_ )
        {
            switch ( type )
            {
            case PRIMITIVETYPE_LINELIST:
                device_->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_LINELIST ); break;
            case PRIMITIVETYPE_TRILIST:
                device_->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST ); break;
            case PRIMITIVETYPE_TRISTRIP:
                device_->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP ); break;
            default:
                hcAssert( hFalse ); break;
            }
            primType_ = type;
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdDX11RenderSubmissionCtx::DrawPrimitive( hUint32 nPrimatives, hUint32 start )
    {
        hUint32 verts;
        switch ( primType_ )
        {
        case PRIMITIVETYPE_LINELIST:
            verts = nPrimatives / 2; break;
        case PRIMITIVETYPE_TRILIST:
            verts = nPrimatives * 3; break;
        case PRIMITIVETYPE_TRISTRIP:
            verts = nPrimatives + 2; break;
        }
        device_->Draw( verts, start );
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdDX11RenderSubmissionCtx::DrawIndexedPrimitive( hUint32 nPrimatives, hUint32 start )
    {
        hUint32 verts;
        switch ( primType_ )
        {
        case PRIMITIVETYPE_LINELIST:
            verts = nPrimatives / 2; break;
        case PRIMITIVETYPE_TRILIST:
            verts = nPrimatives * 3; break;
        case PRIMITIVETYPE_TRISTRIP:
            verts = nPrimatives + 2; break;
        }
        device_->DrawIndexed( verts, start, 0 );
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdDX11RenderSubmissionCtx::Map( hdDX11Texture* tex, hUint32 level, hdDX11LockedResourceData* data )
    {
        hcAssert( tex && data );
        device_->Map( tex->dx11Texture_, level, D3D11_MAP_WRITE_DISCARD, 0, data );
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdDX11RenderSubmissionCtx::Unmap( hdDX11Texture* tex, hUint32 level, hdDX11LockedResourceData* data )
    {
        device_->Unmap( tex->dx11Texture_, level );
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdDX11RenderSubmissionCtx::SetConstantBlock( hdDX11ParameterConstantBlock* block )
    {
        block->Flush( device_ );
        device_->VSSetConstantBuffers( block->slot_, 1, &block->constBuffer_ );
        device_->PSSetConstantBuffers( block->slot_, 1, &block->constBuffer_ );
    }

}