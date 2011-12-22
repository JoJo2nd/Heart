/********************************************************************
	created:	2008/07/13
	created:	13:7:2008   10:27
	filename: 	Vec4.h
	author:		James Moran
	
	purpose:	
*********************************************************************/
#ifndef hmVec4_h__
#define hmVec4_h__

#include <math.h>
#include "hTypes.h"
#include "hVector.h"
#include <xnamath.h>

namespace Heart
{

    typedef XMFLOAT4 hCPUVec4;

    struct hVec4
    {
        hVec128 v;

        
        hVec4() {}
        hVec4( const hVec128& rhs );
        explicit hVec4( const hCPUVec4& rhs );
        hVec4( hFloat x, hFloat y, hFloat z, hFloat w );
        hVec4& operator = ( const hCPUVec4& b );
        operator hCPUVec4 () const;
        operator hFloatInVec() const { return v; }
    };


namespace hVec4Func
{
    hFORCEINLINE hVec4 zeroVector()
    {
        return XMVectorZero();
    }

    //functions
    hFORCEINLINE hFloatInVec length( const hVec4& v )
    {
        return XMVector4Length( v.v ); 
    }

    hFORCEINLINE hFloatInVec lengthFast( const hVec4& v )
    {
        return XMVector4LengthEst( v.v );
    }

    hFORCEINLINE hFloatInVec lengthSquare( const hVec4& v )
    {
        return XMVector4Length( v.v ); 
    }

    hFORCEINLINE hVec4 normalise( const hVec4& v )
    {
        return XMVector4Normalize( v.v );
    }

    hFORCEINLINE hVec4 normaliseFast( const hVec4& v )
    {
        return XMVector4NormalizeEst( v.v );
    }

    hFORCEINLINE hVec4 add( const hVec4& a, const hVec4& b )
    {
        return XMVectorAdd( a.v, b.v );
    }

    hFORCEINLINE hVec4 sub( const hVec4& a, const hVec4& b )
    {
        return XMVectorSubtract( a.v, b.v );
    }

    hFORCEINLINE hVec4 scale( const hVec4& a, const hFloat s )
    {
        return XMVectorScale( a.v, s );
    }

    hFORCEINLINE hVec4 div( const hVec4& a, const hVec4& d )
    {
        return XMVectorDivide( a.v, d.v );
    }

    hFORCEINLINE hFloatInVec dot( const hVec4& a, const hVec4& b )
    {
        return XMVector4Dot( a.v, b.v );
    }

    hFORCEINLINE hVec4 neg( const hVec4& a )
    {
        return XMVectorNegate( a.v );
    }

    hFORCEINLINE hVec4 cross( const hVec4& a, const hVec4& b, const hVec4& c )
    {
        return XMVector4Cross( a.v, b.v, c.v );
    }

    hFORCEINLINE hVec4 set( hFloat x, hFloat y, hFloat z, hFloat w )
    {
        return XMVectorSet( x, y, z, w );
    }

    hFORCEINLINE hBool compare( const hVec4& a, const hVec4& b )
    {
        return XMVector4Equal( a.v, b.v ) > 0;
    }

    hFORCEINLINE void store( const hVec4& a, hCPUVec4* b )
    {
        XMStoreFloat4( b, a.v );
    }

    hFORCEINLINE void load( hVec4& a, const hCPUVec4* b )
    {
        a.v = XMLoadFloat4( b );
    }

}

    hFORCEINLINE hVec4 operator + ( const hVec4& a, const hVec4& b )
    {
        return hVec4Func::add( a, b );
    }

    hFORCEINLINE hVec4 operator += ( const hVec4& a, const hVec4& b )
    {
        return hVec4Func::add( a, b );
    }

    hFORCEINLINE hVec4 operator - ( const hVec4& a, const hVec4& b )
    {
        return hVec4Func::sub( a, b );
    }

    hFORCEINLINE hVec4 operator -= ( const hVec4& a, const hVec4& b )
    {
        return hVec4Func::sub( a, b );
    }

    hFORCEINLINE hVec4 operator * ( const hVec4& v, const hFloat s )
    {
        return hVec4Func::scale( v, s );
    }

    hFORCEINLINE hVec4 operator *= ( const hVec4& v, hFloat s )
    {
        return hVec4Func::scale( v, s );
    }

    hFORCEINLINE hVec4 operator / ( const hVec4& v, const hVec4& s ) 
    {
        return hVec4Func::div( v, s );
    }

    hFORCEINLINE hVec4 operator /= ( const hVec4& v, const hVec4& s )
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

    hFORCEINLINE hVec4 operator - ( const hVec4& a )
    {
        return hVec4Func::neg( a );
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hFORCEINLINE hVec4::hVec4( const hVec128& rhs ) 
        : v(rhs)
    {
    }

    hFORCEINLINE hVec4::hVec4( const hCPUVec4& rhs ) 
    {
        *this = hVec4Func::set( rhs.x, rhs.y, rhs.z, rhs.w );
    }

    hFORCEINLINE hVec4::hVec4( hFloat x, hFloat y, hFloat z, hFloat w )
    {
        *this = hVec4Func::set( x, y, z, w );
    }

    hFORCEINLINE hVec4& hVec4::operator = ( const hCPUVec4& b )
    {
        hVec4Func::load( *this, &b );
        return *this;
    }

    hFORCEINLINE hVec4::operator hCPUVec4 () const
    {
        hCPUVec4 r;
        hVec4Func::store( *this, &r );
        return r;
    }

    SERIALISE_WORKER_TYPE_SPECIALISATION( hVec4, hSerialisedElementHeader::Type_User );
}

#endif // hmVec4_h__