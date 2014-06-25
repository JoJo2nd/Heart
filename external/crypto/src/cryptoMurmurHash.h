//-----------------------------------------------------------------------------
// MurmurHash3 was written by Austin Appleby, and is placed in the public
// domain.

#ifndef _MURMURHASH3_H_
#define _MURMURHASH3_H_

#include "cryptoCommon.h"

//-----------------------------------------------------------------------------
// Platform-specific functions and macros

// Microsoft Visual Studio

#if defined(_MSC_VER) && (_MSC_VER < 1600)

typedef unsigned char uint8_t;
typedef unsigned int uint32_t;
typedef unsigned __int64 uint64_t;

// Other compilers

#else   // defined(_MSC_VER)

#include <stdint.h>

#endif // !defined(_MSC_VER)

//-----------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif//

CY_DLLEXPORT void CY_API cyMurmurHash3_x86_32  ( const void * key, int len, uint32_t seed, void * out );
CY_DLLEXPORT void CY_API cyMurmurHash3_x86_128 ( const void * key, int len, uint32_t seed, void * out );
CY_DLLEXPORT void CY_API cyMurmurHash3_x64_128 ( const void * key, int len, uint32_t seed, void * out );

#ifdef __cplusplus
};
#endif
//-----------------------------------------------------------------------------

#endif // _MURMURHASH3_H_
