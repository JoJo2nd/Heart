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

    hFUNCTOR_TYPEDEF(hBool(*)(const hFileInfo*), hEnumerateFilesCallback);

    class  hIFileSystem
    {
    public:
        virtual ~hIFileSystem() {}

        virtual hIFile*	OpenFile( const hChar* filename, hFileMode mode ) const = 0;
        virtual void	CloseFile( hIFile* pFile ) const = 0;
        virtual void	EnumerateFiles( const hChar* path, hEnumerateFilesCallback fn ) const = 0;
        virtual void    createDirectory(const hChar* path) = 0;
        virtual hBool   WriteableSystem() const { return hTrue; }

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

    typedef hBool (HEART_API *hEnumerateFilesProc)(const hFileInfo2*);

    struct hFileSystemInterface {
        hBool (HEART_API *hrt_initialise_filesystem)();

        FileError (HEART_API *hrt_fileOpComplete)(hFileOpHandle);
        void (HEART_API *hrt_fileOpWait)(hFileOpHandle);
        void (HEART_API *hrt_fileOpClose)(hFileOpHandle);

        hFileOpHandle (HEART_API *hrt_openFile)(const hChar* filename, hInt mode, hFileHandle* outhandle);
        void (HEART_API *hrt_closeFile)(hFileHandle);
        hFileOpHandle (HEART_API *hrt_openDir)(const hChar* path, hFileHandle* outhandle);
        hFileOpHandle (HEART_API *hrt_readDir)(hFileHandle dir, hDirEntry* out);
        void (HEART_API *hrt_closeDir)(hFileHandle dir);

        hFileOpHandle (HEART_API *hrt_freadAsync)(hFileHandle file, void* buffer, hUint32 size, hUint64 offset);
        hFileOpHandle (HEART_API *hrt_fwriteAsync)(hFileHandle file, const void* buffer, hUint32 size, hUint64 offset);
        hFileOpHandle (HEART_API *hrt_fstatAsync)(hFileHandle file, hFileStat* out);

        void (HEART_API *hrt_mountPoint)(const hChar* path, const hChar* mount);
        void (HEART_API *hrt_unmountPoint)(const hChar* mount);
        void (HEART_API *hrt_getCurrentWorkingDir)(hChar* out, hUint bufsize);
        void (HEART_API *hrt_getProcessDirectory)(hChar* outdir, hUint size);
        /*
            void HEART_API hdGetCurrentWorkingDir(hChar* out, hUint bufsize);
            void HEART_API hdGetProcessDirectory(hChar* outdir, hUint size);
            hBool HEART_API hdIsAbsolutePath(const hChar* path);
            void HEART_API hdGetSystemPath(const hChar* path, hChar* outdir, hUint size);
            hUint HEART_API hdGetSystemPathSize(const hChar* path);
        */
    };

    hBool loadFileSystemInterface(const char* component_name, hFileSystemInterface* out);
}
