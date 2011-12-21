/********************************************************************

	filename: 	DataMesh.h	
	
	Copyright (c) 23:1:2011 James Moran
	
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

#ifndef DATAMESH_H__
#define DATAMESH_H__

#include "stdafx.h"
#include "SceneDatabase.h"
#include "Serialiser.h"

struct aiScene;
struct aiMesh;
struct Vec3;

namespace Data
{

	void BuildFromaiMesh( u32 meshIndex, const aiMesh* pmeshin, Mesh* pmeshout, const std::vector< int >& matmap );

	struct Face
	{
		static const u32 NUM_INDICES = 3;
		u32		indices_[NUM_INDICES];
	};

	class Mesh : public Serialiser
	{
	public:
		Mesh() :
			nVertices_( 0 )
			,pVertices_( NULL )
			,nNormals_( 0 )
			,pNormals_( NULL )
			,nTangents_( 0 )
			,pTangents_( NULL )
			,nBinormals_( 0 )
			,pBinormals_( NULL )
			,nUVChannels_( 0 )
			,uvElementCount_( NULL )
			,pnUVVertices_( NULL )
			,pUVChannels_( NULL )
			,materialIndex_( 0xFFFFFFFF )
			,vertexFlags_( 0 )
		{

		}
		~Mesh()
		{
			DestoryAllData();
		}
		
		const char*			GetName() const { return name_.c_str(); }
		void				SetName( const char* val ) { name_ = val; }
		u32					VertexFlags() const { return vertexFlags_; }
		u32					VertexStride() const { return vertexStride_; }
		void*				DrawVertex() const { return pDrawVertexArray_; }
		u16*				DrawIndex() const { return pDrawIndexArray_; }
		u32					DrawIndexCount() const { return nDrawIndices_; }
		u32					DrawVertexCount() const { return nDrawVertices_; }
		u32					GetMeshMaterialIndex() const { return materialIndex_; }
		u32					GetMeshSceneIndex() const { return meshIndex_; }
		void				SetMeshSceneIndex( u32 idx ) { meshIndex_ = idx; }

		// Data That we would serialise 
		u32					GetnVertices() const { return nVertices_; }
		Vec3*				GetpVertices() const { return pVertices_; }
		u32					GetnNormals() const { return nNormals_; }
		Vec3*				GetpNormals() const { return pNormals_; }
		u32					GetnTangents() const { return nTangents_; }
		Vec3*				GetpTangents() const { return pTangents_; }
		u32					GetnBinormals() const { return nBinormals_; }
		Vec3*				GetpBinormals() const { return pBinormals_; }
		u32					GetnUVChannels() const { return nUVChannels_; }
		u32					GetUVChannelsElementCount( u32 channelIdx ) { return uvElementCount_[channelIdx]; }
		u32					GetUVChannelCount( u32 channelIdx ) { return pnUVVertices_[channelIdx]; }
		Vec3*				GetpUVChannel( u32 channelIdx ) { return pUVChannels_[channelIdx]; }
		u32					GetnColourChannels() { return nColourChannels_; }
		u32					GetColourChannelCount( u32 ) { return nVertices_; }
		Vec4*				GetpColourChannel( u32 channelIdx ) { return pColourChannels_[channelIdx]; }
		u32					GetnFaces() { return nFaces_; }
		Face*				GetpFaces() { return pFaces_; }
		const AABB*			GetAABB() { return &aabb_; }

		bool				Serialise( TiXmlElement* toelement );
		bool				Deserialise( TiXmlElement* fromelement );

	private:

		friend void		BuildFromaiMesh( u32 meshIndex, const aiMesh* pmeshin, Mesh* pmeshout, const std::vector< int >& matmap );

		void			BuildVolatileDrawData();
		void			DestoryAllData();
		void			DestoryVolatileDrawData();

		//Serialized data
		std::string		name_;
		u32				nVertices_;
		Vec3*			pVertices_;
		u32				nNormals_;
		Vec3*			pNormals_;
		u32				nTangents_;
		Vec3*			pTangents_;
		u32				nBinormals_;//number of binormals
		Vec3*			pBinormals_;//array of binormals (x nBinormals_)
		u32				nUVChannels_;//number of uv's per vertex
		u32*			uvElementCount_;//number of components used by each uv channel (x, x&y or x&y&z)
		u32*			pnUVVertices_;//Number of uvs in each vertex channel
		Vec3**			pUVChannels_;//UV data
		u32				nColourChannels_;
		Vec4**			pColourChannels_;//size will match nVertices
		u32				nFaces_;
		Face*			pFaces_;
		AABB			aabb_;
		u32				meshIndex_;
		u32				materialIndex_;
		
		//Non-Serialized data
		u32				vertexFlags_;
		u32				vertexStride_;
		u32				nDrawVertices_;
		u8*				pDrawVertexArray_;
		u32				nDrawIndices_;
		u16*			pDrawIndexArray_;
	};
}

#endif // DATAMESH_H__