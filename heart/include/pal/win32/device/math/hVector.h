/********************************************************************

	filename: 	hVector.h	
	
	Copyright (c) 1:4:2012 James Moran
	
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

#ifndef MATH_VECTOR_H__
#define MATH_VECTOR_H__

#if defined (HEART_USE_XNAMATH)
    typedef XMVECTOR            hVec128;
#else
    typedef DirectX::XMVECTOR   hVec128;
    using DirectX::XMLoadFloat;
    using DirectX::XMStoreFloat;
    using DirectX::XMVectorGetX;
    using DirectX::XMVectorGetY;
    using DirectX::XMVectorGetZ;
    using DirectX::XMVectorGetW;
    using DirectX::XMVectorSetX;
    using DirectX::XMVectorSetY;
    using DirectX::XMVectorSetZ;
    using DirectX::XMVectorSetW;
    using DirectX::XMVectorSplatX;
    using DirectX::XMVectorSplatY;
    using DirectX::XMVectorSplatZ;
    using DirectX::XMVectorSplatW;
    using DirectX::XMVectorAbs;
    using DirectX::XMVectorMin;
    using DirectX::XMVectorMax;
    using DirectX::XMVectorLess;
    using DirectX::XMVectorLessOrEqual;
    using DirectX::XMVectorEqual;
    using DirectX::XMVectorEqualR;
    using DirectX::XMVectorGreaterOrEqualR;
    using DirectX::XMComparisonAllFalse;
    using DirectX::XMVectorGreaterOrEqual;
    using DirectX::XMVectorGreater;
    using DirectX::XMVectorGreaterR;
    using DirectX::XMComparisonAllTrue;
#endif

namespace Heart
{

    hFORCEINLINE void hVec128Load( hVec128& v, const hFloat* f )
    {
        v = XMLoadFloat( f );
    }

    hFORCEINLINE void hVec128Store( const hVec128& v, hFloat* f )
    {
        XMStoreFloat( f, v );
    }

    hFORCEINLINE hFloat hVec128GetX( const hVec128& v )
    {
        return XMVectorGetX( v );
    }

    hFORCEINLINE hFloat hVec128GetY( const hVec128& v )
    {
        return XMVectorGetY( v );
    }

    hFORCEINLINE hFloat hVec128GetZ( const hVec128& v )
    {
        return XMVectorGetZ( v );
    }

    hFORCEINLINE hFloat hVec128GetW( const hVec128& v )
    {
        return XMVectorGetW( v );
    }

    hFORCEINLINE hVec128 hVec128SetX( const hVec128& v, hFloat x )
    {
        return XMVectorSetX( v, x );
    }

    hFORCEINLINE hVec128 hVec128SetY( const hVec128& v, hFloat y )
    {
        return XMVectorSetY( v, y );
    }

    hFORCEINLINE hVec128 hVec128SetZ( const hVec128& v, hFloat z )
    {
        return XMVectorSetZ( v, z );
    }

    hFORCEINLINE hVec128 hVec128SetW( const hVec128& v, hFloat w )
    {
        return XMVectorSetW( v, w );
    }

    hFORCEINLINE hVec128 hVec128SplatX( const hVec128& v )
    {
        return XMVectorSplatX( v );
    }

    hFORCEINLINE hVec128 hVec128SplatY( const hVec128& v )
    {
        return XMVectorSplatY( v );
    }

    hFORCEINLINE hVec128 hVec128SplatZ( const hVec128& v )
    {
        return XMVectorSplatZ( v );
    }

    hFORCEINLINE hVec128 hVec128SplatW( const hVec128& v )
    {
        return XMVectorSplatW( v );
    }

    hFORCEINLINE hVec128 hVec128Abs( const hVec128& v )
    {
        return XMVectorAbs( v );
    }

    hFORCEINLINE hVec128 hVec128Min( const hVec128& lhs, const hVec128& rhs )
    {
        return XMVectorMin( lhs, rhs );
    }

    hFORCEINLINE hVec128 hVec128Max( const hVec128& lhs, const hVec128& rhs )
    {
        return XMVectorMax( lhs, rhs );
    }

    hFORCEINLINE hVec128 hVec128Less( const hVec128& lhs, const hVec128& rhs )
    {
        return XMVectorLess( lhs, rhs );
    }

    hFORCEINLINE hVec128 hVec128LessEqual( const hVec128& lhs, const hVec128& rhs )
    {
        return XMVectorLessOrEqual( lhs, rhs );
    }

    hFORCEINLINE hVec128 hVec128Equal( const hVec128& lhs, const hVec128& rhs )
    {
        return XMVectorEqual( lhs, rhs );
    }

    hFORCEINLINE hVec128 hVec128GreaterEqual( const hVec128& lhs, const hVec128& rhs )
    {
        return XMVectorGreaterOrEqual( lhs, rhs );
    }

    hFORCEINLINE hVec128 hVec128Greater( const hVec128& lhs, const hVec128& rhs )
    {
        return XMVectorGreater( lhs, rhs );
    }

    hFORCEINLINE hBool hVec128AllLess( const hVec128& lhs, const hVec128& rhs )
    {
        UINT r;
        XMVectorGreaterOrEqualR( &r, lhs, rhs );
        return XMComparisonAllFalse( r );
    }

    hFORCEINLINE hBool hVec128AllLessEqual( const hVec128& lhs, const hVec128& rhs )
    {
        UINT r;
        XMVectorGreaterR( &r, lhs, rhs );
        return XMComparisonAllFalse( r );
    }

    hFORCEINLINE hBool hVec128AllEqual( const hVec128& lhs, const hVec128& rhs )
    {
        UINT r;
        XMVectorEqualR( &r, lhs, rhs );
        return XMComparisonAllTrue( r );
    }

    hFORCEINLINE hBool hVec128AllGreaterEqual( const hVec128& lhs, const hVec128& rhs )
    {
        UINT r;
        XMVectorGreaterOrEqualR( &r, lhs, rhs );
        return XMComparisonAllTrue( r );
    }

    hFORCEINLINE hBool hVec128AllGreater( const hVec128& lhs, const hVec128& rhs )
    {
        UINT r;
        XMVectorGreaterR( &r, lhs, rhs );
        return XMComparisonAllTrue( r );
    }

    hFORCEINLINE hUint32 hVec128LessSelect( const hVec128& lhs, const hVec128& rhs )
    {
        UINT r;
        XMVectorGreaterOrEqualR( &r, lhs, rhs );
        return r;
    }

    hFORCEINLINE hUint32 hVec128LessEqualSelect( const hVec128& lhs, const hVec128& rhs )
    {
        UINT r;
        XMVectorGreaterR( &r, lhs, rhs );
        return r;
    }

    hFORCEINLINE hUint32 hVec128EqualSelect( const hVec128& lhs, const hVec128& rhs )
    {
        UINT r;
        XMVectorEqualR( &r, lhs, rhs );
        return r;
    }

    hFORCEINLINE hUint32 hVec128GreaterEqualSelect( const hVec128& lhs, const hVec128& rhs )
    {
        UINT r;
        XMVectorGreaterOrEqualR( &r, lhs, rhs );
        return r;
    }

    hFORCEINLINE hUint32 hVec128GreaterSelect( const hVec128& lhs, const hVec128& rhs )
    {
        UINT r;
        XMVectorGreaterR( &r, lhs, rhs );
        return r;
    }


    struct hFloatInVec
    {
        hVec128 v;

        hFloatInVec() {}
        hFloatInVec( const hVec128& rhs ) : v(rhs) {}

        hFORCEINLINE operator hFloat ()
        {
            hFloat a;
            hVec128Store( v, &a );
            return a;
        }
        hFORCEINLINE operator hVec128 () const { return v; }
        hFORCEINLINE hVec128 Get128() const { return v; }
        hFORCEINLINE hFloatInVec& operator = ( hFloat b )
        {
            hVec128Load( v, &b );
            return *this;
        }
    };
}

#endif// MATH_VECTOR;