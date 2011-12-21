/********************************************************************
	created:	2008/07/13
	created:	13:7:2008   10:27
	filename: 	Vec2.h
	author:		James Moran
	
	purpose:	
*********************************************************************/
#ifndef hmVec2_h__
#define hmVec2_h__

#include <math.h>
#include "hTypes.h"

namespace Heart
{

	//-----------------------------------------------------------------------
	///
	/// @class Vec2
	/// @brief 
	/// @author James Moran
	/// @date [13/7/2008]
	/// @version 1.0
	///
	//-----------------------------------------------------------------------
	class hVec2
	{
	public:

		hFloat    x;
		hFloat    y;

		/**
		* 
		*
		* @return   	
		*/
		hVec2();

		/**
		* 
		*
		* @param 		x
		* @param 		y
		* @return   	
		*/
		hVec2( hFloat x, hFloat y );

		/**
		* 
		*
		* @param 		a
		* @return   	Vec2
		*/
		hVec2					operator + ( const hVec2& a ) const;

		/**
		* 
		*
		* @param 		a
		* @return   	Vec2&
		*/
		hVec2&					operator += ( const hVec2& a );

		/**
		* 
		*
		* @param 		a
		* @return   	Vec2
		*/
		hVec2					operator - ( const hVec2& a ) const;

		/**
		* 
		*
		* @param 		a
		* @return   	Vec2&
		*/
		hVec2&					operator -= ( const hVec2& a );

		/**
		* 
		*
		* @param 		s
		* @return   	Vec2
		*/
		hVec2					operator * ( const hFloat s ) const;

		/**
		* 
		*
		* @param 		s
		* @return   	Vec2&
		*/
		hVec2&					operator *= ( const hFloat s );

		/**
		* 
		*
		* @param 		s
		* @return   	Vec2
		*/
		hVec2					operator / ( const hFloat s ) const;

		/**
		* 
		*
		* @param 		s
		* @return   	Vec2&
		*/
		hVec2&					operator /= ( const hFloat s );

		/**
		* 
		*
		* @param 		a
		* @return   	bool
		*/
		bool					operator == ( const hVec2& a ) const;

		/**
		* 
		*
		* @param 		a
		* @return   	bool
		*/
		bool					operator != ( const hVec2& a ) const;

		/**
		* Vector dot product
		*
		* @param 		a
		* @return   	hFloat
		*/
		hFloat					operator * ( const hVec2& a ) const;

		/**
		* 
		*
		* @return   	hFloat
		*/
		hFloat					mag() const;

		/**
		* 
		*
		* @return   	hFloat
		*/
		hFloat					magSqr() const;

		/**
		* 
		*
		* @return   	void
		*/
		void					normalise();

		//////////////////////////////////////////////////////////////////////////
		// C style functions

		/**
		* 
		*
		* @param 		a
		* @param 		b
		* @param 		dst
		* @return   	void
		*/
		static void					add( const hVec2* a, const hVec2* b, hVec2* dst );
		/**
		* 
		*
		* @param 		a
		* @param 		b
		* @param 		dst
		* @return   	void
		*/
		static void					sub( const hVec2* a, const hVec2* b, hVec2* dst );
		/**
		* 
		*
		* @param 		a
		* @param 		norm
		* @return   	void
		*/
		static void					normalise( const hVec2* a, hVec2* norm );
		/**
		* 
		*
		* @param 		a
		* @return   	hFloat
		*/
		static hFloat				mag( const hVec2* a );
		/**
		* 
		*
		* @param 		a
		* @return   	hFloat
		*/
		static hFloat				magSqr( const hVec2* a );

		/**
		* Dot 
		*
		* @param 	const Vec2 & a
		* @param 	const Vec2 & b
		* @return   hFloat
		*/
		static hFloat				Dot( const hVec2& a, const hVec2& b )
		{
			return a.x*b.x + a.y*b.y;
		}

		static hFloat				Cross( const hVec2& a, const hVec2& b )
		{
			return a.x*b.y - a.y*b.x;
		}

		static void					Set( hFloat x, hFloat y, hVec2& out )
		{
			out.x = x;
			out.y = y;
		}
	};

	//////////////////////////////////////////////////////////////////////////
	//
	inline hVec2::hVec2() 
	{

	}

	//////////////////////////////////////////////////////////////////////////
	//
	inline hVec2::hVec2( hFloat x, hFloat y ) :
		x( x ),
		y( y )
	{

	}

	//////////////////////////////////////////////////////////////////////////
	//
	inline hVec2 hVec2::operator + ( const hVec2& a ) const
	{
		return hVec2( x + a.x, y + a.y );
	}

	//////////////////////////////////////////////////////////////////////////
	//
	inline hVec2& hVec2::operator += ( const hVec2& a )
	{
		x += a.x;
		y += a.y;

		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	//
	inline hVec2 hVec2::operator - ( const hVec2& a ) const 
	{
		return hVec2( x - a.x, y - a.y );
	}

	//////////////////////////////////////////////////////////////////////////
	//
	inline hVec2& hVec2::operator -= ( const hVec2& a )
	{
		x -= a.x;
		y -= a.y;

		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	//
	inline hVec2 hVec2::operator * ( const hFloat s ) const 
	{
		return hVec2( x * s, y * s );
	}

	//////////////////////////////////////////////////////////////////////////
	//
	inline hVec2& hVec2::operator *= ( const hFloat s )
	{
		x *= s;
		y *= s;

		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	//
	inline hVec2 hVec2::operator / ( const hFloat s ) const 
	{
		return hVec2( x / s, y / s );
	}

	//////////////////////////////////////////////////////////////////////////
	//
	inline hVec2& hVec2::operator /= ( const hFloat s )
	{
		x /= s;
		y /= s;

		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	//
	inline bool hVec2::operator == ( const hVec2& a ) const
	{
		return x == a.x && y == a.y;
	}

	//////////////////////////////////////////////////////////////////////////
	//
	inline bool hVec2::operator != ( const hVec2& a ) const
	{
		return !(*this == a);
	}

	//////////////////////////////////////////////////////////////////////////
	//
	inline hFloat hVec2::operator * ( const hVec2& a ) const
	{
		return ( x * a.x ) + ( y * a.y );
	}

	//////////////////////////////////////////////////////////////////////////
	//
	inline hFloat hVec2::mag() const
	{
		return sqrt( magSqr() ); 
	}

	//////////////////////////////////////////////////////////////////////////
	//
	inline hFloat hVec2::magSqr() const
	{
		return ( x * x ) + ( y * y );
	}

	//////////////////////////////////////////////////////////////////////////
	//
	inline void hVec2::normalise()
	{
		hFloat k = mag();
		x /= k;
		y /= k;
	}

	//////////////////////////////////////////////////////////////////////////
	//
	inline void hVec2::add( const hVec2* a, const hVec2* b, hVec2* dst )
	{
		dst->x = a->x + b->x;
		dst->y = a->y + b->y;
	}

	//////////////////////////////////////////////////////////////////////////
	//
	inline void hVec2::sub( const hVec2* a, const hVec2* b, hVec2* dst )
	{
		dst->x = a->x - b->x;
		dst->y = a->y - b->y;
	}

	//////////////////////////////////////////////////////////////////////////
	//
	inline void hVec2::normalise( const hVec2* a, hVec2* norm )
	{
		hFloat k = hVec2::mag( a );
		norm->x = a->x / k;
		norm->y = a->y / k;
	}

	//////////////////////////////////////////////////////////////////////////
	//
	inline hFloat hVec2::mag( const hVec2* a )
	{
		return sqrt( ( a->x * a->x ) + ( a->y * a->y ) );
	}

	//////////////////////////////////////////////////////////////////////////
	//
	inline hFloat hVec2::magSqr( const hVec2* a )
	{
		return ( a->x * a->x ) + ( a->y * a->y );
	}

	//////////////////////////////////////////////////////////////////////////
	//
	inline hVec2 operator * ( const hFloat s, const hVec2& v )
	{
		return hVec2( v.x * s, v.y * s );
	}

	//////////////////////////////////////////////////////////////////////////
	//
	inline hVec2 operator - ( const hVec2& a )
	{
		return hVec2( -a.x, -a.y );
	}

}

#endif // hmVec2_h__