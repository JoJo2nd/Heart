/********************************************************************
	created:	2008/08/10
	created:	10:8:2008   23:42
	filename: 	Texture.cpp
	author:		James Moran
	
	purpose:	
*********************************************************************/

#include "Common.h"
#include "Heart.h"
#include "hTexture.h"
#include "hRenderer.h"
#include "hRenderCommon.h"

namespace Heart
{
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hTexture::Release()
	{
		renderer_->NewRenderCommand< Cmd::ReleaseTexture >( this );
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hTexture::Lock( hUint16 level, hTextureLockInfo* info )
	{
		hcAssert( !lockPtr_ );
		if ( level < nLevels_ && info )
		{
			info->level_ = level;
			info->pitch_ = levelDescs_[level].width_*4;//TODO: fix pixel depth!
			info->ptr_ = renderer_->AquireTempRenderMemory( levelDescs_[level].width_*info->pitch_ );
			lockPtr_ = info->ptr_;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hTexture::Unlock( hTextureLockInfo* info )
	{
		hcAssert( lockPtr_ );
		renderer_->NewRenderCommand<Cmd::FlushTextureLevel>( this, info->level_, info->pitch_, info->ptr_ );
		lockPtr_ = NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hTexture::FlushLevel( hUint32 level, hUint32 inpitch, void* data )
	{
		hUint32 p;
		void* mapped = pImpl()->Map( level, &p );

		if ( mapped )
		{
			// The device layer supports mapping a texture
			// directly to memory, allowing us to copy into it
			if ( p == inpitch )
			{
				//fast path
				memcpy( mapped, data, inpitch*levelDescs_[level].height_ );
			}
			else
			{
				//slow path
				hByte* sptr = (hByte*)data;
				hByte* dptr = (hByte*)mapped;
				for ( hUint32 i = 0; i < levelDescs_[level].height_; ++i )
				{
					memcpy( dptr, sptr, inpitch );
					sptr += inpitch;
					dptr += p;
				}
			}
		}
		else 
		{
			// The device layer doesn't support mapping a texture
			// to a memory address, in this case the Unmap call takes a 
			// memory address and uploads the data to the texture
			mapped = data;
		}

		pImpl()->Unmap( level, mapped );
		renderer_->ReleaseTempRenderMemory( data );
	}

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hTexture::Serialise( hSerialiser* ser ) const
    {
        SERIALISE_ELEMENT( (hUint32&)format_ );
        SERIALISE_ELEMENT( nLevels_ );
        SERIALISE_ELEMENT_COUNT( levelDescs_, nLevels_ );
        SERIALISE_ELEMENT( totalDataSize_ );
        SERIALISE_ELEMENT_COUNT( textureData_, totalDataSize_ );
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hTexture::Deserialise( hSerialiser* ser )
    {
        DESERIALISE_ELEMENT( (hUint32&)format_ );
        DESERIALISE_ELEMENT( nLevels_ );
        DESERIALISE_ELEMENT( levelDescs_ );
        DESERIALISE_ELEMENT( totalDataSize_ );
        DESERIALISE_ELEMENT( textureData_ );
    }

}

