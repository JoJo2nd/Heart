/********************************************************************

	filename: 	CRC32.h
	
	Copyright (c) 2011/02/15 James Moran
	
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
#pragma once

#ifndef _CRC32_H__
#define _CRC32_H__

#define HU_CRC32_POLYNOMIAL 0x04c11db7 //@This is the official polynomial used by CRC-32 in PKZip, WinZip and Ethernet.
#define HU_CRC32BUFSZ 1024 //@Used for FileCRC()

namespace CRC32
{
	/**
	* 
	*
	* @param 		sData - data to use to generate the crc
	* @param 		ulLength - lenght of the data
	* @return   	unsigned long - the crc of sData
	*/
	unsigned long FullCRC(const char *sData, long ulLength);
	/**
	* 
	*
	* @param 		string
	* @return   	unsigned long __stdcall
	*/
	inline unsigned long StringCRC( const char* string )
	{
		return FullCRC( (const char*)string, (unsigned long)strlen( string ) );
	}

}

#endif // _CRC32_H__