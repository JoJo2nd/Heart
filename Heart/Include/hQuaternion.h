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

namespace Heart
{

	//-----------------------------------------------------------------------
	///
	/// @class Quaternion
	/// @brief 
	/// @author James Moran
	/// @date [13/7/2008]
	/// @version 1.0
	///
	//-----------------------------------------------------------------------
	class hQuaternion
	{
	public:
		union
		{
			hFloat q[ 4 ];
			struct
			{
				hFloat w;
				hFloat x;
				hFloat y;
				hFloat z;
			};
		};

		hQuaternion() 
		{

		}

		hQuaternion( hFloat cw, hFloat cx, hFloat cy, hFloat cz ) :
			w( cw ),
			x( cx ),
			y( cy ),
			z( cz )
		{

		}

		/**
		* 
		*
		* @return   	void
		*/
		void							Identity();
		/**
		* 
		*
		* @param 		dst
		* @return   	Quaternion*
		*/
		static hQuaternion&			identity( hQuaternion& dst )
		{
			dst.w = 1.0f;
			dst.x = 0.0f;
			dst.y = 0.0f;
			dst.z = 0.0f;

			return dst;
		}

		static hQuaternion&				RotateAxis( hFloat x, const hVec3& axis, hQuaternion& dst )
		{
			hFloat theta = x * 0.5f;

			hVec3 a;
			hVec3::normalise( axis, a );

			dst.w = cos( theta );
			dst.x = sin( theta )*a.x;
			dst.y = sin( theta )*a.y;
			dst.z = sin( theta )*a.z;

			return dst;
		}

		/**
		* 
		*
		* @param 		x
		* @param 		dst
		* @return   	void
		*/
		static hQuaternion&				rotateX( hFloat x, hQuaternion& dst )
		{
			hFloat theta = x * 0.5f;

			dst.w = cos( theta );
			dst.x = sin( theta );
			dst.y = 0.0f;
			dst.z = 0.0f;

			return dst;
		}

		static hQuaternion&				rotateY( hFloat y, hQuaternion& dst )
		{
			hFloat theta = y * 0.5f;

			dst.w = cos( theta );
			dst.x = 0.0f;
			dst.y = sin( theta );
			dst.z = 0.0f;

			return dst;
		}

		static hQuaternion&				rotateZ( hFloat z, hQuaternion& dst )
		{
			hFloat theta = z * 0.5f;

			dst.w = cos( theta );
			dst.x = 0.0f;
			dst.y = 0.0f;
			dst.z = sin( theta );

			return dst;
		}

		/**
		* TODO: fix so rotation order is XYZ
		*
		* @param 		x
		* @param 		y
		* @param 		z
		* @param 		dst
		* @return   	Quaternion*
		*/
		static hQuaternion&			rotate( hFloat x, hFloat y, hFloat z, hQuaternion& dst )
		{
			hFloat sx, sy, sz;
			hFloat cx, cy, cz;
			hFloat x2 = x * 0.5f;
			hFloat y2 = y * 0.5f;
			hFloat z2 = z * 0.5f;

			sx = sin( x2 ); cx = cos( x2 );
			sy = sin( y2 ); cy = cos( y2 );
			sz = sin( z2 ); cz = cos( z2 );

	//  		dst.w = ( cy * cx * cz ) + ( sy * sx * sz );
	//  		dst.x = ( cy * sx * cz ) + ( sy * cx * sz );
	//  		dst.y = ( -cy * sx * sz ) + ( sy * cx * cz );
	//  		dst.z = ( -sy * sx * cz ) + ( cy * cx * sz );

 			dst.w = ( cx * cy * cz ) - ( sx * sy * sz );
 			dst.x = ( sx * cy * cz ) + ( cx * sy * sz );
 			dst.y = ( cx * sy * cz ) - ( sx * cy * sz );
 			dst.z = ( cx * cy * sz ) + ( sx * sy * cz );

			return dst;
		}

		static hQuaternion&			mult( const hQuaternion& a, const hQuaternion& b, hQuaternion& c )
		{
			hQuaternion ret;
			hQuaternion* o = &c;
			if ( &a == &c || &b == &c )
			{
				o = &ret;
			}

	// 		o->w = ( a.w * b.w ) - ( a.x * b.x ) - ( a.y * b.y ) - ( a.z * b.z );
	// 		o->x = ( a.w * b.x ) + ( a.x * b.w ) + ( a.z * b.y ) - ( a.y * b.z );
	// 		o->y = ( a.w * b.y ) + ( a.y * b.w ) + ( a.x * b.z ) - ( a.z * b.x );
	// 		o->z = ( a.w * b.z ) + ( a.z * b.w ) + ( a.y * b.x ) - ( a.x * b.y );

			o->w = ( a.w * b.w ) - ( a.x * b.x ) - ( a.y * b.y ) - ( a.z * b.z );
			o->x = ( a.w * b.x ) + ( a.x * b.w ) + ( a.y * b.z ) - ( a.z * b.y );
			o->y = ( a.w * b.y ) + ( a.y * b.w ) + ( a.z * b.x ) - ( a.x * b.z );
			o->z = ( a.w * b.z ) + ( a.z * b.w ) + ( a.x * b.y ) - ( a.y * b.x );


			if ( o == &ret )
			{
				c = *o;
			}

			return c;
		}

		/**
		* 
		*
		* @param 		a
		* @return   	void
		*/
		void							operator * ( const hQuaternion& a );

		bool							operator == ( const hQuaternion& b )
		{
			return ( x == b.x && y == b.y && z == b.z && w == b.w );
		}

		/**
		* 
		*
		* @return   	void
		*/
		void							Normalise();
		/**
		* 
		*
		* @param 		a
		* @param 		norm
		* @return   	Quaternion*
		*/
		static hQuaternion&			normalise( const hQuaternion& a, hQuaternion& norm )
		{
			hFloat mag = sqrt( ( a.w * a.w ) + ( a.x * a.x ) + ( a.y * a.y ) + ( a.z * a.z ) );

			hcAssert( mag > 0.0f );
			
			if ( mag > 0.0f )
			{
				float d = 1.0f / mag;
				norm.w = a.w * d; 
				norm.x = a.x * d;
				norm.y = a.y * d;
				norm.z = a.z * d;
			}

			return norm;
		}

		static hFloat					length( const hQuaternion& a )
		{
			return sqrt( ( a.w * a.w ) + ( a.x * a.x ) + ( a.y * a.y ) + ( a.z * a.z ) );
		}

		/**
		* 
		*
		* @param 		a
		* @param 		b
		* @return   	hFloat
		*/
		static hFloat					dot( const hQuaternion& a, const hQuaternion& b )
		{
			return ( a.w * b.w ) + ( a.x * b.x ) + ( a.y * b.y ) + ( a.z * b.y );
		}

		static hQuaternion&			slerp( const hQuaternion& a, const hQuaternion& b, hFloat dt, hQuaternion& out )
		{
			if ( dt <= 0.0f )
			{
				out.w = a.w;
				out.x = a.x;
				out.y = a.y;
				out.z = a.z;

				return out;
			}
			if ( dt >= 1.0f )
			{
				out.w = b.w;
				out.x = b.x;
				out.y = b.y;
				out.z = b.z;

				return out;
			}

			hFloat cosomega = hQuaternion::dot( a, b );

			if ( fabs( cosomega ) == 1.0f )
			{
				//quaternions are the same or not normalised, return a
				out = a;
				return out;
			}

			hFloat bw = b.w;
			hFloat bx = b.x;
			hFloat by = b.y;
			hFloat bz = b.z;

			// if cosomega is < 0 angle between a & b is obtuse and we a the acute one [11/27/2008 James]
			if ( cosomega < 0.0f )
			{
				cosomega = -cosomega;
				bw = -bw;
				bx = -bx;
				by = -by;
				bz = -bz;
			}

			// check quaternions are unit [11/27/2008 James]
			hcAssert( cosomega < 1.1f );

			hFloat k0,k1;

			// use sin^2 + cos^2 == 1 to get sin of angle [11/27/2008 James]
			hFloat sinomega = sqrt( 1.0f - ( cosomega * cosomega ) );
			hFloat omega = atan2( sinomega, cosomega );//acos( cosomega );
			hFloat oneoversin = 1.0f / sinomega;

			// get the lerping values [11/27/2008 James]
			k0 = sin( ( 1.0f - dt ) * omega ) * oneoversin;
			k1 = sin( dt * omega ) * oneoversin;

			out.w = ( k0 * a.w ) + ( k1 * bw );
			out.x = ( k0 * a.x ) + ( k1 * bx );
			out.y = ( k0 * a.y ) + ( k1 * by );
			out.z = ( k0 * a.z ) + ( k1 * bz );

			normalise( out, out );

			return out;
		}

		static hQuaternion& conjugate( hQuaternion& a, hQuaternion& out )
		{
			out.w = a.w;

			out.x = -a.x;
			out.y = -a.y;
			out.z = -a.z;

			return out;
		}

		static hQuaternion build( hFloat x, hFloat y, hFloat z )
		{
			hFloat w = 1.0f - (x * x) - (y * y) - (z * z);
 			w = sqrt( fabs( w ) );
			hQuaternion q;
			hQuaternion::normalise( hQuaternion ( w, x, y, z ), q );
			return q;
		}

		static hBool IsIdentity( const hQuaternion& q )
		{
			return q.w == 1.0f && q.x == 0.0f && q.y == 0.0f && q.z == 0.0f;
		}
	};

}

#endif // HMQUATERNION_H__
