/********************************************************************

    filename: 	hdevicefilesystem.cpp	
    
    Copyright (c) 18:7:2012 James Moran
    
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
    enum hdFileOp
    {
        FILEOP_NONE,
        FILEOP_READ,
        FILEOP_WRITE,
        FILEOP_SEEK,

        FILEOP_MAX
    };

    class hdFileHandle
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

    HEART_DLLEXPORT
    hBool HEART_API hdFopen( const hChar* filename, const hChar* mode, hdFileHandle** pOut )
    {
        DWORD access = 0;
        DWORD share = 0;// < always ZERO, dont let things happen to file in use!
        LPSECURITY_ATTRIBUTES secatt = NULL;// could be a prob if passed across threads>?
        DWORD creation = 0;
        DWORD flags = FILE_ATTRIBUTE_NORMAL;
        HANDLE fhandle;

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

        
        fhandle = CreateFile( filename, access, share, secatt, creation, flags, NULL );

        if ( fhandle == INVALID_HANDLE_VALUE )
        {
            return hFalse;
        }

        (*pOut) = hNEW( GetGlobalHeap(), hdFileHandle );
        (*pOut)->fileHandle_ = fhandle;
        (*pOut)->filePos_	= 0;
        (*pOut)->opPending_ = FILEOP_NONE;

        return hTrue;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    HEART_DLLEXPORT
    hBool HEART_API hdFclose( hdFileHandle* pHandle )
    {
        CloseHandle( pHandle->fileHandle_ );

        hDELETE(GetGlobalHeap(), pHandle);
        return hTrue;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    HEART_DLLEXPORT
    hdFileError HEART_API hdFread( hdFileHandle* pHandle, void* pBuffer, hUint32 size, hUint32* read )
    {
        pHandle->operation_.Offset = (DWORD)(pHandle->filePos_ & 0xFFFFFFFF);;
        pHandle->operation_.OffsetHigh = (LONG)((pHandle->filePos_ & 0xFFFFFFFF00000000) >> 32);

        if ( ReadFile( pHandle->fileHandle_, pBuffer, size, read, NULL ) == 0 )
        {
            return FILEERROR_FAILED;
        }

        pHandle->filePos_ += size;
        if ( pHandle->filePos_ > hdFsize( pHandle ) )
        {
            pHandle->filePos_ = hdFsize( pHandle );
        }

        pHandle->opPending_ = FILEOP_READ;
        return FILEERROR_NONE;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    HEART_DLLEXPORT
    hdFileError HEART_API hdFseek( hdFileHandle* pHandle, hUint64 offset, hdSeekOffset from )
    {
        hUint64 size = hdFsize( pHandle );
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

    HEART_DLLEXPORT
    hUint64 HEART_API hdFtell( hdFileHandle* pHandle )
    {
        return pHandle->filePos_;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    HEART_DLLEXPORT
    hUint64 HEART_API hdFsize( hdFileHandle* pHandle )
    {
        return GetFileSize( pHandle->fileHandle_, NULL );
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    HEART_DLLEXPORT
    hdFileError HEART_API hdFwrite( hdFileHandle* pHandle, const void* pBuffer, hUint32 size, hUint32* written )
    {
        pHandle->operation_.Offset = (DWORD)(pHandle->filePos_ & 0xFFFFFFFF);;
        pHandle->operation_.OffsetHigh = (LONG)((pHandle->filePos_ & 0xFFFFFFFF00000000) >> 32);

        if ( WriteFile( pHandle->fileHandle_, pBuffer, size, written, NULL ) == 0 )
        {
            return FILEERROR_FAILED;
        }

        pHandle->filePos_ += size;
        if ( pHandle->filePos_ > hdFsize( pHandle ) )
        {
            pHandle->filePos_ = hdFsize( pHandle );
        }

        pHandle->opPending_ = FILEOP_WRITE;
        return FILEERROR_NONE;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    HEART_DLLEXPORT
    void HEART_API hdEnumerateFiles( const hChar* path, hdEnumerateFilesCallback fn )
    {
        WIN32_FIND_DATA found;
        hUint32 searchPathLen = hStrLen(path) + 3;
        hChar* searchPath = (hChar*)alloca( searchPathLen );

        hStrCopy( searchPath, searchPathLen, path );
        hStrCat( searchPath, searchPathLen, "/*" );

        HANDLE searchHandle = FindFirstFile( searchPath, &found );

        if ( searchHandle == INVALID_HANDLE_VALUE )
            return;

        do 
        {
            hdFileHandleInfo info;
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

    HEART_DLLEXPORT
    hdFileStat HEART_API hdFstat( hdFileHandle* handle )
    {
        hdFileStat stat;
        BY_HANDLE_FILE_INFORMATION fileInfo;

        GetFileInformationByHandle(handle->fileHandle_, &fileInfo);

        stat.createTime_     = (((hUint64)fileInfo.ftCreationTime.dwHighDateTime) << 32)   | fileInfo.ftCreationTime.dwLowDateTime;
        stat.lastModTime_    = (((hUint64)fileInfo.ftLastWriteTime.dwHighDateTime) << 32)  | fileInfo.ftLastWriteTime.dwLowDateTime;
        stat.lastAccessTime_ = (((hUint64)fileInfo.ftLastAccessTime.dwHighDateTime) << 32) | fileInfo.ftLastAccessTime.dwLowDateTime;

        return stat;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    HEART_DLLEXPORT
    void        HEART_API hdCreateDirectory( const hChar* path )
    {
        CreateDirectory(path, NULL);
    }

}
