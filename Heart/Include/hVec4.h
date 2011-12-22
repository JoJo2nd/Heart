/********************************************************************
	created:	2009/01/06

	filename: 	Vec4.h

	author:		James Moran
	
	purpose:	
*********************************************************************/
#ifndef __HMVEC4_H__
#define __HMVEC4_H__

#include <math.h>
#include "hTypes.h"
#include <xnamath.h>

namespace Heart
{

    typedef XMVECTOR hVec4;
    typedef XMFLOAT4 hCPUVec4;

namespace hVec4Func
{
    //functions
    hFORCEINLINE hFloat length( const hVec4& v )
    {
        return XMVector4Length( v ); 
    }

    hFORCEINLINE hFloat lengthFast( const hVec4& v )
    {
        return XMVector4LengthEst( v );
    }

    hFORCEINLINE hFloat lengthSquare( const hVec4& v ) const
    {
        return XMVector4Length( v ); 
    }

    hFORCEINLINE hVec4& normalise( const hVec4& v )
    {
        return XMVector4Normalize( v );
    }

    hFORCEINLINE hVec4& normaliseFast( const hVec4& v )
    {
        return XMVector4NormalizeEst( v );
    }

    hFORCEINLINE hVec4& add( const hVec4& a, const hVec4& b )
    {
        return XMVectorAdd( a, b );
    }

    hFORCEINLINE hVec4& sub( const hVec4& a, const hVec4& b, hVec4& out )
    {
        return XMVectorSubtract( a, b );
    }

    hFORCEINLINE hVec4& scale( const hVec4& a, const hFloat s )
    {
        return XMVectorScale( a, s );
    }

    hFORCEINLINE hVec4& div( const hVec4& a, const hFloat d )
    {
        return XMVectorDivide( a, d );
    }

    hFORCEINLINE hFloat dot( const hVec4& a, const hVec4& b )
    {
        return XMVector4Dot( a, b );
    }

    hFORCEINLINE hVec4& neg( const hVec4& a )
    {
        return XMVectorNegate( a );
    }

    hFORCEINLINE hVec4& cross( const hVec4& a, const hVec4& b, const hVec4& c )
    {
        return XMVector4Cross( a, b, c );
    }

    hFORCEINLINE hVec4& set( hFloat x, hFloat y, hFloat z )
    {
        return XMVectorSet( x, y, z, 1.f );
    }

    hFORCEINLINE hBool compare( const hVec4& a, const hVec4& b )
    {
        return XMVector4Equal( a, b );
    }

    hFORCEINLINE void store( const hVec4& a, hCPUVec4* b )
    {
        XMStoreFloat4( b, a );
    }

    hFORCEINLINE void load( const hVec4& a, hCPUVec4* b )
    {
        a = XMLoadFloat4( b );
    }

}

    hFORCEINLINE hVec4& operator + ( const hVec4& a, const hVec4& b )
    {
        return hVec4Func::add( a, b );
    }

    hFORCEINLINE hVec4& operator += ( const hVec4& a, const hVec4& b )
    {
        return hVec4Func::add( a, b );
    }

    hFORCEINLINE hVec4& operator - ( const hVec4& a, const hVec4& b )
    {
        return hVec4Func::sub( a, b );
    }

    hFORCEINLINE hVec4& operator -= ( const hVec4& a, const hVec4& b )
    {
        return hVec4Func::sub( a, b );
    }

    hFORCEINLINE hVec4& operator * ( const hVec4& v, const hFloat s )
    {
        return hVec4Func::scale( v, s );
    }

    hFORCEINLINE hVec4& operator *= ( const hVec4& v, const hFloat s )
    {
        return hVec4Func::scale( v, s );
    }

    hFORCEINLINE hVec4& operator / ( const hVec4& v, const hFloat s ) 
    {
        return hVec4Func::div( v, s );
    }

    hFORCEINLINE hVec4& operator /= ( const hVec4& v, const hFloat s )
    {
        return hVec4Func::div( v, s );
    }

    hFORCEINLINE hBool operator == ( const hVec4& a, const hVec4& b )
    {
        return hVec4Func::compare( a, b );
    }

    hFORCEINLINE hBool operator != (  const hVec4& a, const hVec4& b )
    {
        return !(a == b);
    }

    hFORCEINLINE hVec4& operator - ( const hVec4& a )
    {
        return hVec4Func::neg( a );
    }

    hFORCEINLINE hCPUVec4& operator = ( const hCPUVec4& a, const hVec4& b )
    {
        hVec4Func::store( b, &a );
    }

    hFORCEINLINE hVec4& operator = ( const hVec4& a, const hCPUVec4& b )
    {
        hVec4Func::load( a, &b );
    }

}

#endif //__HMVEC4_H__