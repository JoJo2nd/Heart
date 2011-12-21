/********************************************************************
	created:	2008/12/22

	filename: 	SceneGraph.h

	author:		James Moran
	
	purpose:	
*********************************************************************/
#ifndef __HRSCENEGRAPH_H__
#define __HRSCENEGRAPH_H__

#include "hTypes.h"
#include "hSceneNodeBase.h"
#include "hCamera.h"
#include "hResource.h"
#include "hResourceManager.h"

namespace Heart
{
	class hResourceManager;
	class hResourceDependencyList;
	class hSceneGraphVisitorBase;
	class hSceneNodeBase;

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	class hSceneGraph
	{
	public:

		static const hChar								ROOT_NAME[];

		enum VisitOrder
		{
			TOP_DOWN,
			BOTTOM_UP,
		};

														hSceneGraph();
		virtual											~hSceneGraph();

		void											Initialise( hResourceManager* pResourceManager );
		void											Destroy();
		hSceneNodeBase*				                    pRootNode() { return rootNode_; }
		void											VisitScene( hSceneGraphVisitorBase* pVisitor, VisitOrder visitOrder, hBool useMatrixStack );
		Heart::hMatrix*									matrixStackTop()
		{
			hcAssert( buildingMatrixStack_ );
			return MatrixStack_.top();
		}

		hUint32											OnSceneNodeLoad( const hChar* pExt, void* pLoadedData, void* pUserData, hResourceManager* pResourceManager );
		hUint32											OnSceneNodeUnload( const hChar* pExt, void* pLoadedData, void* pUserDat, hResourceManager* pResourceManager );

		hUint32											OnSceneNodeLocatorLoad( const hChar* pExt, void* pLoadedData, void* pUserData, hResourceManager* pResourceManager );
		hUint32											OnSceneNodeLocatorUnload( const hChar* pExt, void* pLoadedData, void* pUserData, hResourceManager* pResourceManager );
		hUint32											OnSceneNodeLightLoad( const hChar* pExt, void* pLoadedData, void* pUserData, hResourceManager* pResourceManager );
		hUint32											OnSceneNodeLightUnload( const hChar* pExt, void* pLoadedData, void* pUserData, hResourceManager* pResourceManager );
		hUint32											OnSceneNodeMeshCollectionLoad( const hChar* pExt, void* pLoadedData, void* pUserData, hResourceManager* pResourceManager );
		hUint32											OnSceneNodeMeshCollectionUnload( const hChar* pExt, void* pLoadedData, void* pUserData, hResourceManager* pResourceManager );
		hUint32											OnSceneNodeCameraLoad( const hChar* pExt, void* pLoadedData, void* pUserData, hResourceManager* pResourceManager );
		hUint32											OnSceneNodeCameraUnload( const hChar* pExt, void* pLoadedData, void* pUserData, hResourceManager* pResourceManager );

		hUint32											OnSceneLoad( const hChar* pExt, void* pLoadedData, void* pUserData, hResourceManager* pResourceManager );
		hUint32											OnSceneUnload( const hChar* pExt, void* pLoadedData, void* pUserData, hResourceManager* pResourceManager );

	private:

		void											VisitTopDown( hSceneGraphVisitorBase* pVisitor, hSceneNodeBase* pNode, hBool useMatrixStack );
		void											VisitBottomUp( hSceneGraphVisitorBase* pVisitor, hSceneNodeBase* pNode, hBool useMatrixStack );

		hResourceManager*							    pResourceManager_;
		hSceneNodeBase*                      			rootNode_;
		hStack< hMatrix, 256 >					        MatrixStack_;
		hBool											buildingMatrixStack_;
		hBool											dirtyFlag_;

	};
}

#define HEART_SCENE_ROOT_NAME   "RootNode"

#endif //__HRSCENEGRAPH_H__