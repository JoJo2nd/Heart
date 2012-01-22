/********************************************************************
	created:	2009/12/29
	created:	29:12:2009   18:51
	filename: 	TextureBase.h	
	author:		James
	
	purpose:	
*********************************************************************/
#ifndef HITEXTUREBASE_H__
#define HITEXTUREBASE_H__

#include "hTypes.h"
#include "hPtrImpl.h"
#include "hIReferenceCounted.h"
#include "hResource.h"

namespace Heart
{

	class hRenderer;

	class hTextureBase : public hPtrImpl< hdTexture >, 
						 public hResourceClassBase
	{
	public:
		hTextureBase( hRenderer* prenderer ) 
			: renderer_( prenderer )
			, created_( hFalse )
		{

		}
		virtual ~hTextureBase()
		{
		}

		virtual hUint32			Width( hUint32 level = 0 ) = 0;
		virtual hUint32			Height( hUint32 level = 0 ) = 0;

	protected:

		friend class hRenderer;
		virtual void			Release() = 0;

		hRenderer*				renderer_;
		volatile hBool			created_;
	};

}

#endif // HITEXTUREBASE_H__