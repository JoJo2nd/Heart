/********************************************************************

	filename: 	hSceneGraph.h	
	
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

#ifndef __HRSCENEGRAPH_H__
#define __HRSCENEGRAPH_H__

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