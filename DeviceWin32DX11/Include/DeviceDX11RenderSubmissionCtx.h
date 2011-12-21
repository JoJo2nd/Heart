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
    class hdDX11Texture;
    class hdDX11IndexBffer;
    class hdDX11VertexBuffer;
    class hdDX11VertexDecl;

    class hdDX11RenderSubmissionCtx
    {
    public:
        hdDX11RenderSubmissionCtx() {}
        ~hdDX11RenderSubmissionCtx() {}

        void	SetIndexStream( hdDX11IndexBffer* pIIBuf );
        void	SetVertexStream( hdDX11VertexBuffer* pIVBuf, hdDX11VertexDecl* pDecl );
        void	SetRenderStateBlock( const hdDX11BlendState& );
        void	SetRenderStateBlock( const hdDX11DepthStencilState& );
        void	SetRenderStateBlock( const hdDX11RasterizerState& );
        void	SetRenderStateBlock( const hdDX11SamplerState& );
        void	SetVertexFormat( hdDX11VertexDecl* );
        void	SetRenderTarget( hUint32 idx , hdDX11Texture* pTarget );
        void	SetViewport( const hViewport& viewport );
        void	SetScissorRect( const ScissorRect& scissor );
        void	ClearTarget( hBool clearColour, hColour& colour, hBool clearZ, hFloat z );
        void	DrawPrimitive( hUint32 nPrimatives );
        void	DrawVertexStream( PrimitiveType primType );

        void                    SetDeviceCtx( ID3D11DeviceContext* device ) { device_ = device; }
        ID3D11DeviceContext*    GetDeviceCtx() const { return device_; };

    private:

        friend class hdDX11RenderDevice;

        ID3D11RenderTargetView* renderTargetView_;
        ID3D11DepthStencilView* depthStencilView_;
        ID3D11DeviceContext*    device_;
    };
}


#endif // DEVICEDX11RENDERSUBMISSIONCTX_H__