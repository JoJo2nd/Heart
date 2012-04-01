/********************************************************************

	filename: 	hMesh.h	
	
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

#ifndef hiMesh_h__
#define hiMesh_h__

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