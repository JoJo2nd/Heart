/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/
#pragma  once

#include <stddef.h>

#if defined PLATFORM_WINDOWS
#   define mt_api __cdecl
#elif PLATFORM_LINUX
#   if BUILD_64_BIT
#       define mt_api
#   else
#       define mt_api __attribute__((cdecl))
#   endif
#else
#   error
#endif

#if defined (PLATFORM_WINDOWS)
#   if defined (memtrackerlib_EXPORTS)
#       define mt_dll_export __declspec(dllexport)
#   else
#       define mt_dll_export __declspec(dllimport)
#   endif
#else
#   define mt_dll_export
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned long long  mt_uint64; 
typedef long long           mt_int64; 
typedef unsigned long       mt_uint32; 
typedef long                mt_int32; 
typedef char                mt_int8; 
typedef unsigned char       mt_uint8; 

enum mt_chunk_id {
    mt_chunk_id_memory_ext              = 255,
    mt_chunk_id_memory_alloc_event      = 254,
    mt_chunk_id_memory_free_event       = 253,
    mt_chunk_id_marker_event            = 252,
    mt_chunk_id_win32_sym_event        = 251,
};

#if defined (PLATFORM_WINDOWS)
#   pragma pack (push, 1)
#elif defined (PLATFORM_LINUX)
#   pragma pack(1)
#else
#   error
#endif

typedef struct mt_trace_header {
    mt_uint32 addressSizeBytes;
    mt_uint64 baseAddress;
    mt_uint32 modulePathLen;
} mt_trace_header_t;

typedef struct mt_trace_chunk {
    mt_uint64 eventID;
    mt_uint64 threadID;
    mt_uint64 chunkLen;
    mt_uint8  chunkID;
} mt_trace_chunk_t;

typedef struct mt_trace_backtrace {
    mt_uint64 address;
    mt_uint64 size; // zero when address is being released.
    mt_uint32 stackSize; // number of entries in the stack
    mt_uint32 tagLen; // string tag length
} mt_trace_backtrace_t;

typedef struct mt_trace_win32_symbols {
    mt_uint64 baseAddress;
    mt_uint64 moduleByteLen;
    mt_uint32 nameLen;
    mt_uint32 pathLen;
} mt_trace_win32_symbols_t;

#if defined (PLATFORM_WINDOWS)
#   pragma pack (pop)
#elif defined (PLATFORM_LINUX)
#   pragma options align=reset
#else
#   error
#endif

mt_dll_export void mt_api mem_track_alloc(void* ptr, size_t size, const char* tag);
mt_dll_export void mt_api mem_track_free(void* ptr);
mt_dll_export void mt_api mem_track_marker(const char* marker);
mt_dll_export void mt_api mem_track_load_symbols();

#ifdef __cplusplus
}
#endif
