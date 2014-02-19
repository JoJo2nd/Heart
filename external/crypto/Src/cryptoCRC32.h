/********************************************************************

	filename: 	cryptoCRC32.h	
	
	Copyright (c) 13:10:2011 James Moran
	
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

#ifndef CRYPTOCRC32_H__
#define CRYPTOCRC32_H__

#include "cryptoCommon.h"
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif//

    CY_DLLEXPORT cyUint32 CY_API cyFullCRC32( const cyChar* data, cyUint len );
    CY_DLLEXPORT void     CY_API cyStartCRC32( cyUint32* incrc, const cyByte* data, cyUint32 len );
    CY_DLLEXPORT void     CY_API cyContinueCRC32( cyUint32* incrc, const cyByte* data, cyUint32 len );
    CY_DLLEXPORT cyUint32 CY_API cyFinaliseCRC32( cyUint32* incrc );

#define cyStringCRC32( string ) cyFullCRC32( string, (cyUint32)strlen( string ) )

#ifdef __cplusplus
};
#endif

#endif // CRYPTOCRC32_H__

