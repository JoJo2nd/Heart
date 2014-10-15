/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#ifndef __HMPLANE_H__
#define __HMPLANE_H__

#include "base/hTypes.h"
#include "math/hVec3.h"    

namespace Heart
{
    struct hAABB;
    
    struct hPlane
    {
    	hVec3 n;// normal to plane 
        hFloat d;// d = dot( n, p ) for any point on the plane
    	// a*p.X + b*p.Y + c*p.Z + p.W = 0
        //hVec4     p;
    };
    
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
    		hPlane p;
            p.n = normalize(cross(b - a, c - a));
    		p.d = dot(p.n, a);
            return p;
    }
    

    hFORCEINLINE hBool pointBehindPlane( const hVec3& a, const hPlane& p )
    {
    	return ( dot( a, p.n ) - p.d ) < 0.0f;
        //return hVec128AllLess( XMPlaneDotCoord( p.p, a.v ), hVec3Func::zeroVector() );
    }
    
    hFORCEINLINE hFloat distFromPlane( const hVec3& a, const hPlane& p )
    {
        return dot( a, p.n ) - p.d;
        //return hVec128GetX( XMPlaneDotCoord( p.p, a.v ) );
    }
    
    hBool intersectMovingAABB( const hAABB& a, const hVec3& d, const hPlane& p );
    
    //Return +ive number if true, -ive if false
    hFloat AABBInfrontOfPlane( const hAABB& a, const hPlane& p );

}

#endif //__HMPLANE_H__