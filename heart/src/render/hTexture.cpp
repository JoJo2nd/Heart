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

    void hTexture::ReleaseCPUTextureData()
    {
        if (singleAlloc_)
        {
            hHeapFree(heap_, levelDescs_[0].mipdata_);
            for (hUint32 i = 0; i < nLevels_; ++i)
            {
                levelDescs_[i].mipdata_ = NULL;
            }
        }
        else
        {
            for (hUint32 i = 0; i < nLevels_; ++i)
            {
                hDELETE_ARRAY_SAFE(heap_, levelDescs_[i].mipdata_);
            }
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

