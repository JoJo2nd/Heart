/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
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

    class hdFileHandle {
    public:
        FILE*   file_;
    };

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    
    hdFileHandle* HEART_API hdFopen(const hChar* filename, const hChar* mode) {
        hUint syspathlen=hdGetSystemPathSize(filename);
        hChar* syspath=(hChar*)hAlloca(syspathlen+1);
        hdGetSystemPath(filename, syspath, syspathlen+1);
        
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
        hUint syspathlen=hdGetSystemPathSize(path);
        hChar* syspath=(hChar*)hAlloca(syspathlen+1);
        hdGetSystemPath(path, syspath, syspathlen+1);
        mkdir(syspath, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
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
