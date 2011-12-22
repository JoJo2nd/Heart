/********************************************************************
	created:	2008/07/13
	created:	13:7:2008   19:50
	filename: 	Vec3.h
	author:		James Moran
	
	purpose:	
*********************************************************************/
#include <math.h>
#include "hTypes.h"
#include <xnamath.h>

namespace Heart
{

    typedef XMVECTOR hVec3;
    typedef XMFLOAT3 hCPUVec3;

namespace hVec3Func
{
	//functions
	hFORCEINLINE hFloat length( const hVec3& v )
	{
		return XMVector3Length( v ); 
	}

    hFORCEINLINE hFloat lengthFast( const hVec3& v )
    {
        return XMVector3LengthEst( v );
    }

	hFORCEINLINE hFloat lengthSquare( const hVec3& v ) const
	{
		return XMVector3Length( v ); 
	}

	hFORCEINLINE hVec3& normalise( const hVec3& v )
	{
	    return XMVector3Normalize( v );
	}

    hFORCEINLINE hVec3& normaliseFast( const hVec3& v )
    {
        return XMVector3NormalizeEst( v );
    }

	hFORCEINLINE hVec3& add( const hVec3& a, const hVec3& b )
	{
        return XMVectorAdd( a, b );
	}

	hFORCEINLINE hVec3& sub( const hVec3& a, const hVec3& b, hVec3& out )
	{
        return XMVectorSubtract( a, b );
	}

	hFORCEINLINE hVec3& scale( const hVec3& a, const hFloat s )
	{
        return XMVectorScale( a, s );
	}

	hFORCEINLINE hVec3& div( const hVec3& a, const hFloat d )
	{
        return XMVectorDivide( a, d );
	}

	hFORCEINLINE hFloat dot( const hVec3& a, const hVec3& b )
	{
        return XMVector3Dot( a, b );
	}

	hFORCEINLINE hVec3& neg( const hVec3& a )
	{
        return XMVectorNegate( a );
	}

	hFORCEINLINE hVec3& cross( const hVec3& a, const hVec3& b )
	{
        return XMVector3Cross( a, b );
	}

	hFORCEINLINE hVec3& set( hFloat x, hFloat y, hFloat z )
	{
        return XMVectorSet( x, y, z, 1.f );
	}

    hFORCEINLINE hBool compare( const hVec3& a, const hVec3& b )
    {
        return XMVector3Equal( a, b );
    }

    hFORCEINLINE void store( const hVec3& a, hCPUVec3* b )
    {
        XMStoreFloat3( b, a );
    }

    hFORCEINLINE void load( const hVec3& a, hCPUVec3* b )
    {
        a = XMLoadFloat3( b );
    }

}

    hFORCEINLINE hVec3& operator + ( const hVec3& a, const hVec3& b )
	{
        return hVec3Func::add( a, b );
	}

	hFORCEINLINE hVec3& operator += ( const hVec3& a, const hVec3& b )
	{
        return hVec3Func::add( a, b );
	}

	hFORCEINLINE hVec3& operator - ( const hVec3& a, const hVec3& b )
	{
        return hVec3Func::sub( a, b );
	}

	hFORCEINLINE hVec3& operator -= ( const hVec3& a, const hVec3& b )
	{
        return hVec3Func::sub( a, b );
	}

	hFORCEINLINE hVec3& operator * ( const hVec3& v, const hFloat s )
	{
        return hVec3Func::scale( v, s );
	}

	hFORCEINLINE hVec3& operator *= ( const hVec3& v, const hFloat s )
	{
		return hVec3Func::scale( v, s );
	}

	hFORCEINLINE hVec3& operator / ( const hVec3& v, const hFloat s ) 
	{
        return hVec3Func::div( v, s );
	}

	hFORCEINLINE hVec3& operator /= ( const hVec3& v, const hFloat s )
	{
		return hVec3Func::div( v, s );
	}

    hFORCEINLINE hBool operator == ( const hVec3& a, const hVec3& b )
	{
        return hVec3Func::compare( a, b );
	}

	hFORCEINLINE hBool operator != (  const hVec3& a, const hVec3& b )
	{
		return !(a == b);
	}

	hFORCEINLINE hVec3& operator - ( const hVec3& a )
	{
        return hVec3Func::neg( a );
	}

}
