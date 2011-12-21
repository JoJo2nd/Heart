/********************************************************************

	filename: 	SceneGraph.cpp	
	
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
#include "SceneGraph.h"
#include "assimp.h"
#include "aiScene.h"
#include "aiLight.h"
#include "aiCamera.h"
#include "SceneNodeLight.h"
#include "SceneNodeCamera.h"
#include "SceneNodeMeshCollection.h"
#include "SceneNodeLocator.h"
#include "LogWindow.h"
#include "SceneDatabase.h"
#include "DataMesh.h"
#include "DataLight.h"
#include "DataCamera.h"
#include "tinyxml.h"

namespace HScene
{
namespace 
{
	void RecursiveAddNodes( SceneGraph* pScene, const aiScene* aiScene, aiNode* pAiNode, Node* parent, const std::vector<int>& meshMap )
	{
		Node* pNode = NULL;
		//determine what kind of pAiNode this is
		aiLight* plight = NULL;
		aiCamera* pcamera = NULL;
		aiBone* pbone = NULL;

		for ( u32 i = 0; i < aiScene->mNumLights; ++i )
		{
			aiLight* ptmplight = aiScene->mLights[i];
			if ( strcmp( ptmplight->mName.data, pAiNode->mName.data ) == 0 )
			{
				plight = ptmplight;
				break;
			}
		}
		for ( u32 i = 0; i < aiScene->mNumCameras; ++i )
		{
			aiCamera* ptmpcam = aiScene->mCameras[i];
			if ( strcmp( ptmpcam->mName.data, pAiNode->mName.data ) == 0 )
			{
				pcamera = ptmpcam;
				break;
			}
		}

		D3DXMATRIX nodeMatrix;

		nodeMatrix._11 = pAiNode->mTransformation.a1;
		nodeMatrix._21 = pAiNode->mTransformation.a2;
		nodeMatrix._31 = pAiNode->mTransformation.a3;
		nodeMatrix._41 = pAiNode->mTransformation.a4;
		nodeMatrix._12 = pAiNode->mTransformation.b1;
		nodeMatrix._22 = pAiNode->mTransformation.b2;
		nodeMatrix._32 = pAiNode->mTransformation.b3;
		nodeMatrix._42 = pAiNode->mTransformation.b4;
		nodeMatrix._13 = pAiNode->mTransformation.c1;
		nodeMatrix._23 = pAiNode->mTransformation.c2;
		nodeMatrix._33 = pAiNode->mTransformation.c3;
		nodeMatrix._43 = pAiNode->mTransformation.c4;
		nodeMatrix._14 = pAiNode->mTransformation.d1;
		nodeMatrix._24 = pAiNode->mTransformation.d2;
		nodeMatrix._34 = pAiNode->mTransformation.d3;
		nodeMatrix._44 = pAiNode->mTransformation.d4;

		if ( plight )
		{
			Light* pTNode = new Light();
			//pTNode->SetSceneInfo( aiScene, lightIndex );
			
			pTNode->lightType_ = (LightType)(plight->mType-1);
			pTNode->brightness_ = 1.0f;
			pTNode->direction_.x = plight->mDirection.x;
			pTNode->direction_.y = plight->mDirection.y;
			pTNode->direction_.z = plight->mDirection.z;
			pTNode->attenuation_ = plight->mAttenuationConstant;
			pTNode->attenuationLinear_ = plight->mAttenuationLinear;
			pTNode->attenuationQuadratic_ = plight->mAttenuationQuadratic;
			pTNode->innerCone_ = plight->mAngleInnerCone;
			pTNode->outerCone_ = plight->mAngleOuterCone;
			pTNode->diffuse_.r = plight->mColorDiffuse.r;
			pTNode->diffuse_.g = plight->mColorDiffuse.g;
			pTNode->diffuse_.b = plight->mColorDiffuse.b;
			pTNode->diffuse_.a = 1.0f;
			pTNode->specular_.r = plight->mColorSpecular.r;
			pTNode->specular_.g = plight->mColorSpecular.g;
			pTNode->specular_.b = plight->mColorSpecular.b;
			pTNode->specular_.a = 1.0f;
			pTNode->ambient_.r = plight->mColorAmbient.r;
			pTNode->ambient_.g = plight->mColorAmbient.g;
			pTNode->ambient_.b = plight->mColorAmbient.b;
			pTNode->ambient_.a = 1.0f;


			switch ( pTNode->lightType_ )
			{
			case LightType_Direction:
				{
					//remove any scale
					D3DXMATRIX iScale;
					D3DXVECTOR3 xa( nodeMatrix._11, nodeMatrix._12, nodeMatrix._13 );
					D3DXVECTOR3 ya( nodeMatrix._21, nodeMatrix._22, nodeMatrix._23 );
					D3DXVECTOR3 za( nodeMatrix._31, nodeMatrix._32, nodeMatrix._33 );
					D3DXVECTOR3 t( nodeMatrix._41, nodeMatrix._42, nodeMatrix._43 );
					D3DXMatrixScaling( &iScale, 1.0f/D3DXVec3Length( &xa ), 1.0f/D3DXVec3Length( &ya ), 1.0f/D3DXVec3Length( &za ) );

					nodeMatrix = iScale * nodeMatrix;

					pTNode->direction_.x = nodeMatrix._13;
					pTNode->direction_.y = nodeMatrix._23;
					pTNode->direction_.z = nodeMatrix._33;
				}
				break;
			case LightType_Point:
				{
					D3DXMATRIX iScale;
					D3DXVECTOR3 xa( nodeMatrix._11, nodeMatrix._12, nodeMatrix._13 );
					D3DXVECTOR3 ya( nodeMatrix._21, nodeMatrix._22, nodeMatrix._23 );
					D3DXVECTOR3 za( nodeMatrix._31, nodeMatrix._32, nodeMatrix._33 );
					D3DXVECTOR3 t( nodeMatrix._41, nodeMatrix._42, nodeMatrix._43 );
					D3DXMatrixScaling( &iScale, 1.0f/D3DXVec3Length( &xa ), 1.0f/D3DXVec3Length( &ya ), 1.0f/D3DXVec3Length( &za ) );

					nodeMatrix = iScale * nodeMatrix;

					pTNode->minRadius_ = 0.0f;
					pTNode->maxRadius_ = plight->mAttenuationLinear;
				}
				break;
			case LightType_Spot:
				{
					//remove any scale
					D3DXMATRIX iScale;
					D3DXVECTOR3 xa( nodeMatrix._11, nodeMatrix._12, nodeMatrix._13 );
					D3DXVECTOR3 ya( nodeMatrix._21, nodeMatrix._22, nodeMatrix._23 );
					D3DXVECTOR3 za( nodeMatrix._31, nodeMatrix._32, nodeMatrix._33 );
					D3DXVECTOR3 t( nodeMatrix._41, nodeMatrix._42, nodeMatrix._43 );
					D3DXMatrixScaling( &iScale, 1.0f/D3DXVec3Length( &xa ), 1.0f/D3DXVec3Length( &ya ), 1.0f/D3DXVec3Length( &za ) );

					nodeMatrix = iScale * nodeMatrix;

					//Falloff is based on the length of z
					pTNode->falloff_ = D3DXVec3Length( &za );
				}
				break;
			}

			pNode = pTNode;
		}
		else if ( pcamera )
		{
			Camera* pTNode = new Camera();
			//pTNode->SetSceneInfo( aiScene, camIndex );
			pTNode->aspect_ = pcamera->mAspect;
			pTNode->nearPlane_ = pcamera->mClipPlaneNear;
			pTNode->farPlane_ = pcamera->mClipPlaneFar;
			pTNode->fov_ = pcamera->mHorizontalFOV;
			
			pNode = pTNode;
		}
		else if ( pAiNode->mNumMeshes > 0 )
		{
			MeshCollection* pTNode = new MeshCollection( pScene );
			for ( u32 i = 0; i < pAiNode->mNumMeshes; ++i )
			{
				u32 meshID = meshMap[pAiNode->mMeshes[i]];
				pTNode->AppendMesh( meshID );
			}
			pNode = pTNode;
		}
		else 
		{
			pNode = new Locator();
		}

		//TODO: Add params
		pNode->matrix_ = nodeMatrix;

		std::string newnodename = pAiNode->mName.data;
		u32 namecount = CountMatchingNames( pScene, newnodename.c_str() );;
		for ( u32 i = 1; namecount != 0; ++i )
		{
			char tmp[12];
			newnodename = pAiNode->mName.data;
			sprintf_s( tmp, 12, "_%03d", i );
			newnodename += tmp;
			namecount = CountMatchingNames( pScene, newnodename.c_str() );
		}

		pNode->pName( newnodename.c_str() );

		//TODO: Attach to parent
		if ( parent )
		{
			parent->AttachChild(pNode);
		}
		else
		{
			//This is the root element
			pScene->pRoot( pNode );
		}

		UI::Log::WriteLog( "Added Node %s (type: %s) to Scene Graph", pNode->pName(), pNode->NodeTypeName() );

		UI::Log::PushTabIndent();

		// Add Children
		for ( u32 i = 0; i < pAiNode->mNumChildren; ++i )
		{
			RecursiveAddNodes( pScene, aiScene, pAiNode->mChildren[i], pNode, meshMap );
		}

		UI::Log::PopTabIndent();
	}

}

	BEGIN_ENUM_NAMES( SceneData )
		ENUM_NAME( TextureInstance )
		ENUM_NAME( Material )
		ENUM_NAME( Mesh )

		//Common ID's
		ENUM_NAME( ID )
	END_ENUM_NAMES();

	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////

	u32 SceneGraph::ImportMeshToScene( aiMesh* mesh, const std::vector< int >& matMap )
	{
		Data::Mesh* nmesh = new Data::Mesh;
		u32 id = meshes_.size();
		Data::BuildFromaiMesh( id, mesh, nmesh, matMap );

		//check for duplicate names
		bool nameOk;
		std::string orig = nmesh->GetName();
		u32 meshid = 0;
		do 
		{
			nameOk = true;

			for ( u32 i = 0, e = meshes_.size(); i < e && nameOk; ++i )
			{
				if ( strcmp( meshes_[i]->GetName(), nmesh->GetName() ) == 0 )
					nameOk = false;
			}

			if ( !nameOk )
			{
				++meshid;
				char namebuf[ 1024 ];
				sprintf_s( namebuf, 1024, "%s_%03u", orig.c_str(), meshid );
				nmesh->SetName( namebuf );
			}
		} while( !nameOk );

		meshes_.push_back( nmesh );

		return id;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////

	u32 SceneGraph::ImportMaterialToScene( aiMaterial* material )
	{
		Data::Material* nmat = new Data::Material( this );
		u32 id = material_.size();
		Data::BuildFromaiMaterial( id, this, material, nmat );

		bool nameOk;
		std::string name = nmat->GetName();
		u32 matid = 0;
		do 
		{
			nameOk = true;

			for ( u32 i = 0, e = material_.size(); i < e && nameOk; ++i )
			{
				if ( strcmp( material_[i]->GetName(), nmat->GetName() ) == 0 )
					nameOk = false;
			}

			if ( !nameOk )
			{
				++matid;
				char namebuf[ 1024 ];
				sprintf_s( namebuf, 1024, "%s_%03u", name.c_str(), matid );
				nmat->SetName( namebuf );
			}
		} while( !nameOk );

		material_.push_back( nmat );

		return id;
	}

	//////////////////////////////////////////////////////////////////////////
	// 22:41:05 ////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	u32 SceneGraph::ImportTextureToScene( const char* filepath )
	{
		for (  u32 i = 0, e = textures_.size(); i < e; ++i )
		{
			if ( textures_[i].filename_ == filepath )
			{
				return i;
			}
		}

		u32 texId = textures_.size();
		Data::TextureInstance tex;
		tex.filename_ = filepath;
		tex.format_ = Data::TextureFormat_RGBA8;

		textures_.push_back( tex );

		return texId;
	}

	//////////////////////////////////////////////////////////////////////////
	// 20:49:50 ////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	bool SceneGraph::SerialiseNodes( Node* node, TiXmlElement* parent )
	{
		TiXmlElement* nodeele = new TiXmlElement( "node" );
		nodeele->SetAttribute( "type", node->NodeTypeName() );

		if ( !node->Serialise( nodeele ) )
			return false;

		for ( Node::ChildListType::const_iterator i = node->pChildren()->begin(), e = node->pChildren()->end(); i != e; ++i )
		{
			if ( !SerialiseNodes( *i, nodeele ) )
				return false;
		}

		parent->LinkEndChild( nodeele );

		return true;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////

	bool SceneGraph::Serialise( TiXmlElement* toelement )
	{
		XML_LINK_STRING( lastExportPath_, toelement );
		XML_LINK_INT( sceneKey_, toelement );
		//Textures first
		XML_LINK_INT_AS_STRING( "nTextures_", textures_.size(), toelement );
		for ( u32 i = 0, e = textures_.size(); i < e; ++i )
		{
			XML_LINK_OBJECT_ID( textures_, i, toelement );
		}

		//Save materials next
		XML_LINK_INT_AS_STRING( "nMaterials_", material_.size(), toelement );
		for ( u32 i = 0, e = material_.size(); i < e; ++i )
		{
			XML_LINK_PTR_OBJECT_ID( material_, i, toelement );
		}

		//Save Meshes now
		XML_LINK_INT_AS_STRING( "nMeshes_", meshes_.size(), toelement );
		for ( u32 i = 0, e = meshes_.size(); i < e; ++i )
		{
			XML_LINK_PTR_OBJECT_ID( meshes_, i, toelement );
		}

		//Save Nodes recursively
		if ( pRootNode_ )
		{
			SerialiseNodes( pRootNode_, toelement );
		}

		return true;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////

	bool SceneGraph::Deserialise( TiXmlElement* fromelement )
	{
		XML_UNLINK_STRING( lastExportPath_, fromelement );
		XML_UNLINK_INT( sceneKey_, fromelement );
		u32 count;
		XML_UNLINK_INT_FROM_ELEMENT( "nTextures_", count, fromelement );
		textures_.resize( count );
		XML_UNLINK_INT_FROM_ELEMENT( "nMaterials_", count, fromelement );
		material_.resize( count );
		XML_UNLINK_INT_FROM_ELEMENT( "nMeshes_", count, fromelement );
		meshes_.resize( count );

		XML_UNLINK_OBJECT_ID( textures_, fromelement );
		for ( u32 i = 0; i < material_.size(); ++i ) material_[i] = new Data::Material( this );
		XML_UNLINK_PTR_OBJECT_ID( material_, fromelement );
		for ( u32 i = 0; i < meshes_.size(); ++i ) meshes_[i] = new Data::Mesh();
		XML_UNLINK_PTR_OBJECT_ID( meshes_, fromelement );

		pRootNode_ = DeserialiseNodes( NULL, fromelement );

		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	// 23:44:12 ////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	Node* SceneGraph::DeserialiseNodes( Node* parent, TiXmlElement* fromelement )
	{
		Node* ret = NULL;
		for ( TiXmlElement* i = fromelement->FirstChildElement( "node" ); i ; i = i->NextSiblingElement( "node" ) )
		{
			const char* nodetype = i->Attribute( "type" );
			if ( !nodetype )
				return NULL;
			ret = CreateNode( nodetype );
			if ( ret )
			{
				ret->Deserialise( i );
				if ( parent )
				{
					parent->AttachChild( ret );
				}
				DeserialiseNodes( ret, i );
			}
		}

		return ret;
	}

	//////////////////////////////////////////////////////////////////////////
	// 23:55:10 ////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	Node* SceneGraph::CreateNode( const char* tname )
	{
		//it crap that these strings are hard coded here,
		//TODO: handle better
		if ( strcmp( tname, "Locator" ) == 0 ) 
		{
			return new Locator;
		}
		else if ( strcmp( tname, "MeshCollection" ) == 0 ) 
		{
			return new MeshCollection( this );
		}
		else if ( strcmp( tname, "Camera" ) == 0 ) 
		{
			return new Camera;
		}
		else if ( strcmp( tname, "Light" ) == 0 ) 
		{
			return new Light;
		}

		return NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	// 14:47:27 ////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void SceneGraph::ClearScene()
	{
		for ( u32 i = 0, e = material_.size(); i < e; ++i )
		{
			delete material_[i];
		}
		material_.clear();
		for ( u32 i = 0, e = meshes_.size(); i < e; ++i )
		{
			delete meshes_[i];
		}
		meshes_.clear();
		textures_.clear();

		delete pRootNode_;

		pRootNode_ = NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	bool ImportToScene( SceneGraph* pScene, const aiScene* aiScene, Node* pat )
	{
		UI::Log::WriteLog( "Importing aiScene to Scene Graph" );
		UI::Log::PushTabIndent();

		Node* proot = pat;
		if ( proot == NULL )
		{
			proot = pScene->pRoot();
		}

		//Import materials into scene, build a list of id's to match to aiScene id's
		std::vector< int > materialMap;
		for ( u32 i = 0; i < aiScene->mNumMaterials; ++i )
		{
			//BuildFromaiMaterial( paiScene, paiScene->mMaterials[i],  );
			materialMap.push_back( pScene->ImportMaterialToScene( aiScene->mMaterials[i] ) );
		}

		//Import meshes into scene, build a list of id's to match to aiScene id's
		std::vector< int > meshMap;
		for ( u32 i = 0; i < aiScene->mNumMeshes; ++i )
		{
			meshMap.push_back( pScene->ImportMeshToScene( aiScene->mMeshes[i], materialMap ) );
		}

		//Import animations into scene, build a list of id's to match to aiScene id's
		RecursiveAddNodes( pScene, aiScene, aiScene->mRootNode, proot, meshMap );

		UI::Log::PopTabIndent();

		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	Node* FindNode( Node* proot, const std::string& name )
	{
		if ( strcmp( name.c_str(), proot->pName() ) == 0 )
		{
			return proot;
		}

		for ( Node::ChildListType::const_iterator i = proot->pChildren()->begin(), iend = proot->pChildren()->end(); i != iend; ++i )
		{
			Node* ret = FindNode( *i, name.c_str() );
			if ( ret )
			{
				return ret;
			}
		}

		return NULL;
	}

	void RecurseVisitScene( Node* pParent, Node* pNode, VisitorBase* pvisitor )
	{
		if ( pParent )
		{
			pNode->globalMatrix_ = pNode->matrix_ * pParent->globalMatrix_;
		}
		else
		{
			pNode->globalMatrix_ = pNode->matrix_;
		}

		pNode->Accept( pvisitor );

		for ( Node::ChildListType::const_iterator i = pNode->pChildren()->begin(); i != pNode->pChildren()->end(); ++i )
		{
			RecurseVisitScene( pNode, *i, pvisitor );
		}
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void VisitScene( SceneGraph* pScene, VisitorBase* pvisitor )
	{
		RecurseVisitScene( NULL, pScene->pRoot(), pvisitor );
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void RecurseNameMatch( const char* inname, Node* pnode, u32* count )
	{
		if ( strcmp( inname, pnode->pName() ) == 0 )
		{
			++(*count);
		}

		for ( Node::ChildListType::const_iterator i = pnode->pChildren()->begin(), iend = pnode->pChildren()->end(); i != iend; ++i )
		{
			RecurseNameMatch( inname, *i, count );
		}
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	u32 CountMatchingNames( SceneGraph* pScene, const char* iname )
	{
		u32 ret = 0;

		if ( pScene->pRoot() )
		{
			RecurseNameMatch( iname, pScene->pRoot(), &ret );
		}

		return ret;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void RecurseNameGet( Node* pnode, std::vector<std::string>* outnames )
	{
		outnames->push_back( pnode->pName() );
		for ( Node::ChildListType::const_iterator i = pnode->pChildren()->begin(), iend = pnode->pChildren()->end(); i != iend; ++i )
		{
			RecurseNameGet( *i, outnames );
		}
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void GetNodeNames( SceneGraph* pScene, std::vector<std::string>* outnames )
	{
		outnames->clear();
		if ( pScene->pRoot() )
		{
			RecurseNameGet( pScene->pRoot(), outnames );
		}
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void RecurseNodeGet( Node* pnode, std::vector<Node*>* outnodes )
	{
		outnodes->push_back( pnode );
		for ( Node::ChildListType::const_iterator i = pnode->pChildren()->begin(), iend = pnode->pChildren()->end(); i != iend; ++i )
		{
			RecurseNodeGet( *i, outnodes );
		}
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void GetNodeList( SceneGraph* pScene, std::vector<Node*>* nodes )
	{
		nodes->clear();
		if ( pScene->pRoot() )
		{
			RecurseNodeGet( pScene->pRoot(), nodes );
		}
	}

}