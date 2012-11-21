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
    static void EncodeBlock(hByte* in, hByte* out, hUint32 len)
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
    static void DecodeBlock(hByte* in, hByte* out)
    {   
        out[ 0 ] = (hByte) (in[0] << 2 | in[1] >> 4);
        out[ 1 ] = (hByte) (in[1] << 4 | in[2] >> 2);
        out[ 2 ] = (hByte) (((in[2] << 6) & 0xc0) | in[3]);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
   
    hUint32 Encode(void* inputbuf, hUint32 inputCount, void* outputbuf, hUint32 outputLimit)
    {
        hByte* in;
        hByte* out;
        hUint32 len, blocksout = 0, bytesin = inputCount;

        in = (hByte*)inputbuf;
        out = (hByte*)outputbuf;
        while(((blocksout+1)*4) < outputLimit || bytesin == 0) 
        {
            len = bytesin > 3 ? 3 : bytesin;
            bytesin -= len;
            if( len > 0 ) 
            {
                EncodeBlock( in, out, len );
                blocksout++;
            }
        }
        return blocksout*4;
    }

    hUint32 DecodeCalcRequiredSize(void* inputbuf, hUint32 inputCount);
    hUint32 Decode(void* inputbuf, hUint32 inputCount, void* outputbuf, hUint32 outputLimit);

    /*
    static void decodeblock( unsigned char *in, unsigned char *out )
    {   
    out[ 0 ] = (unsigned char ) (in[0] << 2 | in[1] >> 4);
    out[ 1 ] = (unsigned char ) (in[1] << 4 | in[2] >> 2);
    out[ 2 ] = (unsigned char ) (((in[2] << 6) & 0xc0) | in[3]);
    }

    *
    ** decode
    **
    ** decode a base64 encoded stream discarding padding, line breaks and noise
    *
    static int decode( FILE *infile, FILE *outfile )
    {
        int retcode = 0;
        unsigned char in[4];
        unsigned char out[3];
        int v;
        int i, len;

        *in = (unsigned char) 0;
        *out = (unsigned char) 0;
        while( feof( infile ) == 0 ) {
            for( len = 0, i = 0; i < 4 && feof( infile ) == 0; i++ ) {
                v = 0;
                while( feof( infile ) == 0 && v == 0 ) {
                    v = getc( infile );
                    if( v != EOF ) {
                        v = ((v < 43 || v > 122) ? 0 : (int) cd64[ v - 43 ]);
                        if( v != 0 ) {
                            v = ((v == (int)'$') ? 0 : v - 61);
                        }
                    }
                }
                if( feof( infile ) == 0 ) {
                    len++;
                    if( v != 0 ) {
                        in[ i ] = (unsigned char) (v - 1);
                    }
                }
                else {
                    in[i] = (unsigned char) 0;
                }
            }
            if( len > 0 ) {
                decodeblock( in, out );
                for( i = 0; i < len - 1; i++ ) {
                    if( putc( (int) out[i], outfile ) == 0 ){
                        if( ferror( outfile ) != 0 )      {
                            perror( b64_message( B64_FILE_IO_ERROR ) );
                            retcode = B64_FILE_IO_ERROR;
                        }
                        break;
                    }
                }
            }
        }
        return( retcode );
    }
    */
}
}