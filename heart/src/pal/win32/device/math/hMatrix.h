/********************************************************************

	filename: 	hMatrix.h	
	
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

#ifndef MATH_MATRIX_H__
#define MATH_MATRIX_H__

//////////////////////////////////////////
//include 

namespace Heart
{
    //////////////////////////////////////////////////////////////////////////
    // NOTE: all rotations for these matrices will be in XYZ order!!! ////////
    //////////////////////////////////////////////////////////////////////////
#if defined (HEART_USE_XNAMATH)
	typedef XMMATRIX hMatrix;
#else
    typedef DirectX::XMMATRIX hMatrix;
    using DirectX::XMMatrixLookAtLH;
    using DirectX::XMMatrixOrthographicLH;
    using DirectX::XMMatrixOrthographicOffCenterLH;
    using DirectX::XMMatrixPerspectiveFovLH;
    using DirectX::XMMatrixPerspectiveOffCenterLH;
    using DirectX::XMMatrixRotationRollPitchYaw;
    using DirectX::XMMatrixRotationAxis;
    using DirectX::XMMatrixRotationX;
    using DirectX::XMMatrixRotationY;
    using DirectX::XMMatrixRotationZ;
    using DirectX::XMMatrixTranslationFromVector;
    using DirectX::XMMatrixDeterminant;
    using DirectX::XMMatrixTranspose;
    using DirectX::XMMatrixInverse;
    using DirectX::XMMatrixIsIdentity;
    using DirectX::XMMatrixIdentity;
    using DirectX::XMMatrixMultiply;
    using DirectX::XMVector2Transform;
    using DirectX::XMVector2TransformNormal;
    using DirectX::XMVector3Transform;
    using DirectX::XMVector3TransformNormal;
    using DirectX::XMVector4Transform;
    using DirectX::XMMatrixRotationQuaternion;
    using DirectX::XMMatrixScaling;
    using DirectX::XMMatrixIsInfinite;
    using DirectX::XMMatrixIsNaN;
    using DirectX::XMMatrixDecompose;

#endif

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

    hFORCEINLINE hMatrix rotate( hFloat theta, const hVec3& axis )
    {
        return XMMatrixRotationAxis( axis, theta);
    }

    hFORCEINLINE hMatrix RotationX( hFloat theta )
    {
        return XMMatrixRotationX( theta );
    }

    hFORCEINLINE hMatrix RotationY( hFloat theta )
    {
        return XMMatrixRotationY( theta );
    }

    hFORCEINLINE hMatrix RotationZ( hFloat theta )
    {
        return XMMatrixRotationZ( theta );
    }

	hFORCEINLINE hMatrix translation( const hVec3& tran )
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
#if defined (HEART_USE_XNAMATH)
        return XMMatrixIsIdentity( in ) > 0;
#else
        return XMMatrixIsIdentity( in );
#endif 
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

    hFORCEINLINE void setTranslation(hMatrix& m, const hVec3& v) {
        m.r[3]=v.Get128();
        hVec128SetW(m.r[3], 1.f);
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

}

#endif 