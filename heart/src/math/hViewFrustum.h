/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#ifndef __HRVIEWFRUSTUM_H__
#define __HRVIEWFRUSTUM_H__

#include "base/hTypes.h"
#include "base/hArray.h"
#include "math/hAABB.h"
#include "math/hVec3.h"
#include "math/hPlane.h"

namespace Heart {

	class  hViewFrustum {
	public:
        enum class Point : hUint {
            FarTopLeft=0,
            FarTopRight=1,
            FarBottomLeft=2,
            FarBottomRight=3,
            NearTopLeft=4, 
            NearTopRight=5,
            NearBottomLeft=6, 
            NearBottomRight=7,
        };

		void							updateFromCamera(const hVec3& EyeVec, const hVec3& LookDir, const hVec3& Up, hFloat fov, hFloat Aspect, hFloat Near, hFloat Far);
        void                            updateFromOrthoCamera(const hVec3& EyeVec, const hVec3& LookDir, const hVec3& Up, hFloat width, hFloat height, hFloat Near, hFloat Far);
		hBool							testAABB( const hAABB& AABB );
		hBool							testMovingAABB( const hAABB& aabb, const hVec3& dir );
		const hVec3&					frustumPoint(Point i) const { return frustumPoints[(hUint)i]; }


	private:

		hAABB							viewFrustumAABB_;
		hMatrix							currentLookAt_;
		hArray< hVec3, 8 >				frustumPoints;
		hPlane							near_, far_, left_, right_, top_, bottom_;

	};

}

#endif //__HRVIEWFRUSTUM_H__