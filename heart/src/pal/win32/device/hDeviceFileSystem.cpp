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

class hdFileSystemMountInfo
{
public:
    hdFileSystemMountInfo() 
        : mountCount_(0)
    {}
    ~hdFileSystemMountInfo() {
        while (mountCount_) {
            unmount(mounts_[0].mountName_);
        }
    }

    static const hUint s_maxMountLen=8;
    static const hUint s_maxMounts=64;

    void lock() { mountAccessMutex_.Lock(); }
    void unlock() { mountAccessMutex_.Unlock(); }
    void mount(const hChar* mount, const hChar* path) {
        mountAccessMutex_.Lock();
        hcAssert(findMountIndex(mount)==s_maxMounts);

        hStrNCopy(mounts_[mountCount_].mountName_, s_maxMountLen, mount);
        mounts_[mountCount_].pathLen_=hMax(hStrLen(path),1);
        mounts_[mountCount_].mountPath_=hNEW_ARRAY(hChar, mounts_[mountCount_].pathLen_+1);
        hStrCopy(mounts_[mountCount_].mountPath_, mounts_[mountCount_].pathLen_+1, path);
        if (mounts_[mountCount_].mountPath_[mounts_[mountCount_].pathLen_-1]=='/' || mounts_[mountCount_].mountPath_[mounts_[mountCount_].pathLen_-1]=='\\') {
            mounts_[mountCount_].mountPath_[mounts_[mountCount_].pathLen_-1]=0;
            --mounts_[mountCount_].pathLen_;
        }
        hcPrintf("Mounting [%s] to %s:/", mounts_[mountCount_].mountPath_, mounts_[mountCount_].mountName_);
        ++mountCount_;
        mountAccessMutex_.Unlock();
    }
    void unmount(const hChar* mount) {
        mountAccessMutex_.Lock();
        if (mountCount_ > 0) {
            hUint midx=findMountIndex(mount);
            hcPrintf("Unmounting [%s] from %s:/", mounts_[midx].mountPath_, mounts_[midx].mountName_);
            hDELETE_SAFE(mounts_[midx].mountPath_);
            --mountCount_;
            mounts_[midx]=mounts_[mountCount_];
        }
        mountAccessMutex_.Unlock();
    }
    hUint getMountPathLenght(const hChar* mount) {
        mountAccessMutex_.Lock();
        hUint len=0;
        hUint midx=findMountIndex(mount);
        if (midx < s_maxMounts) {
            len = mounts_[midx].pathLen_;
        }
        mountAccessMutex_.Unlock();
        return len;
    }
    hUint getMount(const hChar* mount, hChar* outpath, hUint size) {
        hcAssert(outpath && mount);
        hUint ret=0;
        mountAccessMutex_.Lock();
        outpath[0]=0;
        hUint midx=findMountIndex(mount);
        if (midx < s_maxMounts) {
            ret=mounts_[midx].pathLen_;
            hStrCopy(outpath, size, mounts_[midx].mountPath_);
        }
        mountAccessMutex_.Unlock();
        return ret;
    }

private:

    struct hdMount
    {
        hChar   mountName_[s_maxMountLen];
        hUint   pathLen_;
        hChar*  mountPath_;
    };

    hUint findMountIndex(const hChar* mount) const {
        for (hUint i=0; i<mountCount_; ++i) {
            if (hStrCmp(mount, mounts_[i].mountName_) == 0) {
                return i;
            }
        }
        return s_maxMounts;
    }

    hdW32Mutex mountAccessMutex_;
    hUint      mountCount_;
    hdMount    mounts_[s_maxMounts];
};

    hdFileSystemMountInfo   g_fileSystemInfo;
    hdW32Mutex              g_mmapAccessMutex;
    hBool                           g_mmapInit=hFalse;
    hLinkedList<hdMemoryMappedFile> g_freeList;
    hLinkedList<hdMemoryMappedFile> g_usedList;
    hdMemoryMappedFile              g_mmapFiles[128];

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    HEART_DLLEXPORT
    hBool HEART_API hdFopen(const hChar* filename, const hChar* mode, hdFileHandle* pOut)
    {
        DWORD access = 0;
        DWORD share = 0;// < always ZERO, dont let things happen to file in use!
        LPSECURITY_ATTRIBUTES secatt = NULL;// could be a prob if passed across threads>?
        DWORD creation = 0;
        DWORD flags = FILE_ATTRIBUTE_NORMAL;
        HANDLE fhandle;

        hcAssert( pOut != hNullptr );

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

        g_fileSystemInfo.lock();
        hUint syspathlen=hdGetSystemPathSize(filename);
        hChar* syspath=(hChar*)hAlloca(syspathlen+1);
        hdGetSystemPath(filename, syspath, syspathlen+1);
        g_fileSystemInfo.unlock();
        
        fhandle = CreateFile(syspath, access, share, secatt, creation, flags, hNullptr);

        if ( fhandle == INVALID_HANDLE_VALUE )
        {
            return hFalse;
        }

        pOut->fileHandle_ = fhandle;
        pOut->filePos_	= 0;
        pOut->opPending_ = FILEOP_NONE;

        return hTrue;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    HEART_DLLEXPORT
    hBool HEART_API hdFclose(hdFileHandle* pHandle)
    {
        CloseHandle( pHandle->fileHandle_ );
        pHandle->fileHandle_=INVALID_HANDLE_VALUE;

        return hTrue;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    HEART_DLLEXPORT
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

    HEART_DLLEXPORT
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

    HEART_DLLEXPORT
    hUint64 HEART_API hdFtell(hdFileHandle* pHandle)
    {
        return pHandle->filePos_;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    HEART_DLLEXPORT
    hUint64 HEART_API hdFsize(hdFileHandle* pHandle)
    {
        return GetFileSize( pHandle->fileHandle_, NULL );
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    HEART_DLLEXPORT
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

    HEART_DLLEXPORT
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

    HEART_DLLEXPORT
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

    HEART_DLLEXPORT
    void        HEART_API hdCreateDirectory(const hChar* path) {
        g_fileSystemInfo.lock();
        hUint syspathlen=hdGetSystemPathSize(path);
        hChar* syspath=(hChar*)hAlloca(syspathlen+1);
        hdGetSystemPath(path, syspath, syspathlen+1);
        g_fileSystemInfo.unlock();
        CreateDirectory(syspath, NULL);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdMountPoint(const hChar* path, const hChar* mount) {
        if (hdIsAbsolutePath(path)) {
            hUint syspathlen=hdGetSystemPathSize(path);
            hChar* syspath=(hChar*)hAlloca(syspathlen+1);
            hdGetSystemPath(path, syspath, syspathlen+1);
            g_fileSystemInfo.mount(mount, syspath);
        } else {
            g_fileSystemInfo.mount(mount, path);
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdUnmountPoint(const hChar* mount) {
        g_fileSystemInfo.unmount(mount);
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

    void hdGetSystemPath(const hChar* path, hChar* outdir, hUint size) {
        g_fileSystemInfo.lock();
        hChar mnt[hdFileSystemMountInfo::s_maxMountLen+1];
        const hChar* term=hStrChr(path, ':');
        if (term && (hUint)((hPtrdiff_t)term-(hPtrdiff_t)path) < hdFileSystemMountInfo::s_maxMountLen) {
            hUint os=(hUint)((hPtrdiff_t)term-(hPtrdiff_t)path);
            hStrNCopy(mnt, os+1, path);
            hUint mountlen=g_fileSystemInfo.getMount(mnt, outdir, size);
            if (mountlen==0) {
                hStrCopy(outdir, size, path);
            } else {
                hStrCopy(outdir+mountlen, size-mountlen, path+os+1);
            }
        } else {
            hStrCopy(outdir, size, path);
        }
        g_fileSystemInfo.unlock();
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hUint hdGetSystemPathSize(const hChar* path) {
        g_fileSystemInfo.lock();
        hChar mnt[hdFileSystemMountInfo::s_maxMountLen+1];
        hUint ret=hStrLen(path);
        const hChar* term=hStrChr(path, ':');
        if (term && (hUint)((hPtrdiff_t)term-(hPtrdiff_t)path) < hdFileSystemMountInfo::s_maxMountLen) {
            hStrNCopy(mnt, (hUint)((hPtrdiff_t)term-(hPtrdiff_t)path)+1, path);
            ret+=g_fileSystemInfo.getMountPathLenght(mnt);
        }
        g_fileSystemInfo.unlock();
        return ret;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hdMemoryMappedFile* hdMMap(hdFileHandle* handle, hSizeT offset, hSizeT size, hdMMapMode mode) {
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
        if (sizeof(hSizeT)==8) {
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

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdUnmap(hdMemoryMappedFile* mmapview) {
        UnmapViewOfFile(mmapview->basePtr_);
        CloseHandle(mmapview->mmap_);

        mmapview->basePtr_=hNullptr;
        mmapview->mmap_=INVALID_HANDLE_VALUE;

        g_usedList.remove(mmapview);
        g_freeList.addHead(mmapview);
    }

}
