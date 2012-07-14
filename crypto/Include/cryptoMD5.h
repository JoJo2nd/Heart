/********************************************************************

	filename: 	cryptoMD5.h	
	
	Copyright (c) 25:11:2011 James Moran
	
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

#ifndef CRYPTOMD5_H__
#define CRYPTOMD5_H__

#include "cryptoCommon.h"

#ifdef __cplusplus
extern "C" {
#endif//

#define cyMD5_BUFLEN	64

typedef struct 
{
    union 
    {
        cyUint32	md5_state32[4];
        cyByte  	md5_state8[16];
    } cy_md5_st;

    #define md5_sta		cy_md5_st.md5_state32[0]
    #define md5_stb		cy_md5_st.md5_state32[1]
    #define md5_stc		cy_md5_st.md5_state32[2]
    #define md5_std		cy_md5_st.md5_state32[3]
    #define md5_st8		cy_md5_st.md5_state8

    union 
    {
        cyUint64	md5_count64;
        cyByte      md5_count8[8];
    } cy_md5_count;

    #define md5_n	cy_md5_count.md5_count64
    #define md5_n8	cy_md5_count.md5_count8

    cyInt	    md5_i;
    cyByte  	md5_buf[cyMD5_BUFLEN];
} cy_md5_ctxt;

CY_DLLEXPORT void CY_API __cyInitMD5( cy_md5_ctxt* );
CY_DLLEXPORT void CY_API __cyLoopMD5( cy_md5_ctxt*, const cyByte*, cyUint );
CY_DLLEXPORT void CY_API __cyPadMD5( cy_md5_ctxt* );
CY_DLLEXPORT void CY_API __cyResultMD5( cyByte*, cy_md5_ctxt* );
CY_DLLEXPORT void CY_API __cyMD5ToString( const cyByte*, cyChar* );

/* compatibility */
#define cyMD5_CTX		cy_md5_ctxt
#define cyMD5Init( md5Ctx )	__cyInitMD5((md5Ctx))
#define cyMD5Update( md5Ctx, data, size )	__cyLoopMD5((md5Ctx), (const cyByte*)(data), (size))
#define cyMD5Final( md5Ctx, digest ) \
    do { \
        __cyPadMD5((md5Ctx)); \
        __cyResultMD5((digest), (md5Ctx)); \
    } while (0)
#define cyMD5DigestToString( digest, str ) __cyMD5ToString( digest, str )

#ifdef __cplusplus
};
#endif

#endif // CRYPTOMD5_H__