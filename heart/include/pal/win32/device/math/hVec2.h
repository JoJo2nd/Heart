/********************************************************************

	filename: 	hVec2.h	
	
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

#ifndef hmVec2_h__
#define hmVec2_h__

namespace Heart
{

    typedef XMFLOAT2 hCPUVec2;

    struct hVec2
    {
        hVec128 v;
        
        hVec2() {}
        hVec2( const hVec128& rhs );
        explicit hVec2( const hCPUVec2& rhs );
        hVec2( hFloat x, hFloat y );
        hVec2& operator = ( const hCPUVec2& b );
        operator hCPUVec2 () const;
        operator hFloatInVec() const { return v; }
        operator hVec128() const { return v; }
        hVec128 Get128() const { return v; }
    };


namespace hVec2Func
{
    hFORCEINLINE hVec2 zeroVector()
    {
        return XMVectorZero();
    }

    //functions
    hFORCEINLINE hFloatInVec length( const hVec2& v )
    {
        return XMVector2Length( v.v ); 
    }

    hFORCEINLINE hFloatInVec lengthFast( const hVec2& v )
    {
        return XMVector2LengthEst( v.v );
    }

    hFORCEINLINE hFloatInVec lengthSquare( const hVec2& v )
    {
        return XMVector2Length( v.v ); 
    }

    hFORCEINLINE hVec2 normalise( const hVec2& v )
    {
        return XMVector2Normalize( v.v );
    }

    hFORCEINLINE hVec2 normaliseFast( const hVec2& v )
    {
        return XMVector2NormalizeEst( v.v );
    }

    hFORCEINLINE hVec2 add( const hVec2& a, const hVec2& b )
    {
        return XMVectorAdd( a.v, b.v );
    }

    hFORCEINLINE hVec2 sub( const hVec2& a, const hVec2& b )
    {
        return XMVectorSubtract( a.v, b.v );
    }

    hFORCEINLINE hVec2 scale( const hVec2& a, const hFloat s )
    {
        return XMVectorScale( a.v, s );
    }

    hFORCEINLINE hVec2 div( const hVec2& a, const hVec2& d )
    {
        return XMVectorDivide( a.v, d.v );
    }

    hFORCEINLINE hFloatInVec dot( const hVec2& a, const hVec2& b )
    {
        return XMVector2Dot( a.v, b.v );
    }

    hFORCEINLINE hVec2 neg( const hVec2& a )
    {
        return XMVectorNegate( a.v );
    }

    hFORCEINLINE hVec2 cross( const hVec2& a, const hVec2& b )
    {
        return XMVector2Cross( a.v, b.v );
    }

    hFORCEINLINE hVec2 set( hFloat x, hFloat y )
    {
        return XMVectorSet( x, y, 0.f, 0.f );
    }

    hFORCEINLINE hBool compare( const hVec2& a, const hVec2& b )
    {
        return XMVector2Equal( a.v, b.v ) > 0;
    }

    hFORCEINLINE void store( const hVec2& a, hCPUVec2* b )
    {
        XMStoreFloat2( b, a.v );
    }

    hFORCEINLINE void load( hVec2& a, const hCPUVec2* b )
    {
        a.v = XMLoadFloat2( b );
    }

}

    hFORCEINLINE hVec2 operator + ( const hVec2& a, const hVec2& b )
    {
        return hVec2Func::add( a, b );
    }

    hFORCEINLINE hVec2 operator += ( const hVec2& a, const hVec2& b )
    {
        return hVec2Func::add( a, b );
    }

    hFORCEINLINE hVec2 operator - ( const hVec2& a, const hVec2& b )
    {
        return hVec2Func::sub( a, b );
    }

    hFORCEINLINE hVec2 operator -= ( const hVec2& a, const hVec2& b )
    {
        return hVec2Func::sub( a, b );
    }

    hFORCEINLINE hVec2 operator * ( const hVec2& v, const hFloat s )
    {
        return hVec2Func::scale( v, s );
    }

    hFORCEINLINE hVec2 operator *= ( const hVec2& v, hFloat s )
    {
        return hVec2Func::scale( v, s );
    }

    hFORCEINLINE hVec2 operator / ( const hVec2& v, const hVec2& s ) 
    {
        return hVec2Func::div( v, s );
    }

    hFORCEINLINE hVec2 operator /= ( const hVec2& v, const hVec2& s )
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

    hFORCEINLINE hVec2 operator - ( const hVec2& a )
    {
        return hVec2Func::neg( a );
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hFORCEINLINE hVec2::hVec2( const hVec128& rhs ) 
        : v(rhs)
    {
    }

    hFORCEINLINE hVec2::hVec2( const hCPUVec2& rhs ) 
    {
        *this = hVec2Func::set( rhs.x, rhs.y );
    }

    hFORCEINLINE hVec2::hVec2( hFloat x, hFloat y )
    {
        *this = hVec2Func::set( x, y );
    }

    hFORCEINLINE hVec2& hVec2::operator = ( const hCPUVec2& b )
    {
        hVec2Func::load( *this, &b );
        return *this;
    }

    hFORCEINLINE hVec2::operator hCPUVec2 () const
    {
        hCPUVec2 r;
        hVec2Func::store( *this, &r );
        return r;
    }

    SERIALISE_WORKER_TYPE_SPECIALISATION( hVec2, hSerialisedElementHeader::Type_User );
    SERIALISE_WORKER_TYPE_SPECIALISATION( hCPUVec2, hSerialisedElementHeader::Type_User );
}

#endif // hmVec2_h__