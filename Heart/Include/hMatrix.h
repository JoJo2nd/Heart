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
#include <d3dx9math.h>

//#define CHECK_FOR_NAN_IN_MATRIX

#if defined HEART_DEBUG && defined CHECK_FOR_NAN_IN_MATRIX

#define MATRIX_CHECK( x )	hcAssertMsg( hMatrix::checkForNAN( x ), "a Matrix looks broken\n" );

#else

#define MATRIX_CHECK( x )

#endif

namespace Heart
{

	enum Axis     
	{
		X_AXIS,
		Y_AXIS,
		Z_AXIS,
	};

	//-----------------------------------------------------------------------
	///
	/// @class Matrix
	/// @brief 
	/// @author James Moran
	/// @date [28/7/2008]
	/// @version 1.0
	///
	//-----------------------------------------------------------------------
	class hMatrix
	{
	public:
		/*/////////////////////////////

			matrix arragement is:
			  x cols
			row M_[ 0 ] , M_[ 1 ] , M_[ 2 ] , M_[ 3 ]
				M_[ 4 ] , M_[ 5 ] , M_[ 6 ] , M_[ 7 ]
				M_[ 8 ] , M_[ 9 ] , M_[ 10 ], M_[ 11 ]
				M_[ 12 ], M_[ 13 ], M_[ 14 ], M_[ 15 ]

			NOTE: all rotations for these matrices will be in XYZ order!!!

		/////////////////////////////*/
		union
		{
			hFloat m[ 16 ];
			struct
			{
				hFloat m11, m12, m13, m14;
				hFloat m21, m22, m23, m24;
				hFloat m31, m32, m33, m34;
				hFloat m41, m42, m43, m44;	
			};
		};

		/**
		* 
		*
		* @param 		out
		* @param 		eye
		* @param 		lookat
		* @param 		up
		* @return   	Matrix*
		*/
		static hMatrix* lookAt( hMatrix* out, const hVec3& eye, const hVec3& lookat, const hVec3& up )
		{
			hVec3 za;
			hVec3 xa;
			hVec3 ya;

			hVec3::sub( lookat,  eye, za );
			hVec3::normalise( za, za );

			hVec3::cross( up, za, xa );
			hVec3::normalise( xa, xa );

			hVec3::cross( za, xa, ya );

			out->m11  = xa.x;			out->m12  = ya.x;				out->m13 = za.x;			out->m14  = 0;
			out->m21  = xa.y;			out->m22  = ya.y;				out->m23 = za.y;			out->m24  = 0;
			out->m31  = xa.z;			out->m32  = ya.z;				out->m33 = za.z;			out->m34 = 0;
			out->m41 = -(xa * eye);		out->m42 = -(ya * eye);			out->m43 = -(za * eye);		out->m44 = 1;

			return out;
		}

		/**
		* 
		*
		* @param 		out
		* @param 		width
		* @param 		height
		* @param 		znear
		* @param 		zfar
		* @return   	Matrix*
		*/
		static hMatrix* orthoProj( hMatrix* out, hFloat width, hFloat height, hFloat znear, hFloat zfar )
		{
			hFloat* m = out->m;

			out->m11  = 2 / width;	out->m12  = 0;			out->m13  = 0;                         out->m14  = 0;
			out->m21  = 0;			out->m22  = 2 / height;	out->m23  = 0;                         out->m24  = 0;
			out->m31  = 0;			out->m32  = 0;			out->m33 = 1 / ( zfar - znear );		out->m34 = 0;
			out->m41 = 0;			out->m42 = 0;			out->m43 = -znear / ( zfar - znear );	out->m44 = 1; 

			return out;
		}

		static hMatrix* orthoProjOffCentre( hMatrix* out, hFloat l, hFloat r, hFloat b, hFloat t, hFloat znear, hFloat zfar )
		{
			out->m11 = 2 / (r-l);		out->m12 = 0;				out->m13 = 0;							out->m14 = 0;
			out->m21 = 0;				out->m22 = 2 / (t-b);		out->m23 = 0;							out->m24 = 0;
			out->m31 = 0;				out->m32 = 0;				out->m33 = 1 / ( zfar - znear );		out->m34 = 0;
			out->m41 = -(r+l)/(r-l);	out->m42 = -(t+b)/(t-b);	out->m43 = -znear / ( zfar - znear );	out->m44 = 1; 

			return out;
		}

		/**
		* 
		*
		* @param 		out
		* @param 		fovy
		* @param 		aspect
		* @param 		znear
		* @param 		zfar
		* @return   	Matrix*
		*/
		static hMatrix* perspectiveFOV( hMatrix* out, hFloat fovy, hFloat aspect, hFloat znear, hFloat zfar )
		{
			/*
				this produces W values between 0..1
			*/
			hFloat ys = 1 / tan( fovy / 2 );
			hFloat xs = ys / aspect;

			out->m11 = xs;			out->m12 = 0;			out->m13 = 0;										out->m14 = 0;
			out->m21 = 0;			out->m22 = ys;			out->m23 = 0;										out->m24 = 0;
			out->m31 = 0;			out->m32 = 0;			out->m33 = zfar / ( zfar - znear );					out->m34 = 1;
			out->m41 = 0;			out->m42 = 0;			out->m43 = -znear * zfar / ( zfar - znear );		out->m44 = 0; 

			return out;
		}

		/**
		* perspective 
		*
		* @param 	Matrix * out
		* @param 	hFloat left
		* @param 	hFloat right
		* @param 	hFloat top
		* @param 	hFloat bottom
		* @param 	hFloat znear
		* @param 	hFloat zfar
		* @return   Matrix*
		*/
		static hMatrix* perspective( hMatrix* out, hFloat left, hFloat right, hFloat top, hFloat bottom, hFloat znear, hFloat zfar )
		{
			/*
				this produces W values between 0..1
			*/
			out->m11 = (2*znear)/(right-left);		out->m12 = 0;							out->m13 = 0;								out->m14 = 0;
			out->m21 = 0;							out->m22 = (2*znear)/(top-bottom);		out->m23 = 0;								out->m24 = 0;
			out->m31 = (left+right)/(left-right);	out->m32 = (top+bottom)/(bottom-top);	out->m33 = zfar / ( zfar - znear );			out->m34 = 1;
			out->m41 = 0;							out->m42 = 0;							out->m43 = -znear * zfar / ( zfar - znear );out->m44 = 0; 

			return out;
		}

		/**
		* 
		* rotation order is XYZ
		* 
		* @param 		xt
		* @param 		yt
		* @param 		zt
		* @param 		out
		* @return   	Matrix*
		*/
		static hMatrix* rotate( hFloat xt, hFloat yt, hFloat zt, hMatrix* out )
		{
			//pitch = x, yaw = y = heading, bank = z = roll
			hFloat sx, cx, sy, cy, sz, cz;

			sx = sin( xt ); cx = cos( xt );
			sy = sin( yt ); cy = cos( yt );
			sz = sin( zt ); cz = cos( zt );

			out->m11 = cy * cz;
			out->m21 = -sy * -sx * cz + cx * sz; 
			out->m31 = -sy * cx * cz + sx * sz;

			out->m12 = sz * -cy;
			out->m22 = -sy * -sx * -sz + cx * cz;
			out->m32 = -sy * cx * -sz + sx * cz;

			out->m13 = sy;
			out->m23 = -sx * cy;
			out->m33 = cx * cy;

			out->m14 = 0.0f;
			out->m24 = 0.0f;
			out->m34 = 0.0f;
			out->m41 = 0.0f;
			out->m42 = 0.0f;
			out->m43 = 0.0f;
			out->m44 = 1.0f;

			return out;
		}

		/**
		* 
		*
		* @param 		axis
		* @param 		theta
		* @param 		out
		* @return   	Matrix*
		*/
		static hMatrix* rotate( Axis axis, hFloat theta ,hMatrix* out )
		{
			hFloat costh = cos( theta );
			hFloat sinth = sin( theta );
	        
			hFloat* m = out->m;

			switch ( axis )
			{
			case X_AXIS:
				out->m11 = 1.0f; out->m12 = 0.0f;  out->m13 = 0.0f;
				out->m21 = 0.0f; out->m22 = costh; out->m23 = sinth;
				out->m31 = 0.0f; out->m32 = -sinth; out->m33 = costh;
				break;
			case Y_AXIS:
				out->m11 = costh; out->m12 = 0.0f;  out->m13 = -sinth;
				out->m21 = 0.0f;  out->m22 = 1.0f;  out->m23 = 0.0f;
				out->m31 = sinth;out->m32 = 0.0f;  out->m33 = costh;
				break;
			case Z_AXIS:
				out->m11 = costh;  out->m12 = sinth; out->m13 = 0.0f;
				out->m21 = -sinth; out->m22 = costh; out->m23 = 0.0f;
				out->m31 = 0.0f;   out->m32 = 0.0f;  out->m33 = 1.0f;
				break;
			default:
				break;
			}

			out->m14 = 0.0f;
			out->m24 = 0.0f;
			out->m34 = 0.0f;
			out->m41 = 0.0f;
			out->m42 = 0.0f;
			out->m43 = 0.0f;
			out->m44 = 1.0f;

			MATRIX_CHECK( out );

			return out;
		}

		static hMatrix* rotate( hVec3 axis, hFloat theta ,hMatrix* out )
		{
			hcAssert( axis.mag() - 1.0f <= Limits::FLOAT_EPSILON );
			
			hFloat ct = cos( theta );
			hFloat st = sin( theta );

			//some common expressions
			float a = 1.0f - ct;
			float ax = a * axis.x;
			float ay = a * axis.y;
			float az = a * axis.z;

			out->m11 = ax * axis.x + ct;
			out->m12 = ax * axis.y + axis.z * st;
			out->m13 = ax * axis.z - axis.y * st;
			out->m14 = 0.0f;

			out->m21 = ay * axis.x - axis.z * st;
			out->m22 = ay * axis.y + ct;
			out->m23 = ay * axis.z + axis.x * st;
			out->m24 = 0.0f;

			out->m31 = az * axis.x + axis.y * st;
			out->m32 = az * axis.y - axis.x * st;
			out->m33 = az * axis.z + ct;
			out->m34 = 0.0f;

			//zero translation
			out->m41 = 0.0f;
			out->m42 = 0.0f;
			out->m43 = 0.0f;
			out->m44 = 1.0f;

			MATRIX_CHECK( out );

			return out;
		}

		/**
		* 
		*
		* @param 		tran
		* @return   	void
		*/
		void setTransform( const hVec3& tran )
		{
			m41 = tran.x;
			m42 = tran.y;
			m43 = tran.z;
			m44 = 1.0f;

		}

		/**
		* 
		*
		* @param 		tran
		* @param 		out
		* @return   	Matrix*
		*/
		static hMatrix* setTransform( const hVec3& tran, hMatrix* out )
		{

			out->m41 = tran.x;
			out->m42 = tran.y;
			out->m43 = tran.z;
			out->m44 = 1.0f;

			MATRIX_CHECK( out );

			return out;
		}

		/**
		* 
		*
		* @param 		tran
		* @param 		out
		* @return   	Matrix*
		*/
		static hMatrix* transformation( const hVec3& tran, hMatrix* out  )
		{
			out->m11 = 1.0f;
			out->m12 = 0.0f;
			out->m13 = 0.0f;
			out->m14 = 0.0f;

			out->m21 = 0.0f;
			out->m22 = 1.0f;
			out->m23 = 0.0f;
			out->m24 = 0.0f;

			out->m31 = 0.0f;
			out->m32 = 0.0f;
			out->m33 = 1.0f;
			out->m34 = 0.0f;

			out->m41 = tran.x;
			out->m42 = tran.y;
			out->m43 = tran.z;
			out->m44 = 1.0f;

			MATRIX_CHECK( out );

			return out;
		}

		/**
		* 
		*
		* @param 		in
		* @return   	hFloat
		*/
		static hFloat determinant( const hMatrix* in )
		{    
			MATRIX_CHECK( in );

			return in->m11*( in->m22*( in->m33*in->m44 - in->m34*in->m43 ) - in->m32*( in->m23*in->m44 + in->m24*in->m43 ) + in->m42*( in->m23*in->m34 - in->m24*in->m33 ) )
				 - in->m21*( in->m12*( in->m33*in->m44 - in->m34*in->m43 ) - in->m32*( in->m13*in->m44 + in->m14*in->m43 ) + in->m42*( in->m13*in->m34 - in->m14*in->m33 ) )
				 + in->m31*( in->m12*( in->m23*in->m44 - in->m24*in->m43 ) - in->m22*( in->m13*in->m44 + in->m14*in->m43 ) + in->m42*( in->m13*in->m24 - in->m14*in->m23 ) )
				 - in->m41*( in->m21*( in->m23*in->m34 - in->m24*in->m33 ) - in->m22*( in->m13*in->m34 + in->m14*in->m33 ) + in->m32*( in->m13*in->m24 - in->m14*in->m23 ) );
		}

		/**
		* 
		*
		* @param 		in
		* @param 		out
		* @return   	Matrix*
		*/
		static hMatrix* transpose( const hMatrix* in, hMatrix* out )
		{

			out->m11 = in->m11; out->m12 = in->m21; out->m13 = in->m31; out->m14 = in->m41;
			out->m21 = in->m12; out->m22 = in->m22; out->m23 = in->m32; out->m24 = in->m42;
			out->m31 = in->m13; out->m32 = in->m23; out->m33 = in->m33; out->m34 = in->m43;
			out->m41 = in->m14; out->m42 = in->m24; out->m43 = in->m34; out->m44 = in->m44;

			return out;
		}

		/**
		* 
		*
		* @param 		in
		* @param 		out
		* @return   	Matrix*
		*/
		static hMatrix* inverse( const hMatrix* in, hMatrix* out )
		{
			//hFloat det;// = determinant( in );
			hMatrix* use;
			hMatrix m;

			use = in == out ? &m : out;

			//TODO: Write Matrix Inverse
			//#pragma message ("TODO: Write Matrix Inverse")
			float meh;
			D3DXMatrixInverse( (D3DXMATRIX*)out, &meh, (D3DXMATRIX*)in );

			if ( in == out )
			{
				*out = *use;
			}

			MATRIX_CHECK( out );

			return out;

		}

		/**
		* 
		*
		* @return   	bool
		*/
		bool isIdentity()
		{
			return ( m11 == 1.0f &&  m12 == 0.0f &&  m13 == 0.0f &&  m14 == 0.0f &&
					 m21 == 0.0f &&  m22 == 1.0f &&  m23 == 0.0f &&  m24 == 0.0f &&
					 m31 == 0.0f &&  m32 == 0.0f &&  m33 == 1.0f &&  m34 == 0.0f &&
					 m41 == 0.0f &&  m42 == 0.0f &&  m43 == 0.0f &&  m44 == 1.0f);
		}

		/**
		* 
		*
		* @param 		m
		* @return   	Matrix*
		*/
		static hMatrix* identity( hMatrix* m )
		{
			m->m11 = 1.0f;  m->m12 = 0.0f;  m->m13 = 0.0f;  m->m14 = 0.0f;
			m->m21 = 0.0f;  m->m22 = 1.0f;  m->m23 = 0.0f;  m->m24 = 0.0f;
			m->m31 = 0.0f;  m->m32 = 0.0f;  m->m33 = 1.0f;  m->m34 = 0.0f;
			m->m41 = 0.0f;  m->m42 = 0.0f;  m->m43 = 0.0f;  m->m44 = 1.0f;

			return m;
		}

		/**
		* 
		*
		* @param 		a
		* @param 		b
		* @param 		ab
		* @return   	Matrix*
		*/
		static hMatrix* mult( const hMatrix* a, const hMatrix* b, hMatrix* ab )
		{
			hMatrix ret;
			hMatrix* o;

			o = ab;
			if ( a == ab || b == ab )
			{
				o = &ret;
			}

			o->m11 = ( a->m11 * b->m11 ) + ( a->m12 * b->m21 ) + ( a->m13 * b->m31 ) + ( a->m14 * b->m41 );
			o->m12 = ( a->m11 * b->m12 ) + ( a->m12 * b->m22 ) + ( a->m13 * b->m32 ) + ( a->m14 * b->m42 );
			o->m13 = ( a->m11 * b->m13 ) + ( a->m12 * b->m23 ) + ( a->m13 * b->m33 ) + ( a->m14 * b->m43 );
			o->m14 = ( a->m11 * b->m14 ) + ( a->m12 * b->m24 ) + ( a->m13 * b->m34 ) + ( a->m14 * b->m44 );

			o->m21 = ( a->m21 * b->m11 ) + ( a->m22 * b->m21 ) + ( a->m23 * b->m31 ) + ( a->m24 * b->m41 );
			o->m22 = ( a->m21 * b->m12 ) + ( a->m22 * b->m22 ) + ( a->m23 * b->m32 ) + ( a->m24 * b->m42 );
			o->m23 = ( a->m21 * b->m13 ) + ( a->m22 * b->m23 ) + ( a->m23 * b->m33 ) + ( a->m24 * b->m43 );
			o->m24 = ( a->m21 * b->m14 ) + ( a->m22 * b->m24 ) + ( a->m23 * b->m34 ) + ( a->m24 * b->m44 );

			o->m31 = ( a->m31 * b->m11 ) + ( a->m32 * b->m21 ) + ( a->m33 * b->m31 ) + ( a->m34 * b->m41 );
			o->m32 = ( a->m31 * b->m12 ) + ( a->m32 * b->m22 ) + ( a->m33 * b->m32 ) + ( a->m34 * b->m42 );
			o->m33 = ( a->m31 * b->m13 ) + ( a->m32 * b->m23 ) + ( a->m33 * b->m33 ) + ( a->m34 * b->m43 );
			o->m34 = ( a->m31 * b->m14 ) + ( a->m32 * b->m24 ) + ( a->m33 * b->m34 ) + ( a->m34 * b->m44 );

			o->m41 = ( a->m41 * b->m11 ) + ( a->m42 * b->m21 ) + ( a->m43 * b->m31 ) + ( a->m44 * b->m41 );
			o->m42 = ( a->m41 * b->m12 ) + ( a->m42 * b->m22 ) + ( a->m43 * b->m32 ) + ( a->m44 * b->m42 );
			o->m43 = ( a->m41 * b->m13 ) + ( a->m42 * b->m23 ) + ( a->m43 * b->m33 ) + ( a->m44 * b->m43 );
			o->m44 = ( a->m41 * b->m14 ) + ( a->m42 * b->m24 ) + ( a->m43 * b->m34 ) + ( a->m44 * b->m44 );

			if ( o == &ret )
			{
				*ab = *o;
			}

			MATRIX_CHECK( ab );

			return ab;	
		}

		/**
		* 
		*
		* @param 		v
		* @param 		m
		* @param 		vout
		* @return   	Vec3*
		*/
		static hVec3& mult( const hVec3& v, const hMatrix* m, hVec3& vout )
		{
			hVec3 ret;
			hVec3* o;

			o = &vout;
			if ( &v == &vout )
			{
				o = &ret;
			}

			// assumes a 1 for w component of vector [7/28/2008 James]
			o->x = ( v.x * m->m11 ) + ( v.y * m->m21 ) + ( v.z * m->m31 ) + m->m41;
			o->y = ( v.x * m->m12 ) + ( v.y * m->m22 ) + ( v.z * m->m32 ) + m->m42;
			o->z = ( v.x * m->m13 ) + ( v.y * m->m23 ) + ( v.z * m->m33 ) + m->m43;

			if ( &ret == o )
			{
				vout = *o;
			}

			return vout;
		}

		/**
		* 
		*
		* @param 		v
		* @param 		m
		* @param 		vout
		* @return   	Vec3*
		*/
		static hVec3& multRotOnly( const hVec3& v, const hMatrix* m, hVec3& vout )
		{
			hVec3 ret;
			hVec3* o;

			o = &vout;
			if ( &v == &vout )
			{
				o = &ret;
			}

			// assumes a 1 for w component of vector [7/28/2008 James]
			//o->x = ( v.x * m->m11 ) + ( v.y * m->m12 ) + ( v.z * m->m13 );
			//o->y = ( v.x * m->m21 ) + ( v.y * m->m22 ) + ( v.z * m->m23 );
			//o->z = ( v.x * m->m31 ) + ( v.y * m->m32 ) + ( v.z * m->m33 );
			o->x = ( v.x * m->m11 ) + ( v.y * m->m21 ) + ( v.z * m->m31 );
			o->y = ( v.x * m->m12 ) + ( v.y * m->m22 ) + ( v.z * m->m32 );
			o->z = ( v.x * m->m13 ) + ( v.y * m->m23 ) + ( v.z * m->m33 );


			if ( &ret == o )
			{
				vout = *o;
			}

			return vout;
		}

		/**
		* 
		*
		* @param 		v
		* @param 		m
		* @param 		vout
		* @return   	Vec4*
		*/
		static hVec4& mult( const hVec4& v, const hMatrix* m, hVec4& vout )
		{
			hVec4 ret;
			hVec4* o;

			o = &vout;
			if ( &v == &vout )
			{
				o = &ret;
			}

			o->x = ( v.x * m->m11 ) + ( v.y * m->m21 ) + ( v.z * m->m31 ) + ( v.w * m->m41 );
			o->y = ( v.x * m->m12 ) + ( v.y * m->m22 ) + ( v.z * m->m32 ) + ( v.w * m->m42 );
			o->z = ( v.x * m->m13 ) + ( v.y * m->m23 ) + ( v.z * m->m33 ) + ( v.w * m->m43 );
			o->w = ( v.x * m->m14 ) + ( v.y * m->m24 ) + ( v.z * m->m34 ) + ( v.w * m->m44 );

			if ( &ret == o )
			{
				vout = *o;
			}

			return vout;
		}

		static hMatrix* rotationFromQuaternion( const hQuaternion& q, hMatrix* m )
		{
			hFloat ww = 2.0f * q.w; 
			hFloat xx = 2.0f * q.x;
			hFloat yy = 2.0f * q.y;
			hFloat zz = 2.0f * q.z;
	  
 			m->m11 = 1.0f - yy*q.y - zz*q.z;
 			m->m12 = xx*q.y - ww*q.z;
 			m->m13 = xx*q.z + ww*q.y;
 			m->m14 = 0.0f;
	 
 			m->m21 = xx*q.y + ww*q.z;
 			m->m22 = 1.0f - xx*q.x - zz*q.z;
 			m->m23 = yy*q.z - ww*q.x;
 			m->m24 = 0.0f;
	 
 			m->m31 = xx*q.z - ww*q.y;
 			m->m32 = yy*q.z + ww*q.x;
 			m->m33 = 1.0f - xx*q.x - yy*q.y;
 			m->m34 = 0.0f;
	    
			m->m41 = 0.0f;
			m->m42 = 0.0f;
			m->m43 = 0.0f;
			m->m44 = 1.0f;

			MATRIX_CHECK( m );
	  
  			return m;
		}

		static hMatrix* scale( hFloat x, hFloat y, hFloat z, hMatrix* m )
		{

			m->m11 = x;     m->m12 = 0.0f;  m->m13 = 0.0f;  m->m14 = 0.0f;
			m->m21 = 0.0f;  m->m22 = y;		m->m23 = 0.0f;  m->m24 = 0.0f;
			m->m31 = 0.0f;  m->m32 = 0.0f;  m->m33 = z;		m->m34 = 0.0f;
			m->m41 = 0.0f;  m->m42 = 0.0f;  m->m43 = 0.0f;  m->m44 = 1.0f;

			return m;
		}

		static hBool checkForNAN( const hMatrix* m )
		{
			for ( int i = 0; i < 16; ++i )
			{ 
				if ( _finite( m->m[ i ] ) == 0 )
				{
					return hFalse;
				}
			}

			return hTrue;
		}

		bool operator == ( const hMatrix& b )
		{
			if (
			b.m11 == m11 && b.m12 == m12 && b.m13 == m13 && b.m14 == m14 &&
			b.m21 == m21 && b.m22 == m22 && b.m23 == m23 && b.m24 == m24 &&
			b.m31 == m31 && b.m32 == m32 && b.m33 == m33 && b.m34 == m34 &&
			b.m41 == m41 && b.m42 == m42 && b.m43 == m43 && b.m44 == m44 )
				return true;
			return false;
		}

		static hVec3 getScale( const hMatrix& t )
		{
			hVec3 scl;

			hVec3::set( 
				sqrt( t.m11*t.m11 + t.m21*t.m21 + t.m31*t.m31 ),
				sqrt( t.m12*t.m12 + t.m22*t.m22 + t.m32*t.m32 ),
				sqrt( t.m13*t.m13 + t.m23*t.m23 + t.m33*t.m33 ),
				scl
				);

			return scl;
		}

		static hVec3 GetTranslation( const hMatrix& t )
		{
			return hVec3( t.m41, t.m42, t.m43 );
		}

		static hVec3 GetTranslation( const hMatrix* t )
		{
			return hVec3( t->m41, t->m42, t->m43 );
		}

		static void decomposeEuler( const hMatrix& m, hVec3& trans, hVec3& scl, hVec3& rotDeg )
		{
 			// transpose [2/2/2009 James]
 			hMatrix t = m;
 			//transpose( &m, &t );
	 
 			// transform [2/2/2009 James]
 			hVec3::set( t.m41, t.m42, t.m43, trans );
	 	
 			// scale [2/2/2009 James]
 			hVec3::set( 
 				sqrt( t.m11*t.m11 + t.m12*t.m12 + t.m13*t.m13 ),
 				sqrt( t.m21*t.m21 + t.m22*t.m22 + t.m23*t.m23 ),
 				sqrt( t.m31*t.m31 + t.m32*t.m32 + t.m33*t.m33 ),
 				scl
 				);
	 
 			t.m11 /= scl.x;
 			t.m21 /= scl.x;
 			t.m31 /= scl.x;
 			t.m41 = 0.0f;
	 
 			t.m12 /= scl.y;
 			t.m22 /= scl.y;
 			t.m32 /= scl.y;
 			t.m42 = 0.0f;
	 
 			t.m13 /= scl.z;
 			t.m23 /= scl.z;
 			t.m33 /= scl.z;
 			t.m43 = 0.0f;
	 
 			//heading = y , pitch = x, bank = z
			hFloat thetaX,thetaY,thetaZ;
			if ( t.m13 < 1.0f )
			{
				if ( t.m13 > -1.0f )
				{
					thetaY = asin( t.m13 );
					thetaX = atan2( -t.m23, t.m33 );
					thetaZ = atan2( -t.m12, t.m11 );
				}
				else
				{
					// Not a unique solution: thetaZ - thetaX = atan2(r21,r22)
					thetaY = -hmPI/2;
					thetaX = -atan2( t.m21, t.m22 );
					thetaZ = 0;
				}
			}
			else //m13 > 1.0f
			{
				//no unqiue solution
				// Not a unique solution: thetaZ + thetaX = atan2(r21,r22)
				thetaY = hmPI/2;
				thetaX = atan2( t.m21, t.m22 );
				thetaZ = 0;
			}

			rotDeg.x = hmRadToDeg( thetaX );
			rotDeg.y = hmRadToDeg( thetaY );
			rotDeg.z = hmRadToDeg( thetaZ );
		}

		static void decomposeQuat( const hMatrix& m, hVec3& trans, hVec3& scl, hQuaternion& q )
		{
			// transpose [2/2/2009 James]
			hMatrix t = m;
			//transpose( &m, &t );

			// transform [2/2/2009 James]
			hVec3::set( t.m41, t.m42, t.m43, trans );

			// scale [2/2/2009 James]
			hVec3::set( 
				sqrt( t.m11*t.m11 + t.m12*t.m12 + t.m13*t.m13 ),
				sqrt( t.m21*t.m21 + t.m22*t.m22 + t.m23*t.m23 ),
				sqrt( t.m31*t.m31 + t.m32*t.m32 + t.m33*t.m33 ),
				scl
				);

			t.m11 /= scl.x;
			t.m21 /= scl.x;
			t.m31 /= scl.x;
			t.m41 = 0.0f;

			t.m12 /= scl.y;
			t.m22 /= scl.y;
			t.m32 /= scl.y;
			t.m42 = 0.0f;

			t.m13 /= scl.z;
			t.m23 /= scl.z;
			t.m33 /= scl.z;
			t.m43 = 0.0f;

 			q.w = sqrt( hMax( 0.0f, 1 + t.m11 + t.m22 + t.m33 ) ) / 2.0f;
 			q.x = sqrt( hMax( 0.0f, 1 + t.m11 - t.m22 - t.m33 ) ) / 2.0f;
 			q.y = sqrt( hMax( 0.0f, 1 - t.m11 + t.m22 - t.m33 ) ) / 2.0f;
 			q.z = sqrt( hMax( 0.0f, 1 - t.m11 - t.m22 + t.m33 ) ) / 2.0f;

			q.x = (hFloat)_copysign( q.x, t.m32 - t.m23 );
			q.y = (hFloat)_copysign( q.y, t.m13 - t.m31 );
			q.z = (hFloat)_copysign( q.z, t.m21 - t.m12 );

		}
	};

	inline hVec2 operator * ( const hVec2& v, const hMatrix& m )
	{
		//	we assume 1 for z & w components of the vector
		return hVec2( ( m.m11 * v.x ) + ( m.m21 * v.y ) + m.m31 + m.m41, ( m.m12 * v.x ) + ( m.m22 * v.y ) + m.m32 + m.m42 );
	}

	static const hMatrix IdentityMatrix = 
	{ 
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f,
	};

}

#endif 