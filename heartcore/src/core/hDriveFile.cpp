/********************************************************************

	filename: 	hDriveFile.cpp	
	
	Copyright (c) 20:3:2011 James Moran
	
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

	hUint64 hDriveFile::Tell()
	{
		return hdFtell( fileHandle_ );
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	hUint64 hDriveFile::Length()
	{
		return hdFsize( fileHandle_ );
	}

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hUint32 hDriveFile::Read( void* pBuffer, hUint32 size )
    {
        hUint32 ret;
        if ( hdFread( fileHandle_, pBuffer, size, &ret ) != FILEERROR_NONE )
            return 0;
        return ret;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hUint32 hDriveFile::Write( const void* pBuffer, hUint32 size )
    {
        hUint32 ret;
        if ( hdFwrite( fileHandle_, pBuffer, size, &ret ) != FILEERROR_NONE )
            return 0;
        return ret;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hUint32 hDriveFile::Seek( hUint64 offset, hdSeekOffset from )
    {
        hdSeekOffset devFrom = (hdSeekOffset)from;
        hdFseek( fileHandle_, offset, devFrom );
        return 0;
    }

}