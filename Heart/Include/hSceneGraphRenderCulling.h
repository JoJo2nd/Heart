/********************************************************************
	created:	2009/12/14
	created:	14:12:2009   22:34
	filename: 	SceneGraphRenderVisitorCulling.h	
	author:		James
	
	purpose:	
*********************************************************************/
#ifndef HRSCENEGRAPHVISITORCULLINGBASE_H__
#define HRSCENEGRAPHVISITORCULLINGBASE_H__

#include "hSceneGraphVisitorBase.h"
#include "hResource.h"

namespace Heart
{
	class hRendererCamera;

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	class hSceneGraphRenderVisitorCulling : public hSceneGraphVisitorBase
	{
	public:

#if 0
		typedef hResourceHandle< hSceneNodeCamera > CameraNode;
#endif

		hSceneGraphRenderVisitorCulling() {}

		ENGINE_DEFINE_VISITOR();

		virtual void	PreVisit( hSceneGraph* );
		virtual void	Visit( hSceneNodeLocator& visit );
		virtual void	Visit( hSceneNodeMesh& visit );

#if 0
		void			SetCamera( const CameraNode& camera )
		{
			if ( cameraNode_.HasData() )
			{
				cameraNode_.Release();
			}
			camera.Acquire();
			cameraNode_ = camera;
		}
#endif

	private:

#if 0
		CameraNode		cameraNode_;
#endif
		//Camera* camera_;
	};
}

#endif // HRSCENEGRAPHVISITORCULLINGBASE_H__