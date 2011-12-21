/********************************************************************
	created:	2010/06/14
	created:	14:6:2010   22:05
	filename: 	DeviceTexture.h	
	author:		James
	
	purpose:	
*********************************************************************/
#ifndef DEVICETEXTURE_H__
#define DEVICETEXTURE_H__

namespace Heart
{
	class hRenderer;

	class hdD3D9TextureBase
	{
	public:
		hdD3D9TextureBase() {}
		virtual ~hdD3D9TextureBase() {}
	};

	class hdD3D9Texture : public hdD3D9TextureBase
	{
	public:

		enum SurfaceFlags
		{
			SF_TEXTURE		= 1,
			SF_DEPTH		= 1 << 1,
			SF_RENDERTARGET	= 1 << 2,
		};

		hdD3D9Texture();
		virtual ~hdD3D9Texture();

		hBool		Bound() const { return pD3DTexture_ != NULL; }
		void*		Map( hUint32 level, hUint32* pitch ) 
		{
			D3DLOCKED_RECT lock;
			pD3DTexture_->LockRect( level, &lock, NULL, 0 );
			*pitch = lock.Pitch;
			return lock.pBits;
		}
		void		Unmap( hUint32 level, void* ptr ) 
		{
			pD3DTexture_->UnlockRect( level );
		}

	private:

		friend class TextureBuilder;
		friend class hdD3D9Renderer;
		friend class hdD3D9Material;

		hUint32						renderTarget_;
		LPDIRECT3DTEXTURE9			pD3DTexture_;
		LPDIRECT3DSURFACE9			pRenderTargetSurface_;

	};
}

#endif // DEVICETEXTURE_H__