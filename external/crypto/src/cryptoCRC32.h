/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/
#pragma once

#ifndef CRYPTOCRC32_H__
#define CRYPTOCRC32_H__

#include "cryptoCommon.h"
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif//

    CY_DLLEXPORT cyUint32 CY_API cyFullCRC32( const cyByte* data, cyUint len );
    CY_DLLEXPORT void     CY_API cyStartCRC32( cyUint32* incrc, const cyByte* data, cyUint32 len );
    CY_DLLEXPORT void     CY_API cyContinueCRC32( cyUint32* incrc, const cyByte* data, cyUint32 len );
    CY_DLLEXPORT cyUint32 CY_API cyFinaliseCRC32( cyUint32* incrc );

#define cyStringCRC32( string ) cyFullCRC32((const cyByte*)string, (cyUint32)strlen(string ) )

#ifdef __cplusplus
};
#endif

#endif // CRYPTOCRC32_H__

