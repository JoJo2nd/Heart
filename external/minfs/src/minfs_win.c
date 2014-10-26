/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#include "minfs.h"
#include "minfs_common.h"
#include <windows.h>
#include <Shlwapi.h>
#include <malloc.h>

static HANDLE open_file_handle_read_only(const char* filepath) {
    DWORD access = GENERIC_READ;
    DWORD share = FILE_SHARE_READ;
    LPSECURITY_ATTRIBUTES secatt = NULL;// could be a prob if passed across threads>?
    DWORD creation = OPEN_EXISTING;
    DWORD flags = FILE_ATTRIBUTE_NORMAL;

    return CreateFile(filepath, access, share, secatt, creation, flags, NULL);
}

static int get_file_attrib(const char* filepath, WIN32_FILE_ATTRIBUTE_DATA* fileinfo) {
    wchar_t* uc2filepath;
    UTF8_TO_UC2_STACK(filepath, uc2filepath);

    if (!GetFileAttributesExW(uc2filepath, GetFileExInfoStandard, fileinfo)) {
        return -1;
    }
    return 0;
}

minfs_uint64_t minfs_get_current_file_time() {
    FILETIME filetime;
    SYSTEMTIME systime;
    GetSystemTime(&systime);
    SystemTimeToFileTime(&systime, &filetime);
    return ((((minfs_uint64_t)filetime.dwHighDateTime) << 32) | filetime.dwLowDateTime);
}

minfs_uint64_t minfs_get_file_mdate(const char* filepath) {
    WIN32_FILE_ATTRIBUTE_DATA fileinfo;
    if (get_file_attrib(filepath, &fileinfo) != 0) {
        return 0;
    }
    return ((((minfs_uint64_t)fileinfo.ftLastWriteTime.dwHighDateTime) << 32)  | fileinfo.ftLastWriteTime.dwLowDateTime);
}

minfs_uint64_t minfs_get_file_size(const char* filepath) {
    WIN32_FILE_ATTRIBUTE_DATA fileinfo;
    if (get_file_attrib(filepath, &fileinfo) != 0) {
        return 0;
    }
    return ((((minfs_uint64_t)fileinfo.nFileSizeHigh) << 32)  | fileinfo.nFileSizeLow);
}

int minfs_path_exist(const char* filepath) {
    wchar_t* uc2filepath;
    UTF8_TO_UC2_STACK(filepath, uc2filepath);
    return GetFileAttributesW(uc2filepath) != INVALID_FILE_ATTRIBUTES;
}

int minfs_is_file(const char* filepath) {
    WIN32_FILE_ATTRIBUTE_DATA fileinfo;
    if (get_file_attrib(filepath, &fileinfo) != 0) {
        return 0;
    }
    return (fileinfo.dwFileAttributes & (FILE_ATTRIBUTE_DIRECTORY)) == 0;
}

int minfs_is_directory(const char* filepath) {
    WIN32_FILE_ATTRIBUTE_DATA fileinfo;
    if (get_file_attrib(filepath, &fileinfo) != 0) {
        return 0;
    }
    return (fileinfo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
}

 int minfs_is_sym_link(const char* filepath) {
    WIN32_FILE_ATTRIBUTE_DATA fileinfo;
    if (get_file_attrib(filepath, &fileinfo) != 0) {
        return 0;
    }
    return (fileinfo.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT);
}

int minfs_create_directories(const char* filepath) {
    wchar_t* mrk;
    wchar_t* uc2filepath;
    UTF8_TO_UC2_STACK(filepath, uc2filepath);
    while (mrk = wcschr(uc2filepath, L'\\')) { 
        *mrk = '/';
    }
    mrk = uc2filepath;
    while (mrk = wcschr(mrk, '/')) {
        *mrk = 0;
        if (GetFileAttributesW(uc2filepath) == INVALID_FILE_ATTRIBUTES) {
            CreateDirectoryW(uc2filepath, NULL);
        }
        *mrk = '/';
        ++mrk;
    }
    if (GetFileAttributesW(uc2filepath) == INVALID_FILE_ATTRIBUTES) {
        CreateDirectoryW(uc2filepath, NULL);
    }
    return OK;
}

size_t minfs_canonical_path(const char* filepath, char* outpath, size_t buf_size) {
    BOOL ret;
    size_t rlen = 0;
    wchar_t* mrk, *prevmrk;
    wchar_t* uc2filepath;
    wchar_t canonical[MAX_PATH+1];
    UTF8_TO_UC2_STACK(filepath, uc2filepath);
    while (mrk = wcschr(uc2filepath, '/')) { 
        *mrk = '\\';
    }
    ret = PathCanonicalizeW(canonical, uc2filepath);
    while (mrk = wcschr(canonical, '\\')) { 
        *mrk = '/';
    }
    //strip any double slashes
    rlen = wcslen(canonical);
    mrk = canonical;
    prevmrk = NULL;
    while (*mrk) { 
        if (*mrk == '/' && prevmrk && *prevmrk == '/') {
            memmove(prevmrk, mrk, (rlen+1)*sizeof(wchar_t));
        }
        prevmrk = mrk;
        ++mrk;
        --rlen;
    }
    //strip any trailing slashes
    if (ret == TRUE) {
        rlen = uc2_to_utf8(canonical, outpath, buf_size);
        if (canonical[rlen-1] == '/') {
            canonical[rlen-1] = 0;
            rlen--;
        }
    }
    return rlen;
}

minfs_uint32_t minfs_current_working_directory_len() {
    return GetCurrentDirectoryW(0, NULL);
}

size_t minfs_current_working_directory(char* out_cwd, size_t buf_size) {
    minfs_uint32_t len = GetCurrentDirectoryW(0, NULL);
    wchar_t* tmpbuf = alloca(len*sizeof(wchar_t));
    
    GetCurrentDirectoryW(len, tmpbuf);
    return uc2_to_utf8(tmpbuf, out_cwd, buf_size);
}

size_t minfs_path_parent(const char* filepath, char* out_cwd, size_t buf_size) {
    char* mrk;
    char* lastmrk = NULL;
    size_t outsize;
    size_t len = strlen(filepath);
    while (mrk = strchr(filepath, '\\')) { 
        *mrk = '/';
    }
    lastmrk = strrchr(filepath, '/');
    if (lastmrk == NULL && buf_size > len) {
        strncpy(out_cwd, filepath, buf_size);
        return len;
    } else {
        outsize = (uintptr_t)lastmrk - (uintptr_t)filepath;

        if (buf_size > outsize) {
            strncpy(out_cwd, filepath, outsize);
            out_cwd[outsize]=0;
            return outsize;
        }
    }
    // no space to write
    return NO_MEM;
}

size_t minfs_path_leaf(const char* filepath, char* out_cwd, size_t buf_size) {
    char* mrk;
    char* lastmrk = NULL;
    size_t outsize;
    size_t len = strlen(filepath);
    while (mrk = strchr(filepath, '\\')) { 
        *mrk = '/';
        lastmrk = mrk;
    }
    lastmrk = strchr(filepath, '/');
    if (lastmrk == NULL && buf_size > len) {
        strncpy(out_cwd, filepath, buf_size);
        return len;
    } else {
        outsize = len - ((uintptr_t)lastmrk - (uintptr_t)filepath);

        if (buf_size > outsize) {
            strncpy(out_cwd, lastmrk+1, outsize);
            return outsize;
        }
    }
    // no space to write
    return NO_MEM;
}

size_t minfs_path_without_ext(const char* filepath, char* out_cwd, size_t buf_size) {
    char* mrk;
    char* lastmrk = NULL;
    size_t outsize;
    size_t len = strlen(filepath);
    while (mrk = strchr(filepath, '\\')) { 
        *mrk = '/';
    }
    lastmrk = strrchr(filepath, '.');
    if (lastmrk == NULL && buf_size > len) {
        strncpy(out_cwd, filepath, buf_size);
        return len;
    } else {
        outsize = ((uintptr_t)lastmrk - (uintptr_t)filepath);

        if (buf_size > outsize) {
            strncpy(out_cwd, filepath, outsize);
            out_cwd[outsize]=0;
            return outsize;
        }
    }
    // no space to write
    return NO_MEM;
}

int minfs_read_directory(const char* filepath, void* scratch, size_t scratchlen, minfs_read_dir_callback cb, void* opaque) {
    WIN32_FIND_DATAW found;
    HANDLE searchhandle;
    size_t foundlen;
    wchar_t* uc2filepath;
    UTF8_TO_UC2_STACK_PAD(filepath, uc2filepath, 4);
    wcscat(uc2filepath, L"/*");

    searchhandle = FindFirstFileW(uc2filepath, &found);

    if (searchhandle == INVALID_HANDLE_VALUE) {
        return 0;
    }

    do {
        if (wcscmp(found.cFileName, L".") && wcscmp(found.cFileName, L"..")){
            foundlen = wcslen(found.cFileName);
            if(uc2_to_utf8(found.cFileName, (char*)scratch, scratchlen) != foundlen) {
                return NO_MEM;
            }
            cb(filepath, (char*)scratch, opaque);
        }
    }
    while ( FindNextFileW( searchhandle, &found ) );

    FindClose( searchhandle );
    return OK;
}

MinFSDirectoryEntry_t* minfs_read_directory_entries(const char* filepath, void* scratch, size_t scratchlen) {
    WIN32_FIND_DATAW found;
    HANDLE searchhandle;
    size_t foundlen;
    wchar_t* uc2filepath;
    MinFSDirectoryEntry_t* currententry = NULL, *first = NULL;
    UTF8_TO_UC2_STACK_PAD(filepath, uc2filepath, 3);
    wcscat(uc2filepath, L"/*");

    searchhandle = FindFirstFileW(uc2filepath, &found);

    if (searchhandle == INVALID_HANDLE_VALUE || scratchlen < sizeof(MinFSDirectoryEntry_t)) {
        return NULL;
    }

    --scratchlen;

    first = scratch;
    first->entryName[0] = 0;
    first->entryNameLen = 0;
    first->next = NULL;

    do {
        if (wcscmp(found.cFileName, L".") && wcscmp(found.cFileName, L"..")){
            currententry = scratch;
            if (scratchlen < sizeof(MinFSDirectoryEntry_t)) {
                FindClose( searchhandle );
                return NULL;
            }
            scratchlen -= sizeof(MinFSDirectoryEntry_t);
            foundlen = wcslen(found.cFileName);
            if(uc2_to_utf8(found.cFileName, (char*)currententry->entryName, scratchlen) != foundlen) {
                FindClose( searchhandle );
                return NULL;
            }
            scratchlen -= foundlen+1;
            scratch = ((char*)scratch)+sizeof(MinFSDirectoryEntry_t)+foundlen;
            currententry->next = scratch;
            currententry->entryNameLen = foundlen;
        }
    } while ( FindNextFileW( searchhandle, &found ) );

    FindClose( searchhandle );
    if (currententry)
        currententry->next = NULL;
    return first;
}