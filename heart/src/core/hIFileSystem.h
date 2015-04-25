/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#pragma once

#include "base/hTypes.h"
#include "base/hFunctor.h"

namespace Heart
{
    class hIFile;

    enum hFileMode
    {
        FILEMODE_READ,
        FILEMODE_WRITE,

        FILEMODE_MAX
    };

    enum class FileError {
        Ok = 0,
        Pending,
        Failed,
        EndOfFile,
    };

    enum class FileEntryType {
        Dir = 0x80,
        File = 0x40,
        SymLink = 0x20,
    };

#define HEART_MAX_PATH (2048)

    struct hFileInfo
    {
        const hChar*	path_;
        const hChar*	name_;
        hByte			directory_;
    };


    typedef struct hFile*   hFileHandle;
    typedef struct hFileOp* hFileOpHandle;

    struct hFileInfo2
    {
        const hChar*	path_;
        const hChar*	name_;
        hByte			directory_;
    };

    struct hFileStat {
        hUint64 filesize;
        hTime   modifiedDate;
    };

    struct hDirEntry {
        hChar filename[HEART_MAX_PATH];
        hUint32 typeFlags; // of FileEntryType
    };

    struct hFileSystemInterface {
        hBool (HEART_API *hrt_initialise_filesystem)();

        FileError (HEART_API *hrt_fileOpComplete)(hFileOpHandle);
        FileError (HEART_API *hrt_fileOpWait)(hFileOpHandle);
        void (HEART_API *hrt_fileOpClose)(hFileOpHandle);

        hFileOpHandle (HEART_API *hrt_openFile)(const hChar* filename, hInt mode, hFileHandle* outhandle);
        void (HEART_API *hrt_closeFile)(hFileHandle);
        hFileOpHandle (HEART_API *hrt_openDir)(const hChar* path, hFileHandle* outhandle);
        hFileOpHandle (HEART_API *hrt_readDir)(hFileHandle dir, hDirEntry* out);
        void (HEART_API *hrt_closeDir)(hFileHandle dir);

        hFileOpHandle (HEART_API *hrt_freadAsync)(hFileHandle file, void* buffer, hSize_t size, hUint64 offset);
        hFileOpHandle (HEART_API *hrt_fwriteAsync)(hFileHandle file, const void* buffer, hSize_t size, hUint64 offset);
        hFileOpHandle (HEART_API *hrt_fstatAsync)(hFileHandle file, hFileStat* out);

        void (HEART_API *hrt_mountPoint)(const hChar* path, const hChar* mount);
        void (HEART_API *hrt_unmountPoint)(const hChar* mount);
        hBool (HEART_API *hrt_isAbsolutePath)(const hChar* path);
        void (HEART_API *hrt_getCurrentWorkingDir)(hChar* out, hUint bufsize);
        void (HEART_API *hrt_getProcessDirectory)(hChar* outdir, hUint size);

    };

    hBool loadFileSystemInterface(const char* component_name, hFileSystemInterface* out);

    class hIFileSystem {
    public:
        hIFileSystem(const hFileSystemInterface& in_fn) {
            fn = in_fn;
        }
        hBool initialiseFilesystem() {
            return fn.hrt_initialise_filesystem();
        }
        FileError fileOpComplete(hFileOpHandle handle) {
            return fn.hrt_fileOpComplete(handle);
        }
        FileError fileOpWait(hFileOpHandle handle) {
            return fn.hrt_fileOpWait(handle);
        }
        void fileOpClose(hFileOpHandle handle) {
            fn.hrt_fileOpClose(handle);
        }
        hFileOpHandle openFile(const hChar* filename, hInt mode, hFileHandle* outhandle) {
            return fn.hrt_openFile(filename, mode, outhandle);
        }
        void closeFile(hFileHandle handle) {
            fn.hrt_closeFile(handle);
        }
        hFileOpHandle openDir(const hChar* path, hFileHandle* outhandle) {
            return fn.hrt_openDir(path, outhandle);
        }
        hFileOpHandle readDir(hFileHandle dir, hDirEntry* out) {
            return fn.hrt_readDir(dir, out);
        }
        void closeDir(hFileHandle dir) {
            fn.hrt_closeDir(dir);        
        }
        hFileOpHandle freadAsync(hFileHandle file, void* buffer, hSize_t size, hUint64 offset) {
            return fn.hrt_freadAsync(file, buffer, size, offset);
        }
        hFileOpHandle fwriteAsync(hFileHandle file, const void* buffer, hSize_t size, hUint64 offset) {
            return fn.hrt_fwriteAsync(file, buffer, size, offset);
        }
        hFileOpHandle fstatAsync(hFileHandle file, hFileStat* out) {
            return fn.hrt_fstatAsync(file, out);
        }
        void  mountPoint(const hChar* path, const hChar* mount) {
            fn.hrt_mountPoint(path, mount);
        }
        void  unmountPoint(const hChar* mount) {
            fn.hrt_unmountPoint(mount);
        }
        hBool isAbsolutePath(const hChar* path) {
            return fn.hrt_isAbsolutePath(path);        
        }
        void  getCurrentWorkingDir(hChar* out, hUint bufsize) {
            fn.hrt_getCurrentWorkingDir(out, bufsize);
        }
        void  getProcessDirectory(hChar* outdir, hUint size) {
            fn.hrt_getProcessDirectory(outdir, size);
        }

    private:
        hFileSystemInterface fn;
    };
}
