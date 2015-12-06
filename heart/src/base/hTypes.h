/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/
#pragma once

#include "base/hHeartConfig.h"
#include "base/hTypeTraits.h"

#include <stddef.h>
#include <stdint.h>
#include <time.h>
#include <float.h>

typedef	char				hChar;
typedef unsigned char		hByte;
typedef char                hInt8;
typedef unsigned char       hUint8;
typedef unsigned short		hUint16;
typedef short				hInt16;
typedef unsigned long		hUint32;
typedef long				hInt32;
typedef unsigned long long  hUint64;
typedef long long			hInt64;
typedef int					hInt;
typedef unsigned int        hUint;
typedef size_t              hSize_t;
typedef float				hFloat;
typedef hUint16             hHalfFloat;
typedef double				hDouble;
typedef bool				hBool;
typedef time_t              hTime;
typedef uintptr_t           hUintptr_t;
typedef ptrdiff_t           hPtrdiff_t;
struct HEART_CLASS_EXPORT hUuid_t {
    hByte internalData[16];
    bool operator == (const hUuid_t& rhs) const;
};

class hNullType {};
struct hcEmptyType {};

namespace Heart
{
    struct hResourceBinHeader
    {
        union{
            hUint32 resourceType;
            hChar   fourCC[4];
        };
        hUint64 key;
        hUint32 pad;
    };
}

#if defined (PLATFORM_WINDOWS)
#   define HEART_ALIGN_VAR( alignment, type, name ) type __declspec( align(alignment) ) name
#   define HEART_ALIGNMENT_BEGIN( x ) __declspec( align(x) )
#   define HEART_ALIGNMENT_END( x )
#elif defined (PLATFORM_LINUX)
#   define HEART_ALIGN_VAR( alignment, type, name )
#   define HEART_ALIGNMENT_BEGIN( x )
#   define HEART_ALIGNMENT_END( x )
#else
#   error ("Unknown platform")
#endif

#if defined (HEART_PLAT_WINDOWS)
#   define hRestrict     __restrict
#elif defined (HEART_PLAT_LINUX)
#   define hRestrict     __restrict__
#else
#   error ("Unknown platform ")
#endif

namespace Heart
{
    class ClassTypeBase
    {
    public:
        virtual const hUint32* GetTypeID() const = 0;

    };

    template< typename _Ty >
    class ClassType : public ClassTypeBase
    {
    public:

        const hUint32* GetTypeID() const { return &ID; }

    private:

        static const hUint32 ID = 0;

    };

    template< typename _Ty, typename _Sy >
    bool ClassTypesMatch( ClassType< _Ty > a, ClassType< _Sy > b ) { return a.GetTypeID() == b.GetTypeID(); }
}

#define	hFalse				(false)
#define	hTrue				(true)
#define hErrorCode			(~0)

#define hAlignPower2( x, align ) ( ( (hUintptr_t)x + (hUintptr_t)align - 1 ) & ~((hUintptr_t)align - 1) )
#define hPadPower2( x, align ) ( (align + ((x - 1) & ~(align - 1))) - x )
#define hAlign( x, align ) ( (hUintptr_t)(x) + align - ((hUintptr_t)(x) % align) )
#define hAlloca( x ) alloca( x )
#define hOffsetOf( s, m ) (hUintptr_t)(&(((s*)0)->m))
#define MAX_LUA_FILESYSTEMS  (6)

//#define hStaticArraySize( x ) (sizeof(x)/sizeof(x[0]))
//Improved SizeofArray
#   ifndef HEART_PLAT_WINDOWS
    template <typename t_ty, hSize_t n> 
    constexpr hSize_t hArraySize(const t_ty (&)[n]) { return n; }
    #define hStaticArraySize(x) hArraySize(x)
    #define hStaticArraySizeConstExpr(x) hArraySize(x)
#else
    template < typename t_ty, hSize_t n >
    hSize_t hArraySize(const t_ty (&)[n]) { return n; }
    #define hStaticArraySize(x) hArraySize(x)
    #define hStaticArraySizeConstExpr(x) (sizeof(x)/sizeof(x[0]))
#   endif

#define HEART_PRIVATE_COPY(x) private: x(const x&); x& operator = (const x&)

#ifndef NULL
    #define NULL 0
#endif

#ifdef HEART_PLAT_WINDOWS 
#   if _MSC_VER < 1500
#       define hAlignOf(x) __alignof(x)
#       define hNullptr     (NULL)
#   else
#       define hAlignOf(x)  hAlignment_of<##x>::value
#       define hNullptr     (nullptr)
#   endif
#elif defined (HEART_PLAT_LINUX)
#       define hAlignOf(x)  hAlignment_of< x >::value
#       define hNullptr     (nullptr)
#else
#   error ("Unknown platform ")
#endif

#define hMAKE_FOURCC(a,b,c,d) \
        ((DWORD)(BYTE)(a)        | ((DWORD)(BYTE)(b) << 8) |       \
        ((DWORD)(BYTE)(c) << 16) | ((DWORD)(BYTE)(d) << 24 ))

#include "base/hDebugMacros.h"
#include "base/hAtomic.h"

inline hHalfFloat hFloatToHalf(hFloat f) {
    union Data32 {
        unsigned int u32;
        float f32;
    };

    Data32 d;
    d.f32 = f;

    unsigned int sign = d.u32 >> 31;
    unsigned int exponent = (d.u32 >> 23) & ((1 << 8) - 1);
    unsigned int mantissa = d.u32 & ((1 << 23) - 1);;

    if (exponent == 0) {
        // zero or denorm -> zero
        mantissa = 0;

    }
    else if (exponent == 255 && mantissa != 0) {
        // nan -> infinity
        exponent = 31;
        mantissa = 0;

    }
    else if (exponent >= 127 - 15 + 31) {
        // overflow or infinity -> infinity
        exponent = 31;
        mantissa = 0;

    }
    else if (exponent <= 127 - 15) {
        // underflow -> zero
        exponent = 0;
        mantissa = 0;

    }
    else {
        exponent -= 127 - 15;
        mantissa >>= 13;
    }

    return (hHalfFloat)((sign << 15) | (exponent << 10) | mantissa);
}

#define hMin( x, y ) (( x < y ) ? x : y)
#define hMax( x, y ) (( x > y ) ? x : y)
#define hClamp(a, v1, v2) (hMin(hMax(a, v1), v2))

#define hFabs( x ) (fabs( x ))
#define hFloor( x ) (floor(x))

#if defined (_MSC_VER)
#   define hNoExcept(func) __declspec(nothrow) func
#else 
#   define hNoExcept(func) func noexcept 
#endif

#include "cryptoMurmurHash.h"

namespace std {
    template<>
    struct hash < hUuid_t > {
        size_t operator () (const hUuid_t& rhs) const { hUint32 r; cyMurmurHash3_x86_32(rhs.internalData, (hUint)hStaticArraySize(rhs.internalData), hGetMurmurHashSeed(), &r); return r; }
    };
}

