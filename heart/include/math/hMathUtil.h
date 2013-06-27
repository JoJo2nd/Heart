/********************************************************************

	filename: 	MathUtil.h	
	
	Copyright (c) 15:1:2011 James Moran
	
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

#ifndef __HMCOMMON_H__
#define __HMCOMMON_H__

#define HEART_PI						(3.14159265358979323846f)// long winded PI
#define HEART_COS35  (0.81915204428899178968448838591684)
#define HEART_COS45  (0.70710678118654752440084436210485f)
#define HEART_COS60  (0.5f)
#define HEART_COS90  (0.0f)
#define HEART_COS120 (-0.5)

#define hDegToRad( deg )			( deg * ( HEART_PI / 180.0f )) 
#define hRadToDeg( rad )			( rad * ( 180.0f/ HEART_PI ))

template< typename _Ty >
inline void hmXORSwap( _Ty& x, _Ty& y )
{
	y = ( x ^ y );
	x = ( x ^ y );
	y = ( x ^ y );
}

template< typename _Ty >
inline void hmSwap( _Ty& x, _Ty& y )
{
	_Ty i = x;
	x = y;
	y = i;
}

namespace Heart
{
    inline hFloat hCos(hFloat x) {
        return cosf(x);
    }

    inline hFloat hSin(hFloat x) {
        return sinf(x);
    }

    inline hFloat hTan(hFloat x) {
        return tanf(x);
    }

namespace Math
{
namespace Util
{
	template< typename _Ty >
	_Ty Clamp( _Ty a, _Ty amin, _Ty amax )
	{
		if ( a < amin ) return amin;
		if ( a > amax ) return amax;
		return a;
	}

	template< typename _Ty >
	_Ty SmoothStep( _Ty minv, _Ty maxv, _Ty inv )
	{
		if ( inv < minv ) return 0.0f;
		if ( inv > maxv ) return 1.0f;

		_Ty x = (inv-minv)/(maxv-minv);
		return -2.0f*( x*x*x ) + 3.0f*(x*x);
	}

	template< typename _Ty >
	_Ty Lerp( _Ty minv, _Ty maxv, _Ty inv )
	{
		if ( inv < minv ) return 0.0f;
		if ( inv > maxv ) return 1.0f;

		return (inv-minv)/(maxv-minv);
	}
}
}
}

#endif //__HMCOMMON_H__