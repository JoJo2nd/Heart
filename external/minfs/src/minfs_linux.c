/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#include "minfs.h"
#include "minfs_common.h"
#include <malloc.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <alloca.h>
#include <limits.h>
#include <stdlib.h>

minfs_uint64_t minfs_get_current_file_time() {
    time_t t;
    time(&t);
    return (minfs_uint64_t)t;
}

minfs_uint64_t minfs_get_file_mdate(const char* filepath) {
    struct stat s;
    if (stat(filepath, &s) != 0)
        return 0;
    return (minfs_uint64_t)s.st_mtime;
}

minfs_uint64_t minfs_get_file_size(const char* filepath) {
    struct stat s;
    if (stat(filepath, &s) != 0)
        return 0;
    return s.st_size;
}

int minfs_path_exist(const char* filepath) {
    struct stat s;
    return stat(filepath, &s) == 0;
}

int minfs_is_file(const char* filepath) {
    struct stat s;
    if (stat(filepath, &s) != 0)
        return 0;
    return S_ISDIR(s.st_mode) ? 0 : 1;
}

int minfs_is_directory(const char* filepath) {
    struct stat s;
    if (stat(filepath, &s) != 0)
        return 0;
    return S_ISDIR(s.st_mode) ? 1 : 0;
}

int minfs_is_sym_link(const char* filepath) {
     struct stat s;
     if (stat(filepath, &s) != 0)
         return 0;
     return S_ISLNK(s.st_mode) ? 1 : 0;
}

int minfs_create_directories(const char* in_filepath) {
    struct stat s;
    char* mrk;
    UTF8_WRITABLE_STACK(in_filepath, filepath);
    while (mrk = strchr(filepath, L'\\')) {
        *mrk = '/';
    }
    mrk = filepath;
    while (mrk = strchr(mrk, '/')) {
        *mrk = 0;
        if (stat(filepath, &s) != 0) {
            mkdir(filepath, S_IRWXU);
        }
        *mrk = '/';
        ++mrk;
    }
    if (stat(filepath, &s) != 0) {
        mkdir(filepath, S_IRWXU);
    }
    return OK;
}

minfs_uint32_t minfs_current_working_directory_len() {
    char* cwd = getcwd(NULL, 0);
    minfs_uint32_t l = strlen(cwd)+1;
    free(cwd);
    return l;
}

size_t minfs_current_working_directory(char* out_cwd, size_t buf_size) {
    *out_cwd=0;
    getcwd(out_cwd, buf_size);
    return strlen(out_cwd);
}

int minfs_read_directory(const char* filepath, void* scratch, size_t scratchlen, minfs_read_dir_callback cb, void* opaque) {
    DIR* dir;
    struct dirent* ent;

    dir = opendir(filepath);
    if(!dir)
        return OK;

    while ((ent = readdir(dir)) != NULL) {
        if (strcmp(ent->d_name, ".") == 0)
            continue;

        if (strcmp(ent->d_name, "..") == 0)
            continue;

        cb(filepath, ent->d_name, opaque);
    }
    return OK;
}

MinFSDirectoryEntry_t* minfs_read_directory_entries(const char* filepath, void* scratch, size_t scratchlen) {
    DIR* dir;
    struct dirent* ent;
    MinFSDirectoryEntry_t* currententry, *first;
    size_t foundlen;

    dir = opendir(filepath);
    if(!dir)
        return NULL;

    first = scratch;
    first->entryName[0] = 0;
    first->entryNameLen = 0;
    first->next = NULL;

    while ((ent = readdir(dir)) != NULL) {
        if (strcmp(ent->d_name, ".") && strcmp(ent->d_name, "..")) {
            currententry = scratch;
            if (scratchlen < sizeof(MinFSDirectoryEntry_t))
                return NULL;
            scratchlen -= sizeof(MinFSDirectoryEntry_t);
            foundlen = strlen(ent->d_name);
            if (scratchlen < foundlen+1)
                return NULL;
            scratchlen -= foundlen+1;
            scratch = ((char*)scratch)+sizeof(MinFSDirectoryEntry_t)+foundlen;
            currententry->next = scratch;
        }
    }
    if (currententry)
        currententry->next = NULL;
    return first;
}

size_t minfs_canonical_path(const char* filepath, char* outpath, size_t buf_size) {
    char* canonpath = realpath(filepath, NULL);
    size_t len = strlen(canonpath);
    if (buf_size < len) {
        return 0;
    }
    strcpy(outpath, canonpath);
    free(canonpath);
    return len;
}
