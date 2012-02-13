/********************************************************************
	created:	2008/08/31
	created:	31:8:2008   22:35
	filename: 	Mesh.h
	author:		James Moran
	
	purpose:	
**********w***********************************************************/
#ifndef hiMesh_h__
#define hiMesh_h__

#include "hTypes.h"
#include "hResource.h"
#include "hVertexBuffer.h"
#include "hIndexBuffer.h"
#include "hMaterial.h"
#include "hTexture.h"
#include "hAABB.h"

namespace Heart
{
	class hRenderer;

	//////////////////////////////////////////////////////////////////////////
	// A Mesh simply describes a collection of triangles that can be /////////
	// rendered by the renderer. High level mesh manipulation of character ///
	// meshes is handled by the scene graph (e.g. animation) /////////////////
	//////////////////////////////////////////////////////////////////////////
	class hMesh
	{
	public:

#if 0
		typedef hResourceHandle< hVertexBuffer >	VertexBufferResource;
		typedef hResourceHandle< hIndexBuffer >		IndexBufferResource;
		typedef hResourceHandle< hMaterial >		MaterialResource;
#endif

		hMesh() {}
		virtual ~hMesh() 
		{
#if 0
			if ( vertices_.HasData() )
			{
				vertices_.Release();
			}
			if ( indices_.HasData() )
			{
				indices_.Release();
			}
			if ( material_.HasData() )
			{
				material_.Release();
			}
#endif
		}

#if 0
		const VertexBufferResource&				GetVertexBuffer() const { return vertices_; }
		const IndexBufferResource&				GetIndexBuffer() const { return indices_; }
#endif
		hUint32									GetPrimativeCount() const { return nPrimatives_; }
#if 0
		const MaterialResource&					GetMaterial() const { return material_; }
#endif
		const Heart::hAABB*						GetAABB() const { return &aabb_; }
		void									SetAABB( const Heart::hAABB& aabb ) { aabb_ = aabb; }

#if 0
		void									SetVertexBuffer( const VertexBufferResource& verts )
		{
			if ( vertices_.HasData() )
			{
				vertices_.Release();
			}
			verts.Acquire();
			vertices_ = verts;
		}
		void									SetIndexBuffer( const IndexBufferResource& idx )
		{
			if ( indices_.HasData() )
			{
				indices_.Release();
			}
			idx.Acquire();
			indices_ = idx;

			nPrimatives_ = indices_->GetIndexCount() / 3;
		}
		void									SetMaterial( const MaterialResource& mat )
		{
			if ( material_.HasData() )
			{
				material_.Release();
			}
			mat.Acquire();
			material_ = mat;
		}
#endif
	
	private:

		friend class hRenderer;
		friend class MeshBuilder;

#if 0
		VertexBufferResource	vertices_;
		IndexBufferResource		indices_;
#endif
		hUint32					nPrimatives_;
#if 0
		MaterialResource		material_;
#endif
		Heart::hAABB		aabb_;
	};
}

#endif // hiMesh_h__