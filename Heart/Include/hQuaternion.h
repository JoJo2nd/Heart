/********************************************************************
	created:	2008/07/13
	created:	13:7:2008   19:54
	filename: 	Quaternion.h
	author:		James Moran
	
	purpose:	
*********************************************************************/
#ifndef HMQUATERNION_H__
#define HMQUATERNION_H__

#include "hTypes.h"
#include "hDebugMacros.h"
#include <float.h>
#include <xnamath.h>

namespace Heart
{

    typedef XMVECTOR hQuaternion;
    typedef XMFLOAT4 hCPUQuaternion;

namespace hQuaternionFunc
{
    hFORCEINLINE hQuaternion&	identity()
	{
        return XMQuaternionIdentity();
	}

	hFORCEINLINE hQuaternion&	RotateAxis( hFloat t, const hVec3& axis )
	{
        return XMQuaternionRotationAxis( axis, t );
	}

	hFORCEINLINE hQuaternion&	rotateX( hFloat t )
	{
        return RotateAxis( t, hCPUVec3( 1.f, 0.f, 0.f ) );
	}

    hFORCEINLINE hQuaternion&	rotateY( hFloat t )
    {
        return RotateAxis( t, hCPUVec3( 0.f, 1.f, 0.f ) );
    }

    hFORCEINLINE hQuaternion&	rotateZ( hFloat t )
    {
        return RotateAxis( t, hCPUVec3( 0.f, 0.f, 1.f ) );
    }

    hFORCEINLINE hQuaternion& rotate( hFloat x, hFloat y, hFloat z )
    {
        return XMQuaternionRotationRollPitchYaw( x, y, z );
    }

    hFORCEINLINE hQuaternion& rotate( const hVec3& v )
    {
        return XMQuaternionRotationRollPitchYawFromVector( v );
    }

	hFORCEINLINE hQuaternion& mult( const hQuaternion& a, const hQuaternion& b )
	{
        return XMQuaternionMultiply( a, b );
	}

    hFORCEINLINE hQuaternion& normalise( const hQuaternion& a)
	{
        return XMQuaternionNormalize( a );
	}

    hFORCEINLINE hQuaternion& normaliseFast( const hQuaternion& a)
    {
        return XMQuaternionNormalize( a );
    }

	hFORCEINLINE hFloat length( const hQuaternion& a )
	{
		return XMQuaternionLength( a );
	}

    hFORCEINLINE hFloat lengthSquare( const hQuaternion& a )
    {
        return XMQuaternionLengthSq( a );
    }

    hFORCEINLINE hFloat dot( const hQuaternion& a, const hQuaternion& b )
	{
		return XMQuaternionDot( a, b );
	}

	hFORCEINLINE hQuaternion& slerp( const hQuaternion& a, const hQuaternion& b, hFloat dt )
	{
        return XMQuaternionSlerp( a, b, dt );
	}

	hFORCEINLINE hQuaternion& conjugate( hQuaternion& a )
	{
        return XMQuaternionConjugate( a );
	}

	hFORCEINLINE hQuaternion& decompress( hFloat x, hFloat y, hFloat z )
	{
		hFloat w = 1.0f - (x * x) - (y * y) - (z * z);
		w = sqrt( fabs( w ) );
		return normalise( hCPUQuaternion( w, x, y, z ) );
	}

    hFORCEINLINE hBool IsIdentity( const hQuaternion& q )
	{
		return XMQuaternionIsIdentity( q );
	}

    hFORCEINLINE void store( const hQuaternion& a, hCPUQuaternion* b )
    {
        XMStoreFloat4( b, a );
    }

    hFORCEINLINE void load( const hQuaternion& a, hCPUQuaternion* b )
    {
        a = XMLoadFloat4( b );
    }
}

    hFORCEINLINE hCPUQuaternion& operator = ( const hCPUQuaternion& a, const hQuaternion& b )
    {
        hQuaternionFunc::store( b, &a );
    }

    hFORCEINLINE hQuaternion& operator = ( const hQuaternion& a, const hCPUQuaternion& b )
    {
        hQuaternionFunc::load( a, &b );
    }
    
}

#endif // HMQUATERNION_H__
