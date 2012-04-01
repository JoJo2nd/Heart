/********************************************************************

	filename: 	hViewFrustum.h	
	
	Copyright (c) 31:3:2012 James Moran
	
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

#ifndef __HRVIEWFRUSTUM_H__
#define __HRVIEWFRUSTUM_H__

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