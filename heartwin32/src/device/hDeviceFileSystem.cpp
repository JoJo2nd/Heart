/********************************************************************
	created:	2010/05/31
	created:	31:5:2010   15:37
	filename: 	FileSystem.cpp	
	author:		James
	
	purpose:	
*********************************************************************/


namespace Heart
{
	enum FileOp
	{
		FILEOP_NONE,
		FILEOP_READ,
		FILEOP_WRITE,
		FILEOP_SEEK,

		FILEOP_MAX
	};

	class hFileHandle
	{
	public:

		HANDLE				fileHandle_;
		OVERLAPPED			operation_;
		hUint64				filePos_;
		hUint32				opPending_;
	};

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

    HEARTDEV_SLIBEXPORT
	hBool HEART_API Fopen( const hChar* filename, const hChar* mode, hFileHandle** pOut )
	{
		DWORD access = 0;
		DWORD share = 0;// < always ZERO, dont let things happen to file in use!
		LPSECURITY_ATTRIBUTES secatt = NULL;// could be a prob if passed across threads>?
		DWORD creation = 0;
		DWORD flags = FILE_ATTRIBUTE_NORMAL;

		hcAssert( pOut != NULL );

		if ( mode[ 0 ] == 'r' || mode[ 0 ] == 'R' )
		{
			access = GENERIC_READ;
			creation = OPEN_EXISTING;
		}
		else if ( mode[ 0 ] == 'w' || mode[ 0 ] == 'W' )
		{
			access = GENERIC_WRITE;
			creation = CREATE_ALWAYS;
		}

        (*pOut) = hNEW( GetGlobalHeap(), hFileHandle );
		(*pOut)->fileHandle_ = CreateFile( filename, access, share, secatt, creation, flags, NULL );

		if ( (*pOut)->fileHandle_ == INVALID_HANDLE_VALUE )
		{
			hDELETE(GetGlobalHeap(), *pOut);
			return hFalse;
		}

		(*pOut)->filePos_	= 0;
		(*pOut)->opPending_ = FILEOP_NONE;

		return hTrue;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

    HEARTDEV_SLIBEXPORT
	hBool HEART_API Fclose( hFileHandle* pHandle )
	{
		CloseHandle( pHandle->fileHandle_ );

		hDELETE(GetGlobalHeap(), pHandle);
		return hTrue;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

    HEARTDEV_SLIBEXPORT
	hFileError HEART_API Fread( hFileHandle* pHandle, void* pBuffer, hUint32 size, hUint32* read )
	{
		pHandle->operation_.Offset = (DWORD)(pHandle->filePos_ & 0xFFFFFFFF);;
		pHandle->operation_.OffsetHigh = (LONG)((pHandle->filePos_ & 0xFFFFFFFF00000000) >> 32);

		if ( ReadFile( pHandle->fileHandle_, pBuffer, size, read, NULL ) == 0 )
		{
			return FILEERROR_FAILED;
		}

		pHandle->filePos_ += size;
		if ( pHandle->filePos_ > Fsize( pHandle ) )
		{
			pHandle->filePos_ = Fsize( pHandle );
		}

		pHandle->opPending_ = FILEOP_READ;
		return FILEERROR_NONE;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

    HEARTDEV_SLIBEXPORT
	hFileError HEART_API Fseek( hFileHandle* pHandle, hUint64 offset, hSeekOffset from )
	{
		hUint64 size = Fsize( pHandle );
		switch ( from )
		{
		case SEEKOFFSET_BEGIN:
			{
				pHandle->filePos_ = offset;
			}
			break;
		case SEEKOFFSET_CURRENT:	
			{
				pHandle->filePos_ += offset;
			}
			break;
		case SEEKOFFSET_END:
			{
				pHandle->filePos_ = size;
				pHandle->filePos_ += offset;
			}
			break;
		}

		pHandle->opPending_ = FILEOP_SEEK;
		
		if ( pHandle->filePos_ > size )
		{
			pHandle->filePos_ = size;
		}

        SetFilePointer( pHandle->fileHandle_, (LONG)pHandle->filePos_, 0, FILE_BEGIN );

		return FILEERROR_NONE;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

    HEARTDEV_SLIBEXPORT
	hUint64 HEART_API Ftell( hFileHandle* pHandle )
	{
		return pHandle->filePos_;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

    HEARTDEV_SLIBEXPORT
	hUint64 HEART_API Fsize( hFileHandle* pHandle )
	{
		return GetFileSize( pHandle->fileHandle_, NULL );
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

    HEARTDEV_SLIBEXPORT
	hFileError HEART_API Fwrite( hFileHandle* pHandle, const void* pBuffer, hUint32 size, hUint32* written )
	{
		pHandle->operation_.Offset = (DWORD)(pHandle->filePos_ & 0xFFFFFFFF);;
		pHandle->operation_.OffsetHigh = (LONG)((pHandle->filePos_ & 0xFFFFFFFF00000000) >> 32);

		if ( WriteFile( pHandle->fileHandle_, pBuffer, size, written, NULL ) == 0 )
		{
			return FILEERROR_FAILED;
		}

		pHandle->filePos_ += size;
		if ( pHandle->filePos_ > Fsize( pHandle ) )
		{
			pHandle->filePos_ = Fsize( pHandle );
		}

		pHandle->opPending_ = FILEOP_WRITE;
		return FILEERROR_NONE;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

    HEARTDEV_SLIBEXPORT
	void HEART_API EnumerateFiles( const hChar* path, hEnumerateDeviceFilesCallback fn )
	{
		WIN32_FIND_DATA found;
		hUint32 searchPathLen = strlen(path) + 3;
		hChar* searchPath = (hChar*)alloca( searchPathLen );

        hStrCopy( searchPath, searchPathLen, path );
		hStrCat( searchPath, searchPathLen, "/*" );

		HANDLE searchHandle = FindFirstFile( searchPath, &found );

		if ( searchHandle == INVALID_HANDLE_VALUE )
			return;

		do 
		{
			hFileHandleInfo info;
			info.path_ = path;
			info.name_ = found.cFileName;
			info.directory_ = ( found.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) == FILE_ATTRIBUTE_DIRECTORY;

			if ( fn( &info ) == false )
			{
				FindClose( searchHandle );
				return;
			}
		}
		while ( FindNextFile( searchHandle, &found ) );

		FindClose( searchHandle );
	}

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    HEARTDEV_SLIBEXPORT
    hFileStat HEART_API Fstat( hFileHandle* handle )
    {
        hFileStat stat;
        BY_HANDLE_FILE_INFORMATION fileInfo;

        GetFileInformationByHandle(handle->fileHandle_, &fileInfo);

        stat.createTime_     = (((hUint64)fileInfo.ftCreationTime.dwHighDateTime) << 32)   | fileInfo.ftCreationTime.dwLowDateTime;
        stat.lastModTime_    = (((hUint64)fileInfo.ftLastWriteTime.dwHighDateTime) << 32)  | fileInfo.ftLastWriteTime.dwLowDateTime;
        stat.lastAccessTime_ = (((hUint64)fileInfo.ftLastAccessTime.dwHighDateTime) << 32) | fileInfo.ftLastAccessTime.dwLowDateTime;

        return stat;
    }

}