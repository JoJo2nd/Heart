/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#ifndef _CCRC32_H
#define _CCRC32_H

#include "cryptoCRC32.h"

namespace Heart
{
namespace hCRC32
{

    hFORCEINLINE 
    void HEART_API StartCRC32( hUint32* crc, const hChar* data, hUint32 len )
    {
        hcAssert( crc );
        cyStartCRC32( crc, (const hByte*)data, len );
    }

    hFORCEINLINE
    void HEART_API ContinueCRC32( hUint32* crc, const hChar* data, hUint32 len )
    {
        hcAssert( crc );
        cyContinueCRC32( crc, (const hByte*)data, len );
    }

    hFORCEINLINE
    hUint32 HEART_API FinishCRC32( hUint32* crc )
    {
        hcAssert( crc );
        return cyFinaliseCRC32( crc );
    }

    hFORCEINLINE
    hUint32 HEART_API FullCRC( const hChar* sData, hUint32 ulLength)
    {
        return cyFullCRC32( (const hByte*)sData, ulLength );
    }

    hFORCEINLINE
	hUint32 HEART_API StringCRC( const hChar* string )
    {
        return cyStringCRC32(string);
    }

}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif