/********************************************************************
	created:	2008/07/13
	created:	13:7:2008   19:50
	filename: 	Vec3.h
	author:		James Moran
	
	purpose:	
*********************************************************************/
#include <math.h>
#include "hTypes.h"

namespace Heart
{
	//-----------------------------------------------------------------------
	///
	/// @class Vec3
	/// @brief 
	/// @author James Moran
	/// @date [13/7/2008]
	/// @version 1.0
	///
	//-----------------------------------------------------------------------
	class hVec3
	{
	public:

		union
		{
			struct
			{
				hFloat   	x;
				hFloat   	y;
				hFloat   	z;
			};
			struct
			{
				hFloat		v[ 3 ];
			};
		};

		hVec3() 
		{

		}

		hVec3( hFloat x, hFloat y, hFloat z ) :
			x( x ),
			y( y ),
			z( z )
		{

		}

		//operators
		//add
		hVec3 operator + ( const hVec3& a ) const
		{
			return hVec3( x + a.x, y + a.y, z + a.z );
		}

		hVec3& operator += ( const hVec3& a )
		{
			x += a.x;
			y += a.y;
			z += a.z;

			return *this;
		}

		//minus
		hVec3 operator - ( const hVec3& a ) const 
		{
			return hVec3( x - a.x, y - a.y, z - a.z );
		}

		hVec3& operator -= ( const hVec3& a )
		{
			x -= a.x;
			y -= a.y;
			z -= a.z;

			return *this;
		}

		// scale
		hVec3 operator * ( const hFloat s ) const 
		{
			return hVec3( x * s, y * s, z * s );
		}

		hVec3& operator *= ( const hFloat s )
		{
			x *= s;
			y *= s;
			z *= s;

			return *this;
		}

		//divide
		hVec3 operator / ( const hFloat s ) const 
		{
			return hVec3( x / s, y / s, z / s );
		}

		hVec3& operator /= ( const hFloat s )
		{
			x /= s;
			y /= s;
			z /= s;

			return *this;
		}

		bool operator == ( const hVec3& a ) const
		{
			return x == a.x && y == a.y && z == a.z;
		}

		bool operator != ( const hVec3& a )
		{
			return !(*this == a);
		}

		//dot 
		hFloat operator * ( const hVec3& a ) const
		{
			return ( x * a.x ) + ( y * a.y ) + ( z * a.z );
		}

		//functions
		hFloat mag() const
		{
			return sqrt( magSqr() ); 
		}

		hFloat magSqr() const
		{
			return ( x * x ) + ( y * y ) + ( z * z );
		}

		void normalise()
		{
			hFloat k = mag();
			x /= k;
			y /= k;
			z /= k;
		}

		//////////////////////////////////////////////////////////////////////////
		// C style funcitons


		/**
		* 
		*
		* @param 		v
		* @return   	hFloat
		*/
		static hFloat magSqr( const hVec3& v )
		{
			return ( v.x * v.x ) + ( v.y * v.y ) + ( v.z * v.z );
		}

		/**
		* 
		*
		* @param 		v
		* @return   	hFloat
		*/
		static hFloat mag( const hVec3& v )
		{
			return sqrt( magSqr( v ) );
		}

		/**
		* 
		*
		* @param 		v
		* @param 		out
		* @return   	Vec3*
		*/
		static hVec3& normalise( const hVec3& v, hVec3& out )
		{
			hFloat m = mag( v );

			out.x = v.x / m;
			out.y = v.y / m;
			out.z = v.z / m;

			return out;
		}

		/**
		*	a + b = out
		*
		* @param 		a
		* @param 		b
		* @param 		out
		* @return   	Vec3*
		*/
		static hVec3* add( const hVec3* a, const hVec3* b, hVec3* out )
		{
			out->x = a->x + b->x;
			out->y = a->y + b->y;
			out->z = a->z + b->z;

			return out;
		}

		/**
		*	a - b = out
		*
		* @param 		a
		* @param 		b
		* @param 		out
		* @return   	Vec3*
		*/
		static hVec3& sub( const hVec3& a, const hVec3& b, hVec3& out )
		{
			out.x = a.x - b.x;
			out.y = a.y - b.y;
			out.z = a.z - b.z;

			return out;
		}

		/**
		*	a * s = out
		*
		* @param 		a
		* @param 		s
		* @param 		out
		* @return   	Vec3*
		*/
		static hVec3* mul( const hVec3* a, const hFloat s, hVec3* out )
		{
			out->x = a->x * s;
			out->y = a->y * s;
			out->z = a->z * s;

			return out;
		}

		/**
		*	a / s = out
		*
		* @param 		a
		* @param 		s
		* @param 		out
		* @return   	Vec3*
		*/
		static hVec3* div( const hVec3* a, const hFloat s, hVec3* out )
		{
			out->x = a->x / s;
			out->y = a->y / s;
			out->z = a->z / s;

			return out;
		}

		/**
		*  a *dot* b = return value
		*
		* @param 		a
		* @param 		b
		* @return   	hFloat
		*/
		static hFloat dot( const hVec3& a, const hVec3& b )
		{
			return a.x * b.x + a.y * b.y + a.z * b.z;
		}

		/**
		*  -a = out
		*
		* @param 		a
		* @param 		out
		* @return   	Vec3*
		*/
		static hVec3* neg( const hVec3* a, hVec3* out )
		{
			out->x = -a->x;
			out->y = -a->y;
			out->z = -a->z;

			return out;
		}

		/**
		* a *cross* b = out
		*
		* @param 		a
		* @param 		b
		* @param 		out
		* @return   	Vec3*
		*/
		static hVec3& cross( const hVec3& a, const hVec3& b, hVec3& out )
		{
			out.x = ( a.y * b.z ) - ( a.z * b.y );
			out.y = ( a.z * b.x ) - ( a.x * b.z );
			out.z = ( a.x * b.y ) - ( a.y * b.x );

			return out;
		}

		/**
		* 
		*
		* @param 		x
		* @param 		y
		* @param 		z
		* @param 		out
		* @return   	Vec3*
		*/
		static hVec3& set( hFloat x, hFloat y, hFloat z, hVec3& out )
		{
			out.x = x;
			out.y = y;
			out.z = z;
			
			return out;
		}

	};


	inline hVec3 operator * ( const hFloat s, const hVec3& v )
	{
		return hVec3( v.x * s, v.y * s, v.z *s );
	}

	inline hVec3 operator - ( const hVec3& a )
	{
		return hVec3( -a.x , -a.y, -a.z );
	}

}
