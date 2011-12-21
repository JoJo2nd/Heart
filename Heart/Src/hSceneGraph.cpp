/********************************************************************
	created:	2008/12/27

	filename: 	SceneGraph.cpp

	author:		James Moran
	
	purpose:	
*********************************************************************/

#include "Common.h"
#include "Heart.h"
#include "hSceneGraph.h"
#include "hSceneGraphVisitorBase.h"
#include "hSceneNodeLocator.h"
#include "hResourceManager.h"
#include "hSceneBundle.h"
#include "hSceneNodeLight.h"
#include "hSceneNodeMesh.h"
#include "hSceneNodeCamera.h"

namespace Heart
{

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	hSceneGraph::hSceneGraph() :
		buildingMatrixStack_( hFalse ),
		dirtyFlag_( hFalse )
	{

	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	hSceneGraph::~hSceneGraph()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hSceneGraph::Initialise( hResourceManager* pResourceManager )
	{
		pResourceManager_ = pResourceManager;

		rootNode_ = hNEW ( hSceneGraphHeap ) hSceneNodeLocator();
		//CreateResourceNode( HEART_SCENE_ROOT_NAME, RootNode_, pnewnode );

		rootNode_->DefaultState();
		rootNode_->SetMatrix( &Heart::IdentityMatrix );
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hSceneGraph::Destroy()
	{
		delete rootNode_;
        rootNode_ = NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hSceneGraph::VisitScene( hSceneGraphVisitorBase* pVisitor, VisitOrder visitOrder, hBool useMatrixStack )
	{
		if ( useMatrixStack )
		{
			buildingMatrixStack_ = hTrue;

			Heart::hMatrix idnt;
			Heart::hMatrix::identity( &idnt );

			MatrixStack_.push( idnt );
		}

		pVisitor->PreVisit( this );

		switch( visitOrder )
		{
		case TOP_DOWN:
			VisitTopDown( pVisitor, rootNode_, useMatrixStack );
			break;
		case BOTTOM_UP:
			VisitBottomUp( pVisitor, rootNode_, useMatrixStack );
			break;
		}

		pVisitor->PostVisit( this );

		if ( useMatrixStack )
		{
			MatrixStack_.pop();

			buildingMatrixStack_ = hFalse;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hSceneGraph::VisitTopDown( hSceneGraphVisitorBase* pVisitor, hSceneNodeBase* pNode, hBool useMatrixStack )
	{
		if ( useMatrixStack )
		{
			Heart::hMatrix* parent = MatrixStack_.top();
			Heart::hMatrix top;

			//TODO: Fix animation
// 			if ( pNode->pAttachedAnimator() )
// 			{
// 				pNode->pAttachedAnimator()->Update( pNode );
// 			}

			Heart::hMatrix::mult( pNode->GetMatrix(), parent, &top );

			MatrixStack_.push( top );

			pNode->ApplyTransforms( parent );
		}

		//because accept will clear the dirty store it
		hBool dirty = pNode->DirtyTransform_;

		pNode->Accept( pVisitor );

		if ( pVisitor->AbortChildVisit() == hFalse )
		{
			for ( hSceneNodeBase::ChildListNode* i = pNode->children_.GetHead(); i != NULL; i = i->GetNext() )
			{
				if ( dirty )
				{
					i->child_->MakeDirty();
				}
				VisitTopDown( pVisitor, i->child_, useMatrixStack );
			}
		}
		pVisitor->AbortChildVisit( hFalse );

		if ( useMatrixStack )
		{
			pNode->UpdateAABB();
			MatrixStack_.pop();
		}
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void hSceneGraph::VisitBottomUp( hSceneGraphVisitorBase* pVisitor, hSceneNodeBase* pNode, hBool useMatrixStack )
	{
		if ( useMatrixStack )
		{
			Heart::hMatrix* parent = MatrixStack_.top();
			Heart::hMatrix top;

			//TODO: fix animations
// 			if ( pNode->pAttachedAnimator() )
// 			{
// 				pNode->pAttachedAnimator()->Update( pNode );
// 			}

			Heart::hMatrix::mult( pNode->GetMatrix(), parent, &top );

			MatrixStack_.push( top );
		}

		for ( hSceneNodeBase::ChildListNode* i = pNode->children_.GetHead(); i != NULL; i = i->GetNext() )
		{
			if ( pNode->DirtyTransform_ )
			{
				i->child_->MakeDirty();
			}
			VisitTopDown( pVisitor, i->child_, useMatrixStack );
		}

		pNode->Accept( pVisitor );

		if ( useMatrixStack )
		{
			MatrixStack_.pop();
		}
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	hUint32 hSceneGraph::OnSceneNodeLoad( const hChar* pExt, void* pLoadedData, void* pUserData, hResourceManager* pResourceManager )
	{
		hSceneNodeBase* pnode = (hSceneNodeBase*)pLoadedData;

		pnode->DefaultState();
		pnode->IsDiskResource( true );

		return 0;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	hUint32 hSceneGraph::OnSceneNodeUnload( const hChar* pExt, void* pLoadedData, void* pUserData, hResourceManager* pResourceManager )
	{
		//TODO: double check on node being in the scene graph
		hSceneNodeBase* pnode = (hSceneNodeBase*)pLoadedData;
		pnode->UnloadCallback();
		if ( !pnode->IsDiskResource() )
		{
			pnode->~hSceneNodeBase();
		}
		return 0;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////

	hUint32 hSceneGraph::OnSceneLoad( const hChar* pExt, void* pLoadedData, void* pUserData, hResourceManager* pResourceManager )
	{
		hSceneBundle* pbundle = (hSceneBundle*)pLoadedData;
		hByte* pdata = (hByte*)pLoadedData;

		HEART_RESOURCE_DATA_FIXUP( hResourceHandle< hSceneNodeBase >, pdata, pbundle->pNodes_ );
		HEART_RESOURCE_DATA_FIXUP( hResourceHandle< hMesh >, pdata, pbundle->pMeshes_ );
		HEART_RESOURCE_DATA_FIXUP( hSceneBundle::ChildLink, pdata, pbundle->pChildLinks_ );

// 		for ( hUint32 i = 0; i < pbundle->nMeshes_; ++i )
// 		{
// 			pDepsList->GetDependency( pbundle->pMeshes_[i] );
// 		}

		for ( hUint32 i = 0; i < pbundle->nNodes_; ++i )
		{
			//pDepsList->GetDependency( pbundle->pNodes_[i] );
			pbundle->pNodes_[i]->us_ = pbundle->pNodes_[i];
			if ( pbundle->pChildLinks_[i].pChildren_ )
			{
				HEART_RESOURCE_DATA_FIXUP( hUint32, pdata, pbundle->pChildLinks_[i].pChildren_ );
			}
			if( pbundle->pChildLinks_[i].pMeshIdx_ )
			{
				HEART_RESOURCE_DATA_FIXUP( hUint32, pdata, pbundle->pChildLinks_[i].pMeshIdx_ );
			}
			//Add meshes to nodes
			for ( hUint32 i2 = 0; i2 < pbundle->pChildLinks_[i].nMeshes_; ++i2 )
			{
				hcAssert( pbundle->pNodes_[i]->GetTypeID() == SCENENODETYPE_MESH );
				((hSceneNodeMesh*)((hSceneNodeBase*)pbundle->pNodes_[i]))->AppendMesh( pbundle->pMeshes_[pbundle->pChildLinks_[i].pMeshIdx_[i2]] );
			}
		}

		//Link up the children in the scene
		for ( hUint32 i = 0; i < pbundle->nNodes_; ++i )
		{
			for ( hUint32 i2 = 0; i2 < pbundle->pChildLinks_[i].nChildren_; ++i2 )
			{
				pbundle->pNodes_[i]->AttachChild( pbundle->pNodes_[pbundle->pChildLinks_[i].pChildren_[i2]] );
			}
		}

		pbundle->IsDiskResource( true );

		return 0;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////

	hUint32 hSceneGraph::OnSceneUnload( const hChar* pExt, void* pLoadedData, void* pUserData, hResourceManager* pResourceManager )
	{
		hSceneBundle* pbundle = (hSceneBundle*)pLoadedData;

		// Unlink Children
		for ( hUint32 i = 0; i < pbundle->nNodes_; ++i )
		{
			if ( pbundle->pNodes_[i]->parent_.HasData() )
			{
				pbundle->pNodes_[i]->DetachFromParent();
			}
			pbundle->pNodes_[i].Release();
		}

		for ( hUint32 i = 0; i < pbundle->nMeshes_; ++i )
		{
			pbundle->pMeshes_[i].Release();
		}

		return 0;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////

	hUint32 hSceneGraph::OnSceneNodeLocatorLoad( const hChar* pExt, void* pLoadedData, void* pUserData, hResourceManager* pResourceManager )
	{
 		OnSceneNodeLoad( pExt, pLoadedData, pUserData, pResourceManager );

		hSceneNodeLocator* ploc = (hSceneNodeLocator*) pLoadedData;

		new ( ploc ) hSceneNodeLocator();

		ploc->IsDiskResource( true );

		return 0;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////

	hUint32 hSceneGraph::OnSceneNodeLocatorUnload( const hChar* pExt, void* pLoadedData, void* pUserData, hResourceManager* pResourceManager )
	{
		return 0;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////

	hUint32 hSceneGraph::OnSceneNodeLightLoad( const hChar* pExt, void* pLoadedData, void* pUserData, hResourceManager* pResourceManager )
	{
  		OnSceneNodeLoad( pExt, pLoadedData, pUserData, pResourceManager );

		hSceneNodeLight* plig = (hSceneNodeLight*)pLoadedData;
		
		new ( plig ) hSceneNodeLight();

		plig->BuildAABB();
		plig->IsDiskResource( true );

		return 0;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////

	hUint32 hSceneGraph::OnSceneNodeLightUnload( const hChar* pExt, void* pLoadedData, void* pUserData, hResourceManager* pResourceManager )
	{
		return 0;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////

	hUint32 hSceneGraph::OnSceneNodeMeshCollectionLoad( const hChar* pExt, void* pLoadedData, void* pUserData, hResourceManager* pResourceManager )
	{
 		OnSceneNodeLoad( pExt, pLoadedData, pUserData, pResourceManager );
		hSceneNodeMesh* pmc = (hSceneNodeMesh*)pLoadedData;
		new ( pmc ) hSceneNodeMesh();

		pmc->IsDiskResource( true );

		return 0;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////

	hUint32 hSceneGraph::OnSceneNodeMeshCollectionUnload( const hChar* pExt, void* pLoadedData, void* pUserData, hResourceManager* pResourceManager )
	{
		return 0;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////

	hUint32 hSceneGraph::OnSceneNodeCameraLoad( const hChar* pExt, void* pLoadedData, void* pUserData, hResourceManager* pResourceManager )
	{
 		OnSceneNodeLoad( pExt, pLoadedData, pUserData, pResourceManager );
 
		hSceneNodeCamera* pcam = (hSceneNodeCamera*)pLoadedData;
		new ( pcam ) hSceneNodeCamera();

		pcam->DirtyTransform_ = true;
		pcam->UpdateCamera();

		pcam->IsDiskResource( true );

		return 0;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////

	hUint32 hSceneGraph::OnSceneNodeCameraUnload( const hChar* pExt, void* pLoadedData, void* pUserData, hResourceManager* pResourceManager )
	{
		return 0;
	}

}