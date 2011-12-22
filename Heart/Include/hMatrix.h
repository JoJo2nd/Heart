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
        return XMMatrixInverse( NULL, in );
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

// 		hVec3& mult( const hVec3& v, const hMatrix* m, hVec3& vout )
// 		{
// 			hVec3 ret;
// 			hVec3* o;
// 
// 			o = &vout;
// 			if ( &v == &vout )
// 			{
// 				o = &ret;
// 			}
// 
// 			// assumes a 1 for w component of vector [7/28/2008 James]
// 			o->x = ( v.x * m->m11 ) + ( v.y * m->m21 ) + ( v.z * m->m31 ) + m->m41;
// 			o->y = ( v.x * m->m12 ) + ( v.y * m->m22 ) + ( v.z * m->m32 ) + m->m42;
// 			o->z = ( v.x * m->m13 ) + ( v.y * m->m23 ) + ( v.z * m->m33 ) + m->m43;
// 
// 			if ( &ret == o )
// 			{
// 				vout = *o;
// 			}
// 
// 			return vout;
// 		}


// 		static hVec3& multRotOnly( const hVec3& v, const hMatrix* m, hVec3& vout )
// 		{
// 			hVec3 ret;
// 			hVec3* o;
// 
// 			o = &vout;
// 			if ( &v == &vout )
// 			{
// 				o = &ret;
// 			}
// 
// 			// assumes a 1 for w component of vector [7/28/2008 James]
// 			//o->x = ( v.x * m->m11 ) + ( v.y * m->m12 ) + ( v.z * m->m13 );
// 			//o->y = ( v.x * m->m21 ) + ( v.y * m->m22 ) + ( v.z * m->m23 );
// 			//o->z = ( v.x * m->m31 ) + ( v.y * m->m32 ) + ( v.z * m->m33 );
// 			o->x = ( v.x * m->m11 ) + ( v.y * m->m21 ) + ( v.z * m->m31 );
// 			o->y = ( v.x * m->m12 ) + ( v.y * m->m22 ) + ( v.z * m->m32 );
// 			o->z = ( v.x * m->m13 ) + ( v.y * m->m23 ) + ( v.z * m->m33 );
// 
// 
// 			if ( &ret == o )
// 			{
// 				vout = *o;
// 			}
// 
// 			return vout;
// 		}

// 		static hVec4& mult( const hVec4& v, const hMatrix* m, hVec4& vout )
// 		{
// 			hVec4 ret;
// 			hVec4* o;
// 
// 			o = &vout;
// 			if ( &v == &vout )
// 			{
// 				o = &ret;
// 			}
// 
// 			o->x = ( v.x * m->m11 ) + ( v.y * m->m21 ) + ( v.z * m->m31 ) + ( v.w * m->m41 );
// 			o->y = ( v.x * m->m12 ) + ( v.y * m->m22 ) + ( v.z * m->m32 ) + ( v.w * m->m42 );
// 			o->z = ( v.x * m->m13 ) + ( v.y * m->m23 ) + ( v.z * m->m33 ) + ( v.w * m->m43 );
// 			o->w = ( v.x * m->m14 ) + ( v.y * m->m24 ) + ( v.z * m->m34 ) + ( v.w * m->m44 );
// 
// 			if ( &ret == o )
// 			{
// 				vout = *o;
// 			}
// 
// 			return vout;
// 		}

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

// 		bool operator == ( const hMatrix& b )
// 		{
// 			if (
// 			b.m11 == m11 && b.m12 == m12 && b.m13 == m13 && b.m14 == m14 &&
// 			b.m21 == m21 && b.m22 == m22 && b.m23 == m23 && b.m24 == m24 &&
// 			b.m31 == m31 && b.m32 == m32 && b.m33 == m33 && b.m34 == m34 &&
// 			b.m41 == m41 && b.m42 == m42 && b.m43 == m43 && b.m44 == m44 )
// 				return true;
// 			return false;
// 		}

// 		static hVec3 getScale( const hMatrix& t )
// 		{
// 			hVec3 scl;
// 
// 			hVec3::set( 
// 				sqrt( t.m11*t.m11 + t.m21*t.m21 + t.m31*t.m31 ),
// 				sqrt( t.m12*t.m12 + t.m22*t.m22 + t.m32*t.m32 ),
// 				sqrt( t.m13*t.m13 + t.m23*t.m23 + t.m33*t.m33 ),
// 				scl
// 				);
// 
// 			return scl;
// 		}
// 
// 		static hVec3 GetTranslation( const hMatrix& t )
// 		{
// 			return hVec3( t.m41, t.m42, t.m43 );
// 		}
// 
// 		static hVec3 GetTranslation( const hMatrix* t )
// 		{
// 			return hVec3( t->m41, t->m42, t->m43 );
// 		}
// 
// 		static void decomposeEuler( const hMatrix& m, hVec3& trans, hVec3& scl, hVec3& rotDeg )
// 		{
//  			// transpose [2/2/2009 James]
//  			hMatrix t = m;
//  			//transpose( &m, &t );
// 	 
//  			// transform [2/2/2009 James]
//  			hVec3::set( t.m41, t.m42, t.m43, trans );
// 	 	
//  			// scale [2/2/2009 James]
//  			hVec3::set( 
//  				sqrt( t.m11*t.m11 + t.m12*t.m12 + t.m13*t.m13 ),
//  				sqrt( t.m21*t.m21 + t.m22*t.m22 + t.m23*t.m23 ),
//  				sqrt( t.m31*t.m31 + t.m32*t.m32 + t.m33*t.m33 ),
//  				scl
//  				);
// 	 
//  			t.m11 /= scl.x;
//  			t.m21 /= scl.x;
//  			t.m31 /= scl.x;
//  			t.m41 = 0.0f;
// 	 
//  			t.m12 /= scl.y;
//  			t.m22 /= scl.y;
//  			t.m32 /= scl.y;
//  			t.m42 = 0.0f;
// 	 
//  			t.m13 /= scl.z;
//  			t.m23 /= scl.z;
//  			t.m33 /= scl.z;
//  			t.m43 = 0.0f;
// 	 
//  			//heading = y , pitch = x, bank = z
// 			hFloat thetaX,thetaY,thetaZ;
// 			if ( t.m13 < 1.0f )
// 			{
// 				if ( t.m13 > -1.0f )
// 				{
// 					thetaY = asin( t.m13 );
// 					thetaX = atan2( -t.m23, t.m33 );
// 					thetaZ = atan2( -t.m12, t.m11 );
// 				}
// 				else
// 				{
// 					// Not a unique solution: thetaZ - thetaX = atan2(r21,r22)
// 					thetaY = -hmPI/2;
// 					thetaX = -atan2( t.m21, t.m22 );
// 					thetaZ = 0;
// 				}
// 			}
// 			else //m13 > 1.0f
// 			{
// 				//no unqiue solution
// 				// Not a unique solution: thetaZ + thetaX = atan2(r21,r22)
// 				thetaY = hmPI/2;
// 				thetaX = atan2( t.m21, t.m22 );
// 				thetaZ = 0;
// 			}
// 
// 			rotDeg.x = hmRadToDeg( thetaX );
// 			rotDeg.y = hmRadToDeg( thetaY );
// 			rotDeg.z = hmRadToDeg( thetaZ );
// 		}

	hFORCEINLINE void decomposeQuat( const hMatrix& m, hVec3* trans, hVec3* scl, hQuaternion* q )
	{
        XMMatrixDecompose( &scl->v, &q->q, &trans->v, m );
    }
}

// 	inline hVec2 operator * ( const hVec2& v, const hMatrix& m )
// 	{
// 		//	we assume 1 for z & w components of the vector
// 		return hVec2( ( m.m11 * v.x ) + ( m.m21 * v.y ) + m.m31 + m.m41, ( m.m12 * v.x ) + ( m.m22 * v.y ) + m.m32 + m.m42 );
// 	}
// 
// 	static const hMatrix IdentityMatrix = 
// 	{ 
// 		1.0f, 0.0f, 0.0f, 0.0f,
// 		0.0f, 1.0f, 0.0f, 0.0f,
// 		0.0f, 0.0f, 1.0f, 0.0f,
// 		0.0f, 0.0f, 0.0f, 1.0f,
// 	};

}

#endif 