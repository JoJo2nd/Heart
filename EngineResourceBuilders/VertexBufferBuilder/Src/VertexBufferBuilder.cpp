/********************************************************************

	filename: 	MeshBuilder.cpp	
	
	Copyright (c) 27:3:2011 James Moran
	
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

#include "VertexBufferBuilder.h"
#include "ResourceFileSystem.h"
#include "VertexBuffer.h"
#include "DeviceD3D9VtxBuffer.h"
#include "NvTriStrip.h"

VertexBufferBuilder::VertexBufferBuilder()
{

}

VertexBufferBuilder::~VertexBufferBuilder()
{

}

hBool VertexBufferBuilder::BuildResource()
{
	meshDef_.SetCondenseWhiteSpace( false );
	if ( meshDef_.LoadFile( inputFilename_.c_str(), TIXML_ENCODING_UTF8 ) )
	{
		meshDef_.Accept( this );
	}
	else
	{
		ThrowFatalError( "Index Buffer Builder could not parse input mesh def xml %s.Error:"
			"%s(line:%d column:%d)", inputFilename_.c_str(), meshDef_.ErrorDesc(), meshDef_.ErrorRow(), meshDef_.ErrorCol() );
	}

	//Tri Strip the found indices
	PrimitiveGroup* pPrimGroups;
	hUint16 nGroups = 0;
	(void)pPrimGroups;
	//Guess at the GPU cache size
	//SetCacheSize( 64 );
	SetListsOnly( true );
	GenerateStrips( &indices_[0], indices_.size(), &pPrimGroups, &nGroups, true );

	if ( nGroups > 1 )
	{
		ThrowFatalError( "Tri-Stripping produced more that one primitive group. This isn't supported yet." );
	}

	ResourceDataPtr< Heart::Render::VertexBuffer > vbuffer;
	setBufferSize( 0, sizeof( Heart::Render::VertexBuffer ) );
	getDataFromBuffer( sizeof( Heart::Render::VertexBuffer ), vbuffer );

	vbuffer->pOwner_ = NULL;

	hUint32 flags = 0;
	hUint32 vtxSize = 0;
	hUint32 vtxBufferSize = 0;
	if ( vertices_.size() > 0 )
	{
		flags |= Heart::Render::hrVF_XYZ;
		vtxSize += sizeof(hFloat)*3;
	}
	if ( normals_.size() > 0 )
	{
		flags |= Heart::Render::hrVF_NORMAL;
		vtxSize += sizeof(hFloat)*3;
	}
	if ( tangents_.size() > 0 )
	{
		flags |= Heart::Render::hrVF_TANGENT;
		vtxSize += sizeof(hFloat)*3;
	}
	if ( bitangents_.size() > 0 )
	{
		flags |= Heart::Render::hrVF_BINORMAL;
		vtxSize += sizeof(hFloat)*3;
	}
	if ( uvs_[0].size() > 0 )
	{
		flags |= Heart::Render::hrVF_1UV;
		vtxSize += sizeof(hFloat)*2;
	}
	if ( uvs_[1].size() > 0 )
	{
		flags |= Heart::Render::hrVF_2UV;
		vtxSize += sizeof(hFloat)*2;
	}
	if ( uvs_[2].size() > 0 )
	{
		flags |= Heart::Render::hrVF_3UV;
		vtxSize += sizeof(hFloat)*2;
	}
	if ( uvs_[3].size() > 0 )
	{
		flags |= Heart::Render::hrVF_4UV;
		vtxSize += sizeof(hFloat)*2;
	}
	if ( colour_.size() > 0 )
	{
		flags |= Heart::Render::hrVF_COLOR;
		vtxSize += sizeof(hUint32);
	}

	if ( flags == 0 )
	{
		ThrowFatalError( "No Data in vertex buffer" );
	}

	vtxBufferSize = vtxSize*vertices_.size();
	vbuffer->pVtxDecl_ = (Heart::Render::VertexDeclaration*)flags;

	//Create space for pimpls just behind the texture class
	ResourceDataPtr< Heart::Device::D3D9VtxBuffer > pImplData;
	getDataFromBuffer( sizeof( Heart::Device::D3D9VtxBuffer ), pImplData );

	pImplData->pBuffer_ = NULL;
	pImplData->maxVertex_ = vertices_.size();
	pImplData->nVertex_ = vertices_.size();
	pImplData->mode_ = 0;

	ResourceDataPtr< hByte > vtxOut;
	vbuffer->pVtxBuffer_ = getDataFromBuffer( vtxSize*vertices_.size(), vtxOut );
	vbuffer->vtxBufferSize_ = vtxBufferSize;

	for ( hUint32 i = 0, p = 0, e = vertices_.size(); i != e; ++i )
	{
		//indicesOut.GetElement( i ) = pPrimGroups[0].indices[0];
		if ( vertices_.size() > 0 )
		{
			memcpy( &vtxOut.GetElement(p), &vertices_[i], sizeof(hFloat)*3 );
			p += sizeof(hFloat)*3;
		}
		if ( normals_.size() > 0 )
		{
			memcpy( &vtxOut.GetElement(p), &normals_[i], sizeof(hFloat)*3 );
			p += sizeof(hFloat)*3;
		}
		if ( tangents_.size() > 0 )
		{
			memcpy( &vtxOut.GetElement(p), &tangents_[i], sizeof(hFloat)*3 );
			p += sizeof(hFloat)*3;
		}
		if ( bitangents_.size() > 0 )
		{
			memcpy( &vtxOut.GetElement(p), &bitangents_[i], sizeof(hFloat)*3 );
			p += sizeof(hFloat)*3;
		}
		if ( uvs_[0].size() > 0 )
		{
			memcpy( &vtxOut.GetElement(p), &uvs_[0][i], sizeof(hFloat)*2 );
			p += sizeof(hFloat)*2;
		}
		if ( uvs_[1].size() > 0 )
		{
			memcpy( &vtxOut.GetElement(p), &uvs_[1][i], sizeof(hFloat)*2 );
			p += sizeof(hFloat)*2;
		}
		if ( uvs_[2].size() > 0 )
		{
			memcpy( &vtxOut.GetElement(p), &uvs_[2][i], sizeof(hFloat)*2 );
			p += sizeof(hFloat)*2;
		}
		if ( uvs_[3].size() > 0 )
		{
			memcpy( &vtxOut.GetElement(p), &uvs_[3][i], sizeof(hFloat)*2 );
			p += sizeof(hFloat)*2;
		}
		if ( colour_.size() > 0 )
		{
			memcpy( &vtxOut.GetElement(p), &colour_[i], sizeof(hUint32) );
			p += sizeof(hUint32);
		}

		if ( p > vtxBufferSize )
		{
			ThrowFatalError( "Vertex Buffer Overflow" );
		}
	}

	return hTrue;
}

bool VertexBufferBuilder::VisitExit( const TiXmlElement& element )
{
	if ( strcmp( element.Value(), "vertices" ) == 0 )
	{
		vertices_.resize( elementStack_.size() );
		hUint32 idx = elementStack_.size()-1;

		// parse whats on the element stack
		while( !elementStack_.empty() )
		{
			Heart::Math::Vec3 v;
			if( sscanf_s( elementStack_.top()->GetText(), " %f %f %f ", &v.x, &v.y, &v.z ) != 3 )
			{
				ThrowFatalError( "Failed to parse vertices" );
			}

			vertices_[idx--] = v;

			elementStack_.pop();
		}
	}
	else if ( strcmp( element.Value(), "normals" ) == 0 )
	{
		normals_.resize( elementStack_.size() );
		hUint32 idx = elementStack_.size()-1;

		// parse whats on the element stack
		while( !elementStack_.empty() )
		{
			Heart::Math::Vec3 v;
			if( sscanf_s( elementStack_.top()->GetText(), " %f %f %f ", &v.x, &v.y, &v.z ) != 3 )
			{
				ThrowFatalError( "Failed to parse normals" );
			}

			//TODO: Why do these need flipping? is assimp leaving them if a right handed space?
			normals_[idx--] = v;

			elementStack_.pop();
		}
	}
	else if ( strcmp( element.Value(), "tangents" ) == 0 )
	{
		tangents_.resize( elementStack_.size() );
		hUint32 idx = elementStack_.size()-1;

		// parse whats on the element stack
		while( !elementStack_.empty() )
		{
			Heart::Math::Vec3 v;
			if( sscanf_s( elementStack_.top()->GetText(), " %f %f %f ", &v.x, &v.y, &v.z ) != 3 )
			{
				ThrowFatalError( "Failed to parse tangents" );
			}

			//TODO: Why do these need flipping? is assimp leaving them if a right handed space?
			tangents_[idx--] = v;

			elementStack_.pop();
		}
	}
	else if ( strcmp( element.Value(), "bitangents" ) == 0 )
	{
		bitangents_.resize( elementStack_.size() );
		hUint32 idx = elementStack_.size()-1;

		// parse whats on the element stack
		while( !elementStack_.empty() )
		{
			Heart::Math::Vec3 v;
			if( sscanf_s( elementStack_.top()->GetText(), " %f %f %f ", &v.x, &v.y, &v.z ) != 3 )
			{
				ThrowFatalError( "Failed to parse face bitangents" );
			}

			//TODO: Why do these need flipping? is assimp leaving them if a right handed space?
			bitangents_[idx--] = v;

			elementStack_.pop();
		}
	}
	else if ( strcmp( element.Value(), "UV" ) == 0 )
	{
		int uvidx;
		if ( element.QueryIntAttribute( "channel", &uvidx ) == TIXML_SUCCESS )
		{
			if ( uvidx > MAX_UVS )
			{
				ThrowFatalError( "Too Many UVs in mesh file" ) ;
			}

			uvs_[uvidx].resize( elementStack_.size() );
			hUint32 idx = elementStack_.size()-1;

			// parse whats on the element stack
			while( !elementStack_.empty() )
			{
				Heart::Math::Vec2 v;
				if( sscanf_s( elementStack_.top()->GetText(), " %f %f ", &v.x, &v.y ) != 2 )
				{
					ThrowFatalError( "Failed to parse uvs" );
				}

				uvs_[uvidx][idx--] = v;

				elementStack_.pop();
			}
		}
		else
		{
			ThrowFatalError( "Couldn't find channel attribute for uv element" );
		}
	}
	else if ( strcmp( element.Value(), "Colour" ) == 0 )
	{
		//TODO: atm, just pop off whatever is on the stack
		while( !elementStack_.empty() )
		{
			elementStack_.pop();
		}
	}
	else if ( strcmp( element.Value(), "indices" ) == 0 )
	{
		indices_.resize( elementStack_.size()*3 );
		hUint32 idx = (elementStack_.size()*3)-1;

		// parse whats on the element stack
		while( !elementStack_.empty() )
		{
			hUint16 i1,i2,i3;
			if( sscanf_s( elementStack_.top()->GetText(), " %hu %hu %hu ", &i1, &i2, &i3 ) != 3 )
			{
				ThrowFatalError( "Failed to parse face indices" );
			}

			indices_[idx--] = i3;
			indices_[idx--] = i2;
			indices_[idx--] = i1;

			elementStack_.pop();
		}
	}
	else if ( strcmp( element.Value(), "face" ) == 0 )
	{
		elementStack_.push( &element );
	}
	else if ( strcmp( element.Value(), "vertex" ) == 0 )
	{
		elementStack_.push( &element );
	}
	return true;
}
