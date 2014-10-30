/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#include "minfs.h"
#include "minfs_common.h" 
#ifdef PLATFORM_LINUX
#   include <unistd.h>
#elif defined PLATFORM_WINDOWS
#   include <memory.h>
#endif
#include <stdint.h>

minfs_uint32_t utf8_codepoint(const char* uft8In, minfs_uint16_t* ucOut) {
    minfs_uint32_t ret = 0;
    if( (uft8In[ ret ] & UTF8_MASK3BYTES) == UTF8_MASK3BYTES ) { // 1110xxxx 10xxxxxx 10xxxxxx
        *ucOut = ( (uft8In[ret] & 0x0F)     << 12)    |
            ( (uft8In[ret+1] & UTF8_MASKBITS) << 6 )|
            ( (uft8In[ret+3] & UTF8_MASKBITS) );
        ret=3;
    } else if((uft8In[ ret ] & UTF8_MASK2BYTES) == UTF8_MASK2BYTES) { // 110xxxxx 10xxxxxx
        *ucOut = ( ( uft8In[ ret ] & 0x1F ) << 6 ) | ( uft8In[ ret+1 ] & UTF8_MASKBITS );
        ret=2;
    } else if(uft8In[ ret ] < UTF8_MASKBYTE) { // 0xxxxxxx
        *ucOut = uft8In[ ret ];
        ret=1;
    }
    return ret;
}

void utf8_to_uc2(const char* src, minfs_uint16_t* dst, size_t len) {
    len -= sizeof(minfs_uint16_t);   /* save room for null char. */
    while (len >= sizeof(minfs_uint16_t))
    {
        minfs_uint32_t cp = utf8_codepoint(src, dst);
        if (cp == 0)
            break;

        dst++;// = (minfs_uint16_t)(cp & 0xFFFF);
        src++;
        len -= sizeof(minfs_uint16_t);
    } /* while */

    *dst = 0;
}

/*
 * Returns number of bytes written excluding the null terminator
 */
size_t uc2_to_utf8(minfs_uint16_t* uc_in, char* utf8_out, size_t buf_size) {
    size_t ret = 0;
    while (buf_size > 0 && *uc_in) {    // 0xxxxxxx
        if(*uc_in < 0x80 && buf_size > 0) {
            utf8_out[ ret++ ] = 0x007F & *uc_in;
            buf_size -= 1;
        } else if(*uc_in < 0x800 && buf_size > 1) {// 110xxxxx 10xxxxxx
            utf8_out[ ret++ ] = 0x00FF & ( UTF8_MASK2BYTES | ( *uc_in >> 6 ) );
            utf8_out[ ret++ ] = 0x00FF & ( UTF8_MASKBYTE | ( *uc_in & UTF8_MASKBITS ) );
            buf_size -= 2;
        } else if(*uc_in < 0x10000 && buf_size > 2) { // 1110xxxx 10xxxxxx 10xxxxxx
            utf8_out[ ret++ ] = 0x00FF & ( UTF8_MASK3BYTES | ( *uc_in >> 12 ) );
            utf8_out[ ret++ ] = 0x00FF & ( UTF8_MASKBYTE | ( *uc_in >> 6 & UTF8_MASKBITS ) );
            utf8_out[ ret++ ] = 0x00FF & ( UTF8_MASKBYTE | ( *uc_in & UTF8_MASKBITS ) );
            buf_size -= 3;
        }
        ++uc_in;
    }

    if (buf_size) {
        utf8_out[ret] = 0;
    }
    return ret;
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
