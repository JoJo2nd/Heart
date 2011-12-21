/********************************************************************
	created:	2009/11/05
	created:	5:11:2009   20:24
	filename: 	hrSceneGraphNodeCamera.cpp	
	author:		James
	
	purpose:	
*********************************************************************/

#include "Common.h"
#include "hSceneNodeCamera.h"
#include "hSceneGraphVisitorBase.h"

namespace Heart	
{
	hSceneNodeCamera::hSceneNodeCamera() :
		hSceneNodeBase( SCENENODETYPE_CAMERA )
	{
	}

	hSceneNodeCamera::~hSceneNodeCamera()
	{

	}

	ENGINE_ACCEPT_VISITOR( hSceneNodeCamera, hSceneGraphVisitorEngine );

	void hSceneNodeCamera::UpdateCamera()
	{
		using namespace Heart::Math;

		if ( orthoView_ )
		{
			hMatrix::orthoProj( &projectionMatrix_, orthoWidth_, orthoHeight_, near_, far_ );
		}
		else
		{
			hMatrix::perspectiveFOV( &projectionMatrix_, hmDegToRad(fovYDegrees_), aspect_, near_, far_ );
		}

		hMatrix::inverse( GetGlobalMatrix(), &viewMatrix_ );
		hMatrix::mult( &viewMatrix_, &projectionMatrix_, &viewProjectionMatrix_ );

		hVec3 tmp;
		camLookAt_ = hVec3( GetGlobalMatrix()->m31, GetGlobalMatrix()->m32, GetGlobalMatrix()->m33);
		camPos_ = hVec3( GetGlobalMatrix()->m41, GetGlobalMatrix()->m42, GetGlobalMatrix()->m43 );
		camUp_ = hVec3( GetGlobalMatrix()->m21, GetGlobalMatrix()->m22, GetGlobalMatrix()->m23 );
		
		frustum_.UpdateFromCamera( camPos_, camPos_+camLookAt_, camUp_, hmDegToRad(fovYDegrees_), aspect_, near_, far_, orthoView_ );

	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	Heart::hVec3 hSceneNodeCamera::ProjectTo2D( Heart::hVec3 point )
	{
		Heart::hVec4 ret;
		Heart::hVec4 point4( point.x, point.y, point.z, 1.0f );

		//Heart::Matrix::mult( &viewMatrix_, &projectionMatrix_, &viewProj );

		Heart::hMatrix::mult( point4, &viewProjectionMatrix_, ret );
		//Heart::Vec3 r2( ((ret.x/ret.w) * viewport_.width_ / 2), ((ret.y/ret.w) * viewport_.height_ / 2), ret.z/ret.w );
		Heart::hVec3 r2( ret.x/ret.w, ret.y/ret.w, ret.z/ret.w );
		r2.x = ((r2.x + 1.0f) / 2.0f) * viewport_.width_;
		r2.y = ((-r2.y + 1.0f) / 2.0f) * viewport_.height_;

		return r2;
	}

}
