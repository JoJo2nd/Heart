/********************************************************************

	filename: 	hSceneNodeMesh.cpp	
	
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
