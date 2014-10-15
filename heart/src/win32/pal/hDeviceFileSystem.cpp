/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#include "base/hDeviceFileSystem.h"
#include "pal/hMutex.h"
#include "base/hStringUtil.h"

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

    class hdFileHandle {
    public:

        HANDLE              fileHandle_;
        OVERLAPPED          operation_;
        hUint64             filePos_;
        hUint32             opPending_;
    };

    class hdMemoryMappedFile : public hLinkedListElement<hdMemoryMappedFile> {
    public:
        HANDLE  mmap_;
        void*   basePtr_;
    };

    hMutex                          g_mmapAccessMutex;
    hBool                           g_mmapInit=hFalse;
    hLinkedList<hdMemoryMappedFile> g_freeList;
    hLinkedList<hdMemoryMappedFile> g_usedList;
    hdMemoryMappedFile              g_mmapFiles[128];

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    
    hdFileHandle* HEART_API hdFopen(const hChar* filename, const hChar* mode)
    {
        DWORD access = 0;
        DWORD share = 0;// < always ZERO, dont let things happen to file in use!
        LPSECURITY_ATTRIBUTES secatt = NULL;// could be a prob if passed across threads>?
        DWORD creation = 0;
        DWORD flags = FILE_ATTRIBUTE_NORMAL;
        HANDLE fhandle;

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

        hUint syspathlen=hdGetSystemPathSize(filename);
        hChar* syspath=(hChar*)hAlloca(syspathlen+1);
        hdGetSystemPath(filename, syspath, syspathlen+1);
        
        fhandle = CreateFile(syspath, access, share, secatt, creation, flags, hNullptr);

        if ( fhandle == INVALID_HANDLE_VALUE )
        {
            return nullptr;
        }

        hdFileHandle* fd = new hdFileHandle();
        fd->fileHandle_ = fhandle;
        fd->filePos_	= 0;
        fd->opPending_ = FILEOP_NONE;

        return fd;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    
    hBool HEART_API hdFclose(hdFileHandle* pHandle)
    {
        CloseHandle( pHandle->fileHandle_ );
        pHandle->fileHandle_=INVALID_HANDLE_VALUE;

        return hTrue;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    
    hdFileError HEART_API hdFread(hdFileHandle* pHandle, void* pBuffer, hUint32 size, hUint32* read)
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

    
    hdFileError HEART_API hdFseek(hdFileHandle* pHandle, hUint64 offset, hdSeekOffset from)
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

    
    hUint64 HEART_API hdFtell(hdFileHandle* pHandle)
    {
        return pHandle->filePos_;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    
    hUint64 HEART_API hdFsize(hdFileHandle* pHandle)
    {
        return GetFileSize( pHandle->fileHandle_, NULL );
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    
    hdFileError HEART_API hdFwrite(hdFileHandle* pHandle, const void* pBuffer, hUint32 size, hUint32* written)
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

    
    void HEART_API hdEnumerateFiles(const hChar* path, hdEnumerateFilesCallback fn)
    {
        WIN32_FIND_DATA found;

        hUint syspathlen=hdGetSystemPathSize(path);
        hChar* syspath=(hChar*)hAlloca(syspathlen+3);
        hdGetSystemPath(path, syspath, syspathlen+1);

        hStrCat(syspath, syspathlen+3, "/*" );

        HANDLE searchHandle = FindFirstFile(syspath, &found);

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

    
    hdFileStat HEART_API hdFstat(hdFileHandle* handle)
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

    
    void        HEART_API hdCreateDirectory(const hChar* path) {
        hUint syspathlen=hdGetSystemPathSize(path);
        hChar* syspath=(hChar*)hAlloca(syspathlen+1);
        hdGetSystemPath(path, syspath, syspathlen+1);
        CreateDirectory(syspath, NULL);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdGetCurrentWorkingDir(hChar* out, hUint bufsize) {
        hcAssert(out);
        out[bufsize-1] = 0;
        GetCurrentDirectoryA(bufsize-1, out);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdGetProcessDirectory(hChar* out, hUint bufsize) {
        hcAssert(out);
        out[bufsize-1] = 0;
        GetModuleFileNameA(0, out, bufsize-1);
        hChar* path=hStrRChr(out, '\\');
        if (path) {
            *path=0;
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hBool hdIsAbsolutePath(const hChar* path) {
        return hStrChr(path, ':') != hNullptr; //look for ':' e.g. C:/ or data:/ (It's cant be that simple can it?)
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hdMemoryMappedFile* hdMMap(hdFileHandle* handle, hSize_t offset, hSize_t size, hdMMapMode mode) {
        g_mmapAccessMutex.Lock();
        if (!g_mmapInit) {
            for (hUint i=0; i<(hUint)hArraySize(g_mmapFiles); ++i) {
                g_freeList.addHead(&g_mmapFiles[i]);
            }
            g_mmapInit=hTrue;
        }
        g_mmapAccessMutex.Unlock();

        if (g_freeList.getSize()==0) {
            // NoMem
            return hNullptr;
        }

        DWORD protect;
        DWORD access;
        switch(mode) {
        case MMapMode_ReadOnly: protect=PAGE_READONLY; access=FILE_MAP_READ; break;
        case MMapMode_Write:
        case MMapMode_None:
        default: return hNullptr;
        }
        DWORD sizehi=0;
        DWORD sizelow=0;
        DWORD offsethi=0;
        DWORD offsetlow=0;
        if (sizeof(hSize_t)==8) {
            sizehi  = (DWORD)(size >> 32);
            sizelow = (DWORD)size & 0xFFFFFFFF;
            offsethi  = (DWORD)(offset >> 32);
            offsetlow = (DWORD)offset & 0xFFFFFFFF;
        } else {
            sizelow = (DWORD)size;
            offsetlow = (DWORD)offset;
        }

        HANDLE mmaphandle=CreateFileMapping(handle->fileHandle_, NULL, protect, sizehi, sizelow, hNullptr);
        if (mmaphandle==INVALID_HANDLE_VALUE) {
            return hNullptr;
        }
        void* ptr=MapViewOfFile(mmaphandle, access, offsethi, offsetlow, 0);
        if (!ptr) {
            return hNullptr;
        }
        hdMemoryMappedFile* mm=g_freeList.begin();
        g_freeList.remove(mm);
        g_usedList.addHead(mm);

        mm->mmap_=mmaphandle;
        mm->basePtr_=ptr;

        return mm;
    }

    void* hdMMapGetBase(hdMemoryMappedFile *mmap) {
        return mmap ? mmap->basePtr_ : nullptr;
    }

    void hdUnmap(hdMemoryMappedFile* mmapview) {
        UnmapViewOfFile(mmapview->basePtr_);
        CloseHandle(mmapview->mmap_);

        mmapview->basePtr_=hNullptr;
        mmapview->mmap_=INVALID_HANDLE_VALUE;

        g_usedList.remove(mmapview);
        g_freeList.addHead(mmapview);
    }

}
