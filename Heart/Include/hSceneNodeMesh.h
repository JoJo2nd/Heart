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
#include "hRendererCamera.h"
#include "hSceneGraphVisitorBase.h"

namespace Heart
{

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	class hSceneNodeMesh : public hSceneNodeBase
	{
	public:

#if 0
		typedef hResourceHandle< hMesh > MeshRes;
#endif

		hSceneNodeMesh();
		virtual ~hSceneNodeMesh();

		DEFINE_VISITABLE( hSceneNodeMesh );

#if 0
		void				AppendMesh( const MeshRes& mesh );
		void				RemoveMesh( hUint32 idx );
#endif
		hUint32				GetMeshCount() const { return nMeshes_; }
#if 0
        hMesh*		GetMesh( hUint32 idx ) const 
		{ 
			if ( idx >= nMeshes_ )
				return NULL;
			return meshes_[idx];
		}
#endif

	private:

		void				UnloadCallback();

		hUint32		nMeshes_;
		hUint32		arrayReserve_;
#if 0
		MeshRes*	meshes_;
#endif

	};
}

#endif // HRSCENEGRAPHNODEMESH_H__