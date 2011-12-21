/********************************************************************

	filename: 	SceneNodeMeshCollection.cpp	
	
	Copyright (c) 16:1:2011 James Moran
	
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
#include "SceneNodeMeshCollection.h"

namespace HScene
{
	ACCEPT_VISITOR( MeshCollection );

	const char* MeshCollection::StaticNodeTypeName = "MeshCollection";

	//////////////////////////////////////////////////////////////////////////
	// 21:17:40 ////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	bool MeshCollection::Serialise( TiXmlElement* toelement )
	{
		if ( !Node::Serialise( toelement ) )
			return false;

		XML_LINK_INT_AS_STRING( "nMeshes_", meshes_.size(), toelement );
		for ( u32 i = 0; i < meshes_.size(); ++i )
		{
			XML_LINK_INT_ID( meshes_, i, toelement );
		}
	
		return true;	
	}

	//////////////////////////////////////////////////////////////////////////
	// 21:20:53 ////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	bool MeshCollection::Deserialise( TiXmlElement* fromelement )
	{
		if ( !Node::Deserialise( fromelement ) )
			return false;

		u32 count;
		XML_UNLINK_INT_FROM_ELEMENT( "nMeshes_", count, fromelement );
		meshes_.resize( count );
		XML_UNLINK_INT_ID( meshes_, fromelement );

		ExpandAABB();

		return true;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////

	void MeshCollection::AppendMesh( u32 meshid )
	{
		meshes_.push_back( meshid ); 
		ExpandAABB();
	}

	//////////////////////////////////////////////////////////////////////////
	// 1:14:19 ////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void MeshCollection::ExpandAABB()
	{
		ClearAABB( &aabb_ );
		for ( u32 i = 0; i < meshes_.size(); ++i )
		{
			ExpandAABBByAABB( &aabb_, *sceneGraph_->GetMesh( meshes_[i] )->GetAABB() );
		}
	}

}

