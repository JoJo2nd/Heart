/********************************************************************

	filename: 	hCRC32.cpp	
	
	Copyright (c) 1:4:2012 James Moran
	
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
	unsigned long ulTable[256] = // CRC lookup table array.
    {
        2428444049,3887607047,2125561021,162941995,2547177864,3772115230,2044508324,249268274,2657392035,3915621685,1886057615,124634137,2567524794,3993919788,1996959894,0,
        2366115317,4195302755,1661365465,335633487,2321926636,4251122042,1684777152,325883990,2211677639,4107580753,1843258603,450548861,2227061214,4089016648,1789927666,498536548,
        2882616665,3705015759,1172266101,853044451,2898065728,3686517206,1119000684,901097722,2768942443,3524101629,1258607687,1006888145,2724688242,3579855332,1281953886,997073096,
        3060149565,3244367275,1483230225,795835527,2970347812,3322730930,1594198024,671266974,3099436303,3485111705,1454621731,565507253,3218104598,3369554304,1373503546,651767980,
        3865271297,2439277719,141376813,2137656763,3775830040,2517215374,251722036,2013776290,3904427059,2680153253,112637215,1907459465,4023717930,2563907772,31158534,1994146192,
        4224994405,2362670323,366619977,1658658271,4240017532,2344532202,314042704,1706088902,4111451223,2181625025,453092731,1812370925,4066508878,2238001368,476864866,1802195444,
        3708648649,2852801631,855842277,1141124467,3663771856,2909243462,879679996,1131014506,3554079995,2765210733,1037604311,1256170817,3569037538,2747007092,984961486,1303535960,
        3233442989,3082640443,783551873,1504918807,3352799412,2966460450,702138776,1591671054,3462522015,3110523913,544179635,1466479909,3373015174,3188396048,654459306,1342533948,
        2097651377,167816743,2466906013,3826175755,2053790376,225274430,2489596804,3814918930,1943803523,81470997,2647816111,3939845945,1957810842,62317068,2596254646,3988292384,
        1622183637,397917763,2394877945,4189708143,1742555852,282753626,2312317920,4275313526,1852507879,426522225,2154129355,4150417245,1762050814,503444072,2265490386,4027552580,
        1181335161,829329135,2825379669,3747672003,1090812512,906185462,2936675148,3624741850,1219638859,1068828381,2797360999,3518719985,1340076626,953729732,2714866558,3604390888,
        1541320221,752459403,3050360625,3268935591,1555261956,733239954,2998733608,3317316542,1426400815,570562233,3138078467,3423369109,1382605366,628085408,3160834842,3412177804,
        198958881,2094854071,3855990285,2463272603,213261112,2075208622,3803740692,2512341634,83908371,1913087877,3943577151,2617837225,40735498,1969922972,3965973030,2607071920,
        376229701,1634467795,4167216745,2405801727,285281116,1711684554,4279200368,2282248934,414664567,1873836001,4139329115,2176718541,534414190,1759359992,4057260610,2262029012,
        817233897,1202900863,3736837829,2847714899,936918000,1088359270,3654703836,2932959818,1047427035,1231636301,3495958263,2808555105,956543938,1308918612,3608007406,2685067896,
        755167117,1510334235,3272380065,3020668471,711928724,1567103746,3294710456,3009837614,601450431,1423857449,3453421203,3134207493,615818150,1404277552,3401237130,3183342108,
    };
	bool initialised_ = false;

// 	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 
// 	void hCRC32::Initialise(void)
// 	{
// //         hcAssert( !initialised_ );
// // 		memset(&ulTable, 0, sizeof(ulTable));
// // 
// // 		// 256 values representing ASCII character codes.
// // 		for(int iCodes = 0; iCodes <= 0xFF; iCodes++)
// // 		{
// // 			ulTable[iCodes] = Reflect(iCodes, 8) << 24;
// // 
// // 			for(int iPos = 0; iPos < 8; iPos++)
// // 			{
// // 				ulTable[iCodes] = (ulTable[iCodes] << 1) ^
// // 					(ulTable[iCodes] & (1 << 31) ? CRYPTO_CRC32_POLYNOMIAL : 0);
// // 			}
// // 
// // 			ulTable[iCodes] = Reflect(ulTable[iCodes], 32);
// // 		}
// // 
// //         hcPrintf( "{");
// //         for(int iCodes = 0; iCodes <= 0xFF; )
// //         {
// //             hcPrintf( "%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,\n",
// //                 ulTable[iCodes++],ulTable[iCodes++],ulTable[iCodes++],ulTable[iCodes++],ulTable[iCodes++],ulTable[iCodes++],ulTable[iCodes++],ulTable[iCodes++],
// //                 ulTable[iCodes++],ulTable[iCodes++],ulTable[iCodes++],ulTable[iCodes++],ulTable[iCodes++],ulTable[iCodes++],ulTable[iCodes++],ulTable[iCodes++] );
// //         }
// //         hcPrintf( "}");
// // 
// // 
// // 		initialised_ = true;
// 	}
/*
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Reflection is a requirement for the official CRC-32 standard.
	//	You can create CRCs without it, but they won't conform to the standard.

	unsigned long hCRC32::Reflect(unsigned long ulReflect, char cChar)
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

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	unsigned long hCRC32::FullCRC( const char *sData, long ulLength)
	{
		hcAssert( initialised_ );
		long ulCRC = 0xffffffff;
		PartialCRC(&ulCRC, sData, ulLength);
		return ulCRC ^ 0xffffffff;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	//For Example usage example, see FileCRC().
	void hCRC32::PartialCRC( long *ulInCRC, const char *sData, long ulLength)
	{
		while(ulLength--)
		{
			*ulInCRC = (*ulInCRC >> 8) ^ ulTable[(*ulInCRC & 0xFF) ^ *sData++];
		}
	}

	/////////////////////////////////////////////////////////////////////////

	function: StringCRC - 

	purpose: 

	author: James Moran

	/////////////////////////////////////////////////////////////////////////
	unsigned long __stdcall hCRC32::StringCRC( const char* string )
	{
		return FullCRC( reinterpret_cast< const char* >( string ), static_cast< long >( strlen( string ) ) );
	}
*/
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////