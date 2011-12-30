/********************************************************************
	created:	2009/01/01

	filename: 	Plane.h

	author:		James Moran
	
	purpose:	
*********************************************************************/
#ifndef __HMPLANE_H__
#define __HMPLANE_H__

#include "hTypes.h"
#include "hVector.h"

namespace Heart
{
	class hAABB;

	class hPlane
	{
	public:

		//hVec3		n;// normal to plane [1/1/2009 James]
		//hFloat		d;// d = dot( n, p ) for any point on the plane
		// a*p.X + b*p.Y + c*p.Z + p.W = 0
	    hVec128     p;
    };

namespace hPlaneFunc
{
	/**
	*
	* builds a plane that contains the 3 points a,b,c (ordered ccw) 
	*
	* @param		const Vec3 & a
	* @param		const Vec3 & b
	* @param		const Vec3 & c
	* @param		Plane & out
	* @return   	void
	*/
	hFORCEINLINE hPlane buildPlane( const hVec3& a, const hVec3& b, const hVec3& c )
	{
// 			hVec3 tmp;
// 			hVec3Func::cross( b - a, c - a, tmp );
// 			hVec3Func::normalise( tmp, out.n );
// 			out.d = hVec3Func::dot( out.n, a );
 			hPlane p;
            p.p = XMPlaneFromPoints( a.v, b.v, c.v );
	}

	/**
	*
	* return hTrue is a is behind a plane
	*
	* @param		const Vec3 & a
	* @param		Plane & p
	* @return   	hBool
	*/
	hFORCEINLINE hBool pointBehindPlane( const hVec3& a, const hPlane& p )
	{
		//return ( hVec3::dot( a, p.n ) - p.d ) < 0.0f;
        return hVec128AllLess( XMPlaneDotCoord( p.p, a.v ), hVec3Func::zeroVector() );
	}

	static hBool intersectMovingAABB( const hAABB& a, const hVec3& d, const hPlane& p );

	//Return +ive number if true, -ive if false
	static hFloat AABBInfrontOfPlane( const hAABB& a, const hPlane& p );

}

	class Plane2
	{
	public:
/*		
		hVec2	n;// normal to plane
		hFloat	d;// d = dot( n, p ) for any point on the plane

		static inline void BuildPlane( const hVec2& norm, const hVec2& pointOnPlane, Plane2& out )
		{
			out.n = norm;
			out.d = hVec2::Dot( out.n, pointOnPlane );
		}

		static inline hBool PointBehind( const hVec2& a, const Plane2& p )
		{
			return ( hVec2::Dot( a, p.n ) - p.d ) < 0.0f;
		}

		static inline hFloat DistanceFrom( const hVec2& a, const Plane2& p )
		{
			return hVec2::Dot( a, p.n ) - p.d;
		}

		static inline hBool Intersection( const Plane2& a, const Plane2& b, hVec2& is )
		{
		
			//x = (b2d1-b1d2)/(a1b2-a2b1)
			//y = (a1d2-a2d1)/(a1b2-a2b1)
		
			hFloat den = a.n.x*b.n.y - b.n.x*a.n.y;
			if ( den == 0.0f )
			{
				return hFalse;
			}

			hFloat xNum = b.n.y*a.d - a.n.y*b.d;
			hFloat yNum = a.n.x*b.d - b.n.x*a.d;

			is.x = xNum / den;
			is.y = yNum / den;

			return hTrue;
		}
*/
	};
}

#endif //__HMPLANE_H__