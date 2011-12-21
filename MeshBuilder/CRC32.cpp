/********************************************************************

	filename: 	CEC32.cpp
	
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

#include "stdafx.h"
#include "CRC32.h"

namespace CRC32
{

	static void				Initialise();
	static void				PartialCRC( long *ulInCRC, const char *sData, long ulLength);
	static unsigned long	Reflect(unsigned long ulReflect, char cChar);

	static unsigned long	ulTable[256]; // @CRC lookup table array.
	static bool				initialised_ = true;

	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////

	void Initialise()
	{
		memset(&ulTable, 0, sizeof(ulTable));

		// 256 values representing ASCII character codes.
		for(int iCodes = 0; iCodes <= 0xFF; iCodes++)
		{
			ulTable[iCodes] = Reflect(iCodes, 8) << 24;

			for(int iPos = 0; iPos < 8; iPos++)
			{
				ulTable[iCodes] = (ulTable[iCodes] << 1) ^
					(ulTable[iCodes] & (1 << 31) ? HU_CRC32_POLYNOMIAL : 0);
			}

			ulTable[iCodes] = Reflect(ulTable[iCodes], 32);
		}

		initialised_ = true;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Reflection is a requirement for the official CRC-32 standard.
	//	You can create CRCs without it, but they won't conform to the standard.
	unsigned long Reflect(unsigned long ulReflect, char cChar)
	{
		unsigned long ulValue = 0;

		// Swap bit 0 for bit 7 bit 1 For bit 6, etc....
		for(int iPos = 1; iPos < (cChar + 1); iPos++)
		{
			if(ulReflect & 1) ulValue |= 1 << (cChar - iPos);
			ulReflect >>= 1;
		}

		return ulValue;
	}

	
	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////

	unsigned long FullCRC( const char *sData, long ulLength)
	{
		if ( !initialised_ )
		{
			Initialise();
		}
		long ulCRC = 0xffffffff;
		PartialCRC(&ulCRC, sData, ulLength);
		return ulCRC ^ 0xffffffff;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////

	void PartialCRC( long *ulInCRC, const char *sData, long ulLength)
	{
		while(ulLength--)
		{
			*ulInCRC = (*ulInCRC >> 8) ^ ulTable[(*ulInCRC & 0xFF) ^ *sData++];
		}
	}
}