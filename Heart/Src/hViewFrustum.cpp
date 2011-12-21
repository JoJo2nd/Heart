/********************************************************************
	created:	2009/01/06

	filename: 	ViewFrustum.cpp

	author:		James Moran
	
	purpose:	
*********************************************************************/

#include "Common.h"
#include "hViewFrustum.h"

namespace Heart
{

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hViewFrustum::UpdateFromCamera( Heart::hVec3 EyeVec, Heart::hVec3 LookAt, Heart::hVec3 Up, hFloat fov, hFloat Aspect, hFloat Near, hFloat Far, hBool ortho )
	{
		hFloat oNear = Near;
		if ( ortho )
		{
			oNear = Far;
		}

		hFloat FarH = 2 * tan( fov / 2 ) * Far;
		hFloat FarW = FarH * Aspect;
		hFloat NearH = 2 * tan( fov / 2 ) * oNear;
		hFloat NearW = NearH * Aspect;
		Heart::hVec3 Right;
		Heart::hVec3 D;
		Heart::hVec3::normalise( LookAt - EyeVec, D );
		Heart::hVec3 EyeFar = EyeVec + ( D * Far );
		Heart::hVec3 EyeNear = EyeVec + ( D * Near );
		Heart::hVec3::cross( Up, D, Right );

		// re-generate our 6 planes [1/8/2009 James]
	#define ftl 0
	#define ftr 1
	#define fbl 2
	#define fbr 3
	#define ntl 4 
	#define ntr 5
	#define nbl 6 
	#define nbr 7

		Heart::hVec3 vex[ 8 ];
		hFloat FarW2 = FarW / 2;
		hFloat FarH2 = FarH / 2;
		hFloat NearH2 = NearH / 2;
		hFloat NearW2 = NearW / 2;

		vex[ ftl ] = EyeFar + ( Up * FarH2 ) - ( Right * FarW2 );
		vex[ ftr ] = EyeFar + ( Up * FarH2 ) + ( Right * FarW2 );
		vex[ fbl ] = EyeFar - ( Up * FarH2 ) - ( Right * FarW2 );
		vex[ fbr ] = EyeFar - ( Up * FarH2 ) + ( Right * FarW2 );

		vex[ ntl ] = EyeNear + ( Up * NearH2 ) - ( Right * NearW2 );
		vex[ ntr ] = EyeNear + ( Up * NearH2 ) + ( Right * NearW2 );
		vex[ nbl ] = EyeNear - ( Up * NearH2 ) - ( Right * NearW2 );
		vex[ nbr ] = EyeNear - ( Up * NearH2 ) + ( Right * NearW2 );

		for ( hUint32 i = 0; i < 8; ++i )
		{
			frustumPoints_[ i ] = vex[ i ];
		}

		//
		if ( ortho )
		{
			Heart::hPlane::buildPlane( EyeNear, vex[ ntr ], vex[ ntl ], near_ );
			Heart::hPlane::buildPlane( EyeFar, vex[ ftl ], vex[ ftr ], far_ );
			Heart::hPlane::buildPlane( vex[ ntr ], vex[ ftr ], vex[ ftl ], top_ );
			Heart::hPlane::buildPlane( vex[ nbl ], vex[ fbl ], vex[ fbr ], bottom_ );
			Heart::hPlane::buildPlane( vex[ nbr ], vex[ fbr ], vex[ ftr ], right_ );
			Heart::hPlane::buildPlane( vex[ ntl ], vex[ ftl ], vex[ fbl ], left_ );
		}
		else
		{
			Heart::hPlane::buildPlane( EyeNear, vex[ ntr ], vex[ ntl ], near_ );
			Heart::hPlane::buildPlane( EyeFar, vex[ ftl ], vex[ ftr ], far_ );
			Heart::hPlane::buildPlane( EyeVec, vex[ ftr ], vex[ ftl ], top_ );
			Heart::hPlane::buildPlane( EyeVec, vex[ fbl ], vex[ fbr ], bottom_ );
			Heart::hPlane::buildPlane( EyeVec, vex[ fbr ], vex[ ftr ], right_ );
			Heart::hPlane::buildPlane( EyeVec, vex[ ftl ], vex[ fbl ], left_ );
		}

		Heart::hAABB::computeFromPointSet( vex, 8, viewFrustumAABB_ );

	#undef ftl
	#undef ftr
	#undef fbl
	#undef fbr
	#undef ntl
	#undef ntr
	#undef nbl
	#undef nbr

	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	hBool hViewFrustum::TestAABB( const Heart::hAABB& AABB )
	{
		if ( Heart::hAABB::intersect( AABB, viewFrustumAABB_ ) )
		{
			//do culling test against planes
			Heart::hVec3 vex[ 8 ];

			Heart::hVec3::set( AABB.c.x - AABB.r[ 0 ], AABB.c.y - AABB.r[ 1 ], AABB.c.z - AABB.r[ 2 ] , vex[ 0 ] );//bottem left near
			Heart::hVec3::set( AABB.c.x + AABB.r[ 0 ], AABB.c.y - AABB.r[ 1 ], AABB.c.z - AABB.r[ 2 ] , vex[ 1 ] );//bottem right near
			Heart::hVec3::set( AABB.c.x + AABB.r[ 0 ], AABB.c.y - AABB.r[ 1 ], AABB.c.z + AABB.r[ 2 ] , vex[ 2 ] );//bottem right far
			Heart::hVec3::set( AABB.c.x - AABB.r[ 0 ], AABB.c.y - AABB.r[ 1 ], AABB.c.z + AABB.r[ 2 ] , vex[ 3 ] );//bottem left far
			Heart::hVec3::set( AABB.c.x - AABB.r[ 0 ], AABB.c.y + AABB.r[ 1 ], AABB.c.z - AABB.r[ 2 ] , vex[ 4 ] );//top left near
			Heart::hVec3::set( AABB.c.x + AABB.r[ 0 ], AABB.c.y + AABB.r[ 1 ], AABB.c.z - AABB.r[ 2 ] , vex[ 5 ] );//top right near
			Heart::hVec3::set( AABB.c.x + AABB.r[ 0 ], AABB.c.y + AABB.r[ 1 ], AABB.c.z + AABB.r[ 2 ] , vex[ 6 ] );//top right far
			Heart::hVec3::set( AABB.c.x - AABB.r[ 0 ], AABB.c.y + AABB.r[ 1 ], AABB.c.z + AABB.r[ 2 ] , vex[ 7 ] );//top left far

			Heart::hPlane* planes[ 5 ] = {&top_, &bottom_, &left_, &right_, &far_ };
			for ( hUint32 p = 0; p < 5; ++p )
			{
				Heart::hPlane* pPlane = planes[ p ];
				hBool fullyBehindPlane = hTrue;
				for ( hUint32 i = 0; i < 8; ++i )
				{
					if ( !Heart::hPlane::pointBehindPlane( vex[ i ], *pPlane ) )
					{
						fullyBehindPlane = hFalse;
						break;
					}
				}

				if ( fullyBehindPlane )
				{
					return hFalse;
				}
			}

			//is visible
			return hTrue;
		}

		return hFalse;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	hBool hViewFrustum::TestMovingAABB( const Heart::hAABB& aabb, const Heart::hVec3& dir )
	{
		Heart::hPlane* planes[ 6 ] = { &near_, &far_, &top_, &bottom_, &left_, &right_ };
		hFloat f,l;
		hBool inside = hTrue;

		if ( Heart::hAABB::intersectMovingAABB( viewFrustumAABB_, aabb, Heart::ZeroVector3, dir, f, l ) )
		{
 			for ( hUint32 p = 0; p < 6; ++p )
 			{
 				Heart::hPlane* pPlane = planes[ p ];
 				if ( Heart::hPlane::intersectMovingAABB( aabb, dir, *pPlane ) )
 				{
 					return hTrue;
 				}
				inside &= Heart::hPlane::AABBInfrontOfPlane( aabb, *pPlane ) > 0.0f;
 			}
		}

		return inside;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	hViewFrustum::~hViewFrustum()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	hViewFrustum::hViewFrustum()
	{

	}

}