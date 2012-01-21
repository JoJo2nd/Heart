/********************************************************************
	created:	2009/12/29
	created:	29:12:2009   19:09
	filename: 	hiRenderTargetTexture.cpp	
	author:		James
	
	purpose:	
*********************************************************************/

#include "Common.h"
#include "hRenderTargetTexture.h"
#include "Heart.h"
#include "hRenderer.h"


namespace Heart
{
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hRenderTargetTexture::Release()
	{
#ifdef HEART_OLD_RENDER_SUBMISSION
		renderer_->NewRenderCommand< Cmd::ReleaseRenderTarget >( this );
#endif // HEART_OLD_RENDER_SUBMISSION
	}
}
