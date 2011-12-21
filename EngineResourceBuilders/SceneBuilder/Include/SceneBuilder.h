/********************************************************************

	filename: 	SceneBuilder.h	
	
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

#ifndef SCENEBUILDER_H__
#define SCENEBUILDER_H__

#include <stdio.h>
#include <vector>
#include "ResourceBuilder.h"
#include "tinyxml/tinyxml.h"
#include <stack>
#include "RendererConstants.h"
#include "Resource.h"
#include "SceneNodeBase.h"
#include "SceneBundle.h"

class ResourceFile;
struct BaseNode;

struct MeshInfo
{
	hUint32							meshID_;
};

struct MeshCollectionInfo
{
	std::vector< MeshInfo >			meshes_;
};

struct CameraInfo
{
	Heart::Math::Vec3				position_;
	Heart::Math::Vec3				at_;
	Heart::Math::Vec3				up_;
	hFloat							fov_;
	hFloat							near_;
	hFloat							far_;
	hFloat							aspect_;
};

struct LightInfo
{
	Heart::Render::LightSourceType	type_;
	hUint32							constAttenuation_;
	hUint32							linearAttenuation_;
	hUint32							quadraticAttenuation_;
	Heart::Render::Colour			diffuse_;
	Heart::Render::Colour			specular_;
	Heart::Render::Colour			ambient_;
	hFloat							innerCone_;
	hFloat							outerCone_;
	hFloat							power_;
	hFloat							minRadius_;
	hFloat							maxRadius_;
	hFloat							lightFalloff_;
};

class SceneBuilder : public ResourceBuilder,
					 public TiXmlVisitor
{
public:
	SceneBuilder();
	~SceneBuilder();
	hBool								BuildResource();
	hUint32								GetDataSize() { return BufferOffset_; }
	hByte*								GetDataPtr() { return pDataBuffer_; }

private:

	virtual bool						VisitEnter( const TiXmlElement& element, const TiXmlAttribute* firstAttribute );
	virtual bool						VisitExit( const TiXmlElement& element );
	void								AddNodesDeps( 
											BaseNode* pNode, 
											ResourceDataPtr< Heart::Core::ResourceHandle< Heart::Scene::SceneNodeBase > >* nodes, 
											ResourceDataPtr< Heart::Scene::SceneBundle::ChildLink >* links, 
											hUint32* index );
	hUint32								FindMeshIndex( const MeshInfo& mesh );
	TiXmlDocument						sceneDef_;
	std::stack< const TiXmlElement* >	elementStack_;
	BaseNode*							pRootNode_;
	std::stack< BaseNode* >				nodeStack_;
	MeshCollectionInfo*					pMeshCollection_;
	CameraInfo*							pCameraInfo_;
	LightInfo*							pLightInfo_;
	std::vector< std::string >			meshInstanceFilenames_;

	struct 
	{
		hUint32							totalNodes_;
		hUint32							totalLocatorNodes_;
		hUint32							totalCameraNodes_;
		hUint32							totalMeshNodes_;
		hUint32							totalLightNodes_;
	} stats_;

};

#endif // SCENEBUILDER_H__