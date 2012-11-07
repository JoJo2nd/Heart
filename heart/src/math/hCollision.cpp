/********************************************************************

	filename: 	hCollision.cpp	
	
	Copyright (c) 1:4:2012 James Moran
	
	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.
	
	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:
	
	1. The origin of this software must not be misrepresented; you must not
	claim that you wrote the original software. If you use this software
	in a product, an acknowledgment in the product documentation would be
	appreciated but is not required.
	
	2. Altered source versions must be plainly marked as such, and must not be
	misrepresented as being the original software.
	
	3. This notice may not be removed or altered from any source
	distribution.

*********************************************************************/


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
namespace hPlaneFunc
{
	hBool intersectMovingAABB( const hAABB& a, const hVec3& d, const hPlane& p )
	{
		// Get radius of aabb along plane normal
		//hFloat r = ( a.r[ 0 ] * fabs( p.n.x ) ) + ( a.r[ 1 ] * fabs( p.n.y ) ) + ( a.r[ 2 ] * fabs( p.n.z ) );
		hFloat r = hVec3Func::dot( a.r_, hVec128Abs( p.p ) );
		hVec3 p1 = a.c_;
		hVec3 p2 = a.c_ + d;

 		// Get the distance for both a and b from plane p
		//hFloat adist = hVec3Func::dot( p1, p.n ) - p.d;
		//hFloat bdist = hVec3Func::dot( p2, p.n ) - p.d;
        hFloat adist = hPlaneFunc::DistFromPlane( p1, p );
        hFloat bdist = hPlaneFunc::DistFromPlane( p2, p );
		
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

	hFloat AABBInfrontOfPlane( const hAABB& a, const hPlane& p )
	{
		// Get radius of aabb along plane normal
		//hFloat r = ( a.r[ 0 ] * fabs( p.n.x ) ) + ( a.r[ 1 ] * fabs( p.n.y ) ) + ( a.r[ 2 ] * fabs( p.n.z ) );
		//hVec3 p1 = a.c;

		// Get the distance from plane p
		//hFloat adist = hVec3::dot( p1, p.n ) - p.d;
		// Return infront
		//return adist;
		// 
        hFloat r = hVec3Func::dot( a.r_, hVec128Abs( p.p ) );
        return r < hFabs( hPlaneFunc::DistFromPlane( a.c_, p ) );
	}
}
}