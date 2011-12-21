/********************************************************************
	created:	2009/01/08

	filename: 	Camera.cpp

	author:		James Moran
	
	purpose:	
*********************************************************************/

#include "Common.h"
#include "hCamera.h"
#include "hRenderer.h"
#include "Heart.h"

namespace Heart
{
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	Heart::hMatrix& hCamera::GetViewMatrix()
	{
		if ( IsDirty_ == hTrue )
		{
			DirtyEye_ = CameraPos_;
			DirtyLookAt_ = FocusPoint_;
			DirtyUp_ = Up_;

			Heart::hMatrix::lookAt( &ViewMatrix_, DirtyEye_, DirtyLookAt_, DirtyUp_ );

			Frustum_.UpdateFromCamera( DirtyEye_, DirtyLookAt_, DirtyUp_, FOV_, Aspect_, Near_, Far_, isOrtho_ );

			IsDirty_ = hFalse;
		}

		return ViewMatrix_;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	hCamera::hCamera() :
		pRenderer_( NULL )
	{
		DefaultCamera();
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	hCamera::~hCamera()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hCamera::DefaultCamera()
	{
		Heart::hVec3::set( 0.0f, 1.0f, 0.0f, Up_ );
		FOV_ = hmPI / 4.0f;//45.0f
		Heart::hMatrix::identity( &ViewMatrix_ );
		isOrtho_ = hFalse;
		Heart::hVec3::set( 0.0f, 0.0f, 0.0f, FocusPoint_ );
		Heart::hVec3::set( 0.0f, 0.0f, 10.0f, CameraPos_ );
		Viewport_.x_ = 0;
		Viewport_.y_ = 0;
		Viewport_.width_ = 800;
		Viewport_.height_ = 600;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hCamera::SetProjectionParams( hFloat Ratio, hFloat Near, hFloat Far )
	{
		Aspect_ = Ratio;
		Near_ = Near;
		Far_ = Far;
		isOrtho_ = hFalse;

		Heart::hMatrix::perspectiveFOV( &ProjectionMatrix_, FOV_, Aspect_, Near_, Far_ );
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hCamera::SetOrthoParams( hFloat width, hFloat height, hFloat znear, hFloat zfar )
	{
		Aspect_ = width / height;
		Near_ = znear;
		Far_ = zfar;
		isOrtho_ = hTrue;

		Heart::hMatrix::orthoProj( &ProjectionMatrix_, width, height, Near_, Far_ );
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	Heart::hVec3 hCamera::ProjectTo2D( Heart::hVec3 point )
	{
		Heart::hVec4 ret;
		Heart::hMatrix view, proj,viewProj;

		view = GetViewMatrix();
		proj = GetProjectionMatrix();
		Heart::hVec4 point4( point.x, point.y, point.z, 1.0f );
		Heart::hMatrix::mult( &view, &proj, &viewProj );
		Heart::hMatrix::mult( point4, &viewProj, ret );
		Heart::hVec3 r2( ((ret.x/ret.w) * Viewport_.width_ / 2), ((ret.y/ret.w) * Viewport_.height_ / 2), ret.z/ret.w );
		return r2;

	}

}