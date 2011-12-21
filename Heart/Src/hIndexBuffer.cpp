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
#include "hRenderCommon.h"

namespace Heart
{
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hIndexBuffer::Release()
	{
		hcAssert( renderer_ );
		renderer_->NewRenderCommand< Cmd::ReleaseIndexBuffer >( this );
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hIndexBuffer::Lock()
	{
		hcAssert( lockPtr_ == NULL );
		lockPtr_ = (hUint16*)renderer_->AquireTempRenderMemory( nIndices_*sizeof(hUint16) );
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hIndexBuffer::Unlock()
	{
		renderer_->NewRenderCommand< Cmd::FlushIndexBufferData >( this, lockPtr_, nIndices_*sizeof(hUint16) );
		lockPtr_ = NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hIndexBuffer::SetIndex( hUint16 idx, hUint16 val )
	{
		hcAssert( lockPtr_ );
		hcAssert( idx < nIndices_ );
		lockPtr_[idx] = val;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hIndexBuffer::SetData( hUint16* data, hUint32 size )
	{
		hcAssert( lockPtr_ );
		memcpy( lockPtr_, data, size );
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hIndexBuffer::FlushIndexData( void* data, hUint32 size )
	{
		void* mapped = pImpl()->Map();
		if ( mapped )
		{
			memcpy( mapped, data, size );
		}
		pImpl()->Unmap( mapped );
	}

}