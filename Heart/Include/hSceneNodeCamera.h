/********************************************************************

	filename: 	hSceneNodeCamera.h	
	
	Copyright (c) 1:4:2012 James Moran
	
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

#ifndef HRSCENEGRAPHNODECAMERA_H__
#define HRSCENEGRAPHNODECAMERA_H__

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