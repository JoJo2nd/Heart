/********************************************************************
created:	2008/12/24

filename: 	AABB.h

author:		James Moran

purpose:	
*********************************************************************/
#ifndef __HMAABB_H__
#define __HMAABB_H__

#include "hMathUtil.h"
#include "hVector.h"
#include "hMatrix.h"


namespace Heart
{

	class hAABB
	{
	public:

        hVec3                       c_;
        hVec3                       r_;// half widths across x, y & z

		hFORCEINLINE hVec3			Max() const { return c_ + r_; }
		hFORCEINLINE hVec3			Min() const { return c_ - r_; }

	    hFORCEINLINE static hBool hAABB::PointWithinAABBSphere( const hVec3& point, const hAABB& a )
	    {
    //         hCPUVec3 r = a.r_;
    // 		hFloat d = r.x;
    // 		d = hMax( d, r.y );
    // 		d = hMax( d, r.z );
    // 
    // 		if ( hVec3Func::length( a.c_ - point ) < d  )
    // 			return true;
    // 		return false;

            if ( hVec128AllGreaterEqual( hVec3Func::length( a.c_ - point ), a.r_ ) )
                return hFalse;
            return hTrue;
	    }

	    hFORCEINLINE static hBool hAABB::intersect( const hAABB& a, const hAABB& b )
	    {
    // 		if ( abs( ((const hFloat*)&a.c)[ 0 ] - ((const hFloat*)&b.c)[ 0 ] )  > ( a.r[ 0 ] + b.r[ 0 ] ) )		return hFalse;
    // 		if ( abs( ((const hFloat*)&a.c)[ 1 ] - ((const hFloat*)&b.c)[ 1 ] )  > ( a.r[ 1 ] + b.r[ 1 ] ) )		return hFalse;
    // 		if ( abs( ((const hFloat*)&a.c)[ 2 ] - ((const hFloat*)&b.c)[ 2 ] )  > ( a.r[ 2 ] + b.r[ 2 ] ) )		return hFalse;
    // 
    // 		return hTrue;
            if ( hVec128AllLessEqual( hVec128Abs( ( a.c_ - b.c_ ) ), ( a.r_ + b.r_ ) ) )
                return hTrue;

            return hFalse;
	    }

	    hFORCEINLINE static hAABB hAABB::rotate( const hAABB& a, const hMatrix& m )
	    {
    //   		b.c.x = m.m11*a.c.x + m.m21*a.c.y + m.m31*a.c.z + m.m41;
    //   		b.c.y = m.m12*a.c.x + m.m22*a.c.y + m.m32*a.c.z + m.m42;
    //   		b.c.z = m.m13*a.c.x + m.m23*a.c.y + m.m33*a.c.z + m.m43;
    // 	  
    //   		b.r[0] = abs(m.m11)*a.r[0] + abs(m.m21)*a.r[1] + abs(m.m31)*a.r[2];
    //   		b.r[1] = abs(m.m12)*a.r[0] + abs(m.m22)*a.r[1] + abs(m.m32)*a.r[2];
    //   		b.r[2] = abs(m.m13)*a.r[0] + abs(m.m23)*a.r[1] + abs(m.m33)*a.r[2];
            
            hAABB r;
            r.c_ = hMatrixFunc::multNormal( a.c_, m );
            r.r_ = hMatrixFunc::multNormal( a.r_, m );
            return r;
	    }

	    hFORCEINLINE static hAABB hAABB::MatrixMult( const hAABB& a, const hMatrix& m )
	    {
            hAABB r;
		    r.c_ = hMatrixFunc::mult( a.c_, m );
            r.r_ = hMatrixFunc::multNormal( a.r_, m );
            return r;
	    }

	    hFORCEINLINE static hVec3 hAABB::closestPoint( const hAABB& a, const hVec3& vin )
	    {
    // 		for ( hUint32 i = 0; i < 3; ++i )
    // 		{
    // 			hFloat v = vin.v[ i ];
    // 			if ( v < ( a.c.v[ i ] - a.r[ i ] ) ) v = ( a.c.v[ i ] - a.r[ i ] );
    // 			if ( v > ( a.c.v[ i ] + a.r[ i ] ) ) v = ( a.c.v[ i ] - a.r[ i ] );
    // 			vout.v[ i ] = v;
    // 		}

            hVec3 tmp;
            tmp = hVec128Max( a.Min(), vin );
            return hVec128Min( a.Max(), tmp );
	    }

	    hFORCEINLINE static hAABB hAABB::computeFromPointSet( const hVec3* points, hUint32 nPoints )
	    {
    // 		hFloat MinX, MinY, MinZ;
    // 		hFloat MaxX, MaxY, MaxZ;
    // 
    // 		hcAssert( nPoints != 0 );
    // 
    // 		MinX = MaxX = pPoints[ 0 ].x;
    // 		MinY = MaxY = pPoints[ 0 ].y;
    // 		MinZ = MaxZ = pPoints[ 0 ].z;
    // 
    // 		for ( hUint32 i = 0; i < nPoints; ++i )
    // 		{
    // 			MinX = hMin( MinX, pPoints[ i ].x );
    // 			MaxX = hMax( MaxX, pPoints[ i ].x );
    // 			MinY = hMin( MinY, pPoints[ i ].y );
    // 			MaxY = hMax( MaxY, pPoints[ i ].y );
    // 			MinZ = hMin( MinZ, pPoints[ i ].z );
    // 			MaxZ = hMax( MaxZ, pPoints[ i ].z );
    // 		}
    // 
    // 		hVec3::set( MinX + ( (MaxX - MinX) / 2),
    // 			MinY + ( (MaxY - MinY) / 2 ),
    // 			MinZ + ( (MaxZ - MinZ) / 2 ),
    // 			out.c );
    // 		out.r[ 0 ] = (MaxX - MinX) / 2;
    // 		out.r[ 1 ] = (MaxY - MinY) / 2;
    // 		out.r[ 2 ] = (MaxZ - MinZ) / 2;
    // 		
            hVec3 vmin = points[0];
            hVec3 vmax = points[0];
         
            for ( hUint32 i = 0; i < nPoints; ++i )
            {
                vmin = hVec128Min( vmin, points[i] );
                vmax = hVec128Max( vmax, points[i] );
            }  

            hAABB r;
            r.r_ = ((vmin+vmax)*0.5f);
            r.c_ = vmin + r.r_;

            return r;
	    }

	    hFORCEINLINE static hAABB hAABB::expandBy( const hAABB& a, const hVec3& point )
	    {
    // 		hVec3 t[3];
    // 		t[ 2 ] = point;
    // 
    // 		t[ 0 ].x = aabb.c.x + aabb.r[ 0 ];
    // 		t[ 0 ].y = aabb.c.y + aabb.r[ 1 ];
    // 		t[ 0 ].z = aabb.c.z + aabb.r[ 2 ];
    // 
    // 		t[ 1 ].x = aabb.c.x - aabb.r[ 0 ];
    // 		t[ 1 ].y = aabb.c.y - aabb.r[ 1 ];
    // 		t[ 1 ].z = aabb.c.z - aabb.r[ 2 ];
    // 
    // 		computeFromPointSet( t, 3, aabb );
    
            hVec3 vmin = hVec128Min( a.Min(), point );
            hVec3 vmax = hVec128Max( a.Max(), point );
            
            hAABB r;
            r.r_ = ((vmin+vmax)*0.5f);
            r.c_ = vmin + r.r_;

            return r;
	    }

	    hFORCEINLINE static hAABB hAABB::ExpandBy( const hAABB& a, const hAABB& b )
	    {
    // 		//NOTE: this can be quicker!
    // 		hVec3 t[4];
    // 
    // 		t[ 0 ].x = a.c.x + a.r[ 0 ];
    // 		t[ 0 ].y = a.c.y + a.r[ 1 ];
    // 		t[ 0 ].z = a.c.z + a.r[ 2 ];
    // 
    // 		t[ 1 ].x = a.c.x - a.r[ 0 ];
    // 		t[ 1 ].y = a.c.y - a.r[ 1 ];
    // 		t[ 1 ].z = a.c.z - a.r[ 2 ];
    // 
    // 		t[ 2 ].x = b.c.x + b.r[ 0 ];
    // 		t[ 2 ].y = b.c.y + b.r[ 1 ];
    // 		t[ 2 ].z = b.c.z + b.r[ 2 ];
    // 
    // 		t[ 3 ].x = b.c.x - b.r[ 0 ];
    // 		t[ 3 ].y = b.c.y - b.r[ 1 ];
    // 		t[ 3 ].z = b.c.z - b.r[ 2 ];
    // 
    // 		computeFromPointSet( t, 4, a );
    // 		
            hVec3 vmin = hVec128Min( a.Min(), a.Min() );
            hVec3 vmax = hVec128Max( a.Max(), a.Max() );

            hAABB r;
            r.r_ = ((vmin+vmax)*0.5f);
            r.c_ = vmin + r.r_;

            return r;
	    }

	    hFORCEINLINE hBool hAABB::liesWithinA( const hAABB& a, const hAABB& b )
	    {
    // 		hVec3 p1, p2;
    // 
    // 		// for each point in the 2nd aabb, test to see if the points lies within the first aabb [12/31/2008 James]
    // 		// is one doesn't then we exit early [12/31/2008 James]
    // 		hVec3::set( b.c.x + b.r[ 0 ], b.c.y + b.r[ 1 ], b.c.z + b.r[ 2 ], p1 );
    // 		closestPoint( a, p1, p2 );
    // 		if ( p1 == p2 ) return hFalse;
    // 
    // 		hVec3::set( b.c.x + b.r[ 0 ], b.c.y + b.r[ 1 ], b.c.z - b.r[ 2 ], p1 );
    // 		closestPoint( a, p1, p2 );
    // 		if ( p1 == p2 ) return hFalse;
    // 
    // 		hVec3::set( b.c.x + b.r[ 0 ], b.c.y - b.r[ 1 ], b.c.z + b.r[ 2 ], p1 );
    // 		closestPoint( a, p1, p2 );
    // 		if ( p1 == p2 ) return hFalse;
    // 
    // 		hVec3::set( b.c.x + b.r[ 0 ], b.c.y - b.r[ 1 ], b.c.z - b.r[ 2 ], p1 );
    // 		closestPoint( a, p1, p2 );
    // 		if ( p1 == p2 ) return hFalse;
    // 
    // 		hVec3::set( b.c.x - b.r[ 0 ], b.c.y + b.r[ 1 ], b.c.z + b.r[ 2 ], p1 );
    // 		closestPoint( a, p1, p2 );
    // 		if ( p1 == p2 ) return hFalse;
    // 
    // 		hVec3::set( b.c.x - b.r[ 0 ], b.c.y + b.r[ 1 ], b.c.z - b.r[ 2 ], p1 );
    // 		closestPoint( a, p1, p2 );
    // 		if ( p1 == p2 ) return hFalse;
    // 
    // 		hVec3::set( b.c.x - b.r[ 0 ], b.c.y - b.r[ 1 ], b.c.z + b.r[ 2 ], p1 );
    // 		closestPoint( a, p1, p2 );
    // 		if ( p1 == p2 ) return hFalse;
    // 
    // 		hVec3::set( b.c.x - b.r[ 0 ], b.c.y - b.r[ 1 ], b.c.z - b.r[ 2 ], p1 );
    // 		closestPoint( a, p1, p2 );
    // 		if ( p1 == p2 ) return hFalse;
    // 
    // 		return hTrue;
	    }


	    // intersects Ray (p + d) with AABB a
	    // return hTrue on intersection else hFalse
	    // returns tmin as distance to intersect on AABB
	    // returns q as the point of intersect on the AABB
	    hFORCEINLINE hBool hAABB::intersectRayAABB( const hVec3& p, const hVec3& d, const hAABB& a, hFloat& tmin, hVec3& q )
	    {
    // 		tmin = 0;
    // 		hFloat tmax = Limits::FLOAT_MAX;
    // 		register hVec3 min;
    // 		register hVec3 max;
    // 
    // 		hVec3::set( a.c.v[ 0 ] - a.r[ 0 ], a.c.v[ 1 ] - a.r[ 1 ], a.c.v[ 2 ] - a.r[ 2 ], min );
    // 		hVec3::set( a.c.v[ 0 ] + a.r[ 0 ], a.c.v[ 1 ] + a.r[ 1 ], a.c.v[ 2 ] + a.r[ 2 ], max );
    // 
    // 
    // 		//	treat the aabb a 3 slabs across each axis
    // 		for ( hUint32 i = 0; i < 3; ++i )
    // 		{
    // 			if ( abs( d.v[ i ] ) < Limits::FLOAT_EPSILON )
    // 			{
    // 				//	Ray is parallel to a slab. check the start of the ray lies within the 
    // 				// slab else it can never intersect!
    // 				if ( p.v[ i ] < min.v[ i ] || p.v[ i ] > max.v[ i ] )
    // 				{
    // 					return hFalse;
    // 				}
    // 			}
    // 			else
    // 			{
    // 				// compute intersection t value of ray with near and  far plane of slab [3/1/2009 James]
    // 				hFloat ood = 1.0f / d.v[ i ];
    // 				hFloat t1 = ( min.v[ i ] - p.v[ i ] ) * ood;
    // 				hFloat t2 = ( max.v[ i ] - p.v[ i ] ) * ood;
    // 				// Make t1 be intersection with near plane, t2 with far plane [3/1/2009 James]
    // 				if ( t1  > t2 ) 
    // 				{
    // 					hmSwap< hFloat >( t1, t2 );
    // 				}
    // 
    // 				// Compute the intersection of slab intersection intervals [3/1/2009 James]
    // 				if ( t1 > tmin ) 
    // 					tmin = t1;
    // 				if ( t2 < tmax ) 
    // 					tmax = t2;
    // 				// exit with no collision as soon as slab intersection becomes empty [3/1/2009 James]
    // 				if ( tmin > tmax )
    // 					return 0;
    // 
    // 			}
    // 		}
    // 
    // 		// Ray intersect all 3 slab, Return point (q) and intersecitno t value [3/1/2009 James]
    // 		q = p + ( d * tmin );
    // 		return hTrue;
	    }

	    // Intersect AABBs ‘a’ and ‘b’ moving with constant velocities va and vb.
	    // On intersection, return time of first and last contact in tfirst and tlast
	    hFORCEINLINE static hBool hAABB::intersectMovingAABB( const hAABB& a, const hAABB& b, const hVec3& va, const hVec3& vb, hFloat& tfirst, hFloat &tlast )
	    {
    // 		// Exit early if ‘a’ and ‘b’ initially overlapping
    // 		if ( intersect( a, b ) ) 
    // 		{
    // 			tfirst = tlast = 0.0f;
    // 			return hTrue;
    // 		}
    // 
    // 		// Use relative velocity; effectively treating 'a' as stationary
    // 		hVec3 v = vb - va;
    // 
    // 		// Initialize times of first and last contact
    // 		tfirst = 0.0f;
    // 		tlast = 1.0f;
    // 
    // 		// For each axis, determine times of first and last contact, if any
    // 		for (int i = 0; i < 3; i++) 
    // 		{
    // 			if ( v.v[i] < 0.0f ) 
    // 			{
    // 				if ( b.Max(i) < a.Min(i) ) return hFalse; // Nonintersecting and moving apart
    // 				if ( a.Max(i) < b.Min(i) ) tfirst = hMax((a.Max(i) - b.Min(i)) / v.v[i], tfirst);
    // 				if ( b.Max(i) > a.Min(i) ) tlast  = hMin((a.Min(i) - b.Max(i)) / v.v[i], tlast);
    // 			}
    // 			if ( v.v[i] > 0.0f ) 
    // 			{
    // 				if ( b.Min(i) > a.Max(i) ) return hFalse; // Nonintersecting and moving apart
    // 				if ( b.Max(i) < a.Min(i) ) tfirst = hMax( (a.Min(i) - b.Max(i)) / v.v[i], tfirst);
    // 				if ( a.Max(i) > b.Min(i) ) tlast = hMin( (a.Max(i) - b.Min(i)) / v.v[i], tlast);
    // 			}
    // 
    // 			// No overlap possible if time of first contact occurs after time of last contact
    // 			if (tfirst > tlast) return hFalse;
    // 		}
    // 
    // 		return hTrue;

            if ( intersect( a, b ) )
            {
                tfirst = tlast = 0.0f;
                return hTrue;
            }

            // Use relative velocity; effectively treating 'a' as stationary
            hVec3 vVec = vb - va;
            hVec3 aMinVec = a.Min();
            hVec3 aMaxVec = a.Max();
            hVec3 bMinVec = b.Min();
            hVec3 bMaxVec = b.Max();

            hFloat* v = (hFloat*)&vVec;
            hFloat* aMin = (hFloat*)&aMinVec;
            hFloat* aMax = (hFloat*)&aMaxVec;
            hFloat* bMin = (hFloat*)&bMinVec;
            hFloat* bMax = (hFloat*)&bMaxVec;

            // Initialize times of first and last contact
            tfirst = 0.0f;
            tlast = 1.0f;

            // For each axis, determine times of first and last contact, if any
            for (int i = 0; i < 3; i++) 
            {
	            if ( v[i] < 0.0f ) 
	            {
		            if ( bMax[i] < aMin[i] ) return hFalse; // Nonintersecting and moving apart
		            if ( aMax[i] < bMin[i] ) tfirst = hMax((aMax[i] - bMin[i]) / v[i], tfirst);
		            if ( bMax[i] > aMin[i] ) tlast  = hMin((aMin[i] - bMax[i]) / v[i], tlast);
	            }
	            if ( v[i] > 0.0f ) 
	            {
		            if ( bMin[i] > aMax[i] ) return hFalse; // Nonintersecting and moving apart
		            if ( bMax[i] < aMin[i] ) tfirst = hMax( (aMin[i] - bMax[i]) / v[i], tfirst);
		            if ( aMax[i] > bMin[i] ) tlast = hMin( (aMax[i] - bMin[i]) / v[i], tlast);
	            }

	            // No overlap possible if time of first contact occurs after time of last contact
	            if (tfirst > tlast) return hFalse;
            }
	    }

    };
}

#endif //__HMAABB_H__