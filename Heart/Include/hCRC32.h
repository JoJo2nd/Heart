#ifndef _CCRC32_H
#define _CCRC32_H

#define CRYPTO_CRC32_POLYNOMIAL 0x04c11db7 //@This is the official polynomial used by CRC-32 in PKZip, WinZip and Ethernet.
#define CRYPTO_CRC32BUFSZ 1024 //@Used for FileCRC()

namespace Heart
{
namespace hCRC32
{

    hFORCEINLINE void StartCRC32( hUint32* crc, const hChar* data, hUint32 len )
    {
        hcAssert( crc );
        cyStartCRC32( crc, (const hByte*)data, len );
    }

    hFORCEINLINE void ContinueCRC32( hUint32* crc, const hChar* data, hUint32 len )
    {
        hcAssert( crc );
        cyContinueCRC32( crc, (const hByte*)data, len );
    }

    hFORCEINLINE hUint32 FinishCRC32( hUint32* crc )
    {
        hcAssert( crc );
        return cyFinaliseCRC32( crc );
    }

    hFORCEINLINE hUint32 FullCRC( const hChar* sData, hUint32 ulLength)
    {
        return cyFullCRC32( sData, ulLength );
    }

	hFORCEINLINE hUint32 StringCRC( const hChar* string )
    {
        return cyStringCRC32( string );
    }

}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif