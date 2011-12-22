/********************************************************************
	created:	2010/01/17
	created:	17:1:2010   15:36
	filename: 	hmCollision.cpp	
	author:		James
	
	purpose:	
*********************************************************************/

#include "Common.h"
//#include "hAABB.h"
//#include "hPlane.h"
#include "hVec2.h"
#include "hVec3.h"
#include "hVec4.h"
#include "hQuaternion.h"
#include "hMatrix.h"

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

namespace Heart
{
/*
	hBool hPlane::intersectMovingAABB( const hAABB& a, const hVec3& d, const hPlane& p )
	{
		// Get radius of aabb along plane normal
		hFloat r = ( a.r[ 0 ] * fabs( p.n.x ) ) + ( a.r[ 1 ] * fabs( p.n.y ) ) + ( a.r[ 2 ] * fabs( p.n.z ) );
		hVec3 p1 = a.c;
		hVec3 p2 = a.c + d;

 		// Get the distance for both a and b from plane p
		hFloat adist = hVec3::dot( p1, p.n ) - p.d;
		hFloat bdist = hVec3::dot( p2, p.n ) - p.d;
 		// Intersects if on different sides of plane (distances have different signs)
 		if (adist * bdist < 0.0f) 
		{
			return hTrue;
		}

 		// Intersects if start or end position within radius from plane
 		if ( fabs(adist) <= r || fabs(bdist) <= r ) 
		{
			return hTrue;
		}
 		// No intersection
 		return hFalse;
	}

	hFloat hPlane::AABBInfrontOfPlane( const hAABB& a, const hPlane& p )
	{
		// Get radius of aabb along plane normal
		hFloat r = ( a.r[ 0 ] * fabs( p.n.x ) ) + ( a.r[ 1 ] * fabs( p.n.y ) ) + ( a.r[ 2 ] * fabs( p.n.z ) );
		hVec3 p1 = a.c;

		// Get the distance from plane p
		hFloat adist = hVec3::dot( p1, p.n ) - p.d;
		// Return infront
		return adist;
	}
*/
}