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
			projectionMatrix_ = hMatrixFunc::orthoProj( orthoWidth_, orthoHeight_, near_, far_ );
		}
		else
		{
			projectionMatrix_ = hMatrixFunc::perspectiveFOV( hmDegToRad(fovYDegrees_), aspect_, near_, far_ );
		}

		viewMatrix_ = hMatrixFunc::inverse( *GetGlobalMatrix() );
		viewProjectionMatrix_ = hMatrixFunc::mult( viewMatrix_, projectionMatrix_ );

		hVec3 tmp;
        camLookAt_ = (hVec3)hMatrixFunc::getRow( *GetGlobalMatrix(), 2 );//hVec3( GetGlobalMatrix()->m31, GetGlobalMatrix()->m32, GetGlobalMatrix()->m33);
		camPos_ = (hVec3)hMatrixFunc::getRow( *GetGlobalMatrix(), 3 );//hVec3( GetGlobalMatrix()->m41, GetGlobalMatrix()->m42, GetGlobalMatrix()->m43 );
		camUp_ = (hVec3)hMatrixFunc::getRow( *GetGlobalMatrix(), 1 );//hVec3( GetGlobalMatrix()->m21, GetGlobalMatrix()->m22, GetGlobalMatrix()->m23 );
		
		frustum_.UpdateFromCamera( camPos_, camPos_+camLookAt_, camUp_, hmDegToRad(fovYDegrees_), aspect_, near_, far_, orthoView_ );

	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	hVec3 hSceneNodeCamera::ProjectTo2D( const hVec3& point )
	{
		hVec4 ret;
		hVec4 point4( point );

		//Heart::Matrix::mult( &viewMatrix_, &projectionMatrix_, &viewProj );

		ret = point4 * viewProjectionMatrix_;
		//Heart::Vec3 r2( ((ret.x/ret.w) * viewport_.width_ / 2), ((ret.y/ret.w) * viewport_.height_ / 2), ret.z/ret.w );
		//hVec3 r2( ret.x/ret.w, ret.y/ret.w, ret.z/ret.w );
        //r2.x = ((r2.x + 1.0f) / 2.0f) * viewport_.width_;
        //r2.y = ((-r2.y + 1.0f) / 2.0f) * viewport_.height_;
        hVec3 r2 = hVec4Func::div( ret, hVec128SplatW( ret ) );
        r2 = hVec3Func::componentMult( r2, hVec3( 1.f, -1.f, 1.f ) );
        r2 += hVec3( 1.f, 1.f, 0.f );
        r2 = hVec3Func::componentMult( r2, hVec3( (hFloat)viewport_.width_, (hFloat)viewport_.height_, 1.f ) );

		return r2;
	}

}
