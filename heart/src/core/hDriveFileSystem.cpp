/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#include "core/hDriveFileSystem.h"
#include "core/hDriveFile.h"
#include "base/hDeviceFileSystem.h"

namespace Heart
{
namespace 
{
    struct hEnumerateFilesCallbackInfo
    {
        hEnumerateFilesCallback fn_;

        hBool Callback(const hdFileHandleInfo* pInfo )
        {
            hFileInfo fi;
            fi.name_ = pInfo->name_;
            fi.directory_ = pInfo->directory_;
            fi.path_ = pInfo->path_;//+(sizeof(FILE_PREFIX)-1);

            return fn_( &fi );
        }
    };
}

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hIFile* hDriveFileSystem::OpenFile(const hChar* filename, hFileMode mode) const
    {
        hdFileHandle* fh;
        const hChar* devMode;

        if ( mode == FILEMODE_WRITE )
        {
            devMode = "w";
        }
        else if ( mode == FILEMODE_READ )
        {
            devMode = "r";
        }
        else 
        {
            return nullptr;
        }

        if (!(fh = hdFopen(filename, devMode)))
        {
            return nullptr;
        }

        hdFileStat fhstat=hdFstat(fh);
        hUint64 fsize=hdFsize(fh);
        hDriveFile* pFile = new hDriveFile;
        pFile->fileHandle_ = fh;
        pFile->stat_=fhstat;
        pFile->size_=fsize;
        pFile->mmapPos_=0;

        if (mode==FILEMODE_READ) {
            //attempt to memory map the file
            pFile->mmap_=hdMMap(fh, 0, fsize, MMapMode_ReadOnly);
            if (pFile->mmap_) {
                hcPrintf("Opened memory mapped file %s @ 0x%p", filename, hdMMapGetBase(pFile->mmap_));
                hdFclose(pFile->fileHandle_);
            }
        }

        return pFile;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hDriveFileSystem::CloseFile( hIFile* pFile ) const
    {
        if ( !pFile )
        {
            return;
        }
        hDriveFile* f=static_cast<hDriveFile*>(pFile);

        if (f->mmap_) {
            hcPrintf("Closed memory mapped file @ 0x%p", hdMMapGetBase(f->mmap_));
            hdUnmap(f->mmap_);
            f->mmap_=hNullptr;
        } else {
            hdFclose(f->fileHandle_);
        }

        delete pFile;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hDriveFileSystem::EnumerateFiles( const hChar* path, hEnumerateFilesCallback fn ) const
    {
        hEnumerateFilesCallbackInfo cbInfo;
        cbInfo.fn_ = fn;

        hdEnumerateFiles(path, hdEnumerateFilesCallback::bind< hEnumerateFilesCallbackInfo, &hEnumerateFilesCallbackInfo::Callback >( &cbInfo ));
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hDriveFileSystem::createDirectory( const hChar* path )
    {
        hdCreateDirectory(path);
    }

}