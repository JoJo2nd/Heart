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

#include "MeshBuilder.h"
#include "ResourceFileSystem.h"
#include "Mesh.h"

MeshBuilder::MeshBuilder() :
	matResId_( 0 )
{

}

MeshBuilder::~MeshBuilder()
{

}

hBool MeshBuilder::BuildResource()
{
	meshDef_.SetCondenseWhiteSpace( false );
	if ( meshDef_.LoadFile( inputFilename_.c_str(), TIXML_ENCODING_UTF8 ) )
	{
		meshDef_.Accept( this );
	}
	else
	{
		ThrowFatalError( "Mesh Builder could not parse input mesh def xml %s.Error:"
			"%s(line:%d column:%d)", inputFilename_.c_str(), meshDef_.ErrorDesc(), meshDef_.ErrorRow(), meshDef_.ErrorCol() );
	}

	ResourceDataPtr< Heart::Render::Mesh > mesh;
	setBufferSize( 0, sizeof( Heart::Render::Mesh ) );
	getDataFromBuffer( sizeof( Heart::Render::Mesh ), mesh );

	mesh->aabb_ = meshAABB_;

	mesh->material_.SetResID( matResId_ );

	//TODO: Add Index buffer & vertex buffer deps
	hUint32 depResId;
	GameResource* pRes = NULL;
	pRes = AddDependency( "index", inputFilename_, "indexbuffer", depResId );
	mesh->indices_.SetResID( depResId );
	pRes = AddDependency( "vertex", inputFilename_, "vertexbuffer", depResId );
	mesh->vertices_.SetResID( depResId );

	return hTrue;
}

bool MeshBuilder::VisitExit( const TiXmlElement& element )
{
	if ( strcmp( element.Value(), "material" ) == 0 )
	{
		if ( matResId_ != 0 )
		{
			ThrowFatalError( "Found multiple materials in mesh definition file, expected one" );
		}

		GameResource* pRes = AddDependency( "material", element.GetText(), "material", matResId_ );
	}
	else if ( strcmp( element.Value(), "vertex" ) == 0 )
	{
		elementStack_.push( &element );
	}
	else if ( strcmp( element.Value(), "vertices" ) == 0 )
	{
		Heart::Math::Vec3::set( 0.0f, 0.0f, 0.0f, meshAABB_.c );
		meshAABB_.r[0] = 0.0f;
		meshAABB_.r[1] = 0.0f;
		meshAABB_.r[2] = 0.0f;

		while ( !elementStack_.empty() )
		{
			const TiXmlElement* ele = elementStack_.top();
			Heart::Math::Vec3 v;
			if( sscanf_s( ele->GetText(), " %f %f %f ", &v.x, &v.y, &v.z ) != 3 )
			{
				ThrowFatalError( "Failed to parse vertex" );
			}

			Heart::Math::AABB::expandBy( v, meshAABB_ );
			elementStack_.pop();
		}
	}
	else if ( strcmp( element.Value(), "normals" ) == 0 &&
			  strcmp( element.Value(), "tangents" ) == 0 && 
			  strcmp( element.Value(), "bitangents" ) == 0 &&
			  strcmp( element.Value(), "UV" ) == 0 )
	{
		while ( !elementStack_.empty() )
		{
			elementStack_.pop();
		}
	}
	return true;
}
