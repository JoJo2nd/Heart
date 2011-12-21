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

		enum
		{
			hwX = 0,
			hwY = 1,
			hwZ = 2,
		};

		hVec3						c;
		hFloat						r[ 3 ];// half widths across x, y & z [12/24/2008 James]

		inline hFloat				Max( hUint32 a ) const { return c.v[ a ] + r[ a ]; }
		inline hFloat				Min( hUint32 a ) const { return c.v[ a ] - r[ a ]; }

		static hBool 				intersect( const hAABB& a, const hAABB& b );
		static void	 				closestPoint( const hAABB& a, const hVec3& vin, hVec3& vout );
		static hBool 				liesWithinA( const hAABB& a, const hAABB& b );
		static void	 				rotate( hAABB a, const hMatrix& m, hAABB& b );
		static void					MatrixMult( hAABB a, const hMatrix& m, hAABB& b );
		static void					expandBy( const hVec3& point, hAABB& aabb );
		static void					ExpandBy( hAABB& a, const hAABB& b );
		static void  				computeFromPointSet( const hVec3* pPoints, hUint32 nPoints, hAABB& out );
		static hBool 				intersectRayAABB( const hVec3& p, const hVec3& d, const hAABB& a, hFloat& tmin, hVec3& q );
		static hBool				intersectMovingAABB( const hAABB& a, const hAABB& b, const hVec3& va, const hVec3& vb, hFloat& tfirst, hFloat& tlast );
		static hBool				PointWithinAABBSphere( const hVec3& point, const hAABB& a );
	};

	inline hBool hAABB::PointWithinAABBSphere( const hVec3& point, const hAABB& a )
	{
		hFloat d = a.r[0];
		d = hMax( d, a.r[1] );
		d = hMax( d, a.r[2] );

		if ( hVec3::mag( a.c - point ) < d  )
			return true;
		return false;
	}

	inline hBool hAABB::intersect( const hAABB& a, const hAABB& b )
	{
		if ( abs( a.c.v[ 0 ] - b.c.v[ 0 ] )  > ( a.r[ 0 ] + b.r[ 0 ] ) )		return hFalse;
		if ( abs( a.c.v[ 1 ] - b.c.v[ 1 ] )  > ( a.r[ 1 ] + b.r[ 1 ] ) )		return hFalse;
		if ( abs( a.c.v[ 2 ] - b.c.v[ 2 ] )  > ( a.r[ 2 ] + b.r[ 2 ] ) )		return hFalse;

		return hTrue;
	}

	inline void hAABB::rotate( hAABB a, const hMatrix& m, hAABB& b )
	{
  		b.c.x = m.m11*a.c.x + m.m21*a.c.y + m.m31*a.c.z + m.m41;
  		b.c.y = m.m12*a.c.x + m.m22*a.c.y + m.m32*a.c.z + m.m42;
  		b.c.z = m.m13*a.c.x + m.m23*a.c.y + m.m33*a.c.z + m.m43;
	  
  		b.r[ 0 ] = abs(m.m11)*a.r[0] + abs(m.m21)*a.r[1] + abs(m.m31)*a.r[2];
  		b.r[ 1 ] = abs(m.m12)*a.r[0] + abs(m.m22)*a.r[1] + abs(m.m32)*a.r[2];
  		b.r[ 2 ] = abs(m.m13)*a.r[0] + abs(m.m23)*a.r[1] + abs(m.m33)*a.r[2];
	}

	inline void hAABB::MatrixMult( hAABB a, const hMatrix& m, hAABB& b )
	{
		
	}

	inline void hAABB::closestPoint( const hAABB& a, const hVec3& vin, hVec3& vout )
	{
		for ( hUint32 i = 0; i < 3; ++i )
		{
			hFloat v = vin.v[ i ];
			if ( v < ( a.c.v[ i ] - a.r[ i ] ) ) v = ( a.c.v[ i ] - a.r[ i ] );
			if ( v > ( a.c.v[ i ] + a.r[ i ] ) ) v = ( a.c.v[ i ] - a.r[ i ] );
			vout.v[ i ] = v;
		}
	}

	inline void hAABB::computeFromPointSet( const hVec3* pPoints, hUint32 nPoints, hAABB& out )
	{
		hFloat MinX, MinY, MinZ;
		hFloat MaxX, MaxY, MaxZ;

		hcAssert( nPoints != 0 );
		if ( nPoints == 0 )
		{
			return;
		}

		MinX = MaxX = pPoints[ 0 ].x;
		MinY = MaxY = pPoints[ 0 ].y;
		MinZ = MaxZ = pPoints[ 0 ].z;

		for ( hUint32 i = 0; i < nPoints; ++i )
		{
			MinX = hMin( MinX, pPoints[ i ].x );
			MaxX = hMax( MaxX, pPoints[ i ].x );
			MinY = hMin( MinY, pPoints[ i ].y );
			MaxY = hMax( MaxY, pPoints[ i ].y );
			MinZ = hMin( MinZ, pPoints[ i ].z );
			MaxZ = hMax( MaxZ, pPoints[ i ].z );
		}

		hVec3::set( MinX + ( (MaxX - MinX) / 2),
			MinY + ( (MaxY - MinY) / 2 ),
			MinZ + ( (MaxZ - MinZ) / 2 ),
			out.c );
		out.r[ 0 ] = (MaxX - MinX) / 2;
		out.r[ 1 ] = (MaxY - MinY) / 2;
		out.r[ 2 ] = (MaxZ - MinZ) / 2;

	}

	inline void hAABB::expandBy( const hVec3& point, hAABB& aabb )
	{
		hVec3 t[3];
		t[ 2 ] = point;

		t[ 0 ].x = aabb.c.x + aabb.r[ 0 ];
		t[ 0 ].y = aabb.c.y + aabb.r[ 1 ];
		t[ 0 ].z = aabb.c.z + aabb.r[ 2 ];

		t[ 1 ].x = aabb.c.x - aabb.r[ 0 ];
		t[ 1 ].y = aabb.c.y - aabb.r[ 1 ];
		t[ 1 ].z = aabb.c.z - aabb.r[ 2 ];

		computeFromPointSet( t, 3, aabb );
	}

	inline void hAABB::ExpandBy( hAABB& a, const hAABB& b )
	{
		//NOTE: this can be quicker!
		hVec3 t[4];

		t[ 0 ].x = a.c.x + a.r[ 0 ];
		t[ 0 ].y = a.c.y + a.r[ 1 ];
		t[ 0 ].z = a.c.z + a.r[ 2 ];

		t[ 1 ].x = a.c.x - a.r[ 0 ];
		t[ 1 ].y = a.c.y - a.r[ 1 ];
		t[ 1 ].z = a.c.z - a.r[ 2 ];

		t[ 2 ].x = b.c.x + b.r[ 0 ];
		t[ 2 ].y = b.c.y + b.r[ 1 ];
		t[ 2 ].z = b.c.z + b.r[ 2 ];

		t[ 3 ].x = b.c.x - b.r[ 0 ];
		t[ 3 ].y = b.c.y - b.r[ 1 ];
		t[ 3 ].z = b.c.z - b.r[ 2 ];

		computeFromPointSet( t, 4, a );
	}

	inline hBool hAABB::liesWithinA( const hAABB& a, const hAABB& b )
	{
		hVec3 p1, p2;

		// for each point in the 2nd aabb, test to see if the points lies within the first aabb [12/31/2008 James]
		// is one doesn't then we exit early [12/31/2008 James]
		hVec3::set( b.c.x + b.r[ 0 ], b.c.y + b.r[ 1 ], b.c.z + b.r[ 2 ], p1 );
		closestPoint( a, p1, p2 );
		if ( p1 == p2 ) return hFalse;

		hVec3::set( b.c.x + b.r[ 0 ], b.c.y + b.r[ 1 ], b.c.z - b.r[ 2 ], p1 );
		closestPoint( a, p1, p2 );
		if ( p1 == p2 ) return hFalse;

		hVec3::set( b.c.x + b.r[ 0 ], b.c.y - b.r[ 1 ], b.c.z + b.r[ 2 ], p1 );
		closestPoint( a, p1, p2 );
		if ( p1 == p2 ) return hFalse;

		hVec3::set( b.c.x + b.r[ 0 ], b.c.y - b.r[ 1 ], b.c.z - b.r[ 2 ], p1 );
		closestPoint( a, p1, p2 );
		if ( p1 == p2 ) return hFalse;

		hVec3::set( b.c.x - b.r[ 0 ], b.c.y + b.r[ 1 ], b.c.z + b.r[ 2 ], p1 );
		closestPoint( a, p1, p2 );
		if ( p1 == p2 ) return hFalse;

		hVec3::set( b.c.x - b.r[ 0 ], b.c.y + b.r[ 1 ], b.c.z - b.r[ 2 ], p1 );
		closestPoint( a, p1, p2 );
		if ( p1 == p2 ) return hFalse;

		hVec3::set( b.c.x - b.r[ 0 ], b.c.y - b.r[ 1 ], b.c.z + b.r[ 2 ], p1 );
		closestPoint( a, p1, p2 );
		if ( p1 == p2 ) return hFalse;

		hVec3::set( b.c.x - b.r[ 0 ], b.c.y - b.r[ 1 ], b.c.z - b.r[ 2 ], p1 );
		closestPoint( a, p1, p2 );
		if ( p1 == p2 ) return hFalse;

		return hTrue;
	}

	/*/////////////////////////////////////////////////////////////////////////

	function: intersectRayAABB - 

	purpose: intersects Ray (p + d) with AABB a
	return hTrue on intersection else hFalse
	returns tmin as distance to intersect on AABB
	returns q as the point of intersect on the AABB

	author: James Moran

	/////////////////////////////////////////////////////////////////////////*/
	inline hBool hAABB::intersectRayAABB( const hVec3& p, const hVec3& d, const hAABB& a, hFloat& tmin, hVec3& q )
	{
		tmin = 0;
		hFloat tmax = Limits::FLOAT_MAX;
		register hVec3 min;
		register hVec3 max;

		hVec3::set( a.c.v[ 0 ] - a.r[ 0 ], a.c.v[ 1 ] - a.r[ 1 ], a.c.v[ 2 ] - a.r[ 2 ], min );
		hVec3::set( a.c.v[ 0 ] + a.r[ 0 ], a.c.v[ 1 ] + a.r[ 1 ], a.c.v[ 2 ] + a.r[ 2 ], max );


		//	treat the aabb a 3 slabs across each axis
		for ( hUint32 i = 0; i < 3; ++i )
		{
			if ( abs( d.v[ i ] ) < Limits::FLOAT_EPSILON )
			{
				//	Ray is parallel to a slab. check the start of the ray lies within the 
				// slab else it can never intersect!
				if ( p.v[ i ] < min.v[ i ] || p.v[ i ] > max.v[ i ] )
				{
					return hFalse;
				}
			}
			else
			{
				// compute intersection t value of ray with near and  far plane of slab [3/1/2009 James]
				hFloat ood = 1.0f / d.v[ i ];
				hFloat t1 = ( min.v[ i ] - p.v[ i ] ) * ood;
				hFloat t2 = ( max.v[ i ] - p.v[ i ] ) * ood;
				// Make t1 be intersection with near plane, t2 with far plane [3/1/2009 James]
				if ( t1  > t2 ) 
				{
					hmSwap< hFloat >( t1, t2 );
				}

				// Compute the intersection of slab intersection intervals [3/1/2009 James]
				if ( t1 > tmin ) 
					tmin = t1;
				if ( t2 < tmax ) 
					tmax = t2;
				// exit with no collision as soon as slab intersection becomes empty [3/1/2009 James]
				if ( tmin > tmax )
					return 0;

			}
		}

		// Ray intersect all 3 slab, Return point (q) and intersecitno t value [3/1/2009 James]
		q = p + ( d * tmin );
		return hTrue;
	}

	// Intersect AABBs ‘a’ and ‘b’ moving with constant velocities va and vb.
	// On intersection, return time of first and last contact in tfirst and tlast
	inline hBool hAABB::intersectMovingAABB( const hAABB& a, const hAABB& b, const hVec3& va, const hVec3& vb, hFloat& tfirst, hFloat &tlast )
	{
		// Exit early if ‘a’ and ‘b’ initially overlapping
		if ( intersect( a, b ) ) 
		{
			tfirst = tlast = 0.0f;
			return hTrue;
		}

		// Use relative velocity; effectively treating 'a' as stationary
		hVec3 v = vb - va;

		// Initialize times of first and last contact
		tfirst = 0.0f;
		tlast = 1.0f;

		// For each axis, determine times of first and last contact, if any
		for (int i = 0; i < 3; i++) 
		{
			if ( v.v[i] < 0.0f ) 
			{
				if ( b.Max(i) < a.Min(i) ) return hFalse; // Nonintersecting and moving apart
				if ( a.Max(i) < b.Min(i) ) tfirst = hMax((a.Max(i) - b.Min(i)) / v.v[i], tfirst);
				if ( b.Max(i) > a.Min(i) ) tlast  = hMin((a.Min(i) - b.Max(i)) / v.v[i], tlast);
			}
			if ( v.v[i] > 0.0f ) 
			{
				if ( b.Min(i) > a.Max(i) ) return hFalse; // Nonintersecting and moving apart
				if ( b.Max(i) < a.Min(i) ) tfirst = hMax( (a.Min(i) - b.Max(i)) / v.v[i], tfirst);
				if ( a.Max(i) > b.Min(i) ) tlast = hMin( (a.Max(i) - b.Min(i)) / v.v[i], tlast);
			}

			// No overlap possible if time of first contact occurs after time of last contact
			if (tfirst > tlast) return hFalse;
		}

		return hTrue;
	}

}

#endif //__HMAABB_H__