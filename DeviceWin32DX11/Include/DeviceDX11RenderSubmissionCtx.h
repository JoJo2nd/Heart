/********************************************************************

	filename: 	DeviceDX11RenderSubmissionCtx.h	
	
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

#ifndef DEVICEDX11RENDERSUBMISSIONCTX_H__
#define DEVICEDX11RENDERSUBMISSIONCTX_H__

namespace Heart
{
    typedef D3D11_MAPPED_SUBRESOURCE hdDX11LockedResourceData;

    class hdDX11RenderSubmissionCtx
    {
    public:
        hdDX11RenderSubmissionCtx() 
            : depthStencilView_(NULL)
        {
            for ( hUint32 i = 0; i < MAX_RENDERTARGE_VIEWS; ++i )
            {
                renderTargetViews_[i] = NULL;
            }
        }
        ~hdDX11RenderSubmissionCtx() 
        {}

        void	SetIndexStream( hdDX11IndexBuffer* pIIBuf );
        void	SetVertexStream( hUint32 stream, hdDX11VertexBuffer* vtxBuf, hUint32 stride );
        void	SetRenderStateBlock( hdDX11BlendState& st );
        void	SetRenderStateBlock( hdDX11DepthStencilState& st );
        void	SetRenderStateBlock( hdDX11RasterizerState& st );
        void	SetRenderStateBlock( hUint32 samplerIdx, hdDX11SamplerState& st );
        void    SetConstantBlock( hdDX11ParameterConstantBlock* block );
        void    SetPixelShader( hdDX11ShaderProgram* prog );
        void    SetVertexShader( hdDX11ShaderProgram* prog );
        void	SetVertexFormat( hdDX11VertexLayout* vf );
        void	SetRenderTarget( hUint32 idx , hdDX11Texture* target );
        void	SetViewport( const hViewport& viewport );
        void	SetScissorRect( const ScissorRect& scissor );
        void	ClearTarget( hBool clearColour, hColour& colour, hBool clearZ, hFloat z );
        void    SetPrimitiveType( PrimitiveType type );
        void	DrawPrimitive( hUint32 nPrimatives, hUint32 startVertex );
        void    DrawIndexedPrimitive( hUint32 nPrimatives, hUint32 startVertex );
        void    Map( hdDX11Texture* tex, hUint32 level, hdDX11LockedResourceData* data );
        void    Unmap( hdDX11Texture* tex, hUint32 level, hdDX11LockedResourceData* data );
        void    Map( hdDX11IndexBuffer* ib, hdDX11LockedResourceData* data );
        void    Unmap( hdDX11IndexBuffer* ib, hdDX11LockedResourceData* data );
        void    Map( hdDX11VertexBuffer* vb, hdDX11LockedResourceData* data );
        void    Unmap( hdDX11VertexBuffer* vb, hdDX11LockedResourceData* data );

        void                    SetDeviceCtx( ID3D11DeviceContext* device ) { device_ = device; }
        ID3D11DeviceContext*    GetDeviceCtx() const { return device_; };

    private:

        friend class hdDX11RenderDevice;

        static const hUint32    MAX_RENDERTARGE_VIEWS = 4;

        PrimitiveType           primType_;
        ID3D11RenderTargetView* renderTargetViews_[MAX_RENDERTARGE_VIEWS];
        ID3D11DepthStencilView* depthStencilView_;
        ID3D11DeviceContext*    device_;
    };
}


#endif // DEVICEDX11RENDERSUBMISSIONCTX_H__