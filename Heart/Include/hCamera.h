/********************************************************************
	created:	2008/12/24

	filename: 	Camera.h

	author:		James Moran
	
	purpose:	
*********************************************************************/
#ifndef __HRCAMERA_H__
#define __HRCAMERA_H__

#include "hTypes.h"
#include "Common.h"
#include "hMath.h"
#include "hArray.h"
#include "hViewFrustum.h"
#include "hRenderCommon.h"

namespace Heart
{
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	class hCamera
	{
	public:

		enum Axis
		{
			X,
			Y,
			Z,
		};

		hCamera();
		virtual	~hCamera();

		void									SetFieldOfView( hFloat fovDegrees )
		{
			FOV_ = hmDegToRad( fovDegrees );
			MakeDirty();
		}
		void									SetProjectionParams( hFloat Ratio, hFloat Near, hFloat Far );
		void									SetOrthoParams( hFloat width, hFloat height, hFloat znear, hFloat zfar );
		void									SetFocusPoint( const hVec3& Point )
		{
			FocusPoint_ = Point;
			MakeDirty();
		}
		hVec3								FocusPoint() const { return FocusPoint_; }
		hVec3								LookAt() const
		{
			return hVec3Func::normalise( FocusPoint_ - CameraPos_ );
		}
		void									SetUp( const hVec3& up )
		{
			Up_ = up;
			MakeDirty();
		}
		hVec3								Up() const { return Up_; }
		hVec3								Left() const
		{
			return hVec3Func::cross( LookAt(), Up_ );
		}
		void									SetViewMatrix( hMatrix& m ) 
		{ 
			IsDirty_ = hFalse; 
			ViewMatrix_ = m; 
		}
		hMatrix&							GetViewMatrix();
		hMatrix&							GetProjectionMatrix() { return ProjectionMatrix_; }
		hVec3								LookAtVector() 
		{ 
			if ( IsDirty_ )
			{ 
				GetViewMatrix(); 
			} 
			return DirtyEye_ - DirtyLookAt_; 
		}
		hVec3								CameraPosition() 
		{ 
			if ( IsDirty_ )
			{ 
				GetViewMatrix(); 
			} 
			return DirtyEye_; 
		}
		void									DefaultCamera();
		hViewFrustum*							pViewFrustum()
		{
			if ( IsDirty_ == hTrue )
			{
				DirtyEye_ = CameraPos_;
				DirtyLookAt_ = FocusPoint_;
				DirtyUp_ = Up_;

				ViewMatrix_ = hMatrixFunc::LookAt( DirtyEye_, DirtyLookAt_, DirtyUp_ );

				Frustum_.UpdateFromCamera( DirtyEye_, DirtyLookAt_, DirtyUp_, FOV_, Aspect_, Near_, Far_, isOrtho_ );

				IsDirty_ = hFalse;
			}
			return &Frustum_;
		}
		void									CameraPosition( hFloat x, hFloat y, hFloat z )
		{
			CameraPos_ = hVec3Func::set( x, y, z );
			MakeDirty();
		}
		void									CameraPosition( const hVec3& pos ) 
		{ 
			CameraPos_ = pos; 
			MakeDirty(); 
		}
		hVec3								    ProjectTo2D( const hVec3& point );
		void									SetViewport( const hViewport& vp ) { Viewport_ = vp; }
		hViewport						        GetViewport() const { return Viewport_; }
		hFloat									GetFar() const { return Far_; }
		hFloat									GetNear() const { return Near_; }

	private:

		void									MakeDirty()
		{
			IsDirty_ = hTrue;
		}

		hFloat									FOV_;
		hFloat									Aspect_;
		hFloat									Near_;
		hFloat									Far_;
		hBool									isOrtho_;
		hMatrix									ViewMatrix_;
		hMatrix									ProjectionMatrix_;
		hVec3									FocusPoint_;
		hVec3									CameraPos_;
		hVec3									Up_;
		hViewFrustum								Frustum_;
		hViewport								Viewport_;
		hRenderer*								pRenderer_;
		

		// Dirty stuff [1/8/2009 James]
		hVec3								DirtyUp_;
		hVec3								DirtyEye_;
		hVec3								DirtyLookAt_;

		BEGIN_FLAGS()
			hBool								IsDirty_ : 1;
		END_FLAGS()
	};
	
}

#endif //__HRCAMERA_H__