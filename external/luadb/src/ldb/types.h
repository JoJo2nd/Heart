/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#pragma once

#ifndef TYPES_H__
#define TYPES_H__

#include <time.h>
#include <cstddef>
#include <string>
#include <thread>
#include <mutex>
#include <functional>
#include <unordered_map>

typedef	char				ldb_char;
typedef unsigned char		ldb_byte;
typedef char                ldb_int8;
typedef unsigned char       ldb_uint8;
typedef unsigned short		ldb_uint16;
typedef short				ldb_int16;
typedef unsigned long		ldb_uint32;
typedef long				ldb_int32;
typedef unsigned long long  ldb_uint64;
typedef long long			ldb_int64;
typedef int					ldb_int;
typedef unsigned int        ldb_uint;
typedef size_t              ldb_size_t;
typedef float				ldb_float;
typedef double				ldb_double;
typedef bool				ldb_bool;
typedef time_t              ldb_time;
typedef uintptr_t           ldb_uintptr_t;
typedef ptrdiff_t           ldb_ptrdiff_t;
typedef std::string         ldb_string;
typedef std::thread         ldb_thread;
typedef std::recursive_mutex ldb_mutex;
typedef std::lock_guard<ldb_mutex> ldb_mutexsentry;

#if (PLATFORM_WINDOWS)
#   define ldb_api   __cdecl
#elif (PLATFORM_LINUX)
#   if BUILD_64_BIT
#       define ldb_api
#   else
#       define ldb_api   __attribute__((__cdecl__))   
#   endif
#endif

#endif // TYPES_H__