/********************************************************************
	created:	2008/07/13
	created:	13:7:2008   10:27
	filename: 	Vec3.h
	author:		James Moran
	
	purpose:	
*********************************************************************/
#ifndef hmVec3_h__
#define hmVec3_h__

#include <math.h>
#include "hTypes.h"
#include "hVector.h"
#include <xnamath.h>

namespace Heart
{

    typedef XMFLOAT3 hCPUVec3;

    struct hVec3
    {
        hVec128 v;

        
        hVec3() {}
        hVec3( const hVec128& rhs );
        explicit hVec3( const hCPUVec3& rhs );
        hVec3( hFloat x, hFloat y, hFloat z );
        hVec3& operator = ( const hCPUVec3& b );
        operator hCPUVec3 () const;
        operator hFloatInVec() const { return v; }
        operator hVec128() const { return v; }
        hVec128 Get128() const { return v; }
    };


namespace hVec3Func
{
    hFORCEINLINE hVec3 zeroVector()
    {
        return XMVectorZero();
    }

    //functions
    hFORCEINLINE hFloatInVec length( const hVec3& v )
    {
        return XMVector3Length( v.v ); 
    }

    hFORCEINLINE hFloatInVec lengthFast( const hVec3& v )
    {
        return XMVector3LengthEst( v.v );
    }

    hFORCEINLINE hFloatInVec lengthSquare( const hVec3& v )
    {
        return XMVector3LengthSq( v.v ); 
    }

    hFORCEINLINE hVec3 normalise( const hVec3& v )
    {
        return XMVector3Normalize( v.v );
    }

    hFORCEINLINE hVec3 normaliseFast( const hVec3& v )
    {
        return XMVector3NormalizeEst( v.v );
    }

    hFORCEINLINE hVec3 add( const hVec3& a, const hVec3& b )
    {
        return XMVectorAdd( a.v, b.v );
    }

    hFORCEINLINE hVec3 sub( const hVec3& a, const hVec3& b )
    {
        return XMVectorSubtract( a.v, b.v );
    }

    hFORCEINLINE hVec3 scale( const hVec3& a, const hFloat s )
    {
        return XMVectorScale( a.v, s );
    }

    hFORCEINLINE hVec3 componentMult( const hVec3& a, const hVec3& b )
    {
        return XMVectorMultiply( a, b );
    }

    hFORCEINLINE hVec3 div( const hVec3& a, const hVec3& d )
    {
        return XMVectorDivide( a.v, d.v );
    }

    hFORCEINLINE hFloatInVec dot( const hVec3& a, const hVec3& b )
    {
        return XMVector3Dot( a.v, b.v );
    }

    hFORCEINLINE hVec3 neg( const hVec3& a )
    {
        return XMVectorNegate( a.v );
    }

    hFORCEINLINE hVec3 cross( const hVec3& a, const hVec3& b )
    {
        return XMVector3Cross( a.v, b.v );
    }

    hFORCEINLINE hVec3 set( hFloat x, hFloat y, hFloat z )
    {
        return XMVectorSet( x, y, z, 0.f );
    }

    hFORCEINLINE hBool equal( const hVec3& a, const hVec3& b )
    {
        return XMVector3Equal( a.v, b.v ) > 0;
    }

    hFORCEINLINE hBool Less( const hVec3& a, const hVec3& b )
    {
        return XMVector3Less( a.v, b.v ) > 0;
    }

    hFORCEINLINE hBool LessEqual( const hVec3& a, const hVec3& b )
    {
        return XMVector3LessOrEqual( a.v, b.v ) > 0;
    }

    hFORCEINLINE hBool Greater( const hVec3& a, const hVec3& b )
    {
        return XMVector3Greater( a.v, b.v ) > 0;
    }

    hFORCEINLINE hBool GreaterEqual( const hVec3& a, const hVec3& b )
    {
        return XMVector3GreaterOrEqual( a.v, b.v ) > 0;
    }

    hFORCEINLINE void store( const hVec3& a, hCPUVec3* b )
    {
        XMStoreFloat3( b, a.v );
    }

    hFORCEINLINE void load( hVec3& a, const hCPUVec3* b )
    {
        a.v = XMLoadFloat3( b );
    }

}

    hFORCEINLINE hVec3 operator + ( const hVec3& a, const hVec3& b )
    {
        return hVec3Func::add( a, b );
    }

    hFORCEINLINE hVec3 operator += ( const hVec3& a, const hVec3& b )
    {
        return hVec3Func::add( a, b );
    }

    hFORCEINLINE hVec3 operator - ( const hVec3& a, const hVec3& b )
    {
        return hVec3Func::sub( a, b );
    }

    hFORCEINLINE hVec3 operator -= ( const hVec3& a, const hVec3& b )
    {
        return hVec3Func::sub( a, b );
    }

    hFORCEINLINE hVec3 operator * ( const hVec3& v, const hFloat s )
    {
        return hVec3Func::scale( v, s );
    }

    hFORCEINLINE hVec3 operator *= ( const hVec3& v, hFloat s )
    {
        return hVec3Func::scale( v, s );
    }

    hFORCEINLINE hVec3 operator / ( const hVec3& v, const hVec3& s ) 
    {
        return hVec3Func::div( v, s );
    }

    hFORCEINLINE hVec3 operator /= ( const hVec3& v, const hVec3& s )
    {
        return hVec3Func::div( v, s );
    }

    hFORCEINLINE hBool operator == ( const hVec3& a, const hVec3& b )
    {
        return hVec3Func::equal( a, b );
    }

    hFORCEINLINE hBool operator != (  const hVec3& a, const hVec3& b )
    {
        return !(a == b);
    }

    hFORCEINLINE hBool operator < ( const hVec3& lhs, const hVec3& rhs )
    {
        return hVec3Func::Less( lhs, rhs );
    }

    hFORCEINLINE hBool operator <= ( const hVec3& lhs, const hVec3& rhs )
    {
        return hVec3Func::Less( lhs, rhs );
    }

    hFORCEINLINE hBool operator > ( const hVec3& lhs, const hVec3& rhs )
    {
        return hVec3Func::Greater( lhs, rhs );
    }

    hFORCEINLINE hBool operator >= ( const hVec3& lhs, const hVec3& rhs )
    {
        return hVec3Func::GreaterEqual( lhs, rhs );
    }

    hFORCEINLINE hVec3 operator - ( const hVec3& a )
    {
        return hVec3Func::neg( a );
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hFORCEINLINE hVec3::hVec3( const hVec128& rhs ) 
        : v(rhs)
    {
    }

    hFORCEINLINE hVec3::hVec3( const hCPUVec3& rhs ) 
    {
        *this = hVec3Func::set( rhs.x, rhs.y, rhs.z );
    }

    hFORCEINLINE hVec3::hVec3( hFloat x, hFloat y, hFloat z )
    {
        *this = hVec3Func::set( x, y, z );
    }

    hFORCEINLINE hVec3& hVec3::operator = ( const hCPUVec3& b )
    {
        hVec3Func::load( *this, &b );
        return *this;
    }

    hFORCEINLINE hVec3::operator hCPUVec3 () const
    {
        hCPUVec3 r;
        hVec3Func::store( *this, &r );
        return r;
    }

    SERIALISE_WORKER_TYPE_SPECIALISATION( hVec3, hSerialisedElementHeader::Type_User );
}

#endif // hmVec3_h__