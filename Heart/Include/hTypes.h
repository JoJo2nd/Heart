#ifndef hcTypes_h__
#define hcTypes_h__


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
#define hErrorCode			0xFFFFFFFF

#define hAlignPower2( x, align ) ( (hUint32)x + (hUint32)align - 1 ) & ~((hUint32)align - 1) )
#define hPadPower2( x, align ) ( (align + ((x - 1) & ~(align - 1))) - x )
#define hAlign( x, align ) ( x + align - (x % align) )
#define hAlloca( x ) alloca( x )
#define hOffsetOf( s, m ) (hUint32)(&(((s*)0)->m))
#define MAX_LUA_FILESYSTEMS  (6)
#define hStaticArraySize( x ) (sizeof(x)/sizeof(x[0]))

#ifndef NULL
	#define NULL 0
#endif

#ifdef HEART_DLL

#define DLLEXPORT  __declspec(dllexport)

#else

#define DLLEXPORT __declspec(dllexport)

#endif

#ifndef hFORCEINLINE

#define hFORCEINLINE __forceinline

#endif

#include "hReflection.h"
#include "hSerialiser.h"
//#include "hMemory.h"

#endif // hcTypes_h__