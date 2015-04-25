
#include "base/hTypes.h"
#include "base/hMemoryUtil.h"
#include "base/hStringID.h"
#include "base/hMemory.h"
#include "base/hMutex.h"
#include "core/hIFileSystem.h"
#include "base/hStringUtil.h"
#include "base/hMutexAutoScope.h"
#include "base/hUTF8.h"
#include <windows.h>
#include <vector>
#include <algorithm>

namespace Heart {
    struct hFile {
        HANDLE              fileHandle;
    };

    struct hDir : hFile {
        hDirEntry   currentEntry;
    };

    struct hFileOp {
        virtual ~hFileOp() {}
    };
    
    struct hFileOpRW : hFileOp {
        hFileOpRW() {
            hZeroMem(&operation, sizeof(OVERLAPPED));
            operation.hEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);
        }
        ~hFileOpRW() {
            CloseHandle(operation.hEvent);
        }
        HANDLE              hFile;
        OVERLAPPED          operation;
    };

}

using Heart::hString;
using Heart::hFile;
using Heart::hDir;
using Heart::hFileOp;
using Heart::hFileOpRW;
using Heart::hFileOpHandle;
using Heart::hFileHandle;
using Heart::hDirEntry;
using Heart::FileEntryType;
using Heart::FileError;
using Heart::hFileStat;
using Heart::hMutex;

namespace {
    struct hMount {
        hString mountName;
        hString mountPoint;
    };

    // Dummy op to return if operation completes immediately 
    hFileOp g_syncOp;
    hFileOp g_syncOpEOF;
    std::vector< hMount > g_mounts;
    hMutex g_mountMtx;
}

void* operator new(size_t size){
    return Heart::hMalloc(size);
}

void* operator new[](size_t size) {
    return Heart::hMalloc(size);
}

void operator delete(void* ptr) {
    return Heart::hFree(ptr);
}

void operator delete[](void* ptr) {
    return Heart::hFree(ptr);
}

static hBool isAbsPath(const hChar* in_path) {
    return (in_path[0] != '0' && in_path[1] == ':' && in_path[2] == '\\');
}

static void getExpanedPath(const hChar* in_path, hChar* out_path, hSize_t max_len) {
    Heart::hMutexAutoScope sentry(&g_mountMtx);
    Heart::hStrCopy(out_path, HEART_MAX_PATH, in_path);
    do {
        hSize_t offset = 0;
        for (const auto& mnt : g_mounts) {
            if (Heart::hStrNCmp(mnt.mountName.c_str(), out_path, mnt.mountName.size()) == 0) {
                offset = mnt.mountName.size();
                auto plen = mnt.mountPoint.size();
                auto slen = Heart::hStrLen(out_path);
                if ((plen + slen + 1) > HEART_MAX_PATH) {
                    plen = HEART_MAX_PATH-slen;
                }
                Heart::hMemMove(out_path+plen, out_path+offset, (slen-offset)+1);
                Heart::hStrNCopy(out_path, plen, mnt.mountPoint.c_str());
                break;
            }
        }
    } while (!isAbsPath(out_path));
}

static hSize_t getExpanedPathUC2(const hChar* in_path, wchar_t* out_path, hSize_t max_len) {
    Heart::hMutexAutoScope sentry(&g_mountMtx);
     hChar expaned[HEART_MAX_PATH] = { 0 };
//     Heart::hStrCopy(expaned, HEART_MAX_PATH, in_path);
//     do {
//         hSize_t offset = 0;
//         for (const auto& mnt : g_mounts) {
//             if (Heart::hStrNCmp(mnt.mountName.c_str(), expaned, mnt.mountName.size()) == 0) {
//                 Heart::hStrCopy(expaned, HEART_MAX_PATH, mnt.mountPoint.c_str());
//                 offset = mnt.mountName.size();
//                 break;
//             }
//         }
//         Heart::hStrCat(expaned, HEART_MAX_PATH, in_path + offset);
//     } while (!isAbsPath(expaned));
    getExpanedPath(in_path, expaned, HEART_MAX_PATH);
    return Heart::hUTF8::utf8_to_uc2(expaned, (hUint16*)out_path, max_len);
}

template < hSize_t t_array_size >
static hSize_t getExpanedPathUC2(const hChar* in_path, wchar_t (&out_path)[t_array_size]) {
    return getExpanedPathUC2(in_path, out_path, t_array_size);
}

HEART_C_EXPORT
void HEART_API hrt_mountPoint(const hChar* path, const hChar* mount);
HEART_C_EXPORT
void HEART_API hrt_getCurrentWorkingDir(hChar* out, hUint bufsize);

HEART_C_EXPORT
hBool HEART_API hrt_initialise_filesystem() {
    hChar pwd[HEART_MAX_PATH];
    hrt_getCurrentWorkingDir(pwd, HEART_MAX_PATH);
    hrt_mountPoint(pwd, "/");
    return hTrue;
}

HEART_C_EXPORT
FileError HEART_API hrt_fileOpComplete(hFileOpHandle in_op) {
    if (&g_syncOp == in_op) {
        return FileError::Ok;
    }
    if (&g_syncOpEOF == in_op) {
        return FileError::EndOfFile;
    }

    auto* op = static_cast<hFileOpRW*>(in_op);
    DWORD xferred;
    if (GetOverlappedResult(op->hFile, &op->operation, &xferred, FALSE) == FALSE) {
        auto LastErr = GetLastError();
        if (LastErr == ERROR_IO_INCOMPLETE) {
            return FileError::Pending;
        } if (LastErr == ERROR_HANDLE_EOF) {
            return FileError::EndOfFile;
        } else {
            return FileError::Failed;
        }
    }
    //completed
    return FileError::Ok;
}

HEART_C_EXPORT
FileError HEART_API hrt_fileOpWait(hFileOpHandle in_op) {
    if (in_op == nullptr) {
        return FileError::Failed;
    }
    if (&g_syncOp == in_op) {
        return FileError::Ok;
    }
    if (&g_syncOpEOF == in_op) {
        return FileError::EndOfFile;
    }

    auto* op = static_cast<hFileOpRW*>(in_op);
    DWORD xferred;
    if (GetOverlappedResult(op->hFile, &op->operation, &xferred, TRUE) == FALSE) {
        return FileError::Failed;
    }
    //completed
    return FileError::Ok;
}

HEART_C_EXPORT
void HEART_API hrt_fileOpClose(hFileOpHandle in_op) {
    if (&g_syncOp == in_op || &g_syncOpEOF == in_op || !in_op) {
        return;
    }

    delete in_op;
}

HEART_C_EXPORT
hFileOpHandle HEART_API hrt_openFile(const hChar* filename, hInt mode, hFileHandle* outhandle) {
    DWORD access = 0;
    DWORD share = 0;// < always ZERO, dont let things happen to file in use!
    LPSECURITY_ATTRIBUTES secatt = NULL;// could be a prob if passed across threads>?
    DWORD creation = 0;
    DWORD flags = FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED;
    HANDLE fhandle;

    if (mode == Heart::FILEMODE_READ)
    {
        access = GENERIC_READ;
        creation = OPEN_EXISTING;
    }
    else if (mode == Heart::FILEMODE_WRITE)
    {
        access = GENERIC_WRITE;
        creation = CREATE_ALWAYS;
    }

    wchar_t filename_wide[HEART_MAX_PATH];
    getExpanedPathUC2(filename, filename_wide);
    fhandle = CreateFileW(filename_wide, access, share, secatt, creation, flags, nullptr);

    if (fhandle == INVALID_HANDLE_VALUE)
    {
        (*outhandle) = nullptr;
        return nullptr;
    }

    (*outhandle) = new hFile();
    (*outhandle)->fileHandle = fhandle;

    return &g_syncOp;
}

HEART_C_EXPORT
void HEART_API hrt_closeFile(hFileHandle handle) {
    if (handle && handle->fileHandle != INVALID_HANDLE_VALUE) {
        CloseHandle(handle->fileHandle);
    }

    delete handle;
}

HEART_C_EXPORT
hFileOpHandle HEART_API hrt_openDir(const hChar* path, hFileHandle* outhandle) {
    WIN32_FIND_DATAW found;
    auto* dir = new hDir();
    wchar_t path_wide[HEART_MAX_PATH];
    getExpanedPathUC2(path, path_wide);
    dir->fileHandle = FindFirstFileW(path_wide, &found);
    //Heart::hStrCopy(dir->currentEntry.filename, HEART_MAX_PATH, found.cFileName);
    Heart::hUTF8::uc2_to_utf8((hUint16*)found.cFileName, dir->currentEntry.filename, hStaticArraySize(dir->currentEntry.filename));
    dir->currentEntry.typeFlags = 0;
    if (found.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
        dir->currentEntry.typeFlags |= (hUint32)FileEntryType::Dir;
    } else {
        dir->currentEntry.typeFlags |= (hUint32)FileEntryType::File;
    }
    if (found.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) {
        dir->currentEntry.typeFlags |= (hUint32)FileEntryType::SymLink;
    }
    *outhandle = dir;
    return &g_syncOp;
}

HEART_C_EXPORT
hFileOpHandle HEART_API hrt_readDir(hFileHandle dirhandle, hDirEntry* out) {
    auto* dir = static_cast<hDir*>(dirhandle);
    if (dir->fileHandle == INVALID_HANDLE_VALUE) {
        return &g_syncOpEOF;
    }

    *out = dir->currentEntry;
    WIN32_FIND_DATAW found;
    if (FindNextFileW(dir->fileHandle, &found)) {
        Heart::hUTF8::uc2_to_utf8((hUint16*)found.cFileName, dir->currentEntry.filename, hStaticArraySize(dir->currentEntry.filename));
        dir->currentEntry.typeFlags = 0;
        if (found.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            dir->currentEntry.typeFlags |= (hUint32)FileEntryType::Dir;
        }
        else {
            dir->currentEntry.typeFlags |= (hUint32)FileEntryType::File;
        }
        if (found.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) {
            dir->currentEntry.typeFlags |= (hUint32)FileEntryType::SymLink;
        }
    } else {
        CloseHandle(dir->fileHandle);
        dir->fileHandle = INVALID_HANDLE_VALUE;
    }

    return &g_syncOp;
}

HEART_C_EXPORT
void HEART_API hrt_closeDir(hFileHandle dir) {
    delete dir;
}

HEART_C_EXPORT
hFileOpHandle HEART_API hrt_freadAsync(hFileHandle file, void* buffer, hSize_t size, hUint64 offset) {
    auto* new_op = new hFileOpRW();
    new_op->hFile = file->fileHandle;
    new_op->operation.Offset = offset & 0xFFFFFFFF;
    new_op->operation.OffsetHigh = (offset & ((hUint64)0xFFFFFFFF << 32)) >> 32;
    auto Completed = ReadFile(file->fileHandle, buffer, (DWORD)size, nullptr, &new_op->operation);
    if (Completed) {
        delete new_op;
        return &g_syncOp;
    }
    return new_op;
}

HEART_C_EXPORT
hFileOpHandle HEART_API hrt_fwriteAsync(hFileHandle file, const void* buffer, hSize_t size, hUint64 offset) {
    auto* new_op = new hFileOpRW();
    new_op->hFile = file->fileHandle;
    new_op->operation.Offset = offset & 0xFFFFFFFF;
    new_op->operation.OffsetHigh = (offset & ((hUint64)0xFFFFFFFF << 32)) >> 32;
    auto Completed = WriteFile(file->fileHandle, buffer, (DWORD)size, nullptr, &new_op->operation);
    if (Completed) {
        delete new_op;
        return &g_syncOp;
    }
    return new_op;
}

static time_t FILETIMETotime_t(FILETIME const& ft) {
    ULARGE_INTEGER ull;
    ull.LowPart = ft.dwLowDateTime;
    ull.HighPart = ft.dwHighDateTime;
    return ull.QuadPart / 10000000ULL - 11644473600ULL;
}

HEART_C_EXPORT
hFileOpHandle HEART_API hrt_fstatAsync(hFileHandle filename, hFileStat* out) {
    BY_HANDLE_FILE_INFORMATION fileinfo;
    GetFileInformationByHandle(filename->fileHandle, &fileinfo);
    out->filesize = ((hUint64)fileinfo.nFileSizeHigh << 32)| fileinfo.nFileSizeLow;
    out->modifiedDate = FILETIMETotime_t(fileinfo.ftLastWriteTime);
    return &g_syncOp;
}

HEART_C_EXPORT
hBool HEART_API hrt_isAbsolutePath(const hChar* path) {
    if (!path) {
        return hFalse;
    }
    return path[0] == '/';
}

HEART_C_EXPORT
void HEART_API hrt_mountPoint(const hChar* path, const hChar* mount) {
    Heart::hMutexAutoScope sentry(&g_mountMtx);
    hcAssert(hrt_isAbsolutePath(mount));
    hChar expath[HEART_MAX_PATH];
    getExpanedPath(path, expath, HEART_MAX_PATH);
    hcAssert(isAbsPath(expath));
    hMount mnt;
    mnt.mountName = mount;
    mnt.mountPoint = expath;
    g_mounts.push_back(mnt);
    std::stable_sort(g_mounts.begin(), g_mounts.end(), [](const hMount& lhs, const hMount& rhs) {
            return lhs.mountName.size() > rhs.mountName.size();
        });
}

HEART_C_EXPORT
void HEART_API hrt_unmountPoint(const hChar* mount) {
    Heart::hMutexAutoScope sentry(&g_mountMtx);
    std::remove_if(g_mounts.begin(), g_mounts.end(), [=](const hMount& rhs) {
            return Heart::hStrCmp(mount, rhs.mountName.c_str()) == 0;
        });
}

HEART_C_EXPORT
void HEART_API hrt_getCurrentWorkingDir(hChar* out, hUint bufsize) {
    wchar_t wd[HEART_MAX_PATH];
    auto len = GetCurrentDirectoryW(HEART_MAX_PATH-1, wd);
    wd[len]='\\';
    wd[len+1]=0;
    Heart::hUTF8::uc2_to_utf8((hUint16*)wd, out, bufsize);
    hcAssert(isAbsPath(out));
}

HEART_C_EXPORT
void HEART_API hrt_getProcessDirectory(hChar* outdir, hUint size) {
    wchar_t pd[HEART_MAX_PATH];
    GetModuleFileNameW(0, pd, HEART_MAX_PATH-1);
    auto* s = wcsrchr(pd, '\\');
    if (s) {
        *(s+1) = 0;
    }
    Heart::hUTF8::uc2_to_utf8((hUint16*)pd, outdir, size);
    hcAssert(isAbsPath(outdir));
}
