/********************************************************************

	filename: 	DeviceOpenGLRenderer.h	
	
	Copyright (c) 26:7:2011 James Moran
	
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
#ifndef DEVICEOPENGLRENDERER_H__
#define DEVICEOPENGLRENDERER_H__

namespace Heart
{
namespace Device
{
	class Kernel;
}
	//class IRenderResource;
	class hdOGLVtxBuffer;
	class hdOGLIndexBuffer;
	class hdOGLTexture;
	class hdOGLMaterial;
	class hdOGLVtxDecl;
	class CompiledShaderResource;

	class hdOGLRenderer
	{
	public:
		hdOGLRenderer()
			: hWnd_( 0 )
			, hDC_( 0 )
			, hRC_( 0 )
			, pixelFormat_( 0 )
		{

		}
		virtual ~hdOGLRenderer() {}

		void				Create( Device::Kernel* pSysHandle, hUint32 width, hUint32 height, hUint32 bpp, hFloat shaderVersion, hBool fullscreen, hBool vsync );
		void				Destroy();
		void				ActivateContext();
		void				BeginRender();
		void				EndRender();
		void				SwapBuffers();
		void				BeginDebuggerEvent( const hChar* /*name*/ ){}
		void				EndDebuggerEvent(){}

		hUint32				Width() const { return width_; }
		hUint32				Height() const { return height_; }

		//Renderer States
		void				SetIndexStream( hdOGLIndexBuffer* pIIBuf );
		void				SetVertexStream( hdOGLVtxBuffer* pIVBuf, hdOGLVtxDecl* pDecl );
		void				SetRenderState( RENDER_STATE, RENDER_STATE_VALUE );
		void				SetRenderState( RENDER_STATE, hFloat );
		void				SetVertexFormat( hdOGLVtxDecl* );
		void				SetTexture( hdOGLTexture* pTexture, hUint32 idx );
		void				SetRenderTarget( hUint32 idx , hdOGLTexture* pTarget );
		void				SetDepthSurface( hdOGLTexture* pSurface );
		void				SetViewport( const hViewport& viewport );
		void				SetScissorRect( const ScissorRect& scissor );

		//Draw Calls
		void				ClearTarget( hBool clearColour, hColour& colour, hBool clearZ, hFloat z );

		void				DrawPrimitive( hUint32 nPrimatives );
		void				DrawVertexStream( PrimitiveType primType );

		void				BeginPrim() {}
		void				EndPrim() {}

		//Render Resources Create Calls
		CompiledShaderResource*		CompileShader( const hChar* shaderProg, hUint32 len );
		void						ReleaseCompiledShader( CompiledShaderResource* shader );
		void						CreateVertexDeclaration( hdOGLVtxDecl* ppOut, hUint32 vtxFlags, hUint32& stride, hUint32* pOffsets );
		void						DestroyVertexDeclaration( hdOGLVtxDecl* pIVD );
		void						GetVertexStrideOffset( hUint32 vtxFlags, hUint32& stride, hUint32* pOffsets );
		void						CreateMaterial( hdOGLMaterial* pIOut, CompiledShaderResource* shader, hColour diffuse, hColour ambient, hColour Emissive, hColour specular, hFloat power );
		void						OnMaterialLoad( hdOGLMaterial* pIOut, void* pStart );
		void						DestoryMaterial( hdOGLMaterial* pIOut );
		void						CreateTexture( hdOGLTexture* pOut, hUint32 width, hUint32 height, hUint32 levels, TextureFormat format );
		void						DestoryTexture( hdOGLTexture* pOut );
		void						CreateRenderTarget( hdOGLTexture* pOut, hUint32 width, hUint32 height, TextureFormat format );
		void						DestroyRenderTarget( hdOGLTexture* pOut );
		void						CreateIndexBuffer( hdOGLIndexBuffer* pIBuf, hUint16* pIndices, hUint16 nIndices, PrimitiveType primType, hUint32 flags );
		void						DestoryIndexBuffer( hdOGLIndexBuffer* pIBuf );
		void						CreateVertexBuffer( hdOGLVtxBuffer* pBuf, hUint32 count, hUint32 formatStride, hUint32 flags );
		void						DestoryVertexBuffer( hdOGLVtxBuffer* pBuf );

	private:

		typedef huFunctor< void (*)( RENDER_STATE_VALUE ) >::type	SetRenderStateDWord;
		typedef huFunctor< void (*)( hFloat ) >::type						SetRenderStateFloat;

		void ReleaseContext();

		void SetCullMode( RENDER_STATE_VALUE val );
		void SetFillMode( RENDER_STATE_VALUE val );
		void SetDepthBias( hFloat val );
		void SetScissorTest( RENDER_STATE_VALUE val );
		void SetZTest( RENDER_STATE_VALUE val );
		void SetZWrite( RENDER_STATE_VALUE val );
		void SetZCompareFunction( RENDER_STATE_VALUE val );
		void SetStencilTest( RENDER_STATE_VALUE val );
		void SetStencilFail( RENDER_STATE_VALUE val );
		void SetStencilZFail( RENDER_STATE_VALUE val );
		void SetStencilPass( RENDER_STATE_VALUE val );
		void SetStencilFunction( RENDER_STATE_VALUE val );
		void SetStencilRef( RENDER_STATE_VALUE val );
		void SetStencilWriteMask( RENDER_STATE_VALUE val );
		void SetStencilMask( RENDER_STATE_VALUE val );
		void SetAlphaBlend( RENDER_STATE_VALUE val );
		void SetSrcAlphaBlend( RENDER_STATE_VALUE val );
		void SetDstAlphaBlend( RENDER_STATE_VALUE val );
		void SetAlphaBlendFunction( RENDER_STATE_VALUE val );
		void SetColourWrite1( RENDER_STATE_VALUE val );
		void SetColourWrite2( RENDER_STATE_VALUE val );
		void SetColourWrite3( RENDER_STATE_VALUE val );
		void SetColourWrite4( RENDER_STATE_VALUE val );

		PIXELFORMATDESCRIPTOR		pixelFormatDesc_;
		HWND						hWnd_;
		HDC							hDC_; //Device Context
		HGLRC						hRC_; //Rendering Context
		GLuint						pixelFormat_;
		hUint32						width_;
		hUint32						height_;
	};
}

#endif // DEVICEOPENGLRENDERER_H__