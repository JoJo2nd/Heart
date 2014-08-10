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

#include "base/hDeviceFileSystem.h"
#include "pal/hMutex.h"
#include "base/hStringUtil.h"
#include <ftw.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>

namespace Heart {
    enum hdFileOp
    {
        FILEOP_NONE,
        FILEOP_READ,
        FILEOP_WRITE,
        FILEOP_SEEK,

        FILEOP_MAX
    };

struct hdMemoryMappedFile {
    void* mmap_;
    hUint64 len_;
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
        mounts_[mountCount_].mountPath_=new hChar[mounts_[mountCount_].pathLen_+1];
        hStrCopy(mounts_[mountCount_].mountPath_, mounts_[mountCount_].pathLen_+1, path);
        if (mounts_[mountCount_].mountPath_[mounts_[mountCount_].pathLen_-1]=='/' || mounts_[mountCount_].mountPath_[mounts_[mountCount_].pathLen_-1]=='\\') {
            mounts_[mountCount_].mountPath_[mounts_[mountCount_].pathLen_-1]=0;
            --mounts_[mountCount_].pathLen_;
        }
        hcPrintf("Mounting [%s] to /%s", mounts_[mountCount_].mountPath_, mounts_[mountCount_].mountName_);
        ++mountCount_;
        mountAccessMutex_.Unlock();
    }
    void unmount(const hChar* mount) {
        mountAccessMutex_.Lock();
        if (mountCount_ > 0) {
            hUint midx=findMountIndex(mount);
            hcPrintf("Unmounting [%s] from /%s", mounts_[midx].mountPath_, mounts_[midx].mountName_);
            delete mounts_[midx].mountPath_; mounts_[midx].mountPath_ = nullptr;
            --mountCount_;
            mounts_[midx]=mounts_[mountCount_];
        }
        mountAccessMutex_.Unlock();
    }
    hUint getMountPathlength(const hChar* mount) {
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

    struct hdMount {
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

    hMutex     mountAccessMutex_;
    hUint      mountCount_;
    hdMount    mounts_[s_maxMounts];
};

    hdFileSystemMountInfo           g_fileSystemInfo;
    hMutex                          g_mmapAccessMutex;
    hBool                           g_mmapInit=hFalse;

    class hdFileHandle {
    public:
        FILE*   file_;
    };

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    
    hdFileHandle* HEART_API hdFopen(const hChar* filename, const hChar* mode) {
        g_fileSystemInfo.lock();
        hUint syspathlen=hdGetSystemPathSize(filename);
        hChar* syspath=(hChar*)hAlloca(syspathlen+1);
        hdGetSystemPath(filename, syspath, syspathlen+1);
        g_fileSystemInfo.unlock();
        
        FILE* f = fopen(syspath, mode);
        if (!f){
            return nullptr;
        }

        hdFileHandle* out = nullptr;
        out = new hdFileHandle;
        out->file_ = f;

        return out;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    
    hBool HEART_API hdFclose(hdFileHandle* handle) {
        if (!handle) {
            return hFalse;
        }
        fclose(handle->file_);
        delete handle;
        return hTrue;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    
    hdFileError HEART_API hdFread(hdFileHandle* handle, void* buffer, hUint32 size, hUint32* read) {
        hSize_t read_bytes = fread(handle->file_, 1, size, handle->file_);
        return read_bytes == size ? FILEERROR_NONE : FILEERROR_FAILED;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    
    hdFileError HEART_API hdFseek(hdFileHandle* handle, hUint64 offset, hdSeekOffset from) {
        int whence = SEEK_CUR;
        switch(from) {
        case SEEKOFFSET_BEGIN: whence = SEEK_SET; break;
        case SEEKOFFSET_END: whence = SEEK_END; break;
        default:
            break;
        }
        return fseek(handle->file_, offset, whence) == 0 ? FILEERROR_NONE : FILEERROR_FAILED;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    
    hUint64 HEART_API hdFtell(hdFileHandle* handle) {
        return ftell(handle->file_);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    
    hUint64 HEART_API hdFsize(hdFileHandle* handle) {
        struct stat info;
        fstat(fileno(handle->file_), &info);
        return (hUint64)info.st_size;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    
    hdFileError HEART_API hdFwrite(hdFileHandle* handle, const void* buffer, hUint32 size, hUint32* written) {
        *written = fwrite(buffer, 1, size, handle->file_);
        return *written == size ? FILEERROR_NONE : FILEERROR_FAILED;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    static hdEnumerateFilesCallback directoryEnumeratorProc;
    int directoryEnumerator(const char *fpath, const struct stat *sb, int typeflag) {
        hdFileHandleInfo finfo;
        finfo.path_ = fpath;
        finfo.name_ = hStrRChr(fpath, '/');
        finfo.name_ = finfo.name_ ? finfo.name_ : fpath;    
        finfo.directory_ = typeflag == FTW_D;
        if (typeflag == FTW_F || typeflag == FTW_D) {
            directoryEnumeratorProc(&finfo);
        }
        return 0; // return non-zero to stop
    }
    
    void HEART_API hdEnumerateFiles(const hChar* path, hdEnumerateFilesCallback fn) {
        hUint syspathlen=hdGetSystemPathSize(path);
        hChar* syspath=(hChar*)hAlloca(syspathlen+1);
        hdGetSystemPath(path, syspath, syspathlen+1);

        directoryEnumeratorProc = fn;
        ftw(syspath, directoryEnumerator, 32);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    
    hdFileStat HEART_API hdFstat(hdFileHandle* handle) {
        hdFileStat outstat;
        struct stat info;
        fstat(fileno(handle->file_), &info);

        outstat.createTime_     = info.st_ctime;
        outstat.lastModTime_    = info.st_mtime;
        outstat.lastAccessTime_ = info.st_atime;

        return outstat;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    
    void HEART_API hdCreateDirectory(const hChar* path) {
        g_fileSystemInfo.lock();
        hUint syspathlen=hdGetSystemPathSize(path);
        hChar* syspath=(hChar*)hAlloca(syspathlen+1);
        hdGetSystemPath(path, syspath, syspathlen+1);
        g_fileSystemInfo.unlock();
        mkdir(syspath, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
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
        getcwd(out, bufsize-1);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdGetProcessDirectory(hChar* out, hUint bufsize) {
        hcAssert(out);
        hUint64 pid = (hUint64)getpid();
        fprintf(stdout, "Path to current process: '/proc/%llu/'\n", pid);
        snprintf(out, bufsize-1, "/proc/%llu", pid);
        out[bufsize-1] = 0;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hBool hdIsAbsolutePath(const hChar* path) {
        hcAssert(path);
        return path[0] == '/';
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdGetSystemPath(const hChar* path, hChar* outdir, hUint size) {
        hcAssert(path && *path == '/');         // paths must be absolute
        hcAssert(hStrChr(path, ':')==nullptr);  // we don't use the mount:/ style anymore
        ++path;
        g_fileSystemInfo.lock();
        hChar mnt[hdFileSystemMountInfo::s_maxMountLen+1];
        const hChar* term=hStrChr(path, '/');
        if (term && (hUint)((hPtrdiff_t)term-(hPtrdiff_t)path) < hdFileSystemMountInfo::s_maxMountLen) {
            hUint os=(hUint)((hPtrdiff_t)term-(hPtrdiff_t)path);
            hStrNCopy(mnt, os+1, path);
            hUint mountlen=g_fileSystemInfo.getMount(mnt, outdir, size);
            if (mountlen==0) {
                hStrCopy(outdir, size, path-1);
            } else {
                hStrCopy(outdir+mountlen, size-mountlen, path+os);
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
        hcAssert(path && *path == '/');         // paths must be absolute
        hcAssert(hStrChr(path, ':')==nullptr);  // we don't use the mount:/ style anymore
        ++path;
        g_fileSystemInfo.lock();
        hChar mnt[hdFileSystemMountInfo::s_maxMountLen+1];
        hUint ret=hStrLen(path);
        // look for /$mount_name$/
        const hChar* term=hStrChr(path, '/');
        if (term && (hUint)((hPtrdiff_t)term-(hPtrdiff_t)path) < hdFileSystemMountInfo::s_maxMountLen) {
            hStrNCopy(mnt, (hUint)((hPtrdiff_t)term-(hPtrdiff_t)path)+1, path);
            ret+=g_fileSystemInfo.getMountPathlength(mnt);
        }
        g_fileSystemInfo.unlock();
        return ret;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hdMemoryMappedFile* hdMMap(hdFileHandle* handle, hSize_t offset, hSize_t size, hdMMapMode mode) {
        // we only support read only mappings, currently
        int prot;
        int flags;
        switch(mode) {
        case MMapMode_ReadOnly: prot=PROT_READ; flags=MAP_SHARED; break;
        case MMapMode_Write:
        case MMapMode_None:
        default: return nullptr;
        }
        struct stat info;
        int fd = fileno(handle->file_);
        fstat(fd, &info);
        void* mapped = mmap(nullptr, info.st_size, prot, flags, fd, 0);

        hdMemoryMappedFile* mm = new hdMemoryMappedFile;
        mm->mmap_ = mapped;
        mm->len_ = info.st_size;
        return mm;
    }

    void* hdMMapGetBase(hdMemoryMappedFile *mmap) {
        return !mmap ? mmap->mmap_ : nullptr;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hdUnmap(hdMemoryMappedFile* mmapview) {
        if (!mmapview) {
            return;
        }
        munmap(mmapview->mmap_, mmapview->len_);
        delete mmapview;
    }

}
