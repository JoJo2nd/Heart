/********************************************************************

	filename: 	hTexture.cpp	
	
	Copyright (c) 1:4:2012 James Moran
	
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

namespace Heart
{
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hTexture::Release()
	{
#ifdef HEART_OLD_RENDER_SUBMISSION
		renderer_->NewRenderCommand< Cmd::ReleaseTexture >( this );
#endif // HEART_OLD_RENDER_SUBMISSION
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
/*
	void hTexture::Lock( hUint16 level, hTextureMapInfo* info )
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

	void hTexture::Unlock( hTextureMapInfo* info )
	{
		hcAssert( lockPtr_ );
		renderer_->NewRenderCommand<Cmd::FlushTextureLevel>( this, info->level_, info->pitch_, info->ptr_ );
		lockPtr_ = NULL;
	}
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
*/

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hTexture::ReleaseCPUTextureData()
    {
        for (hUint32 i = 0; i < nLevels_; ++i)
        {
            hDELETE_ARRAY_SAFE(GetGlobalHeap(), levelDescs_[i].mipdata_);
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    Heart::hColour hTexture::ReadPixel(hUint32 x, hUint32 y)
    {
        hcAssert(x < Width() && y < Height());
        if (!levelDescs_[0].mipdata_)
        {
            hcPrintf("Attempt was made to read a pixel (%u, %u) from a texture where the CPU resource data"
                     "has been released. This is done automatically for DXT textures on load.", x, y);
            return WHITE;
        }

        hColour ret(
        levelDescs_[0].mipdata_[((y*levelDescs_[0].width_*4)+x*4)+0],
        levelDescs_[0].mipdata_[((y*levelDescs_[0].width_*4)+x*4)+1],
        levelDescs_[0].mipdata_[((y*levelDescs_[0].width_*4)+x*4)+2],
        levelDescs_[0].mipdata_[((y*levelDescs_[0].width_*4)+x*4)+3] );

        return ret;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hUint32 hTexture::GetDXTTextureSize( hBool dxt1, hUint32 width, hUint32 height )
    {
        // compute the storage requirements
        int blockcount = ( ( width + 3 )/4 ) * ( ( height + 3 )/4 );
        int blocksize = (dxt1) ? 8 : 16;
        return blockcount*blocksize;
    }

}

