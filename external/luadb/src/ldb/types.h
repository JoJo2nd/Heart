/********************************************************************

    filename:   types.h  
    
    Copyright (c) 16:12:2013 James Moran
    
    This software is provided 'as-is', without any express or implied
    warranty. In no event will the authors be held liable for any damages
    arising from the use of this software.
    
    Permission is granted to anyone to use this software for any purpose,
    including commercial applications, and to alter it and redistribute it
    freely, subject to the following restrictions:
    
    1. The origin of this software must not be misrepresented; you must not
    claim that you wrote the original software. If you use this software
    in a product, an acknowledgment in the product documentation would be
    appreciated but is not required.
    
    2. Altered source versions must be plainly marked as such, and must not be
    misrepresented as being the original software.
    
    3. This notice may not be removed or altered from any source
    distribution.

*********************************************************************/

#pragma once

#ifndef TYPES_H__
#define TYPES_H__

#include <time.h>
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

#define ldb_api   __cdecl

#endif // TYPES_H__