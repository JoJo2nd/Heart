/////////////////////////////////////////
//
// File: mathMatrix.h
//
// Desc: 
//
// Author: James Moarn
//
// Date: 06 sept 2007
//
/////////////////////////////////////////
#ifndef MATH_MATRIX_H__
#define MATH_MATRIX_H__

//////////////////////////////////////////
//include 
#include "hMathUtil.h"
#include "hVector.h"
#include "hVec2.h"
#include "hVec3.h"
#include "hVec4.h"
#include "hQuaternion.h"
#include <float.h>
#include <math.h>
#include <xnamath.h>


namespace Heart
{

    //////////////////////////////////////////////////////////////////////////
    // NOTE: all rotations for these matrices will be in XYZ order!!! ////////
    //////////////////////////////////////////////////////////////////////////
	typedef XMMATRIX hMatrix;

namespace hMatrixFunc
{
	hFORCEINLINE hMatrix LookAt( const hVec3& eye, const hVec3& lookat, const hVec3& up )
	{
        return XMMatrixLookAtLH( eye.v, lookat.v, up.v );
	}

	hFORCEINLINE hMatrix orthoProj( hFloat width, hFloat height, hFloat znear, hFloat zfar )
	{
        return XMMatrixOrthographicLH( width, height, znear, zfar );
	}

	hFORCEINLINE hMatrix orthoProjOffCentre( hFloat l, hFloat r, hFloat b, hFloat t, hFloat znear, hFloat zfar )
	{
        return XMMatrixOrthographicOffCenterLH( l, r, b, t, znear, zfar );
	}

	hFORCEINLINE hMatrix perspectiveFOV( hFloat fovy, hFloat aspect, hFloat znear, hFloat zfar )
	{
        return XMMatrixPerspectiveFovLH( fovy, aspect, znear, zfar );
	}

	hFORCEINLINE hMatrix perspective( hFloat left, hFloat right, hFloat top, hFloat bottom, hFloat znear, hFloat zfar )
	{
        return XMMatrixPerspectiveOffCenterLH( left, right, top, bottom, znear, zfar );
	}


	hFORCEINLINE hMatrix rotate( hFloat xt, hFloat yt, hFloat zt )
	{
        return XMMatrixRotationRollPitchYaw( xt, yt, zt );
		//pitch = x, yaw = y = heading, bank = z = roll
	}

    hFORCEINLINE hMatrix RotationX( hFloat theta )
    {
        return XMMatrixRotationX( theta );
    }

    hFORCEINLINE hMatrix RotationY( hFloat theta )
    {
        return XMMatrixRotationX( theta );
    }

    hFORCEINLINE hMatrix RotationZ( hFloat theta )
    {
        return XMMatrixRotationX( theta );
    }

	hFORCEINLINE hMatrix Translation( const hVec3& tran )
	{
        return XMMatrixTranslationFromVector( tran.v );
	}

	hFORCEINLINE hFloatInVec determinant( const hMatrix& in )
	{    
        return XMMatrixDeterminant( in );
	}

	hFORCEINLINE hMatrix transpose( const hMatrix& in)
	{
        return XMMatrixTranspose( in );
	}

	hFORCEINLINE hMatrix inverse( const hMatrix& in )
	{
        hVec128 det;
        return XMMatrixInverse( &det, in );
	}

	hFORCEINLINE hBool isIdentity( const hMatrix& in )
	{
        return XMMatrixIsIdentity( in ) > 0;
	}

	hFORCEINLINE hMatrix identity()
	{
        return XMMatrixIdentity();
	}

	hFORCEINLINE hMatrix mult( const hMatrix& a, const hMatrix& b )
	{
        return XMMatrixMultiply( a, b );
	}

    hFORCEINLINE hVec2 mult( const hVec2& a, const hMatrix& b )
    {
        return XMVector2Transform( a, b );
    }

    hFORCEINLINE hVec2 multNormal( const hVec2& a, const hMatrix& b )
    {
        return XMVector2TransformNormal( a, b );
    }

    hFORCEINLINE hVec3 mult( const hVec3& a, const hMatrix& b )
    {
        return XMVector3Transform( a, b );
    }

    hFORCEINLINE hVec3 multNormal( const hVec3& a, const hMatrix& b )
    {
        return XMVector3TransformNormal( a, b );
    }

    hFORCEINLINE hVec4 mult( const hVec4& a, const hMatrix& b )
    {
        return XMVector4Transform( a, b );
    }

	hFORCEINLINE hMatrix rotationFromQuaternion( const hQuaternion& q )
	{
        return XMMatrixRotationQuaternion( q.q );
	}

	hFORCEINLINE hMatrix scale( hFloat x, hFloat y, hFloat z )
	{
        return XMMatrixScaling( z, y, z );
	}

	hFORCEINLINE hBool checkForNAN( const hMatrix& m )
	{
#ifdef HEART_DEBUG
        return hFalse;
#else
        return XMMatrixIsInfinite( m ) || XMMatrixIsNaN( m );
#endif
	}

	hFORCEINLINE void decomposeQuat( const hMatrix& m, hVec3* trans, hVec3* scl, hQuaternion* q )
	{
        XMMatrixDecompose( &scl->v, &q->q, &trans->v, m );
    }

    hFORCEINLINE hVec3 getTranslation( const hMatrix& m )
    {
        return m.r[3];
    }

    hFORCEINLINE hVec4 getRow( const hMatrix& m, hUint32 row )
    {
        return m.r[row];
    }
}

    hFORCEINLINE hMatrix operator * ( const hMatrix& lhs, const hMatrix& rhs )
    {
        return hMatrixFunc::mult( lhs, rhs );
    }

    hFORCEINLINE hVec2 operator * ( const hVec2& lhs, const hMatrix& rhs )
    {
        return hMatrixFunc::mult( lhs, rhs );
    }

    hFORCEINLINE hVec3 operator * ( const hVec3& lhs, const hMatrix& rhs )
    {
        return hMatrixFunc::mult( lhs, rhs );
    }

    hFORCEINLINE hVec4 operator * ( const hVec4& lhs, const hMatrix& rhs )
    {
        return hMatrixFunc::mult( lhs, rhs );
    }

    SERIALISE_WORKER_TYPE_SPECIALISATION( hMatrix, hSerialisedElementHeader::Type_User );

}

#endif 