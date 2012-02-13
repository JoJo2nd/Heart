/********************************************************************
	created:	2009/11/02
	created:	2:11:2009   21:00
	filename: 	hrSceneGraphNodeMesh.cpp	
	author:		James
	
	purpose:	
*********************************************************************/

#include "Common.h"
#include "hSceneNodeMesh.h"
#include "hRenderer.h"
#include "hSceneGraphVisitorBase.h"
#include "Heart.h"

namespace Heart
{
	
	ENGINE_ACCEPT_VISITOR( hSceneNodeMesh, hSceneGraphVisitorEngine );

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	hSceneNodeMesh::hSceneNodeMesh() :
		hSceneNodeBase( SCENENODETYPE_MESH )
		,nMeshes_( 0 )
		,arrayReserve_( 0 )
#if 0
		,meshes_( NULL )
#endif
	{
		ResetAABB();
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	hSceneNodeMesh::~hSceneNodeMesh()
	{
#if 0
		hcAssert( meshes_ == NULL );//Unload not been called?
#endif
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
#if 0
	void hSceneNodeMesh::AppendMesh( const MeshRes& mesh )
	{

		if ( (nMeshes_+1) >= arrayReserve_ )
		{
			if ( arrayReserve_ == 0 )
				arrayReserve_ = 4;
			else
				arrayReserve_ *= 2;
			MeshRes* pnew = hNEW ( hSceneGraphHeap ) MeshRes[arrayReserve_];
			if ( nMeshes_ > 0 )
			{
				memcpy( pnew, meshes_, sizeof(MeshRes)*(nMeshes_-1) );
			}
			delete meshes_;
			meshes_ = pnew;
		}

		mesh.Acquire();
		meshes_[nMeshes_] = mesh;
		++nMeshes_;

		orginAABB_.c_ = hVec3Func::zeroVector();
        orginAABB_.r_ = hVec3Func::zeroVector();

		for ( hUint32 i = 0; i < nMeshes_; ++i )
		{
			hAABB::ExpandBy( orginAABB_, *meshes_[i]->GetAABB() );
		}

	}
#endif
	//////////////////////////////////////////////////////////////////////////
	// 19:21:29 ////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

#if 0
	void hSceneNodeMesh::RemoveMesh( hUint32 idx )
	{
		(void)idx;
		hcBreak;//TODO
	}
#endif

	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////

	void hSceneNodeMesh::UnloadCallback()
	{
#if 0
		for ( hUint32 i = 0; i < nMeshes_; ++i )
		{
			meshes_[i].Release();
		}
		delete meshes_;
		meshes_ = NULL;
		nMeshes_ = 0;
		arrayReserve_ = 0;
#endif
	}

}
