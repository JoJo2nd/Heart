/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/
#pragma once

#include <stdint.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif
enum ErrorCodes{
    OK                         =  0,
    ATTRIBUTE_READ_FAILED      = 0x80000001,
    NO_MEM                     = 0x80000002,
} ErrorCodes_t;

#define FS_SUCCEEDED(x) ((x & 0x80000000) == 0)
#define FS_FAILED(x) (!FS_SUCCEEDED(x))

#define UTF8_MASKBITS   (0x3F)
#define UTF8_MASKBYTE   (0x80)
#define UTF8_MASK2BYTES (0xC0)
#define UTF8_MASK3BYTES (0xE0)
#define UTF8_MASK4BYTES (0xF0)
#define UTF8_MASK5BYTES (0xF8)
#define UTF8_MASK6BYTES (0xFC)

typedef unsigned short      minfs_uint16_t;
typedef unsigned long       minfs_uint32_t;
typedef unsigned long long  minfs_uint64_t;
typedef void (*minfs_read_dir_callback)(const char* origpath, const char* file, void* opaque);

typedef struct MinFSDirectoryEntry {
    size_t                      entryNameLen;
    struct MinFSDirectoryEntry* next;
    char                        entryName[1];
} MinFSDirectoryEntry_t;

minfs_uint64_t minfs_get_current_file_time();
minfs_uint64_t minfs_get_file_mdate(const char* filepath);
minfs_uint64_t minfs_get_file_size(const char* filepath);
size_t minfs_canonical_path(const char* filepath, char* outpath, size_t buf_size);
int minfs_path_exist(const char* filepath);
int minfs_is_file(const char* filepath);
int minfs_is_directory(const char* filepath);
int minfs_is_sym_link(const char* filepath);
int minfs_create_directories(const char* filepath);
minfs_uint32_t minfs_current_working_directory_len();
size_t minfs_current_working_directory(char* out_cwd, size_t buf_size);
size_t minfs_path_parent(const char* filepath, char* out_cwd, size_t buf_size);
size_t minfs_path_leaf(const char* filepath, char* out_cwd, size_t buf_size);
size_t minfs_path_without_ext(const char* filepath, char* out_cwd, size_t buf_size);
size_t minfs_path_join(const char* parent, const char* leaf, char* out_path, size_t buf_size);
int minfs_read_directory(const char* filepath, void* scratch, size_t scratchlen, minfs_read_dir_callback cb, void* opaque);
MinFSDirectoryEntry_t* minfs_read_directory_entries(const char* filepath, void* scratch, size_t scratchlen);

#ifdef __cplusplus
} //extern "C"
#endif