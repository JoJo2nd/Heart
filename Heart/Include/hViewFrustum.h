/********************************************************************
	created:	2009/01/01

	filename: 	ViewFrustum.h

	author:		James Moran
	
	purpose:	
*********************************************************************/
#ifndef __HRVIEWFRUSTUM_H__
#define __HRVIEWFRUSTUM_H__

#include "hMath.h"
#include "hArray.h"

namespace Heart
{

	class hViewFrustum
	{
	public:
										hViewFrustum();
		virtual							~hViewFrustum();

		void							UpdateFromCamera( const hVec3& EyeVec, const hVec3& LookDir, const hVec3& Up, hFloat fov, hFloat Aspect, hFloat Near, hFloat Far, hBool ortho );
		hBool							TestAABB( const hAABB& AABB );
		hBool							TestMovingAABB( const hAABB& aabb, const hVec3& dir );
		const hVec3&					FrustumPoint( hUint32 i ) const { return frustumPoints_[ i ]; }


	private:

		hAABB							viewFrustumAABB_;
		hMatrix							currentLookAt_;
		hArray< hVec3, 8 >				frustumPoints_;
		hPlane							near_, far_, left_, right_, top_, bottom_;

	};

}

#endif //__HRVIEWFRUSTUM_H__