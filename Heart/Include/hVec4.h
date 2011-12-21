/********************************************************************
	created:	2009/01/06

	filename: 	Vec4.h

	author:		James Moran
	
	purpose:	
*********************************************************************/
#ifndef __HMVEC4_H__
#define __HMVEC4_H__

#include <math.h>
#include "hTypes.h"

namespace Heart
{

	class hVec4
	{
	public:

		union
		{
			struct
			{
				hFloat   	x;
				hFloat   	y;
				hFloat   	z;
				hFloat		w;
			};
			struct
			{
				hFloat		v[ 4 ];
			};
		};

		hVec4() 
		{

		}

		hVec4( hFloat x_, hFloat y_, hFloat z_, hFloat w_ ) :
			x( x_ ),
			y( y_ ),
			z( z_ ),
			w( w_ )
		{

		}

		//operators
		//add
		hVec4 operator + ( const hVec4& a ) const
		{
			return hVec4( x + a.x, y + a.y, z + a.z, w + a.w );
		}

		hVec4& operator += ( const hVec4& a )
		{
			x += a.x;
			y += a.y;
			z += a.z;
			w += a.w;

			return *this;
		}

		//minus
		hVec4 operator - ( const hVec4& a ) const 
		{
			return hVec4( x - a.x, y - a.y, z - a.z, w - a.w );
		}

		hVec4& operator -= ( const hVec4& a )
		{
			x -= a.x;
			y -= a.y;
			z -= a.z;
			w -= a.w;

			return *this;
		}

		// scale
		hVec4 operator * ( const hFloat s ) const 
		{
			return hVec4( x * s, y * s, z * s, w * s );
		}

		hVec4& operator *= ( const hFloat s )
		{
			x *= s;
			y *= s;
			z *= s;
			w *= s;

			return *this;
		}

		//divide
		hVec4 operator / ( const hFloat s ) const 
		{
			return hVec4( x / s, y / s, z / s, w / s );
		}

		hVec4& operator /= ( const hFloat s )
		{
			x /= s;
			y /= s;
			z /= s;
			w /= s;

			return *this;
		}

		bool operator == ( const hVec4& a )
		{
			return x == a.x && y == a.y && z == a.z && w == a.w;
		}

		bool operator != ( const hVec4& a )
		{
			return !(*this == a);
		}

		//dot 
		hFloat operator * ( const hVec4& a ) const
		{
			return ( x * a.x ) + ( y * a.y ) + ( z * a.z ) + ( w * a.w );
		}

		//functions
		hFloat mag() const
		{
			return sqrt( magSqr() ); 
		}

		hFloat magSqr() const
		{
			return ( x * x ) + ( y * y ) + ( z * z ) + ( w * w );
		}

		void normalise()
		{
			hFloat k = mag();
			x /= k;
			y /= k;
			z /= k;
			w /= k;
		}

		//////////////////////////////////////////////////////////////////////////
		// C style funcitons


		/**
		* 
		*
		* @param 		v
		* @return   	hFloat
		*/
		static hFloat magSqr( const hVec4& v )
		{
			return ( v.x * v.x ) + ( v.y * v.y ) + ( v.z * v.z ) + ( v.w * v.w );
		}

		/**
		* 
		*
		* @param 		v
		* @return   	hFloat
		*/
		static hFloat mag( const hVec4& v )
		{
			return sqrt( magSqr( v ) );
		}

		/**
		* 
		*
		* @param 		v
		* @param 		out
		* @return   	hmVec3*
		*/
		static hVec4& normalise( const hVec4& v, hVec4& out )
		{
			hFloat m = mag( v );

			out.x = v.x / m;
			out.y = v.y / m;
			out.z = v.z / m;
			out.w = v.w / m;

			return out;
		}

		/**
		*	a + b = out
		*
		* @param 		a
		* @param 		b
		* @param 		out
		* @return   	hmVec3*
		*/
		static hVec4& add( const hVec4& a, const hVec4& b, hVec4& out )
		{
			out.x = a.x + b.x;
			out.y = a.y + b.y;
			out.z = a.z + b.z;
			out.w = a.w + b.w;

			return out;
		}

		/**
		*	a - b = out
		*
		* @param 		a
		* @param 		b
		* @param 		out
		* @return   	hmVec3*
		*/
		static hVec4& sub( const hVec4& a, const hVec4& b, hVec4& out )
		{
			out.x = a.x - b.x;
			out.y = a.y - b.y;
			out.z = a.z - b.z;
			out.w = a.w - b.w;

			return out;
		}

		/**
		*	a * s = out
		*
		* @param 		a
		* @param 		s
		* @param 		out
		* @return   	hmVec3*
		*/
		static hVec4& mul( const hVec4& a, const hFloat s, hVec4& out )
		{
			out.x = a.x * s;
			out.y = a.y * s;
			out.z = a.z * s;
			out.w = a.w * s;

			return out;
		}

		/**
		*	a / s = out
		*
		* @param 		a
		* @param 		s
		* @param 		out
		* @return   	hmVec3*
		*/
		static hVec4& div( const hVec4& a, const hFloat s, hVec4& out )
		{
			out.x = a.x / s;
			out.y = a.y / s;
			out.z = a.z / s;
			out.w = a.w / s;

			return out;
		}

		/**
		*  a *dot* b = return value
		*
		* @param 		a
		* @param 		b
		* @return   	hFloat
		*/
		static hFloat dot( const hVec4& a, const hVec4& b )
		{
			return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
		}

		/**
		*  -a = out
		*
		* @param 		a
		* @param 		out
		* @return   	hmVec3*
		*/
		static hVec4& neg( const hVec4& a, hVec4& out )
		{
			out.x = -a.x;
			out.y = -a.y;
			out.z = -a.z;
			out.w = -a.w;

			return out;
		}

		/**
		* 
		*
		* @param 		x
		* @param 		y
		* @param 		z
		* @param 		out
		* @return   	hmVec3*
		*/
		static hVec4& set( hFloat x, hFloat y, hFloat z, hFloat w, hVec4& out )
		{
			out.x = x;
			out.y = y;
			out.z = z;
			out.w = w;

			return out;
		}

	};


	inline hVec4 operator * ( const hFloat s, const hVec4& v )
	{
		return hVec4( v.x * s, v.y * s, v.z * s, v.w * s );
	}

	inline hVec4 operator -( const hVec4& a )
	{
		return hVec4( -a.x , -a.y, -a.z, -a.w );
	}

}

#endif //__HMVEC4_H__