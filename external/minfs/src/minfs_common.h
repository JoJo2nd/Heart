/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#pragma once

#include "minfs.h"

#define UTF8_TO_UC2_STACK(utf8, uc) UTF8_TO_UC2_STACK_PAD(utf8, uc, 0)

#define UTF8_TO_UC2_STACK_PAD(utf8, uc, pad) \
    uc = (wchar_t*)alloca((strlen(utf8)+1+pad)*sizeof(minfs_uint16_t)); \
    utf8_to_uc2(utf8, uc, (strlen(utf8)+1+pad)*sizeof(minfs_uint16_t));    

#define UTF8_WRITABLE_STACK(utf8, out) char* out = alloca((strlen(utf8)+1)*sizeof(char))

minfs_uint32_t utf8_codepoint(const char* uft8In, minfs_uint16_t* ucOut);
void utf8_to_uc2(const char* src, minfs_uint16_t* dst, size_t len);
/*
 * Returns number of bytes written excluding the null terminator
 */
 size_t uc2_to_utf8(minfs_uint16_t* uc_in, char* utf8_out, size_t buf_size);
