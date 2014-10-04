/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#include "base/hDeviceFileSystem.h"
#include "pal/hMutex.h"
#include "base/hStringUtil.h"

namespace Heart {
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

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void HEART_API hdMountPoint(const hChar* path, const hChar* mount) {
    hcAssert(hdIsAbsolutePath(path));
    g_fileSystemInfo.mount(mount, path);
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void HEART_API hdUnmountPoint(const hChar* mount) {
    g_fileSystemInfo.unmount(mount);
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void HEART_API hdGetSystemPath(const hChar* path, hChar* outdir, hUint size) {
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

hUint HEART_API hdGetSystemPathSize(const hChar* path) {
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

}