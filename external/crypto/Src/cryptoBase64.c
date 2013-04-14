/********************************************************************

    filename:   cryptoBase64.c  
    
    Copyright (c) 14:4:2013 James Moran
    
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

#include "cryptoBase64.h"

/*
** Translation Table as described in RFC1113
*/
static const cyChar cb64[]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

/*
** Translation Table to decode (created by author)
*/
static const cyChar cd64[]="|$$$}rstuvwxyz{$$$$$$$>?@ABCDEFGHIJKLMNOPQRSTUVW$$$$$$XYZ[\\]^_`abcdefghijklmnopq";

/*
** EncodeBlock
**
** encode 3 8-bit binary bytes as 4 '6-bit' characters
*/
static void encodeBlock(const cyByte* in, cyByte* out, cyUint len)
{
    out[0] = (cyByte) cb64[ (int)(in[0] >> 2) ];
    out[1] = (cyByte) cb64[ (int)(((in[0] & 0x03) << 4) | ((in[1] & 0xf0) >> 4)) ];
    out[2] = (cyByte) (len > 1 ? cb64[ (int)(((in[1] & 0x0f) << 2) | ((in[2] & 0xc0) >> 6)) ] : '=');
    out[3] = (cyByte) (len > 2 ? cb64[ (int)(in[2] & 0x3f) ] : '=');
}

/*
** decodeblock
**
** decode 4 '6-bit' characters into 3 8-bit binary bytes
*/
static cyByte decodeValue(cyByte v) {
    v = ((v < 43 || v > 122) ? 0 : (int) cd64[ v - 43 ]);
    if( v != 0 ) {
        v = ((v == (int)'$') ? 0 : v - 61);
    }
    return v-1;
}

static void decodeBlock(const cyByte* in, cyByte* out, cyUint len) {   
    out[ 0 ] = (cyByte) (decodeValue(in[0]) << 2 | decodeValue(in[1]) >> 4);
    if (len > 1) {
        out[ 1 ] = (cyByte) (decodeValue(in[1]) << 4 | decodeValue(in[2]) >> 2);
    }
    if (len > 2) {
        out[ 2 ] = (cyByte) (((decodeValue(in[2]) << 6) & 0xc0) | decodeValue(in[3]));
    }
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

CY_DLLEXPORT cyUint CY_API cyBase64EncodeCalcRequiredSize(cyUint inputCount) {
    cyUint reqsize=(inputCount+2)/3;
    return (reqsize*4);
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
   
CY_DLLEXPORT cyUint CY_API cyBase64Encode(const void* inputbuf, cyUint inputCount, void* outputbuf, cyUint outputLimit) {
    cyByte* in;
    cyByte* out;
    cyUint32 len, blocksout = 0, bytesin = inputCount;

    in = (cyByte*)inputbuf;
    out = (cyByte*)outputbuf;
    while(((blocksout+1)*4) < outputLimit || bytesin != 0) {
        len = bytesin > 3 ? 3 : bytesin;
        bytesin -= len;
        if( len > 0 ) {
            encodeBlock( in, out, len );
            blocksout++;
            in+=3;
            out+=4;
        }
    }
    return blocksout*4;
}

CY_DLLEXPORT cyUint CY_API cyBase64DecodeCalcRequiredSize(const void* inputbuf, cyUint inputCount) {
    cyUint reqlen=((inputCount/4)-1)*3;
    if (((cyChar*)inputbuf)[inputCount-2] == '=') {
        return reqlen+1;
    } else if (((cyChar*)inputbuf)[inputCount-1] == '=') {
        return reqlen+2;
    }
    return reqlen+3;
}

CY_DLLEXPORT cyUint CY_API cyBase64Decode(const void* inputbuf, cyUint inputCount, void* outputbuf, cyUint outputLimit) {
    cyByte* in, *inend;
    cyByte* out, *outend;
    cyInt read=0;
    cyUint bytesleft=outputLimit;

    in = (cyByte*)inputbuf;
    inend=in+inputCount;
    out = (cyByte*)outputbuf;
    outend=out+outputLimit;
    while(in < inend && out < outend) {
        decodeBlock(in, out, bytesleft);
        in+=4;
        out+=cyMin(bytesleft, 3);
        bytesleft-=cyMin(bytesleft, 3);
        read+=4;
    }
    return read;
}
