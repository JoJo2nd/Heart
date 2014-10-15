/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#ifndef HMQUATERNION_H__
#define HMQUATERNION_H__

#include "vectormath/SSE/cpp/vectormath_aos.h"

namespace Heart
{
    typedef Vectormath::Aos::Quat hQuaternion;

namespace hQuaternionFunc
{
#if 0
    hFORCEINLINE hQuaternion identity()
	{
        return XMQuaternionIdentity();
	}

    hFORCEINLINE hQuaternion set( hFloat x, hFloat y, hFloat z, hFloat w )
    {
        return XMVectorSet( x, y, z, w );
    }

	hFORCEINLINE hQuaternion RotateAxis( hFloat t, const hVec3& axis )
	{
        return XMQuaternionRotationAxis( axis.v, t );
	}

	hFORCEINLINE hQuaternion rotateX( hFloat t )
	{
        return RotateAxis( t, hVec3( 1.f, 0.f, 0.f ) );
	}

    hFORCEINLINE hQuaternion rotateY( hFloat t )
    {
        return RotateAxis( t, hVec3( 0.f, 1.f, 0.f ) );
    }

    hFORCEINLINE hQuaternion rotateZ( hFloat t )
    {
        return RotateAxis( t, hVec3( 0.f, 0.f, 1.f ) );
    }

    hFORCEINLINE hQuaternion rotate( hFloat x, hFloat y, hFloat z )
    {
        return XMQuaternionRotationRollPitchYaw( x, y, z );
    }

    hFORCEINLINE hQuaternion rotate( const hVec3& v )
    {
        return XMQuaternionRotationRollPitchYawFromVector( v.v );
    }

	hFORCEINLINE hQuaternion mult( const hQuaternion& a, const hQuaternion& b )
	{
        return XMQuaternionMultiply( a.q, b.q );
	}

    hFORCEINLINE hVec3 mult( const hVec3& lhs, const hQuaternion& rhs )
    {
        return XMVector3Rotate( lhs, rhs );
    }

    hFORCEINLINE hQuaternion normalise( const hQuaternion& a)
	{
        return XMQuaternionNormalize( a.q );
	}

    hFORCEINLINE hQuaternion normaliseFast( const hQuaternion& a)
    {
        return XMQuaternionNormalize( a.q );
    }

	hFORCEINLINE hFloatInVec length( const hQuaternion& a )
	{
		return XMQuaternionLength( a.q );
	}

    hFORCEINLINE hFloatInVec lengthSquare( const hQuaternion& a )
    {
        return XMQuaternionLengthSq( a.q );
    }

    hFORCEINLINE hFloatInVec dot( const hQuaternion& a, const hQuaternion& b )
	{
		return XMQuaternionDot( a.q, b.q );
	}

	hFORCEINLINE hQuaternion slerp( const hQuaternion& a, const hQuaternion& b, hFloat dt )
	{
        return XMQuaternionSlerp( a.q, b.q, dt );
	}

	hFORCEINLINE hQuaternion conjugate( const hQuaternion& a )
	{
        return XMQuaternionConjugate( a.q );
	}

	hFORCEINLINE hQuaternion decompress( hFloat x, hFloat y, hFloat z )
	{
		hFloat w = 1.0f - (x * x) - (y * y) - (z * z);
		w = sqrt( fabs( w ) );
		return normalise( hQuaternion( w, x, y, z ) );
	}

    hFORCEINLINE hBool IsIdentity( const hQuaternion& q )
	{
#if defined (HEART_USE_XNAMATH)
		return XMQuaternionIsIdentity( q.q ) > 0;
#else
        return XMQuaternionIsIdentity( q.q );
#endif
	}

    hFORCEINLINE void store( const hQuaternion& a, hCPUQuaternion* b )
    {
        XMStoreFloat4( b, a.q );
    }

    hFORCEINLINE void load( hQuaternion& a, const hCPUQuaternion* b )
    {
        a.q = XMLoadFloat4( b );
    }
#endif
}
}

#endif // HMQUATERNION_H__
