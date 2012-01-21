/********************************************************************
	created:	2009/12/29
	created:	29:12:2009   18:59
	filename: 	hiRenderTargetTexture.h	
	author:		James
	
	purpose:	
*********************************************************************/
#ifndef HIRENDERTARGETTEXTURE_H__
#define HIRENDERTARGETTEXTURE_H__

#include "hTextureBase.h"

namespace Heart
{
	class hRenderTargetTexture : public hTextureBase
	{
	public:
	 
	 	hUint32			Width( hUint32 = 0 ) { return width_; }
	 	hUint32			Height( hUint32 = 0 ) { return height_; }
		void			Release();

	 private:

		friend class hRenderer;

		hRenderTargetTexture( hRenderer* prenderer ) :
			hTextureBase( prenderer )
		{
		}
		virtual ~hRenderTargetTexture()
		{
		}

		hTextureFormat	format_; 
	 	hUint32					width_;
	 	hUint32					height_;
	};
}

#endif // HIRENDERTARGETTEXTURE_H__