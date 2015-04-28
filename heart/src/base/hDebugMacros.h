/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#pragma once

#include "base/hTypes.h"
#if defined (PLATFORM_LINUX)
#   include <signal.h>
#   include <stdlib.h>
#endif

#define WARNING_NONE ( 0 )
#define WARNING_HIGH ( 1 )
#define WARNING_MED ( 2 )
#define WARNING_LOW ( 3 )

#if defined (PLATFORM_LINUX)
#   define __noop(...)
#endif

typedef void (*hPrintfCallback)(const hChar*);
HEART_EXPORT void HEART_API hcSetOutputStringCallback(hPrintfCallback cb);

#if HEART_DO_ASSERTS

#define MAX_WARNING_LEVEL ( WARNING_HIGH )

#define hcPrintf				hcOutputString
#define hcAssert( x )			{ static hBool ignore = hFalse; \
                                if (!(x)) {\
                                hUint32 ret = hAssertMsgFunc(ignore, __FILE__"(%u) Assert Failed: ("#x")", __LINE__);\
                                if (ret == 0) exit(-1);\
                                if (ret == 1) hcBreak;\
                                if (ret == 2) ignore = hTrue; }}
#define hcAssertMsg( x, y,...)	{ static hBool ignore = hFalse; \
                                if (!(x)) {\
                                hUint32 ret = hAssertMsgFunc(ignore, __FILE__"(%u) Assert Failed: ("#x ")\n" #y, __LINE__, ##__VA_ARGS__);\
                                if (ret == 0) exit(-1);\
                                if (ret == 1) hcBreak;\
                                if (ret == 2) ignore = hTrue; }}
#define hcAssertFailMsg( y, ... ) { static hBool ignore = hFalse; \
                                    hUint32 ret = hAssertMsgFunc(ignore,__FILE__"(%u) Assert Failed: " #y, __LINE__, ##__VA_ARGS__);\
                                    if (ret == 0) exit(-1);\
                                    if (ret == 1) hcBreak;\
                                    if (ret == 2) ignore = hTrue; }
#define hcCompileTimeAssert( expr, msg ) { char unnamed[(expr) ? 1 : 0]; }

#define hcWarningHigh( cond, msg, ... )				hcWarning( WARNING_HIGH, cond, msg, ##__VA_ARGS__ )
#define hcWarningMed( cond, msg, ... )				hcWarning( WARNING_MED, cond, msg, ##__VA_ARGS__ )
#define hcWarningLow( cond, msg, ... )				hcWarning( WARNING_LOW, cond, msg, ##__VA_ARGS__ )
#define hcWarning( lvl, cond, x, ... )				if ( lvl <= MAX_WARNING_LEVEL && cond ) { hcPrintf( "WARNING!: " x, ##__VA_ARGS__ ); }

HEART_EXPORT void HEART_API hcOutputString(const hChar* msg, ...);
HEART_EXPORT hUint32 HEART_API hAssertMsgFunc(hBool ignore, const hChar* msg, ...);

#ifdef HEART_64BIT
#   if defined(PLATFORM_WINDOWS)
#       define hcBreak __debugbreak()
#   elif defined (PLATFORM_LINUX)
#       define hcBreak raise(SIGTRAP)
#   else
#       define hcBreak "Define this!"
#   endif
#else
#   if defined(PLATFORM_WINDOWS)
#       define hcBreak __asm { int 3 }
#   elif defined (PLATFORM_LINUX)
#       define hcBreak raise(SIGTRAP)
#   else
#       define hcBreak "Define this!"
#   endif
#endif

#else
//#else

#define MAX_WARNING_LEVEL ( WARNING_NONE )

#define hcPrintf		hcOutputString

#define hcAssert		__noop
#define hcAssertMsg		__noop
#define hcAssertFailMsg __noop
#define hcBreak	
#define hcCompileTimeAssert

HEART_EXPORT void HEART_API hcOutputString( const hChar* msg, ... );

#define hcWarningHigh( cond, msg, ... )				hcWarning( WARNING_HIGH, cond, msg, __VA_ARGS__ )
#define hcWarningMed( cond, msg, ... )				hcWarning( WARNING_MED, cond, msg, __VA_ARGS__ )
#define hcWarningLow( cond, msg, ... )				hcWarning( WARNING_LOW, cond, msg, __VA_ARGS__ )
#define hcWarning( lvl, cond, x, ... )				__noop()

#endif

#define hStub() /*hcPrintf("STUBBED OUT %s", __FUNCTION__)*/
