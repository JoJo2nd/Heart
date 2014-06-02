/********************************************************************

	filename: 	hPlane.h	
	
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