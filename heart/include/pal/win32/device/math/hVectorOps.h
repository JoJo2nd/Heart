/********************************************************************

    filename:   hVectorOps.h  
    
    Copyright (c) 2:8:2013 James Moran
    
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
#pragma once

#ifndef HVECTOROPS_H__
#define HVECTOROPS_H__

namespace Heart
{
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
#if defined (HEART_USE_XNAMATH)
        return XMVector2Equal( a.v, b.v ) > 0;
#else
        return XMVector2Equal( a.v, b.v );
#endif
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
#if defined (HEART_USE_XNAMATH)
        return XMVector3Equal( a.v, b.v ) > 0;
#else
        return XMVector3Equal( a.v, b.v );
#endif
    }

    hFORCEINLINE hBool Less( const hVec3& a, const hVec3& b )
    {
#if defined (HEART_USE_XNAMATH)
        return XMVector3Less( a.v, b.v ) > 0;
#else
        return XMVector3Less( a.v, b.v );
#endif
    }

    hFORCEINLINE hBool LessEqual( const hVec3& a, const hVec3& b )
    {
#if defined (HEART_USE_XNAMATH)
        return XMVector3LessOrEqual( a.v, b.v ) > 0;
#else
        return XMVector3LessOrEqual( a.v, b.v );
#endif
    }

    hFORCEINLINE hBool Greater( const hVec3& a, const hVec3& b )
    {
#if defined (HEART_USE_XNAMATH)
        return XMVector3Greater( a.v, b.v ) > 0;
#else
        return XMVector3Greater( a.v, b.v );
#endif
    }

    hFORCEINLINE hBool GreaterEqual( const hVec3& a, const hVec3& b )
    {
#if defined (HEART_USE_XNAMATH)
        return XMVector3GreaterOrEqual( a.v, b.v ) > 0;
#else
        return XMVector3GreaterOrEqual( a.v, b.v );
#endif
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

    hFORCEINLINE hVec3 operator += ( hVec3& a, const hVec3& b )
    {
        a = hVec3Func::add( a, b );
        return a;
    }

    hFORCEINLINE hVec3 operator - ( const hVec3& a, const hVec3& b )
    {
        return hVec3Func::sub( a, b );
    }

    hFORCEINLINE hVec3 operator -= ( hVec3& a, const hVec3& b )
    {
        a = hVec3Func::sub( a, b );
        return a;
    }

    hFORCEINLINE hVec3 operator * ( const hVec3& v, const hFloat s )
    {
        return hVec3Func::scale( v, s );
    }

    hFORCEINLINE hVec3 operator *= ( hVec3& v, hFloat s )
    {
        v = hVec3Func::scale( v, s );
        return v;
    }

    hFORCEINLINE hVec3 operator / ( const hVec3& v, const hVec3& s ) 
    {
        return hVec3Func::div( v, s );
    }

    hFORCEINLINE hVec3 operator /= ( hVec3& v, const hVec3& s )
    {
        v = hVec3Func::div( v, s );
        return v;
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
        return XMVector4LengthSq( v.v ); 
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
#if defined (HEART_USE_XNAMATH)
        return XMVector4Equal( a.v, b.v ) > 0;
#else
        return XMVector4Equal( a.v, b.v );
#endif
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

    hFORCEINLINE hVec4 operator + (const hVec4& a, const hVec3& b) {
        return hVec4Func::add(a, hVec4(b, 1.f));
    }

    hFORCEINLINE hVec4 operator += ( const hVec4& a, const hVec4& b )
    {
        return hVec4Func::add( a, b );
    }

    hFORCEINLINE hVec4 operator += (const hVec4& a, const hVec3& b) {
        return hVec4Func::add(a, hVec4(b, 1.f));
    }

    hFORCEINLINE hVec4 operator - ( const hVec4& a, const hVec4& b )
    {
        return hVec4Func::sub( a, b );
    }

    hFORCEINLINE hVec4 operator - (const hVec4& a, const hVec3& b) {
        return hVec4Func::sub(a, hVec4(b, 1.f));
    }

    hFORCEINLINE hVec4 operator -= ( const hVec4& a, const hVec4& b )
    {
        return hVec4Func::sub( a, b );
    }

    hFORCEINLINE hVec4 operator-= (const hVec4& a, const hVec3& b) {
        return hVec4Func::sub(a, hVec4(b, 1.f));
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

    hFORCEINLINE hVec4::hVec4( const hVec3& rhs ) {
        v = rhs.v;
        v = hVec128SetW(v, 1.f );
    }

    hFORCEINLINE hVec4::hVec4(const hVec3& rhs, hFloat w) {
        v = rhs.v;
        v = hVec128SetW(v, w);
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
}

#endif // HVECTOROPS_H__