/********************************************************************
	created:	2010/06/30
	created:	30:6:2010   20:00
	filename: 	DeviceD3D9Renderer.h	
	author:		James
	
	purpose:	
*********************************************************************/
#ifndef DEVICED3D9RENDERER_H__
#define DEVICED3D9RENDERER_H__

namespace Heart
{
namespace Device
{
	class Kernel;
}
	//class IRenderResource;
	class hdD3D9VtxBuffer;
	class hdD3D9IndexBuffer;
	class hdD3D9Texture;

	class hdD3D9Renderer
	{
	public:
		hdD3D9Renderer();
		virtual ~hdD3D9Renderer();

		void				Create( Device::Kernel* pSysHandle, hUint32 width, hUint32 height, hUint32 bpp, hFloat shaderVersion, hBool fullscreen, hBool vsync );
		void				Destroy();
		void				ActivateContext() {}
		void				BeginRender();
		void				EndRender();
		void				SwapBuffers();
		void				BeginDebuggerEvent( const hChar* name );
		void				EndDebuggerEvent();

		hUint32				Width() const { return direct3D9_.backBufferWidth(); }
		hUint32				Height() const { return direct3D9_.backBufferHeight(); }

		//Renderer States
		void				SetIndexStream( hdD3D9IndexBuffer* pIIBuf );
		void				SetVertexStream( hdD3D9VtxBuffer* pIVBuf, hdD3D9VtxDecl* pDecl );
		void				SetRenderState( RENDER_STATE, RENDER_STATE_VALUE );
		void				SetRenderState( RENDER_STATE, hFloat );
		void				SetVertexFormat( hdD3D9VtxDecl* );
		void				SetTexture( hdD3D9Texture* pTexture, hUint32 idx );
		void				SetRenderTarget( hUint32 idx , hdD3D9Texture* pTarget );
		void				SetDepthSurface( hdD3D9Texture* pSurface );
		void				SetViewport( const hViewport& viewport );
		void				SetScissorRect( const ScissorRect& scissor );

		//Draw Calls
		void				ClearTarget( hBool clearColour, hColour& colour, hBool clearZ, hFloat z );

		void				DrawPrimitive( hUint32 nPrimatives );
		void				DrawVertexStream( PrimitiveType primType );


		//Render Resources Create Calls
		void						CompileShader( hdD3D9Shader* out, const hChar* shaderProg, hUint32 len, ShaderType type, const hChar* entry );
		//void						ReleaseCompiledShader( hdD3D9Shader* shader );
		void						GetVertexStrideOffset( hUint32 vtxFlags, hUint32& stride, hUint32* pOffsets );
		void						CreateVertexDeclaration( hdD3D9VtxDecl* ppOut, hUint32 vtxFlags, hUint32& stride, hUint32* pOffsets );
		void						DestroyVertexDeclaration( hdD3D9VtxDecl* pIVD );
		//void						CreateMaterial( hdD3D9Material* pIOut, CompiledShaderResource* shader, hColour diffuse, hColour ambient, hColour Emissive, hColour specular, hFloat power );
		void						OnMaterialLoad( hdD3D9Material* pIOut, void* pStart );
		void						DestoryMaterial( hdD3D9Material* pIOut );
		void						CreateTexture( hdD3D9Texture* pOut, hUint32 width, hUint32 height, hUint32 levels, TextureFormat format );
		void						DestoryTexture( hdD3D9Texture* pOut );
		void						CreateRenderTarget( hdD3D9Texture* pOut, hUint32 width, hUint32 height, TextureFormat format );
		void						DestroyRenderTarget( hdD3D9Texture* pOut );
		void						CreateIndexBuffer( hdD3D9IndexBuffer* pIBuf, hUint16* pIndices, hUint16 nIndices, PrimitiveType primType, hUint32 flags );
		void						DestoryIndexBuffer( hdD3D9IndexBuffer* pIBuf );
		void						CreateVertexBuffer( hdD3D9VtxBuffer* pBuf, hUint32 count, hUint32 formatStride, hUint32 flags );
		void						DestoryVertexBuffer( hdD3D9VtxBuffer* pBuf );

		/* Implementation specific */
		static D3DFORMAT			GetCompatibleFormat( const TextureFormat format );
		static hUint32				GetFormatBPP( const D3DFORMAT d3dFormat );

	private:

		typedef huFunctor< void (*)( RENDER_STATE_VALUE ) >::type	SetRenderStateDWord;
		typedef huFunctor< void (*)( hFloat ) >::type						SetRenderStateFloat;

		void BuildVertexDecl( D3DVERTEXELEMENT9* elements, hUint32 vtxFlags, hUint32* stride, hUint32* pOffsets );
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

		Device::Kernel*					system_;
		Direct3D						direct3D9_;
		LPDIRECT3DDEVICE9				pD3D9Device_;
		LPDIRECT3DSURFACE9				pBackBufferSurface_;
		LPDIRECT3DSURFACE9				pDepthSurface_;
		hdD3D9Texture*					pDepthTarget_;

		//Render State Setters
		SetRenderStateDWord				dwRenderStateSet_[ RS_MAX ];
		SetRenderStateFloat				fRenderStateSet_[ RS_MAX ];
		hdD3D9IndexBuffer*				pCurrentIndexStream_;
		hdD3D9VtxBuffer*				pCurrentVertexStream_;
	};
}

#endif // DEVICED3D9RENDERER_H__