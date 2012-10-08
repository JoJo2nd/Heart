#ifndef _CCRC32_H
#define _CCRC32_H

#define CRYPTO_CRC32_POLYNOMIAL 0x04c11db7 //@This is the official polynomial used by CRC-32 in PKZip, WinZip and Ethernet.
#define CRYPTO_CRC32BUFSZ 1024 //@Used for FileCRC()

namespace Heart
{
namespace hCRC32
{

    hFORCEINLINE HEARTBASE_SLIBEXPORT 
    void HEART_API StartCRC32( hUint32* crc, const hChar* data, hUint32 len )
    {
        hcAssert( crc );
        cyStartCRC32( crc, (const hByte*)data, len );
    }

    hFORCEINLINE HEARTBASE_SLIBEXPORT
    void HEART_API ContinueCRC32( hUint32* crc, const hChar* data, hUint32 len )
    {
        hcAssert( crc );
        cyContinueCRC32( crc, (const hByte*)data, len );
    }

    hFORCEINLINE HEARTBASE_SLIBEXPORT
    hUint32 HEART_API FinishCRC32( hUint32* crc )
    {
        hcAssert( crc );
        return cyFinaliseCRC32( crc );
    }

    hFORCEINLINE HEARTBASE_SLIBEXPORT
    hUint32 HEART_API FullCRC( const hChar* sData, hUint32 ulLength)
    {
        return cyFullCRC32( sData, ulLength );
    }

    hFORCEINLINE HEARTBASE_SLIBEXPORT
	hUint32 HEART_API StringCRC( const hChar* string )
    {
        return cyStringCRC32( string );
    }

}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif