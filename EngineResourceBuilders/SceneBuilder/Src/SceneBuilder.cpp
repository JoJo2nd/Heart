/********************************************************************

	filename: 	SceneBuilder.cpp	
	
	Copyright (c) 14:3:2011 James Moran
	
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

#include "SceneBuilder.h"
#include "SceneBundle.h"
#include "ResourceFileSystem.h"
#include <algorithm>

enum NodeType
{
	NODETYPE_LOCATOR,
	NODETYPE_MESHCOLLECTION,
	NODETYPE_CAMERA,
	NODETYPE_LIGHT,

	NODETYPE_MAX
};

const char* gNodeTypeNames[] = 
{
	"Locator",
	"MeshCollection",
	"Camera",
	"Light"
};

enum NodeElementNames
{
	NODEELEMENTNAMES_MESHINSTANCES,
	NODEELEMENTNAMES_MESHINSTANCE,
	NODEELEMENTNAMES_NODE,
	NODEELEMENTNAMES_NAME,
	NODEELEMENTNAMES_MATRIX,
	NODEELEMENTNAMES_GLOBALMATRIX,
	NODEELEMENTNAMES_MESHES,
	NODEELEMENTNAMES_MESH,
	NODEELEMENTNAMES_MESHINDEX,
	NODEELEMENTNAMES_SCENEINDEX,
	NODEELEMENTNAMES_CAMERADATA,
	NODEELEMENTNAMES_POSITION,
	NODEELEMENTNAMES_UP,
	NODEELEMENTNAMES_AT,
	NODEELEMENTNAMES_LIGHTDATA,
	NODEELEMENTNAMES_TYPE,
	NODEELEMENTNAMES_DIRECTION,
	NODEELEMENTNAMES_ATTENUATION,
	NODEELEMENTNAMES_DIFFUSE,
	NODEELEMENTNAMES_SPECULAR,
	NODEELEMENTNAMES_AMBIENT,
	NODEELEMENTNAMES_EMISSIVE,
	NODEELEMENTNAMES_LIGHTCONE,
	NODEELEMENTNAMES_CASTSHADOW,
	NODEELEMENTNAMES_LIGHTPOWER,
	NODEELEMENTNAMES_MINRADIUS,
	NODEELEMENTNAMES_MAXRADIUS,
	NODEELEMENTNAMES_LIGHTFALLOFF,

	NODEELEMENTNAMES_MAX
};

const hChar* gNodeElementNames [] = 
{
	"meshinstances",
	"meshinstance",
	"node",
	"name",
	"matrix",
	"globalmatrix",
	"meshes",
	"mesh",
	"meshindex",
	"sceneid",
	"cameradata",
	"position",
	"at",
	"up",
	"lightdata",
	"type",
	"direction",
	"attenuation",
	"diffuse",
	"specular",
	"ambient",
	"emissive",
	"lightcone",
	"castshadow",
	"lightpower",
	"minradius",
	"maxradius",
	"falloff",
};

hUint32	GetNodeTagFromString( const hChar* string, const hChar** tagNames )
{
	for ( hUint32 i = 0; i < NODEELEMENTNAMES_MAX; ++i )
	{
		if ( strcmp( string, tagNames[i] ) == 0 )
		{
			return i;
		}
	}

	return NODEELEMENTNAMES_MAX;
}

struct BaseNode
{
	static const hUint32	MAX_NAME_SIZE = 256;

	NodeType				typeID_;
	hChar 					name_[MAX_NAME_SIZE];
	Heart::Math::Matrix		localMatrix_;
	Heart::Math::Matrix		globalMatrix_;
	hUint32					nodeIndex_;

	hUint32					castShadow_;

	std::vector< BaseNode* >	children_;
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

struct MeshCollection : public BaseNode
{
	MeshCollectionInfo*		pCollectionInfo_;
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

struct Light : public BaseNode
{
	LightInfo*				pLightInfo_;
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

struct Camera : public BaseNode
{
	CameraInfo*				pCameraInfo_;
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

struct Locator : public BaseNode
{

};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

SceneBuilder::SceneBuilder() :
	pMeshCollection_( NULL ),
	pCameraInfo_( NULL ),
	pLightInfo_( NULL )
{

}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

SceneBuilder::~SceneBuilder()
{

}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

hBool SceneBuilder::BuildResource()
{
	// Create a dummy node on the stack to have as a root node
	// makes life simpler when parsing multiple root nodes.
	// This node won't be exported, end format can deal with multiple roots.
	pRootNode_ = new BaseNode;
	pRootNode_->typeID_ = NODETYPE_MAX;
	strcpy_s( pRootNode_->name_, BaseNode::MAX_NAME_SIZE, "DUMMY" );
	Heart::Math::Matrix::identity( &pRootNode_->localMatrix_ );
	Heart::Math::Matrix::identity( &pRootNode_->globalMatrix_ );
	nodeStack_.push( pRootNode_ );

	// Clear stats, we use these later to calculate size needed for 
	// nodes in output data.
	stats_.totalNodes_ = 0;
	stats_.totalLocatorNodes_ = 0;
	stats_.totalCameraNodes_ = 0;
	stats_.totalMeshNodes_ = 0;
	stats_.totalLightNodes_ = 0;

	// Parse the XML
	sceneDef_.SetCondenseWhiteSpace( false );
	if ( sceneDef_.LoadFile( inputFilename_.c_str(), TIXML_ENCODING_UTF8 ) )
	{
		sceneDef_.Accept( this );
	}
	else
	{
		ThrowFatalError( "Scene Builder could not parse input scene xml %s.Error:"
			"%s(line:%d column:%d)", inputFilename_.c_str(), sceneDef_.ErrorDesc(), sceneDef_.ErrorRow(), sceneDef_.ErrorCol() );
	}

	// Create a scene node bundle to export to output data
	ResourceDataPtr< Heart::Scene::SceneBundle > bundle;
	setBufferSize( 0, sizeof( Heart::Scene::SceneBundle ) );

	getDataFromBuffer( sizeof( Heart::Scene::SceneBundle ), bundle );
	
	ResourceDataPtr< Heart::Core::ResourceHandle< Heart::Scene::SceneNodeBase > > nodes;
	bundle->nNodes_ = stats_.totalNodes_;
	bundle->pNodes_ = getDataFromBuffer( sizeof( Heart::Core::ResourceHandle< Heart::Scene::SceneNodeBase > )*stats_.totalNodes_, nodes );
	//TODO: Build the children link up array
	ResourceDataPtr< Heart::Scene::SceneBundle::ChildLink > links;
	bundle->pChildLinks_ = getDataFromBuffer( sizeof(Heart::Scene::SceneBundle::ChildLink)*stats_.totalNodes_, links );

	hUint32 index = 0;
	AddNodesDeps( pRootNode_, &nodes, &links, &index );

	//TODO: Mesh deps
	ResourceDataPtr< Heart::Core::ResourceHandle< Heart::Render::Mesh > > meshes;
	bundle->nMeshes_ = meshInstanceFilenames_.size();
	bundle->pMeshes_ = getDataFromBuffer( sizeof(Heart::Core::ResourceHandle< Heart::Render::Mesh >)*meshInstanceFilenames_.size(), meshes );

	for ( hUint32 i = 0; i < meshInstanceFilenames_.size(); ++i )
	{
		hChar tbuf[1024];
		hUint32 resId;

		sprintf_s( tbuf, 1024, "mesh%03d", i );
		GameResource* pRes = AddDependency( tbuf, meshInstanceFilenames_[i], "mesh", resId );

		meshes.GetElement( i ).SetResID( resId );
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void SceneBuilder::AddNodesDeps( 
	BaseNode* pNode, 
	ResourceDataPtr< Heart::Core::ResourceHandle< Heart::Scene::SceneNodeBase > >* nodes,
	ResourceDataPtr< Heart::Scene::SceneBundle::ChildLink >* links,
	hUint32* index )
{
	if ( pNode->typeID_ != NODETYPE_MAX )
	{
		links->GetElement( *index ).nMeshes_ = 0;
		links->GetElement( *index ).pMeshIdx_ = NULL;
	}

	switch ( pNode->typeID_ )
	{
	case NODETYPE_LOCATOR:
		{
			hChar tbuf[512];
			sprintf_s( tbuf, 512, "node%03d", *index );
			std::string nodeFile = GetResourceDependenciesPath();
			nodeFile += "/";
			nodeFile += tbuf;
			nodeFile += ".dat";
			hUint32 nChildren = pNode->children_.size();
			ResourceFile* pFile = pResourceFileSystem()->OpenFile( nodeFile.c_str(), FILEMODE_WRITE );
			pFile->Write( pNode->name_, BaseNode::MAX_NAME_SIZE );
			pFile->Write( pNode->localMatrix_.m, sizeof( pNode->localMatrix_.m ) );
			pFile->Write( pNode->globalMatrix_.m, sizeof( pNode->globalMatrix_.m ) );
			pFile->Write( &pNode->castShadow_, sizeof( pNode->castShadow_ ) );

			pResourceFileSystem()->CloseFile( pFile );

			hUint32 resId;
			GameResource* pRes = AddDependency( tbuf, nodeFile, "locator", resId );
			nodes->GetElement( *index ).SetResID( resId );
		}
		break;
	case NODETYPE_LIGHT:
		{
			hChar tbuf[512];
			sprintf_s( tbuf, 512, "node%03d", *index );
			std::string nodeFile = GetResourceDependenciesPath();
			nodeFile += "/";
			nodeFile += tbuf;
			nodeFile += ".dat";
			hUint32 nChildren = pNode->children_.size();
			ResourceFile* pFile = pResourceFileSystem()->OpenFile( nodeFile.c_str(), FILEMODE_WRITE );
			pFile->Write( pNode->name_, BaseNode::MAX_NAME_SIZE );
			pFile->Write( pNode->localMatrix_.m, sizeof( pNode->localMatrix_.m ) );
			pFile->Write( pNode->globalMatrix_.m, sizeof( pNode->globalMatrix_.m ) );
			pFile->Write( &pNode->castShadow_, sizeof( pNode->castShadow_ ) );

			//Write light attributes
			Light* pLNode = (Light*)(pNode);

			pFile->Write( &pLNode->pLightInfo_->type_,					sizeof( pLNode->pLightInfo_->type_ ) );
			pFile->Write( &pLNode->pLightInfo_->constAttenuation_,		sizeof( pLNode->pLightInfo_->constAttenuation_ ) );
			pFile->Write( &pLNode->pLightInfo_->linearAttenuation_,		sizeof( pLNode->pLightInfo_->linearAttenuation_ ) );
			pFile->Write( &pLNode->pLightInfo_->quadraticAttenuation_,	sizeof( pLNode->pLightInfo_->quadraticAttenuation_ ) );
			pFile->Write( &pLNode->pLightInfo_->innerCone_,				sizeof( pLNode->pLightInfo_->innerCone_ ) );
			pFile->Write( &pLNode->pLightInfo_->outerCone_,				sizeof( pLNode->pLightInfo_->outerCone_ ) );
			pFile->Write( &pLNode->pLightInfo_->diffuse_,				sizeof( pLNode->pLightInfo_->diffuse_ ) );
			pFile->Write( &pLNode->pLightInfo_->specular_,				sizeof( pLNode->pLightInfo_->specular_ ) );
			pFile->Write( &pLNode->pLightInfo_->ambient_,				sizeof( pLNode->pLightInfo_->ambient_ ) );
			pFile->Write( &pLNode->pLightInfo_->power_,					sizeof( pLNode->pLightInfo_->power_ ) );
			pFile->Write( &pLNode->pLightInfo_->minRadius_,				sizeof( pLNode->pLightInfo_->minRadius_ ) );
			pFile->Write( &pLNode->pLightInfo_->maxRadius_,				sizeof( pLNode->pLightInfo_->maxRadius_ ) );
			pFile->Write( &pLNode->pLightInfo_->lightFalloff_,			sizeof( pLNode->pLightInfo_->lightFalloff_ ) );

			pResourceFileSystem()->CloseFile( pFile );

			hUint32 resId;
			GameResource* pRes = AddDependency( tbuf, nodeFile, "light", resId );
			nodes->GetElement( *index ).SetResID( resId );
		}
		break;
	case NODETYPE_CAMERA:
		{
			hChar tbuf[512];
			sprintf_s( tbuf, 512, "node%03d", *index );
			std::string nodeFile = GetResourceDependenciesPath();
			nodeFile += "/";
			nodeFile += tbuf;
			nodeFile += ".dat";
			hUint32 nChildren = pNode->children_.size();
			ResourceFile* pFile = pResourceFileSystem()->OpenFile( nodeFile.c_str(), FILEMODE_WRITE );
			pFile->Write( pNode->name_, BaseNode::MAX_NAME_SIZE );
			pFile->Write( pNode->localMatrix_.m, sizeof( pNode->localMatrix_.m ) );
			pFile->Write( pNode->globalMatrix_.m, sizeof( pNode->globalMatrix_.m ) );
			pFile->Write( &pNode->castShadow_, sizeof( pNode->castShadow_ ) );

			//Write camera attributes
			Camera* pCNode = (Camera*)(pNode);

			pFile->Write( &pCNode->pCameraInfo_->position_, sizeof( pCNode->pCameraInfo_->position_ ) );
			pFile->Write( &pCNode->pCameraInfo_->at_,		sizeof( pCNode->pCameraInfo_->at_ ) );
			pFile->Write( &pCNode->pCameraInfo_->up_,		sizeof( pCNode->pCameraInfo_->up_ ) );
			pFile->Write( &pCNode->pCameraInfo_->fov_,		sizeof( pCNode->pCameraInfo_->fov_ ) );
			pFile->Write( &pCNode->pCameraInfo_->near_,		sizeof( pCNode->pCameraInfo_->near_ ) );
			pFile->Write( &pCNode->pCameraInfo_->far_,		sizeof( pCNode->pCameraInfo_->far_ ) );
			pFile->Write( &pCNode->pCameraInfo_->aspect_,	sizeof( pCNode->pCameraInfo_->aspect_ ) );

			pResourceFileSystem()->CloseFile( pFile );

			hUint32 resId;
			GameResource* pRes = AddDependency( tbuf, nodeFile, "camera", resId );
			nodes->GetElement( *index ).SetResID( resId );
		}
		break;
	case NODETYPE_MESHCOLLECTION:
		{
			hChar tbuf[512];
			sprintf_s( tbuf, 512, "node%03d", *index );
			std::string nodeFile = GetResourceDependenciesPath();
			nodeFile += "/";
			nodeFile += tbuf;
			nodeFile += ".dat";
			hUint32 nChildren = pNode->children_.size();
			ResourceFile* pFile = pResourceFileSystem()->OpenFile( nodeFile.c_str(), FILEMODE_WRITE );
			pFile->Write( pNode->name_, BaseNode::MAX_NAME_SIZE );
			pFile->Write( pNode->localMatrix_.m, sizeof( pNode->localMatrix_.m ) );
			pFile->Write( pNode->globalMatrix_.m, sizeof( pNode->globalMatrix_.m ) );
			pFile->Write( &pNode->castShadow_, sizeof( pNode->castShadow_ ) );

			//Write mesh attributes
			MeshCollection* pMNode = (MeshCollection*)(pNode);
			
			hUint32 nMesh = pMNode->pCollectionInfo_->meshes_.size();

			ResourceDataPtr< hUint32 > meshidx;
			links->GetElement( *index ).nMeshes_ = nMesh;
			links->GetElement( *index ).pMeshIdx_ = getDataFromBuffer( sizeof(hUint32)*nMesh, meshidx );

			pFile->Write( &nMesh, sizeof( nMesh ) );
			for ( hUint32 i = 0; i < nMesh; ++i )
			{
				hUint32 idx = FindMeshIndex( pMNode->pCollectionInfo_->meshes_[i] );
				pFile->Write( &idx, sizeof( idx ) );
				meshidx.GetElement( i ) = idx;
			}

			pResourceFileSystem()->CloseFile( pFile );

			hUint32 resId;
			GameResource* pRes = AddDependency( tbuf, nodeFile, "meshcollection", resId );
			nodes->GetElement( *index ).SetResID( resId );
		}
		break;
	case NODETYPE_MAX:
		break;
	default:
		ThrowFatalError( "Unknown Node Type in Scene (id:%d)", pNode->typeID_ );
		break;
	}

	hUint32 writtenIndex = *index;
	if ( pNode->typeID_ != NODETYPE_MAX )
	{
		++(*index);
	}

	//do children
	for ( hUint32 i = 0; i < pNode->children_.size(); ++i )
	{
		AddNodesDeps( pNode->children_[i], nodes, links, index );
	}

	if ( pNode->typeID_ != NODETYPE_MAX )
	{
		//Map the children links
		ResourceDataPtr< hUint32 > childLinks;
		links->GetElement( writtenIndex ).nChildren_ = pNode->children_.size();
		links->GetElement( writtenIndex ).pChildren_ = NULL;
		if ( pNode->children_.size() )
		{
			links->GetElement( writtenIndex ).pChildren_ = getDataFromBuffer( sizeof( hUint32 )*pNode->children_.size(), childLinks );

			for ( hUint32 i = 0; i < pNode->children_.size(); ++i )
			{
				childLinks.GetElement( i ) = pNode->children_[i]->nodeIndex_;
			}
		}
	}

}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

hUint32 SceneBuilder::FindMeshIndex( const MeshInfo& mesh )
{
	hChar fbuf[1024];
	sprintf_s( fbuf, 1024, "mesh_%03d.xml", mesh.meshID_ );
	for ( hUint32 i = 0; i < meshInstanceFilenames_.size(); ++i )
	{
		if ( strstr( meshInstanceFilenames_[i].c_str(), fbuf ) )
		{
			return i;
		}
	}

	ThrowFatalError( "Can't find mesh in Mesh Collection" );

	return ~0U;
}


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

bool SceneBuilder::VisitEnter( const TiXmlElement& element, const TiXmlAttribute* firstAttribute )
{
	switch( GetNodeTagFromString( element.Value(), gNodeElementNames ) )
	{
	case NODEELEMENTNAMES_NODE:
		{
			BaseNode* pNewNode = NULL;
			const hChar* type = element.Attribute( "type" );
			if ( type )
			{
				if ( strcmp( type, "Light" ) == 0 )
				{
					pNewNode = new Light;
					pNewNode->typeID_ = NODETYPE_LIGHT;
					++stats_.totalLightNodes_;
				}
				else if ( strcmp( type, "Locator" ) == 0 )
				{
					pNewNode = new Locator;
					pNewNode->typeID_ = NODETYPE_LOCATOR;
					++stats_.totalLocatorNodes_;
				}
				else if ( strcmp( type, "Camera" ) == 0 )
				{
					pNewNode = new Camera;
					pNewNode->typeID_ = NODETYPE_CAMERA;
					++stats_.totalCameraNodes_;
				}
				else if ( strcmp( type, "MeshCollection" ) == 0 )
				{
					pNewNode = new MeshCollection;
					pNewNode->typeID_ = NODETYPE_MESHCOLLECTION;
					++stats_.totalMeshNodes_;
				}
				else 
				{
					ThrowFatalError( "Found Node in Scene with Unkown type" );
				}
			}
			else
			{
				ThrowFatalError( "Couldn't Find Type Attribute for node" );
			}

			nodeStack_.top()->children_.push_back( pNewNode );
			nodeStack_.push( pNewNode );
			pNewNode->nodeIndex_ = stats_.totalNodes_++;
		}
		break;
	case NODEELEMENTNAMES_NAME:
		{
			if ( element.GetText() )
			{
				strcpy_s( nodeStack_.top()->name_, BaseNode::MAX_NAME_SIZE, element.GetText() );
			}
			else
			{
				ThrowFatalError( "Found Name Element but no name text" );
			}
		}
		break;
	case NODEELEMENTNAMES_MATRIX:
		{
			if ( element.GetText() )
			{
				float* m = nodeStack_.top()->localMatrix_.m;
				if ( sscanf_s( 
						element.GetText(), 
						" ( %f , %f , %f , %f ) "
						"( %f , %f , %f , %f ) "
						"( %f , %f , %f , %f ) "
						"( %f , %f , %f , %f ) ",
						&m[0] ,&m[1] ,&m[2] ,&m[3],
						&m[4] ,&m[5] ,&m[6] ,&m[7],
						&m[8] ,&m[9] ,&m[10],&m[11],
						&m[12],&m[13],&m[14],&m[15] ) != 16 )
				{
					ThrowFatalError( "Failed to parse Matrix Elements from matrix string" );
				}
			}
			else
			{
				ThrowFatalError( "Found Matrix Element but no matrix numbers" );
			}
		}
		break;
	case NODEELEMENTNAMES_CASTSHADOW:
		{
			if ( element.GetText() )
			{
				hUint32 c = atoi( element.GetText() );
				nodeStack_.top()->castShadow_ = c > 0;
			}
		}
		break;
	case NODEELEMENTNAMES_GLOBALMATRIX:
		{
			if ( element.GetText() )
			{
				float* m = nodeStack_.top()->globalMatrix_.m;
				if ( sscanf_s( 
					element.GetText(), 
					" ( %f , %f , %f , %f ) "
					"( %f , %f , %f , %f ) "
					"( %f , %f , %f , %f ) "
					"( %f , %f , %f , %f ) ",
					&m[0] ,&m[1] ,&m[2] ,&m[3],
					&m[4] ,&m[5] ,&m[6] ,&m[7],
					&m[8] ,&m[9] ,&m[10],&m[11],
					&m[12],&m[13],&m[14],&m[15] ) != 16 )
				{
					ThrowFatalError( "Failed to parse Matrix Elements from matrix string" );
				}
			}
			else
			{
				ThrowFatalError( "Found Matrix Element but no matrix numbers" );
			}
		}
		break;
	case NODEELEMENTNAMES_MESHES:
		{
			if ( pMeshCollection_ != NULL )
			{
				ThrowFatalError( "Found meshes node (name: %s) within another meshes node",  (const char*)element.Value() );
			}

			pMeshCollection_ = new MeshCollectionInfo;
		}
		break;
	default:
		break;
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

bool SceneBuilder::VisitExit( const TiXmlElement& element )
{
	switch( GetNodeTagFromString( element.Value(), gNodeElementNames ) )
	{
	case NODEELEMENTNAMES_NODE:
		{
			const hChar* type = element.Attribute( "type" );
			if ( type )
			{
				if ( strcmp( type, "Light" ) == 0 )
				{
					if ( pLightInfo_ == NULL )
					{
						ThrowFatalError( "Found Light node without light information" );
					}

					Light* pnode = (Light*)nodeStack_.top();
					pnode->pLightInfo_ = pLightInfo_;
					pLightInfo_ = NULL;
				}
				else if ( strcmp( type, "Locator" ) == 0 )
				{
				}
				else if ( strcmp( type, "Camera" ) == 0 )
				{
					if ( pCameraInfo_ == NULL )
					{
						ThrowFatalError( "Found camera node without camera data" );
					}

					Camera* pnode = (Camera*)nodeStack_.top();
					pnode->pCameraInfo_ = pCameraInfo_;
					pCameraInfo_ = NULL;
				}
				else if ( strcmp( type, "MeshCollection" ) == 0 )
				{
				}
				else 
				{
					ThrowFatalError( "Found Node in Scene with Unkown type" );
				}
			}

			nodeStack_.pop();

			//Dont want to push these elements to the element stack
			return true;
		}
		break;
	case NODEELEMENTNAMES_MESHES:
		{
			if ( pMeshCollection_ == NULL )
			{
				ThrowFatalError( "Found mesh Collectoin node without mesh Collection" );
			}

			MeshCollection* pnode = (MeshCollection*)nodeStack_.top();
			pnode->pCollectionInfo_ = pMeshCollection_;
			pMeshCollection_ = NULL;
		}
		break;
	case NODEELEMENTNAMES_MESH:
		{
			if ( pMeshCollection_ == NULL )
			{
				ThrowFatalError( "Found mesh node (name: %s) outside meshes node",  (const char*)element.Value() );
			}

			MeshInfo newInfo;

			while ( !elementStack_.empty() )
			{
				const TiXmlElement* ptop = elementStack_.top();
				switch( GetNodeTagFromString( ptop->Value(), gNodeElementNames ) )
				{
				case NODEELEMENTNAMES_MESHINDEX:
					if ( sscanf_s( ptop->GetText(), " %d ", &newInfo.meshID_ ) != 1 )
					{
						ThrowFatalError( "Couldn't Parse Mesh Index for mesh" );
					}
					break;
				}

				elementStack_.pop();
			}

			pMeshCollection_->meshes_.push_back( newInfo );
			//Dont push ourselves to the element stack, no-one will process us :(
			return true;
		}
	case NODEELEMENTNAMES_CAMERADATA:
		{
			if ( pCameraInfo_ != NULL )
			{
				ThrowFatalError( "Found camera info node (name: %s) in an invalid place!",  (const char*)element.Value() );
			}

			pCameraInfo_ = new CameraInfo;

			if ( element.QueryFloatAttribute( "fov", &pCameraInfo_->fov_ ) != TIXML_SUCCESS )
			{
				ThrowFatalError( "Can't Find Camera FOV" );
			}

			if ( element.QueryFloatAttribute( "near", &pCameraInfo_->near_ ) != TIXML_SUCCESS )
			{
				ThrowFatalError( "Can't Find Camera near plane" );
			}

			if ( element.QueryFloatAttribute( "far", &pCameraInfo_->far_ ) != TIXML_SUCCESS )
			{
				ThrowFatalError( "Can't Find Camera far plane" );
			}

			if ( element.QueryFloatAttribute( "aspect", &pCameraInfo_->aspect_ ) != TIXML_SUCCESS )
			{
				ThrowFatalError( "Can't Find Camera aspect ratio" );
			}

			while ( !elementStack_.empty() )
			{
				const TiXmlElement* ptop = elementStack_.top();
				switch( GetNodeTagFromString( ptop->Value(), gNodeElementNames ) )
				{
				case NODEELEMENTNAMES_POSITION:
					if ( sscanf_s( ptop->GetText(), " %f , %f , %f ", &pCameraInfo_->position_.x, &pCameraInfo_->position_.y, &pCameraInfo_->position_.z ) != 3 )
					{
						ThrowFatalError( "Couldn't Parse Mesh Index for mesh" );
					}
					break;
				case NODEELEMENTNAMES_AT:
					if ( sscanf_s( ptop->GetText(), " %f , %f , %f ", &pCameraInfo_->at_.x, &pCameraInfo_->at_.y, &pCameraInfo_->at_.z ) != 3 )
					{
						ThrowFatalError( "Couldn't Parse Mesh Index for mesh" );
					}
					break;
				case NODEELEMENTNAMES_UP:
					if ( sscanf_s( ptop->GetText(), " %f , %f , %f ", &pCameraInfo_->up_.x, &pCameraInfo_->up_.y, &pCameraInfo_->up_.z ) != 3 )
					{
						ThrowFatalError( "Couldn't Parse Mesh Index for mesh" );
					}
					break;
				}

				elementStack_.pop();
			}
			//Dont push ourselves to the element stack, no-one will process us :(
			return true;
		}
	case NODEELEMENTNAMES_LIGHTDATA:
		{
			if ( pLightInfo_ != NULL )
			{
				ThrowFatalError( "Found light info node (name: %s) in an invalid place!",  (const char*)element.Value() );
			}

			pLightInfo_ = new LightInfo;

			while ( !elementStack_.empty() )
			{
				const TiXmlElement* ptop = elementStack_.top();
				switch( GetNodeTagFromString( ptop->Value(), gNodeElementNames ) )
				{
				case NODEELEMENTNAMES_TYPE:
					if ( ptop->GetText() && strcmp( ptop->GetText(), "LIGHTSOURCETYPE_DIRECTION" ) == 0 )
					{
						pLightInfo_->type_ = Heart::Render::LIGHTSOURCETYPE_DIRECTION;
					}
					else if ( ptop->GetText() && strcmp( ptop->GetText(), "LIGHTSOURCETYPE_SPOT" ) == 0 )
					{
						pLightInfo_->type_ = Heart::Render::LIGHTSOURCETYPE_SPOT;
					}
					else if ( ptop->GetText() && strcmp( ptop->GetText(), "LIGHTSOURCETYPE_POINT" ) == 0 )
					{
						pLightInfo_->type_ = Heart::Render::LIGHTSOURCETYPE_POINT;
					}
					else 
					{
						ThrowFatalError( "Unknown light type %s", ptop->GetText() );
					}
					break;
				case NODEELEMENTNAMES_ATTENUATION:
					if ( ptop->QueryIntAttribute( "constant", (int*)&pLightInfo_->constAttenuation_ ) != TIXML_SUCCESS )
					{
						pLightInfo_->constAttenuation_ = 0;
					}
					if ( ptop->QueryIntAttribute( "linear", (int*)&pLightInfo_->linearAttenuation_ ) != TIXML_SUCCESS )
					{
						pLightInfo_->linearAttenuation_ = 0;
					}
					if ( ptop->QueryIntAttribute( "quadratic", (int*)&pLightInfo_->quadraticAttenuation_ ) != TIXML_SUCCESS )
					{
						pLightInfo_->quadraticAttenuation_ = 0;
					}
					break;
				case NODEELEMENTNAMES_DIFFUSE:
					if ( ptop->QueryFloatAttribute( "red", &pLightInfo_->diffuse_.r_ ) != TIXML_SUCCESS )
					{
						ThrowFatalError( "diffuse colour missing red" );
					}
					if ( ptop->QueryFloatAttribute( "green", &pLightInfo_->diffuse_.g_ ) != TIXML_SUCCESS )
					{
						ThrowFatalError( "diffuse colour missing red" );
					}
					if ( ptop->QueryFloatAttribute( "blue", &pLightInfo_->diffuse_.b_ ) != TIXML_SUCCESS )
					{
						ThrowFatalError( "diffuse colour missing red" );
					}
					if ( ptop->QueryFloatAttribute( "alpha", &pLightInfo_->diffuse_.a_ ) != TIXML_SUCCESS )
					{
						ThrowFatalError( "diffuse colour missing red" );
					}
					break;
				case NODEELEMENTNAMES_AMBIENT:
					if ( ptop->QueryFloatAttribute( "red", &pLightInfo_->ambient_.r_ ) != TIXML_SUCCESS )
					{
						ThrowFatalError( "diffuse colour missing red" );
					}
					if ( ptop->QueryFloatAttribute( "green", &pLightInfo_->ambient_.g_ ) != TIXML_SUCCESS )
					{
						ThrowFatalError( "diffuse colour missing red" );
					}
					if ( ptop->QueryFloatAttribute( "blue", &pLightInfo_->ambient_.b_ ) != TIXML_SUCCESS )
					{
						ThrowFatalError( "diffuse colour missing red" );
					}
					if ( ptop->QueryFloatAttribute( "alpha", &pLightInfo_->ambient_.a_ ) != TIXML_SUCCESS )
					{
						ThrowFatalError( "diffuse colour missing red" );
					}
					break;
				case NODEELEMENTNAMES_SPECULAR:
					if ( ptop->QueryFloatAttribute( "red", &pLightInfo_->specular_.r_ ) != TIXML_SUCCESS )
					{
						ThrowFatalError( "diffuse colour missing red" );
					}
					if ( ptop->QueryFloatAttribute( "green", &pLightInfo_->specular_.g_ ) != TIXML_SUCCESS )
					{
						ThrowFatalError( "diffuse colour missing red" );
					}
					if ( ptop->QueryFloatAttribute( "blue", &pLightInfo_->specular_.b_ ) != TIXML_SUCCESS )
					{
						ThrowFatalError( "diffuse colour missing red" );
					}
					if ( ptop->QueryFloatAttribute( "alpha", &pLightInfo_->specular_.a_ ) != TIXML_SUCCESS )
					{
						ThrowFatalError( "diffuse colour missing red" );
					}
					break;
				case NODEELEMENTNAMES_LIGHTCONE:
					if ( ptop->QueryFloatAttribute( "innerangle", &pLightInfo_->innerCone_ ) != TIXML_SUCCESS )
					{
						ThrowFatalError( "light missing inner angle " );
					}
					if ( ptop->QueryFloatAttribute( "outerangle", &pLightInfo_->outerCone_ ) != TIXML_SUCCESS )
					{
						ThrowFatalError( "light missing outer angle " );
					}
					break;
				case NODEELEMENTNAMES_LIGHTPOWER:
					{
						if ( ptop->GetText() )
						{
							pLightInfo_->power_ = (hFloat)atof( ptop->GetText() );
						}
					}
					break;
				case NODEELEMENTNAMES_MINRADIUS:
					{
						if ( ptop->GetText() )
						{
							pLightInfo_->minRadius_ = (hFloat)atof( ptop->GetText() );
						}
					}
					break;
				case NODEELEMENTNAMES_MAXRADIUS:
					{
						if ( ptop->GetText() )
						{
							pLightInfo_->maxRadius_ = (hFloat)atof( ptop->GetText() );
						}
					}
					break;
				case NODEELEMENTNAMES_LIGHTFALLOFF:
					{
						if ( ptop->GetText() )
						{
							pLightInfo_->lightFalloff_ = (hFloat)atof( ptop->GetText() );
						}
					}
					break;
				}

				elementStack_.pop();
			}

			//Dont push ourselves to the element stack, no-one will process us :(
			return true;
		}
	case NODEELEMENTNAMES_MESHINSTANCES:
		{
			meshInstanceFilenames_.reserve( elementStack_.size() );
			while ( !elementStack_.empty() )
			{
				const TiXmlElement* ptop = elementStack_.top();
				switch( GetNodeTagFromString( ptop->Value(), gNodeElementNames ) )
				{
				case NODEELEMENTNAMES_MESHINSTANCE:
					meshInstanceFilenames_.push_back( ptop->GetText() );
					break;
				}

				elementStack_.pop();
			}

			std::reverse( meshInstanceFilenames_.begin(), meshInstanceFilenames_.end() );
			//Dont push ourselves to the element stack, no-one will process us :(
			return true;
		}
	case NODEELEMENTNAMES_MESHINDEX:
	case NODEELEMENTNAMES_SCENEINDEX:
	case NODEELEMENTNAMES_TYPE:
	case NODEELEMENTNAMES_POSITION:
	case NODEELEMENTNAMES_DIRECTION:
	case NODEELEMENTNAMES_ATTENUATION:
	case NODEELEMENTNAMES_DIFFUSE:
	case NODEELEMENTNAMES_SPECULAR:
	case NODEELEMENTNAMES_AMBIENT:
	case NODEELEMENTNAMES_LIGHTCONE:
	case NODEELEMENTNAMES_AT:
	case NODEELEMENTNAMES_UP:
	case NODEELEMENTNAMES_MESHINSTANCE:
	case NODEELEMENTNAMES_CASTSHADOW:
	case NODEELEMENTNAMES_LIGHTFALLOFF:
	case NODEELEMENTNAMES_LIGHTPOWER:
	case NODEELEMENTNAMES_MINRADIUS:
	case NODEELEMENTNAMES_MAXRADIUS:
		break;
	default:
		// for nodes we dont know don't push them into the stack
		return true;
	}

	elementStack_.push( &element );
	return true;
}
