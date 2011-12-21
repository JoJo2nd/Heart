#ifndef _CCRC32_H
#define _CCRC32_H

#include "hTypes.h"
#include "cryptoCRC32.h"

#define CRYPTO_CRC32_POLYNOMIAL 0x04c11db7 //@This is the official polynomial used by CRC-32 in PKZip, WinZip and Ethernet.
#define CRYPTO_CRC32BUFSZ 1024 //@Used for FileCRC()

namespace Heart
{
	//-----------------------------------------------------------------------
	///
	/// @class huCRC32
	/// @brief class to generate crc's of data blocks and strings
	/// @author James Moran
	/// @date [24/6/2008]
	/// @version 1.0
	///
	//-----------------------------------------------------------------------
	class hCRC32
	{
	public:
		/**
		* 
		*
		* @param 		sData - data to use to generate the crc
		* @param 		ulLength - lenght of the data
		* @return   	unsigned long - the crc of sData
		*/
        static hUint32 FullCRC(const char *sData, long ulLength)
        {
            return cyFullCRC32( sData, ulLength );
        }
		/**
		* 
		*
		* @param 		string
		* @return   	unsigned long __stdcall
		*/
		static hUint32 StringCRC( const char* string )
        {
            return cyStringCRC32( string );
        }

	};
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif