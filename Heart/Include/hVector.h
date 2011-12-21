/********************************************************************
	created:	2008/07/13
	created:	13:7:2008   9:32
	filename: 	hmVector.h
	author:		James Moran
	
	purpose:	
*********************************************************************/
#ifndef MATH_VECTOR_H__
#define MATH_VECTOR_H__

#include "hVec2.h"
#include "hVec3.h"
#include "hVec4.h"
#include "hMathUtil.h"

namespace Heart
{
	/**
	* Reflect 
	*
	* @param 	const _V & a - the point to reflect on to norm
	* @param 	const _V & norm - the vector to reflect on to, must be normalised vector
	* @return   _V - result vector
	*/
	template< typename _V >
	inline _V Reflect( const _V& a, const _V& norm )
	{
		_V v;
		v = a - ( 2 * ( norm  * ( norm * a ) ) );
		return v;
	}

	/**
	* ProjectOnToAxis 
	*
	* @param 	const _V & v - vector to project onto axis
	* @param 	const _V & axis - axis to project onto
	* @return   _V - result vector
	*/
	template< typename _V >
	inline _V ProjectOnToAxis( const _V& v, const _V& axis )
	{
		return ( ( v * axis ) / axis.magSqr() ) * axis;
	}

	/**
	* ProjectOnToAxis 
	*
	* @param 	const _V & v - point to project on to ray
	* @param 	const _V & rayd - ray start to project onto
	* @param 	const _V & ray - ray to project onto
	* @param 	_V & out - result vector
	* @return   hFloat - t distance of result
	*/
	template< typename _V >
	inline hFloat ProjectOnToAxis( const _V& v, const _V& rayd, const _V& ray, _V& out )
	{
		hFloat t = ( ray * ( v-rayd ) ) / ray.mag();
		out = rayd + ( ray * t );
		return t;
	}


	/**
	* ClosestPointLineSegments - 
	* Computes closest points c1 & c2 of S1(s) = p1+s*(q1-p1) & 
	* S2(t) = p2+t*(q2-p2), returnign s and t. Fucntions result is squared
	* distance between S1(s) & S2(t).
	*
	* Check page 149 of Real-Time Collision Dection by Christer Ericson
	*
	* @param 	_V p1
	* @param 	_V q1
	* @param 	_V p2
	* @param 	_V q2
	* @param 	hFloat & s
	* @param 	hFloat & t
	* @param 	_V c1
	* @param 	_V c2
	* @return   hFloat
	*/
	template< typename _V >
	hFloat ClosestPointLineSegments( _V p1, _V q1, _V p2, _V q2, hFloat& s, hFloat& t, _V& c1, _V& c2 )
	{
		_V d1 = q1 - p1;// direction vector of S1
		_V d2 = q2 - p2;// direction vector of S2
		_V r = p1 - p2;
		hFloat a = _V::Dot( d1, d1 ); // squared lenght of S1, never negative
		hFloat e = _V::Dot( d2, d2 ); // squared length of S2, never negative
		hFloat f = _V::Dot( d2, r );

		if ( a <= Limits::FLOAT_EPSILON && e <= Limits::FLOAT_EPSILON )
		{
			// both segments are points
			s = t = 0.0f;
			c1 = p1;
			c2 = p2;

			return _V::Dot( c1 - c2, c1 - c2 );
		}

		if ( a <= Limits::FLOAT_EPSILON )
		{
			//first segment is a point
			s = 0.0f;
			t = f / e;
			t = MathUtil::Clamp( t, 0.0f, 1.0f );
		}
		else
		{
			float c = _V::Dot( d1, r );
			if ( e <= Limits::FLOAT_EPSILON )
			{
				// another point
				t = 0.0f;
				s = MathUtil::Clamp( -c/a, 0.0f, 1.0f );
			}
			else
			{
				// actual interesting work begins here
				hFloat b = _V::Dot( d1, d2 );
				hFloat demon = a*e - b*b; //< never negative

				//if segments are not parallel, compute the closest point on L1 to L2
				// and clamp to segment S1. else pick arbitrary s (here 0)
				if ( demon != 0.0f )
				{
					s = MathUtil::Clamp( (b*f - c*e) / demon, 0.0f, 1.0f );
				}
				else
				{
					s = 0.0f;
				}

				//Compute point on L" closest to S1(s) using
				//t = Dot((p1 + d1*s ) - p2, d1 )/Dot( d2, d2 )
				t = (b*s + f) / e;

				// if t in [0,1] done. Else clamp t, recompute s for the new
				// value of t using s = Dot( p2, d2 * t) - p1, d1 ) / dot ( d1,d1 ) 
				// = ( t*b - c ) / a and clamp s to [0,1]
				if ( t < 0.0f )
				{
					t = 0.0f;
					s = MathUtil::Clamp( -c/a, 0.0f, 1.0f );
				}
				else if ( t > 1.0f )
				{
					t = 1.0f;
					s = MathUtil::Clamp( (b-c) / a, 0.0f, 1.0f );
				}
			}
		}

		c1 = p1 + d1 * s;
		c2 = p2 + d2 * t;
		return _V::Dot( c1 - c2, c1 - c2 );
	}

	static const hVec2 ZeroVector2( 0.0f, 0.0f );
	static const hVec3 ZeroVector3( 0.0f, 0.0f, 0.0f );
	static const hVec4 ZeroVector4( 0.0f, 0.0f, 0.0f, 0.0f );

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    
    SERIALISE_WORKER_TYPE_SPECIALISATION( hVec2, hSerialisedElementHeader::Type_User );
    SERIALISE_WORKER_TYPE_SPECIALISATION( hVec3, hSerialisedElementHeader::Type_User );
    SERIALISE_WORKER_TYPE_SPECIALISATION( hVec4, hSerialisedElementHeader::Type_User );
}

#endif// MATH_VECTOR;