/********************************************************************
	created:	2009/11/02
	created:	2:11:2009   21:45
	filename: 	hrSceneGraphNodeCamera.h	
	author:		James
	
	purpose:	
*********************************************************************/
#ifndef HRSCENEGRAPHNODECAMERA_H__
#define HRSCENEGRAPHNODECAMERA_H__

#include "hSceneNodeBase.h"
#include "hRendererCamera.h"
#include "hSceneGraphVisitorBase.h"

namespace Heart
{

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	class hSceneNodeCamera : public hSceneNodeBase
	{
	public:

							hSceneNodeCamera();
		virtual				~hSceneNodeCamera();

		DEFINE_VISITABLE( hSceneNodeCamera );

		void					UpdateCamera();
		const hMatrix*		GetViewMatrix() { return &viewMatrix_; }
		const hMatrix*		GetProjectionMatrix() { return &projectionMatrix_; }
		const hMatrix*		GetViewProjectionMatrix() { return &viewProjectionMatrix_; }
		hFloat					Far() const { return far_; }
		void					Far(hFloat val) 
		{ 
			far_ = val; 
			MakeDirty();
		}
		hFloat					Near() const { return near_; }
		void					Near(hFloat val) 
		{ 
			near_ = val; 
			MakeDirty();
		}
		hFloat					FovYDegrees() const { return fovYDegrees_; }
		void					FovYDegrees(hFloat val) 
		{ 
			fovYDegrees_ = val; 
			MakeDirty();
		}
		hFloat					Aspect() const { return aspect_; }
		void					Aspect(hFloat val) 
		{ 
			aspect_ = val; 
			MakeDirty();
		}
		const hViewport&	GetViewport() const { return viewport_; }
		void					SetViewport( const hViewport& val ) 
		{ 
			viewport_ = val; 
			MakeDirty();
		}
		hBool					GetOrtho() const { return orthoView_; }
		void					SetOrtho( hBool val ) { orthoView_ = val; }
		hFloat					GetOrthoWidth() const { return orthoWidth_; }
		void					SetOrthoWidth( hFloat val ) { orthoWidth_ = val; }
		hFloat					GetOrthoHeight() const { return orthoHeight_; }
		void					SetOrthoHeight( hFloat val ) { orthoHeight_ = val; }
		hVec3		            ProjectTo2D( const hVec3& point );
		hViewFrustum*			GetViewFrustum() { return &frustum_; } 

#ifndef HEART_RESOURCE_BUILDER
	private:
#endif // HEART_RESOURCE_BUILDER

		friend class SceneGraph;

		hVec3			camPos_;
		hVec3			camLookAt_;
		hVec3			camUp_;
		hViewport	viewport_;
		hFloat				aspect_;
		hFloat				fovYDegrees_;
		hFloat				near_;
		hFloat				far_;
		hFloat				orthoWidth_;
		hFloat				orthoHeight_;
		hMatrix		viewMatrix_;
		hMatrix		viewProjectionMatrix_;
		hMatrix		projectionMatrix_;
		hViewFrustum			frustum_;

		bool				orthoView_;
	};

}

#endif // HRSCENEGRAPHNODECAMERA_H__