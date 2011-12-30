/********************************************************************
	created:	2008/07/13
	created:	13:7:2008   9:32
	filename: 	hmVector.h
	author:		James Moran
	
	purpose:	
*********************************************************************/
#ifndef MATH_VECTOR_H__
#define MATH_VECTOR_H__

#include "hTypes.h"
#include <xnamath.h>

typedef XMVECTOR            hVec128;

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