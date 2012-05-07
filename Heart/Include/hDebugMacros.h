#ifndef hcDebugMacros_h__
#define hcDebugMacros_h__

#define WARNING_NONE ( 0 )
#define WARNING_HIGH ( 1 )
#define WARNING_MED ( 2 )
#define WARNING_LOW ( 3 )

//#define HEART_DO_ASSERTS

#if defined HEART_DEBUG || defined HEART_DO_ASSERTS

#define MAX_WARNING_LEVEL ( WARNING_HIGH )

#define hcPrintf				hcOutputString
#define hcAssert( x )			if ( !(x) ) { hcBreak; }
#define hcAssertMsg( x, y,...)	if ( !(x) ) { hcPrintf( y, __VA_ARGS__ ); hcBreak; }
#define hcAssertFailMsg( y, ... ) { hcPrintf( y, __VA_ARGS__ ); hcBreak; }
#define hcCompileTimeAssert( expr, msg ) { char unnamed[(expr) ? 1 : 0]; }

#define hcWarningHigh( cond, msg, ... )				hcWarning( WARNING_HIGH, cond, msg, __VA_ARGS__ )
#define hcWarningMed( cond, msg, ... )				hcWarning( WARNING_MED, cond, msg, __VA_ARGS__ )
#define hcWarningLow( cond, msg, ... )				hcWarning( WARNING_LOW, cond, msg, __VA_ARGS__ )
#define hcWarning( lvl, cond, x, ... )				if ( lvl <= MAX_WARNING_LEVEL && cond ) { hcPrintf( "WARNING!:"x, __VA_ARGS__ ); }

extern "C" void hcOutputStringRaw( const hChar* msg, ... );
extern "C" void hcOutputString( const hChar* msg, ... );

#define hcBreak					__asm { int 3 } 

#elif defined HEART_RELEASE
//#else

#define MAX_WARNING_LEVEL ( WARNING_NONE )

#define hcPrintf		hcOutputString

#define hcAssert		__noop
#define hcAssertMsg		__noop
#define hcAssertFailMsg __noop
#define hcBreak	
#define hcCompileTimeAssert

extern "C" void hcOutputString( const hChar* msg, ... );

#define hcWarningHigh( cond, msg, ... )				hcWarning( WARNING_HIGH, cond, msg, __VA_ARGS__ )
#define hcWarningMed( cond, msg, ... )				hcWarning( WARNING_MED, cond, msg, __VA_ARGS__ )
#define hcWarningLow( cond, msg, ... )				hcWarning( WARNING_LOW, cond, msg, __VA_ARGS__ )
#define hcWarning( lvl, cond, x, ... )				__noop

#ifndef HEART_RELEASE
#define HEART_RELEASE
#endif 

#else

#error ("Need to define release or debug")

#endif

#endif // hcDebugMacros_h__