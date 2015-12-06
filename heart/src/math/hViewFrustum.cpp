/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#include "math/hViewFrustum.h"
#include "math/hPlane.h"

namespace Heart
{

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hViewFrustum::updateFromCamera( const hVec3& EyeVec, const hVec3& LookDir, const hVec3& Up, hFloat fov, hFloat Aspect, hFloat Near, hFloat Far) {
		hFloat oNear = Near;

		hFloat FarH = 2.f * tanf( fov / 2.f ) * Far;
		hFloat FarW = FarH * Aspect;
		hFloat NearH = 2.f * tanf( fov / 2.f ) * oNear;
		hFloat NearW = NearH * Aspect;
		hVec3 Right;
		hVec3 D = normalize( LookDir );
		hVec3 EyeFar = EyeVec + ( D * Far );
		hVec3 EyeNear = EyeVec + ( D * Near );
		Right = cross( Up, D );

		// re-generate our 6 planes
#define ftl ((hUint)Point::FarTopLeft)
#define ftr ((hUint)Point::FarTopRight)
#define fbl ((hUint)Point::FarBottomLeft)
#define fbr ((hUint)Point::FarBottomRight)
#define ntl ((hUint)Point::NearTopLeft) 
#define ntr ((hUint)Point::NearTopRight)
#define nbl ((hUint)Point::NearBottomLeft)
#define nbr ((hUint)Point::NearBottomRight)

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
			frustumPoints[ i ] = vex[ i ];
		}

		//
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

    void hViewFrustum::updateFromOrthoCamera(const hVec3& EyeVec, const hVec3& LookDir, const hVec3& Up, hFloat width, hFloat height, hFloat Near, hFloat Far) {
        hFloat FarH = height;
        hFloat FarW = width;
        hFloat NearH = height;
        hFloat NearW = width;
        hVec3 Right;
        hVec3 D = normalize(LookDir);
        hVec3 EyeFar = EyeVec + (D * Far);
        hVec3 EyeNear = EyeVec + (D * Near);
        Right = cross(Up, D);

        // re-generate our 6 planes
#define ftl ((hUint)Point::FarTopLeft)
#define ftr ((hUint)Point::FarTopRight)
#define fbl ((hUint)Point::FarBottomLeft)
#define fbr ((hUint)Point::FarBottomRight)
#define ntl ((hUint)Point::NearTopLeft) 
#define ntr ((hUint)Point::NearTopRight)
#define nbl ((hUint)Point::NearBottomLeft)
#define nbr ((hUint)Point::NearBottomRight)

        Heart::hVec3* vex = frustumPoints;
        hFloat FarW2 = FarW / 2;
        hFloat FarH2 = FarH / 2;
        hFloat NearH2 = NearH / 2;
        hFloat NearW2 = NearW / 2;

        vex[ftl] = EyeFar + (Up * FarH2) - (Right * FarW2);
        vex[ftr] = EyeFar + (Up * FarH2) + (Right * FarW2);
        vex[fbl] = EyeFar - (Up * FarH2) - (Right * FarW2);
        vex[fbr] = EyeFar - (Up * FarH2) + (Right * FarW2);

        vex[ntl] = EyeNear + (Up * NearH2) - (Right * NearW2);
        vex[ntr] = EyeNear + (Up * NearH2) + (Right * NearW2);
        vex[nbl] = EyeNear - (Up * NearH2) - (Right * NearW2);
        vex[nbr] = EyeNear - (Up * NearH2) + (Right * NearW2);

        near_ = buildPlane(EyeNear, vex[ntr], vex[ntl]);
        far_ = buildPlane(EyeFar, vex[ftl], vex[ftr]);
        top_ = buildPlane(vex[ntr], vex[ftr], vex[ftl]);
        bottom_ = buildPlane(vex[nbl], vex[fbl], vex[fbr]);
        right_ = buildPlane(vex[nbr], vex[fbr], vex[ftr]);
        left_ = buildPlane(vex[ntl], vex[ftl], vex[fbl]);

        viewFrustumAABB_ = hAABB::computeFromPointSet(vex, 8);

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

	hBool hViewFrustum::testAABB( const Heart::hAABB& AABB ) {
		if ( Heart::hAABB::intersect( AABB, viewFrustumAABB_ ) )
		{
			//do culling test against planes
			hVec3 vex[ 8 ];

			//bottom left near
            vex[0] = AABB.c_ - AABB.r_;
			//bottom right near
            vex[1] = AABB.c_ + mulPerElem( AABB.r_, hVec3( 1.f, -1.f, -1.f ) );
			//bottom right far
            vex[2] = AABB.c_ + mulPerElem( AABB.r_, hVec3( 1.f, -1.f, 1.f ) );
			//bottom left far
            vex[3] = AABB.c_ + mulPerElem( AABB.r_, hVec3( -1.f, -1.f, +1.f ) );
			//top left near
            vex[4] = AABB.c_ + mulPerElem( AABB.r_, hVec3( -1.f, 1.f, -1.f ) );
			//top right near
            vex[5] = AABB.c_ + mulPerElem( AABB.r_, hVec3( 1.f, 1.f, -1.f ) );
			//top right far
            vex[6] = AABB.c_ + AABB.r_;
			//top left far
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

	hBool hViewFrustum::testMovingAABB( const Heart::hAABB& aabb, const Heart::hVec3& dir )	{
		Heart::hPlane* planes[ 6 ] = { &near_, &far_, &top_, &bottom_, &left_, &right_ };
		hFloat f,l;
		hBool inside = hTrue;

        if ( hAABB::intersectMovingAABB( viewFrustumAABB_, aabb, hVec3(0.f, 0.f, 0.f), dir, f, l ) )
		{
 			for ( hUint32 p = 0; p < 6; ++p )
 			{
 				hPlane* pPlane = planes[ p ];
 				if ( intersectMovingAABB( aabb, dir, *pPlane ) )
 				{
 					return hTrue;
 				}
				inside &= AABBInfrontOfPlane( aabb, *pPlane ) > 0.0f;
 			}
		}

		return inside;
	}

}