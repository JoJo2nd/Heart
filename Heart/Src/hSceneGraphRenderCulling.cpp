/********************************************************************
	created:	2009/12/14
	created:	14:12:2009   22:36
	filename: 	SceneGraphRenderVisitorCulling.cpp	
	author:		James
	
	purpose:	
*********************************************************************/

#include "Common.h"
#include "hSceneGraphRenderCulling.h"
#include "hSceneNodeCamera.h"
#include "hSceneNodeLocator.h"
#include "hSceneNodeMesh.h"
#include "hRendererCamera.h"

namespace Heart
{

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hSceneGraphRenderVisitorCulling::PreVisit( hSceneGraph* )
	{
#if 0
		hcAssert( cameraNode_.HasData() );
#endif
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hSceneGraphRenderVisitorCulling::Visit( hSceneNodeMesh& visit )
	{
#if 0
		if ( !cameraNode_->GetViewFrustum()->TestAABB( *visit.GetGlobalAABB() ) )
		{
			AbortChildVisit( hTrue );
		}
		else
		{

		}
#endif
	}

// 	void SceneGraphRenderVisitorCulling::Visit( SceneGraphNodeLight& visit )
// 	{
// 		if ( !camera_->pViewFrustum()->testAABB( visit.GlobalAABB() ) )
// 		{
// 			AbortChildVisit( hTrue );
// 		}
// 	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hSceneGraphRenderVisitorCulling::Visit( hSceneNodeLocator& visit )
	{
#if 0
		if ( !cameraNode_->GetViewFrustum()->TestAABB( *visit.GetGlobalAABB() ) )
		{
			AbortChildVisit( hTrue );
		}
#endif
	}
}