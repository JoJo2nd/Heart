/********************************************************************

	filename: 	hZipFileSystem.cpp	
	
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
	voidpf ZipOpen( voidpf opaque, const void* filename, int mode )
	{
		(void)opaque;
		(void)mode;
		hdFileHandle* fh=new hdFileHandle();
        static hChar rootDir[] = "GAMEDATA/";
        hUint32 maxlen = sizeof(rootDir)+hStrLen((const hChar*)filename)+1;
        hChar* fullpath = (hChar*)hAlloca(maxlen);
        hStrCopy(fullpath, maxlen, rootDir);
        hStrCat(fullpath, maxlen, (const hChar*)filename);
		if ( !hdFopen( (const hChar*)fullpath, "r", fh ) ) {
            delete fh;
			return NULL;
        }

		return fh;
	}

	int ZipClose( voidpf opaque, voidpf stream )
	{
		(void)opaque;
		hdFileHandle* fh = (hdFileHandle*)stream;

		hdFclose( fh );
        delete fh;
        fh=hNullptr;

		return 0;
	}

	int ZipError(voidpf opaque, voidpf stream)
	{
		(void)opaque;
		hdFileHandle* fh = (hdFileHandle*)stream;

		if ( hdFtell( fh ) >= hdFsize( fh ) )
		{
			return 0;
		}

		return 1;
	}

	uLong ZipRead( voidpf opaque, voidpf stream, void* buf, uLong size )
	{
		(void)opaque;
		hdFileHandle* fh = (hdFileHandle*)stream;

        hUint32 bytesRead;
		if ( hdFread( fh, buf, size, &bytesRead ) != FILEERROR_NONE )
		{
			return 0;
		}
		return bytesRead;
	}

	uLong ZipWrite( voidpf opaque, voidpf stream, const void* buf, uLong size )
	{
		(void)opaque;
		hdFileHandle* fh = (hdFileHandle*)stream;

        hUint32 bytesWritten;
		if ( hdFwrite( fh, buf, size, &bytesWritten ) != FILEERROR_NONE )
		{
			return 0;
		}

		return bytesWritten;
	}

	ZPOS64_T ZipTell( voidpf opaque, voidpf stream )
	{
		(void)opaque;
		hdFileHandle* fh = (hdFileHandle*)stream;

		return hdFtell( fh );
	}

	long ZipSeek( voidpf opaque, voidpf stream, ZPOS64_T offset, int origin )
	{
		(void)opaque;
		hdFileHandle* fh = (hdFileHandle*)stream;
		hdSeekOffset devOrigin;

		switch ( origin )
		{
		default:
		case ZLIB_FILEFUNC_SEEK_CUR: devOrigin = SEEKOFFSET_CURRENT; break;
		case ZLIB_FILEFUNC_SEEK_SET: devOrigin = SEEKOFFSET_BEGIN; break;
		case ZLIB_FILEFUNC_SEEK_END: devOrigin = SEEKOFFSET_END; break;
		}

		hdFseek( fh, offset, devOrigin );

		return 0;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hZipFileSystem::Initialise( const hChar* zipFile )
	{
		zipFileIODefs_.zopen64_file = ZipOpen;
		zipFileIODefs_.zclose_file	= ZipClose;
		zipFileIODefs_.zerror_file	= ZipError;
		zipFileIODefs_.zread_file	= ZipRead;
		zipFileIODefs_.zwrite_file	= ZipWrite;
		zipFileIODefs_.zseek64_file = ZipSeek;
		zipFileIODefs_.ztell64_file = ZipTell;
		zipFileIODefs_.opaque		= NULL;

		zipFileHandle_ = unzOpen2_64( zipFile, &zipFileIODefs_ );

        pkgNameLen_ = (hUint32)hStrChr(zipFile,'.') - (hUint32)zipFile;
        ++pkgNameLen_;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hZipFileSystem::Destory()
	{
		unzClose( zipFileHandle_ );
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	hIFile* hZipFileSystem::OpenFile( const hChar* filename, hFileMode mode ) const
	{
		if ( mode != FILEMODE_READ || !IsOpen() || hStrLen(filename) < pkgNameLen_+1)
			return NULL;

		int fileFound = unzLocateFile(zipFileHandle_, filename+pkgNameLen_, 1);

		if ( fileFound == UNZ_END_OF_LIST_OF_FILE )
        {
            unzGoToFirstFile(zipFileHandle_);
			return NULL;
        }

        unz_file_info64 file_info;
        unz64_file_pos file_pos;

        unzGetCurrentFileInfo64( 
            zipFileHandle_,
            &file_info,
            NULL,
            0,
            NULL, 
            0,
            NULL,
            0);
        unzGetFilePos64(zipFileHandle_, &file_pos);

        ++openHandles_;

		return hNEW(hZipFile)(zipFileHandle_,file_info, file_pos);
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hZipFileSystem::CloseFile( hIFile* pFile ) const
	{
		if ( !pFile )
			return;

        --openHandles_;

		hDELETE(pFile);
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hZipFileSystem::EnumerateFiles( const hChar* path, hEnumerateFilesCallback fn ) const
	{
        if (!IsOpen())
            return;

        for ( hInt ret = unzGoToFirstFile( zipFileHandle_ ); ret == UNZ_OK; ret = unzGoToNextFile( zipFileHandle_ ) )
        {
            unz_file_info64 file_info;
            char filepath[1024];

            unzGetCurrentFileInfo64( 
                zipFileHandle_,
                &file_info,
                filepath,
                1024,
                NULL, 
                0,
                NULL,
                0);

            hFileInfo inf;
            inf.directory_ = NULL;
            inf.name_ = hStrRChr(filepath, '/');
            if (!inf.name_) inf.name_ = filepath;
            inf.path_ = filepath;

            fn( &inf );    
        }
	}

}