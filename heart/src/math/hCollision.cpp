/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#include "base/hTypes.h"
#include "math/hVec3.h"
#include "math/hMatrix.h"
#include "math/hPlane.h"
#include "math/hAABB.h"

// Test if sphere with radius r moving from a to b intersects with plane p
// int TestMovingSpherePlane(Point a, Point b, float r, Plane p)
// {
// 	// Get the distance for both a and b from plane p
// 	float adist = Dot(a, p.n) - p.d;
// 	float bdist = Dot(b, p.n) - p.d;
// 	// Intersects if on different sides of plane (distances have different signs)
// 	if (adist * bdist < 0.0f) return 1;
// 	// Intersects if start or end position within radius from plane
// 	if (Abs(adist) <= r || Abs(bdist) <= r) return 1;
// 	// No intersection
// 	return 0;
// }
namespace Heart {
	hBool intersectMovingAABB( const hAABB& a, const hVec3& d, const hPlane& p ) {
		// Get radius of aabb along plane normal
		//hFloat r = ( a.r[ 0 ] * fabs( p.n.x ) ) + ( a.r[ 1 ] * fabs( p.n.y ) ) + ( a.r[ 2 ] * fabs( p.n.z ) );
		hFloat r = dot( a.r_, absPerElem(p.n) );
		hVec3 p1 = a.c_;
		hVec3 p2 = a.c_ + d;

 		// Get the distance for both a and b from plane p
		//hFloat adist = hVec3Func::dot( p1, p.n ) - p.d;
		//hFloat bdist = hVec3Func::dot( p2, p.n ) - p.d;
        hFloat adist = distFromPlane( p1, p );
        hFloat bdist = distFromPlane( p2, p );
		
 		// Intersects if on different sides of plane (distances have different signs)
 		if (adist * bdist < 0.0f) 
		{
			return hTrue;
		}

 		// Intersects if start or end position within radius from plane
 		if ( hFabs( adist ) <= r || hFabs( bdist ) <= r ) 
		{
			return hTrue;
		}
 		// No intersection
 		return hFalse;
	}

	hFloat AABBInfrontOfPlane( const hAABB& a, const hPlane& p ) {
		// Get radius of aabb along plane normal
		//hFloat r = ( a.r[ 0 ] * fabs( p.n.x ) ) + ( a.r[ 1 ] * fabs( p.n.y ) ) + ( a.r[ 2 ] * fabs( p.n.z ) );
		//hVec3 p1 = a.c;

		// Get the distance from plane p
		//hFloat adist = hVec3::dot( p1, p.n ) - p.d;
		// Return infront
		//return adist;
		// 
        hFloat r = dot( a.r_, absPerElem(p.n) );
        return r < hFabs( distFromPlane( a.c_, p ) );
	}
}