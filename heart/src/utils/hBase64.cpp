/********************************************************************

    filename: 	hBase64.cpp	
    
    Copyright (c) 8:11:2012 James Moran
    
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

namespace Heart
{
namespace hBase64
{

    /*
    ** Translation Table as described in RFC1113
    */
    static const hChar cb64[]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    /*
    ** Translation Table to decode (created by author)
    */
    static const hChar cd64[]="|$$$}rstuvwxyz{$$$$$$$>?@ABCDEFGHIJKLMNOPQRSTUVW$$$$$$XYZ[\\]^_`abcdefghijklmnopq";

    /*
    ** EncodeBlock
    **
    ** encode 3 8-bit binary bytes as 4 '6-bit' characters
    */
    static void EncodeBlock(const hByte* in, hByte* out, hUint len)
    {
        out[0] = (hByte) cb64[ (int)(in[0] >> 2) ];
        out[1] = (hByte) cb64[ (int)(((in[0] & 0x03) << 4) | ((in[1] & 0xf0) >> 4)) ];
        out[2] = (hByte) (len > 1 ? cb64[ (int)(((in[1] & 0x0f) << 2) | ((in[2] & 0xc0) >> 6)) ] : '=');
        out[3] = (hByte) (len > 2 ? cb64[ (int)(in[2] & 0x3f) ] : '=');
    }

    /*
    ** decodeblock
    **
    ** decode 4 '6-bit' characters into 3 8-bit binary bytes
    */
    static inline hByte decodeValue(hByte v) {
        v = ((v < 43 || v > 122) ? 0 : (int) cd64[ v - 43 ]);
        if( v != 0 ) {
            v = ((v == (int)'$') ? 0 : v - 61);
        }
        return v-1;
    }

    static void DecodeBlock(const hByte* in, hByte* out, hUint len) {   
        out[ 0 ] = (hByte) (decodeValue(in[0]) << 2 | decodeValue(in[1]) >> 4);
        if (len > 1) {
            out[ 1 ] = (hByte) (decodeValue(in[1]) << 4 | decodeValue(in[2]) >> 2);
        }
        if (len > 2) {
            out[ 2 ] = (hByte) (((decodeValue(in[2]) << 6) & 0xc0) | decodeValue(in[3]));
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    HEART_DLLEXPORT
    hUint HEART_API EncodeCalcRequiredSize(hUint inputCount) {
        hUint reqsize=(inputCount+2)/3;
        return (reqsize*4);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
   
    HEART_DLLEXPORT
    hUint HEART_API Encode(const void* inputbuf, hUint inputCount, void* outputbuf, hUint outputLimit) {
        hByte* in;
        hByte* out;
        hUint32 len, blocksout = 0, bytesin = inputCount;
        hcAssert(inputbuf && outputbuf);

        in = (hByte*)inputbuf;
        out = (hByte*)outputbuf;
        while(((blocksout+1)*4) < outputLimit || bytesin != 0) {
            len = bytesin > 3 ? 3 : bytesin;
            bytesin -= len;
            if( len > 0 ) {
                EncodeBlock( in, out, len );
                blocksout++;
                in+=3;
                out+=4;
            }
        }
        hcAssert(out <= ((hByte*)outputbuf+outputLimit));
        return blocksout*4;
    }

    HEART_DLLEXPORT
    hUint HEART_API DecodeCalcRequiredSize(const void* inputbuf, hUint inputCount) {
        hcAssert(inputbuf);
        hUint reqlen=((inputCount/4)-1)*3;
        if (((hChar*)inputbuf)[inputCount-2] == '=') {
            return reqlen+1;
        } else if (((hChar*)inputbuf)[inputCount-1] == '=') {
            return reqlen+2;
        }
        return reqlen+3;
    }

    HEART_DLLEXPORT
    hUint HEART_API Decode(const void* inputbuf, hUint inputCount, void* outputbuf, hUint outputLimit) {
        hByte* in, *inend;
        hByte* out, *outend;
        hInt read=0;
        hUint bytesleft=outputLimit;
        hcAssert(inputbuf && outputbuf);

        in = (hByte*)inputbuf;
        inend=in+inputCount;
        out = (hByte*)outputbuf;
        outend=out+outputLimit;
        while(in < inend && out < outend) {
            DecodeBlock(in, out, bytesleft);
            in+=4;
            out+=hMin(bytesleft, 3);
            bytesleft-=hMin(bytesleft, 3);
            read+=4;
        }
        hcAssert(out <= outend);
        return read;
    }
}
}