/********************************************************************

	filename: 	hViewFrustum.cpp	
	
	Copyright (c) 30:12:2011 James Moran
	
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

namespace Heart
{

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hViewFrustum::UpdateFromCamera( const hVec3& EyeVec, const hVec3& LookDir, const hVec3& Up, hFloat fov, hFloat Aspect, hFloat Near, hFloat Far, hBool ortho )
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
		hVec3 Right;
		hVec3 D = normalize( LookDir );
		hVec3 EyeFar = EyeVec + ( D * Far );
		hVec3 EyeNear = EyeVec + ( D * Near );
		Right = cross( Up, D );

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
			near_   = buildPlane( EyeNear, vex[ ntr ], vex[ ntl ] );
			far_    = buildPlane( EyeFar, vex[ ftl ], vex[ ftr ] );
			top_    = buildPlane( vex[ ntr ], vex[ ftr ], vex[ ftl ] );
			bottom_ = buildPlane( vex[ nbl ], vex[ fbl ], vex[ fbr ] );
			right_  = buildPlane( vex[ nbr ], vex[ fbr ], vex[ ftr ] );
			left_   = buildPlane( vex[ ntl ], vex[ ftl ], vex[ fbl ] );
		}
		else
		{
			near_   = buildPlane( EyeNear, vex[ ntr ], vex[ ntl ] );
			far_    = buildPlane( EyeFar, vex[ ftl ], vex[ ftr ] );
			top_    = buildPlane( EyeVec, vex[ ftr ], vex[ ftl ] );
			bottom_ = buildPlane( EyeVec, vex[ fbl ], vex[ fbr ] );
			right_  = buildPlane( EyeVec, vex[ fbr ], vex[ ftr ] );
			left_   = buildPlane( EyeVec, vex[ ftl ], vex[ fbl ] );
		}

        viewFrustumAABB_ = hAABB::computeFromPointSet( vex, 8 );

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
			hVec3 vex[ 8 ];

			//hVec3Func::set( AABB.c.x - AABB.r[ 0 ], AABB.c.y - AABB.r[ 1 ], AABB.c.z - AABB.r[ 2 ] , vex[ 0 ] );//bottem left near
            vex[0] = AABB.c_ - AABB.r_;
			//hVec3Func::set( AABB.c.x + AABB.r[ 0 ], AABB.c.y - AABB.r[ 1 ], AABB.c.z - AABB.r[ 2 ] , vex[ 1 ] );//bottem right near
            vex[1] = AABB.c_ + mulPerElem( AABB.r_, hVec3( 1.f, -1.f, -1.f ) );
			//hVec3Func::set( AABB.c.x + AABB.r[ 0 ], AABB.c.y - AABB.r[ 1 ], AABB.c.z + AABB.r[ 2 ] , vex[ 2 ] );//bottem right far
            vex[2] = AABB.c_ + mulPerElem( AABB.r_, hVec3( 1.f, -1.f, 1.f ) );
			//hVec3Func::set( AABB.c.x - AABB.r[ 0 ], AABB.c.y - AABB.r[ 1 ], AABB.c.z + AABB.r[ 2 ] , vex[ 3 ] );//bottem left far
            vex[3] = AABB.c_ + mulPerElem( AABB.r_, hVec3( -1.f, -1.f, +1.f ) );
			//hVec3Func::set( AABB.c.x - AABB.r[ 0 ], AABB.c.y + AABB.r[ 1 ], AABB.c.z - AABB.r[ 2 ] , vex[ 4 ] );//top left near
            vex[4] = AABB.c_ + mulPerElem( AABB.r_, hVec3( -1.f, 1.f, -1.f ) );
			//hVec3Func::set( AABB.c.x + AABB.r[ 0 ], AABB.c.y + AABB.r[ 1 ], AABB.c.z - AABB.r[ 2 ] , vex[ 5 ] );//top right near
            vex[5] = AABB.c_ + mulPerElem( AABB.r_, hVec3( 1.f, 1.f, -1.f ) );
			//hVec3Func::set( AABB.c.x + AABB.r[ 0 ], AABB.c.y + AABB.r[ 1 ], AABB.c.z + AABB.r[ 2 ] , vex[ 6 ] );//top right far
            vex[6] = AABB.c_ + AABB.r_;
			//hVec3Func::set( AABB.c.x - AABB.r[ 0 ], AABB.c.y + AABB.r[ 1 ], AABB.c.z + AABB.r[ 2 ] , vex[ 7 ] );//top left far
            vex[7] = AABB.c_ + mulPerElem( AABB.r_, hVec3( -1.f, 1.f, 1.f ) );

			hPlane* planes[ 5 ] = {&top_, &bottom_, &left_, &right_, &far_ };
			for ( hUint32 p = 0; p < 5; ++p )
			{
				hPlane* pPlane = planes[ p ];
				hBool fullyBehindPlane = hTrue;
				for ( hUint32 i = 0; i < 8; ++i )
				{
					if ( !pointBehindPlane( vex[ i ], *pPlane ) )
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

        if ( hAABB::intersectMovingAABB( viewFrustumAABB_, aabb, hVec3(0.f, 0.f, 0.f), dir, f, l ) )
		{
 			for ( hUint32 p = 0; p < 6; ++p )
 			{
 				hPlane* pPlane = planes[ p ];
 				if ( hPlaneFunc::intersectMovingAABB( aabb, dir, *pPlane ) )
 				{
 					return hTrue;
 				}
				inside &= hPlaneFunc::AABBInfrontOfPlane( aabb, *pPlane ) > 0.0f;
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