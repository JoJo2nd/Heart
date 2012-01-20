/********************************************************************
	created:	2010/11/07
	created:	7:11:2010   20:38
	filename: 	IndexBuffer.cpp	
	author:		James
	
	purpose:	
*********************************************************************/

#include "Common.h"
#include "hIndexBuffer.h"
#include "hRenderer.h"

namespace Heart
{
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hIndexBuffer::Release()
	{
		hcAssert( renderer_ );
#ifdef HEART_OLD_RENDER_SUBMISSION
		renderer_->NewRenderCommand< Cmd::ReleaseIndexBuffer >( this );
#endif // HEART_OLD_RENDER_SUBMISSION
	}

}