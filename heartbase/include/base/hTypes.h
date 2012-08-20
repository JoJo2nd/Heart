/********************************************************************

	filename: 	hTypes.h	
	
	Copyright (c) 31:3:2012 James Moran
	
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
#ifndef hcTypes_h__
#define hcTypes_h__

#include <type_traits>

typedef	char				hChar;
typedef unsigned char		hByte;
typedef unsigned short		hUint16;
typedef short				hInt16;
typedef unsigned long		hUint32;
typedef long				hInt32;
typedef unsigned long long  hUint64;
typedef long long			hInt64;
typedef int					hInt;
typedef float				hFloat;
typedef double				hDouble;
typedef bool				hBool;
typedef time_t              hTime;

class hNullType {};
struct hcEmptyType {};

#define HEART_ALIGN_VAR( alignment, type, name ) type __declspec( align(alignment) ) name
#define HEART_ALIGNMENT_BEGIN( x ) __declspec( align(x) )
#define HEART_ALIGNMENT_END( x )

#ifdef HEART_DEBUG
    #ifndef HEART_TRACK_MEMORY_ALLOCS
        #define HEART_TRACK_MEMORY_ALLOCS
    #endif
#endif

#if defined (HEART_PLAT_WINDOWS)
    #define HEART_THREAD_LOCAL  __declspec(thread)
#else
    #error ("Unknown platform ")
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
#define hErrorCode			(~0U)

#define hAlignPower2( x, align ) ( (hUint32)x + (hUint32)align - 1 ) & ~((hUint32)align - 1) )
#define hPadPower2( x, align ) ( (align + ((x - 1) & ~(align - 1))) - x )
#define hAlign( x, align ) ( x + align - (x % align) )
#define hAlloca( x ) alloca( x )
#define hOffsetOf( s, m ) (hUint32)(&(((s*)0)->m))
#define MAX_LUA_FILESYSTEMS  (6)
#define hStaticArraySize( x ) (sizeof(x)/sizeof(x[0]))

#define HEART_PRIVATE_COPY(x) private: x(const x&); x& operator = (const x&)

#ifndef NULL
	#define NULL 0
#endif

#ifdef HEART_PLAT_WINDOWS
    #define hAlignOf(x) __alignof(x)
#else
    #define hAlignOf(x) std::alignment_of<double>::value>
#endif

#ifndef hFORCEINLINE

#define hFORCEINLINE __forceinline

#endif

#include "base/hDebugMacros.h"
#include "base/hAtomic.h"

#define hMin( x, y ) (( x < y ) ? x : y)
#define hMax( x, y ) (( x > y ) ? x : y)

#define hFabs( x ) (fabs( x ))
#define hFloor( x ) (floor(x))

#endif // hcTypes_h__