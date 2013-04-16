#ifndef hcDebugMacros_h__
#define hcDebugMacros_h__

#define WARNING_NONE ( 0 )
#define WARNING_HIGH ( 1 )
#define WARNING_MED ( 2 )
#define WARNING_LOW ( 3 )

//#define HEART_DO_ASSERTS

typedef void (*hPrintfCallback)(const hChar*);
HEART_DLLEXPORT void HEART_API hcSetOutputStringCallback(hPrintfCallback cb);

#if defined HEART_DEBUG || defined HEART_DO_ASSERTS

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
                                hUint32 ret = hAssertMsgFunc(ignore, __FILE__"(%u) Assert Failed: ("#x ")\n" #y, __LINE__, __VA_ARGS__);\
                                if (ret == 0) exit(-1);\
                                if (ret == 1) hcBreak;\
                                if (ret == 2) ignore = hTrue; }}
#define hcAssertFailMsg( y, ... ) { static hBool ignore = hFalse; \
                                    hUint32 ret = hAssertMsgFunc(ignore,__FILE__"(%u) Assert Failed: " #y, __LINE__, __VA_ARGS__);\
                                    if (ret == 0) exit(-1);\
                                    if (ret == 1) hcBreak;\
                                    if (ret == 2) ignore = hTrue; }
#define hcCompileTimeAssert( expr, msg ) { char unnamed[(expr) ? 1 : 0]; }

#define hcWarningHigh( cond, msg, ... )				hcWarning( WARNING_HIGH, cond, msg, __VA_ARGS__ )
#define hcWarningMed( cond, msg, ... )				hcWarning( WARNING_MED, cond, msg, __VA_ARGS__ )
#define hcWarningLow( cond, msg, ... )				hcWarning( WARNING_LOW, cond, msg, __VA_ARGS__ )
#define hcWarning( lvl, cond, x, ... )				if ( lvl <= MAX_WARNING_LEVEL && cond ) { hcPrintf( "WARNING!: "x, __VA_ARGS__ ); }

HEART_DLLEXPORT void HEART_API hcOutputStringRaw(const hChar* msg, ...);
HEART_DLLEXPORT void HEART_API hcOutputString(const hChar* msg, ...);
HEART_DLLEXPORT hUint32 HEART_API hAssertMsgFunc(hBool ignore, const hChar* msg, ...);

#ifdef HEART_64BIT
#   define hcBreak __debugbreak()
#else
#   define hcBreak __asm { int 3 } 
#endif

#elif defined HEART_RELEASE
//#else

#define MAX_WARNING_LEVEL ( WARNING_NONE )

#define hcPrintf		hcOutputString

#define hcAssert		__noop
#define hcAssertMsg		__noop
#define hcAssertFailMsg __noop
#define hcBreak	
#define hcCompileTimeAssert

HEART_DLLEXPORT void HEART_API hcOutputString( const hChar* msg, ... );

#define hcWarningHigh( cond, msg, ... )				hcWarning( WARNING_HIGH, cond, msg, __VA_ARGS__ )
#define hcWarningMed( cond, msg, ... )				hcWarning( WARNING_MED, cond, msg, __VA_ARGS__ )
#define hcWarningLow( cond, msg, ... )				hcWarning( WARNING_LOW, cond, msg, __VA_ARGS__ )
#define hcWarning( lvl, cond, x, ... )				__noop

#else

#error ("Need to define release or debug")

#endif

#endif // hcDebugMacros_h__