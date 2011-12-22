/********************************************************************
	created:	2008/07/13
	created:	13:7:2008   10:27
	filename: 	Vec2.h
	author:		James Moran
	
	purpose:	
*********************************************************************/
#ifndef hmVec2_h__
#define hmVec2_h__

#include <math.h>
#include "hTypes.h"
#include <xnamath.h>

namespace Heart
{
    typedef XMVECTOR hVec2;
    typedef XMFLOAT2 hCPUVec2;

    namespace hVec2Func
    {
        //functions
        hFORCEINLINE hFloat length( const hVec2& v )
        {
            return XMVector2Length( v ); 
        }

        hFORCEINLINE hFloat lengthFast( const hVec2& v )
        {
            return XMVector2LengthEst( v );
        }

        hFORCEINLINE hFloat lengthSquare( const hVec2& v ) const
        {
            return XMVector2Length( v ); 
        }

        hFORCEINLINE hVec2& normalise( const hVec2& v )
        {
            return XMVector2Normalize( v );
        }

        hFORCEINLINE hVec2& normaliseFast( const hVec2& v )
        {
            return XMVector2NormalizeEst( v );
        }

        hFORCEINLINE hVec2& add( const hVec2& a, const hVec2& b )
        {
            return XMVectorAdd( a, b );
        }

        hFORCEINLINE hVec2& sub( const hVec2& a, const hVec2& b, hVec2& out )
        {
            return XMVectorSubtract( a, b );
        }

        hFORCEINLINE hVec2& scale( const hVec2& a, const hFloat s )
        {
            return XMVectorScale( a, s );
        }

        hFORCEINLINE hVec2& div( const hVec2& a, const hFloat d )
        {
            return XMVectorDivide( a, d );
        }

        hFORCEINLINE hFloat dot( const hVec2& a, const hVec2& b )
        {
            return XMVector2Dot( a, b );
        }

        hFORCEINLINE hVec2& neg( const hVec2& a )
        {
            return XMVectorNegate( a );
        }

        hFORCEINLINE hVec2& cross( const hVec2& a, const hVec2& b )
        {
            return XMVector2Cross( a, b );
        }

        hFORCEINLINE hVec2& set( hFloat x, hFloat y, hFloat z )
        {
            return XMVectorSet( x, y, z, 1.f );
        }

        hFORCEINLINE hBool compare( const hVec2& a, const hVec2& b )
        {
            return XMVector2Equal( a, b );
        }

        hFORCEINLINE void store( const hVec2& a, hCPUVec2* b )
        {
            XMStoreFloat2( b, a );
        }

        hFORCEINLINE void load( const hVec2& a, hCPUVec2* b )
        {
            a = XMLoadFloat2( b );
        }

    }

    hFORCEINLINE hVec2& operator + ( const hVec2& a, const hVec2& b )
    {
        return hVec2Func::add( a, b );
    }

    hFORCEINLINE hVec2& operator += ( const hVec2& a, const hVec2& b )
    {
        return hVec2Func::add( a, b );
    }

    hFORCEINLINE hVec2& operator - ( const hVec2& a, const hVec2& b )
    {
        return hVec2Func::sub( a, b );
    }

    hFORCEINLINE hVec2& operator -= ( const hVec2& a, const hVec2& b )
    {
        return hVec2Func::sub( a, b );
    }

    hFORCEINLINE hVec2& operator * ( const hVec2& v, const hFloat s )
    {
        return hVec2Func::scale( v, s );
    }

    hFORCEINLINE hVec2& operator *= ( const hVec2& v, const hFloat s )
    {
        return hVec2Func::scale( v, s );
    }

    hFORCEINLINE hVec2& operator / ( const hVec2& v, const hFloat s ) 
    {
        return hVec2Func::div( v, s );
    }

    hFORCEINLINE hVec2& operator /= ( const hVec2& v, const hFloat s )
    {
        return hVec2Func::div( v, s );
    }

    hFORCEINLINE hBool operator == ( const hVec2& a, const hVec2& b )
    {
        return hVec2Func::compare( a, b );
    }

    hFORCEINLINE hBool operator != (  const hVec2& a, const hVec2& b )
    {
        return !(a == b);
    }

    hFORCEINLINE hVec2& operator - ( const hVec2& a )
    {
        return hVec2Func::neg( a );
    }

    hFORCEINLINE hCPUVec2& operator = ( const hCPUVec2& a, const hVec2& b )
    {
        hVec2Func::store( b, &a );
    }

    hFORCEINLINE hVec2& operator = ( const hVec2& a, const hCPUVec2& b )
    {
        hVec2Func::load( a, &b );
    }

}

#endif // hmVec2_h__