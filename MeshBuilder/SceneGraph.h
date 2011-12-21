/********************************************************************

	filename: 	Scene.h	
	
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

#ifndef SCENE_H__
#define SCENE_H__

#include "AssImpWrapper.h"
#include "SceneNodeLocator.h"
#include "SceneDatabase.h"
#include "DataMesh.h"
#include "DataMaterial.h"
#include <vector>
#include "DataTexture.h"

namespace Data
{
	class Mesh;
}

namespace HScene
{
	enum SceneData
	{
		SceneData_TextureInstance,
		SceneData_Material,
		SceneData_Mesh,

		//Common ID's
		SceneData_ID,
	};

	DECLARE_ENUM_NAMES( SceneData );

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	class Node;
	class VisitorBase;
	
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	class SceneGraph : public Serialiser
	{
	public:

		SceneGraph() :
			pRootNode_( new Locator() )
		{
			pRootNode_->pName( "SceneRoot" );
		}
		virtual ~SceneGraph()
		{
		}
		
		void			pRoot( Node* pnode ) { pRootNode_ = pnode; }
		Node*			pRoot() const { return pRootNode_; }
		u32				GetMeshCount() { return meshes_.size(); }
		Data::Mesh*		GetMesh( u32 idx ) { return meshes_[idx]; }
		u32				GetMaterialCount() const { return material_.size(); }
		Data::Material*	GetMaterial( u32 idx ) { return material_[idx]; }
		u32				GetTextureCount() const { return textures_.size(); }
		Data::TextureInstance* GetTexture( u32 idx ) { return &textures_[idx]; }

		u32				ImportMeshToScene( aiMesh* mesh, const std::vector< int >& matMap );
		u32				ImportMaterialToScene( aiMaterial* material );
		u32				ImportTextureToScene( const char* filepath );

		bool			Serialise( TiXmlElement* toelement );
		bool			Deserialise( TiXmlElement* fromelement );

		void			ClearScene();
		void			NewScene()
		{
			ClearScene();
			pRootNode_ = new Locator();
			pRootNode_->pName( "SceneRoot" );
		}

		const char*		GetLastExportPath() const { return lastExportPath_.c_str(); }
		void			SetLastExportPath( const char* path ) { lastExportPath_ = path; }

	private:

		bool			SerialiseNodes( Node* node, TiXmlElement* parent );
		Node*			DeserialiseNodes( Node* parent, TiXmlElement* fromelement );
		Node*			CreateNode( const char* tname );

		u32										sceneKey_;
		Node*									pRootNode_;
		std::string								lastExportPath_;
		std::vector< Data::Mesh* >				meshes_;
		std::vector< Data::Material* >			material_;
		std::vector< Data::TextureInstance >	textures_;

	};

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	
	bool			ImportToScene( SceneGraph* pScene, const aiScene* aiScene, Node* pat );
	Node*			FindNode( Node* pRoot, const std::string& name );
	u32				CountMatchingNames( SceneGraph* pScene, const char* iname );
	void			GetNodeNames( SceneGraph* pScene, std::vector<std::string>* outnames );
	void			GetNodeList( SceneGraph* pScene, std::vector<Node*>* nodes );
	void			VisitScene( SceneGraph* pScene, VisitorBase* pvisitor );
}

#endif // SCENE_H__