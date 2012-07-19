/********************************************************************

	filename: 	hZipFile.cpp	
	
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
	
	hZipFile::hZipFile(unzFile zip, unz_file_info64 info) 
        : zipPak_(zip)
        , zipFileInfo_(info)
		, filePos_( 0 )
	{

	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	
	hZipFile::~hZipFile()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	hUint64 hZipFile::Tell()
	{
		return filePos_;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	hUint64 hZipFile::Length()
	{
		return zipFileInfo_.uncompressed_size;
	}

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hUint32 hZipFile::Read( void* pBuffer, hUint32 size )
    {
        hUint32 ret;
        unzGoToFilePos64( zipPak_, &zipFileInfo_ );
        unzOpenCurrentFile( zipPak_ );
        
        unzSetOffset64(filePos_);
        ret = unzReadCurrentFile( zipPak_, pBuffer, size );
        unzCloseCurrentFile( zipPak_ );
        return ret;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hUint32 hZipFile::Write( const void* pBuffer, hUint32 size )
    {
        return 0;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hUint32 hZipFile::Seek( hUint64 offset, hdSeekOffset from )
    {
        hUint64 size = Length();
        switch ( from )
        {
        case SEEKOFFSET_BEGIN:
            {
                filePos_ = offset;
            }
            break;
        case SEEKOFFSET_CURRENT:	
            {
                filePos_ += offset;
            }
            break;
        case SEEKOFFSET_END:
            {
                filePos_ = size;
                filePos_ += offset;
            }
            break;
        }

        if ( filePos_ > size )
        {
            filePos_ = size;
        }
        return offset;
    }
}