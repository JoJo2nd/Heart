/********************************************************************

	filename: 	DataMesh.cpp	
	
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

#include "stdafx.h"
#include "DataMesh.h"
#include "aiMesh.h"

namespace Data
{

#define MAKE_COLOUR( a, r, g, b ) ( (((u8)(a*255))<<24) | (((u8)(r*255))<<16) | (((u8)(g*255))<<8) | (((u8)(b*255))) )

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void BuildFromaiMesh( u32 meshIndex, const aiMesh* pmeshin, Mesh* pmeshout, const std::vector< int >& matmap )
	{
		pmeshout->meshIndex_ = meshIndex;
		pmeshout->materialIndex_ = matmap[pmeshin->mMaterialIndex];

		if ( strlen( pmeshin->mName.data ) > 0 )
		{
			pmeshout->name_ = pmeshin->mName.data;
		}
		else
		{
			pmeshout->name_ = "Mesh";
		}

		if ( pmeshin->HasFaces() )
		{
			if ( pmeshin->HasPositions() )
			{
				pmeshout->nVertices_ = pmeshin->mNumVertices;
				pmeshout->pVertices_ = new Vec3[ pmeshout->nVertices_ ];

				for ( u32 i = 0; i < pmeshin->mNumVertices; ++i )
				{
					pmeshout->pVertices_[i].x = pmeshin->mVertices[i].x;
					pmeshout->pVertices_[i].y = pmeshin->mVertices[i].y;
					pmeshout->pVertices_[i].z = pmeshin->mVertices[i].z;
				}
			}

			if ( pmeshin->HasNormals() )
			{
				pmeshout->nNormals_ = pmeshin->mNumVertices;
				pmeshout->pNormals_ = new Vec3[ pmeshout->nNormals_ ];

				for ( u32 i = 0; i < pmeshin->mNumVertices; ++i )
				{
					pmeshout->pNormals_[i].x = pmeshin->mNormals[i].x;
					pmeshout->pNormals_[i].y = pmeshin->mNormals[i].y;
					pmeshout->pNormals_[i].z = pmeshin->mNormals[i].z;
				}
			}

			if ( pmeshin->HasTangentsAndBitangents() )
			{
				pmeshout->nBinormals_ = pmeshin->mNumVertices;
				pmeshout->nTangents_ = pmeshin->mNumVertices;

				pmeshout->pBinormals_ = new Vec3[ pmeshout->nBinormals_ ];
				pmeshout->pTangents_ = new Vec3[ pmeshout->nTangents_ ];

				for ( u32 i = 0; i < pmeshin->mNumVertices; ++i )
				{
					pmeshout->pTangents_[i].x = pmeshin->mTangents[i].x;
					pmeshout->pTangents_[i].y = pmeshin->mTangents[i].y;
					pmeshout->pTangents_[i].z = pmeshin->mTangents[i].z;

					pmeshout->pBinormals_[i].x = pmeshin->mBitangents[i].x;
					pmeshout->pBinormals_[i].y = pmeshin->mBitangents[i].y;
					pmeshout->pBinormals_[i].z = pmeshin->mBitangents[i].z;
				}
			}

			pmeshout->nUVChannels_ = 0;
			while ( pmeshin->HasTextureCoords( pmeshout->nUVChannels_ ) ) { ++pmeshout->nUVChannels_; }
			
			if ( pmeshout->nUVChannels_ > 0 )
			{
				pmeshout->uvElementCount_ = new u32[pmeshout->nUVChannels_];
				pmeshout->pnUVVertices_ = new u32[pmeshout->nUVChannels_];
				pmeshout->pUVChannels_ = new Vec3*[pmeshout->nUVChannels_];
				for ( u32 i = 0; i < pmeshout->nUVChannels_; ++i )
				{
					pmeshout->uvElementCount_[i] = pmeshin->mNumUVComponents[i] > 2 ? 2 : pmeshin->mNumUVComponents[i];
					pmeshout->pnUVVertices_[i] = pmeshin->mNumVertices;

					pmeshout->pUVChannels_[i] = new Vec3[pmeshout->pnUVVertices_[i]];

					for ( u32 i2 = 0; i2 < pmeshout->pnUVVertices_[i]; ++i2 )
					{
						pmeshout->pUVChannels_[i][i2].x = pmeshin->mTextureCoords[i][i2].x;
						pmeshout->pUVChannels_[i][i2].y = pmeshin->mTextureCoords[i][i2].y;
					}
				}
			}

			pmeshout->nColourChannels_ = 0;
			while ( pmeshin->HasVertexColors( pmeshout->nColourChannels_ ) ) { ++pmeshout->nColourChannels_; }

			if ( pmeshout->nColourChannels_ > 0 )
			{
				pmeshout->pColourChannels_ = new Vec4*[pmeshout->nColourChannels_];
				
				for ( u32 i = 0; i < pmeshout->nColourChannels_; ++i )
				{
					pmeshout->pColourChannels_[i] = new Vec4[pmeshout->nVertices_];
					for ( u32 i2 = 0; i2 < pmeshout->nVertices_; ++i )
					{
						pmeshout->pColourChannels_[i][i2].x = pmeshin->mColors[i][i2].r;
						pmeshout->pColourChannels_[i][i2].y = pmeshin->mColors[i][i2].b;
						pmeshout->pColourChannels_[i][i2].z = pmeshin->mColors[i][i2].g;
						pmeshout->pColourChannels_[i][i2].w = pmeshin->mColors[i][i2].a;
					}
				}
			}

			pmeshout->nFaces_ = pmeshin->mNumFaces;
			pmeshout->pFaces_ = new Face[pmeshout->nFaces_];

			for ( u32 i = 0; i < pmeshout->nFaces_; ++i )
			{
				ASSERT( pmeshin->mFaces[i].mNumIndices == Face::NUM_INDICES );
				
				for ( u32 i2 = 0; i2 < Face::NUM_INDICES; ++i2 )
				{
					pmeshout->pFaces_[i].indices_[i2] = pmeshin->mFaces[i].mIndices[i2];
				}
			}

			ComputeAABBFromPointSet( pmeshout->pVertices_, pmeshout->nVertices_, &pmeshout->aabb_ );

			pmeshout->BuildVolatileDrawData();
		}
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void Mesh::BuildVolatileDrawData()
	{
		//calculate vertex stride
		vertexStride_ = 0;
		if ( nVertices_ > 0 )
		{
			vertexStride_ += 3*sizeof(float);
			vertexFlags_ |= VF_POSITION;
		}
		if ( nNormals_ > 0 )
		{
			vertexStride_ += 3*sizeof(float);
			vertexFlags_ |= VF_NORMAL;
		}
		if ( nTangents_ > 0 )
		{
			vertexStride_ += 3*sizeof(float);
			vertexFlags_ |= VF_TANGENT;
		}
		if ( nBinormals_ > 0 )
		{
			vertexStride_ += 3*sizeof(float);
			vertexFlags_ |= VF_BINORMAL;
		}
		if ( nColourChannels_ > 0 )
		{
			vertexStride_ += nColourChannels_*sizeof(u32);
			if ( nColourChannels_ == 1 ) vertexFlags_ |= VF_COLOUR1;
			if ( nColourChannels_ == 2 ) vertexFlags_ |= VF_COLOUR2;
			if ( nColourChannels_ == 3 ) vertexFlags_ |= VF_COLOUR3;
			if ( nColourChannels_ == 4 ) vertexFlags_ |= VF_COLOUR4;
		}
		for ( u32 i = 0; i < nUVChannels_; ++i )	
		{
			vertexStride_ += uvElementCount_[i]*sizeof(float);
			if ( i == 0 ) vertexFlags_ |= VF_UV1;
			if ( i == 1 ) vertexFlags_ |= VF_UV2;
			if ( i == 2 ) vertexFlags_ |= VF_UV3;
			if ( i == 3 ) vertexFlags_ |= VF_UV4;
		}

		ComputeAABBFromPointSet( pVertices_, nVertices_, &aabb_ );

		//Do draw indices
		nDrawIndices_ = nFaces_*3;
		pDrawIndexArray_ = new u16[nDrawIndices_];
		for ( u32 i = 0, vi = 0; i < nFaces_; ++i )
		{
			for ( u32 i2 = 0; i2 < Face::NUM_INDICES; ++i2 )
			{
				pDrawIndexArray_[vi++] = (u16)pFaces_[i].indices_[i2];
			}
		}

		//Do draw vertices
		nDrawVertices_ = nVertices_;
		pDrawVertexArray_ = new u8[nVertices_*vertexStride_];
		u8* dst = pDrawVertexArray_;
		for ( u32 i = 0; i < nVertices_; ++i )
		{
			if ( nVertices_ > 0 )
			{
				memcpy( dst, &pVertices_[i], 3*sizeof(float) );
				dst += 3*sizeof(float);
			}
			if ( nNormals_ > 0 )
			{
				memcpy( dst, &pNormals_[i], 3*sizeof(float) );
				dst += 3*sizeof(float);
			}
			if ( nTangents_ > 0 )
			{
				memcpy( dst, &pTangents_[i], 3*sizeof(float) );
				dst += 3*sizeof(float);
			}
			if ( nBinormals_ > 0 )
			{
				memcpy( dst, &pBinormals_[i], 3*sizeof(float) );
				dst += 3*sizeof(float);
			}
			for ( u32 i2 = 0; i < nColourChannels_; ++i )
			{
				u32 col = MAKE_COLOUR( pColourChannels_[i2][i].w, pColourChannels_[i2][i].x, pColourChannels_[i2][i].y, pColourChannels_[i2][i].z );
				memcpy( dst, &col, sizeof(u32) );
				dst += sizeof(u32);

			}
			for ( u32 i2 = 0; i2 < nUVChannels_; ++i2 )	
			{
				memcpy( dst, &pUVChannels_[i2][i], uvElementCount_[i2]*sizeof(float) );
				dst += uvElementCount_[i2]*sizeof(float);
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void Mesh::DestoryAllData()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void Mesh::DestoryVolatileDrawData()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	// 21:30:21 ////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	bool Mesh::Serialise( TiXmlElement* toelement )
	{
		XML_LINK_STRING( name_, toelement );
		XML_LINK_INT( meshIndex_, toelement );
		XML_LINK_AABB( aabb_, toelement );
		XML_LINK_INT( nVertices_, toelement );
		for ( u32 i = 0; i < nVertices_; ++i )
		{
			XML_LINK_VEC3_ID( pVertices_, i, toelement );
		}
		XML_LINK_INT( nNormals_, toelement );
		for ( u32 i = 0; i < nNormals_; ++i )
		{
			XML_LINK_VEC3_ID( pNormals_, i, toelement );
		}
		XML_LINK_INT( nTangents_, toelement );
		for ( u32 i = 0; i < nTangents_; ++i )
		{
			XML_LINK_VEC3_ID( pTangents_, i, toelement );
		}
		XML_LINK_INT( nBinormals_, toelement );
		for ( u32 i = 0; i < nBinormals_; ++i )
		{
			XML_LINK_VEC3_ID( pBinormals_, i, toelement );
		}
		XML_LINK_INT( nUVChannels_, toelement );
		for ( u32 i = 0; i < nUVChannels_; ++i )
		{
			TiXmlElement* uvele = new TiXmlElement( "uvchannel" );
			uvele->SetAttribute( "elements", uvElementCount_[i] );
			u32 nUVs_ = pnUVVertices_[i];
			XML_LINK_INT( nUVs_, uvele );
			Vec3* uvs = pUVChannels_[i];
			for ( u32 i2 = 0; i2 < pnUVVertices_[i]; ++i2 )
			{
				if ( uvElementCount_[i] == 3 )
				{	
					XML_LINK_VEC3_ID( uvs, i2, uvele );
				}
				else
				{
					XML_LINK_VEC2_ID( uvs, i2, uvele );
				}
			}

			toelement->LinkEndChild( uvele );
		}
		XML_LINK_INT( nColourChannels_, toelement );
		for ( u32 i = 0; i < nColourChannels_; ++i )
		{
			TiXmlElement* cele = new TiXmlElement( "colourchannel" );
			u32 nColours_ = nVertices_;
			XML_LINK_INT( nColours_, cele );
			Vec4* colours = pColourChannels_[i];
			for ( u32 i2 = 0; i2 < nColours_; ++i2 )
			{
				XML_LINK_VEC4_ID( colours, i2, cele );
			}

			toelement->LinkEndChild( cele );
		}
		XML_LINK_INT( nFaces_, toelement );
		for ( u32 i = 0; i < nFaces_; ++i )
		{
			XML_LINK_FACE_ID( pFaces_, i, toelement );
		}

		XML_LINK_INT( materialIndex_, toelement );

		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	// 21:30:28 ////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	bool Mesh::Deserialise( TiXmlElement* fromelement )
	{
		XML_UNLINK_STRING( name_, fromelement );
		XML_UNLINK_INT( meshIndex_, fromelement );
		XML_UNLINK_AABB( aabb_, fromelement );
		XML_UNLINK_INT( nVertices_, fromelement );
		pVertices_ = new Vec3[nVertices_];
		XML_UNLINK_VEC3_ID( pVertices_, fromelement );
		XML_UNLINK_INT( nNormals_, fromelement );
		pNormals_ = new Vec3[nNormals_];
		XML_UNLINK_VEC3_ID( pNormals_, fromelement );
		XML_UNLINK_INT( nTangents_, fromelement );
		pTangents_ = new Vec3[nTangents_];
		XML_UNLINK_VEC3_ID( pTangents_, fromelement );
		XML_UNLINK_INT( nBinormals_, fromelement );
		pBinormals_ = new Vec3[nBinormals_];
		XML_UNLINK_VEC3_ID( pBinormals_, fromelement );
		XML_UNLINK_INT( nUVChannels_, fromelement );
		u32 c = 0;
		pUVChannels_ = new Vec3*[nUVChannels_];
		pnUVVertices_ = new u32[nUVChannels_];
		uvElementCount_ = new u32[nUVChannels_];
		for ( TiXmlElement* ele = fromelement->FirstChildElement( "uvchannel" ); ele; ele = ele->NextSiblingElement( "uvchannel" ), ++c )
		{
			int ecount;
			if ( ele->QueryIntAttribute( "elements", &ecount ) != TIXML_SUCCESS )
				return false;
			u32 nUVs_;
			uvElementCount_[c] = ecount;
			XML_UNLINK_INT( nUVs_, ele );
			pnUVVertices_[c] = nUVs_;
			pUVChannels_[c] = new Vec3[nUVs_];
			Vec3* uvs = pUVChannels_[c];
			if ( ecount == 3 )
			{
				XML_UNLINK_VEC3_ID( uvs, ele );
			}
			else if ( ecount == 2 )
			{
				XML_UNLINK_VEC2_ID( uvs, ele );
			}
			else
			{
				return false;
			}
		}
		if ( c != nUVChannels_ )
			return false;
		XML_UNLINK_INT( nColourChannels_, fromelement );
		c = 0;
		pColourChannels_ = new Vec4*[nColourChannels_];
		for ( TiXmlElement* ele = fromelement->FirstChildElement( "colourchannel" ); ele; ele = ele->NextSiblingElement( "colourchannel" ), ++c )
		{
			u32 nColours_;
			XML_UNLINK_INT( nColours_, ele );
			pColourChannels_[c] = new Vec4[nColours_];
			Vec4* colours = pColourChannels_[c];
			XML_UNLINK_VEC4_ID( colours, ele );
		}
		XML_UNLINK_INT( nFaces_, fromelement );
		pFaces_ = new Face[nFaces_];
		XML_UNLINK_FACE_ID( pFaces_, fromelement );

		XML_UNLINK_INT( materialIndex_, fromelement );

		BuildVolatileDrawData();

		return true;
	}

}