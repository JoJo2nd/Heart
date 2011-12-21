/********************************************************************
	created:	2009/11/02
	created:	2:11:2009   20:50
	filename: 	hrSceneGraphNodeMesh.h	
	author:		James
	
	purpose:	
*********************************************************************/
#ifndef HRSCENEGRAPHNODEMESH_H__
#define HRSCENEGRAPHNODEMESH_H__


#include "hSceneNodeBase.h"
#include "hMesh.h"
#include "hCamera.h"
#include "hSceneGraphVisitorBase.h"

namespace Heart
{

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	class hSceneNodeMesh : public hSceneNodeBase
	{
	public:

		typedef hResourceHandle< hMesh > MeshRes;

		hSceneNodeMesh();
		virtual ~hSceneNodeMesh();

		DEFINE_VISITABLE( hSceneNodeMesh );

		void				AppendMesh( const MeshRes& mesh );
		void				RemoveMesh( hUint32 idx );
		hUint32				GetMeshCount() const { return nMeshes_; }
		hMesh*		GetMesh( hUint32 idx ) const 
		{ 
			if ( idx >= nMeshes_ )
				return NULL;
			return meshes_[idx];
		}

	private:

		void				UnloadCallback();

		hUint32		nMeshes_;
		hUint32		arrayReserve_;
		MeshRes*	meshes_;

	};
}

#endif // HRSCENEGRAPHNODEMESH_H__