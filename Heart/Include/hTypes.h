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

#define HEART_ALIGN( align ) __declspec( align(align) )
#define HEART_ALIGNMENT( x, align ) __declspec( align(align) ) x

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

// #define BIT1			(1)
// #define BIT2			(1 << 1)
// #define BIT3			(1 << 2)
// #define BIT4			(1 << 3)
// #define BIT5			(1 << 4)
// #define BIT6			(1 << 5)
// #define BIT7			(1 << 6)
// #define BIT8			(1 << 7)
// #define BIT9			(1 << 8)
// #define BIT10			(1 << 9)
// #define BIT11			(1 << 10)
// #define BIT12			(1 << 11)
// #define BIT13			(1 << 12)
// #define BIT14			(1 << 13)
// #define BIT15			(1 << 14)
// #define BIT16			(1 << 15)
// #define BIT17			(1 << 16)
// #define BIT18			(1 << 17)
// #define BIT19			(1 << 18)
// #define BIT20			(1 << 19)
// #define BIT21			(1 << 20)
// #define BIT22			(1 << 21)
// #define BIT23			(1 << 22)
// #define BIT24			(1 << 23)
// #define BIT25			(1 << 24)
// #define BIT26			(1 << 25)
// #define BIT27			(1 << 26)
// #define BIT28			(1 << 27)
// #define BIT29			(1 << 28)
// #define BIT30			(1 << 29)
// #define BIT31			(1 << 30)
// #define BIT32			(1 << 31)

#define	hFalse				(false)
#define	hTrue				(true)
#define hErrorCode			0xFFFFFFFF

#define hAlignPower2( x, align ) ( (hUint32)x + (hUint32)align - 1 ) & ~((hUint32)align - 1) )
#define hPadPower2( x, align ) ( (align + ((x - 1) & ~(align - 1))) - x )
#define hAlign( x, align ) ( x + align - (x % align) )
#define hAlloca( x ) alloca( x )
#define MAX_LUA_FILESYSTEMS  (6)

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