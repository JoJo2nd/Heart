/********************************************************************

	filename: 	Scene.cpp
	
	Copyright (c) 2011/02/18 James Moran
	
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
#include "SceneDatabase.h" 
#include "RefCounted.h"
#include "aiScene.h"
#include "aiLight.h"
#include "assimp.h"
#include "DataMesh.h"
#include "DataMaterial.h"
#include "DataLight.h"
#include <vector>
#include "DataCamera.h"
#include "DataTexture.h"

namespace Data
{
	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////

	class SceneInstance : public ReferenceCounted
	{
	public:
		SceneInstance() :
			pRoot_( NULL ),
			nMeshes_( 0 ),
			pMeshes_( NULL )
		{

		}
		virtual ~SceneInstance()
		{
		}

		aiNode*		pRoot_;
		u32			nMaterials_;
		Material*	pMaterials_;
		u32			nMeshes_;
		Mesh*		pMeshes_;
		u32			nLights_;
		Light*		pLights_;
		u32			nCameras_;
		Camera*		pCameras_;
	};

	
namespace 
{
	std::vector< SceneInstance >	loadedScenes_;
	std::vector< TextureInstance >	loadedTextures_;

	void CopyNodes( aiNode* pparent, aiNode* pinroot, aiNode** ppoutroot )
	{
		ASSERT( *ppoutroot == NULL );

		*ppoutroot = new aiNode;
		(*ppoutroot)->mMeshes = NULL;
		(*ppoutroot)->mChildren = NULL;
		if ( pinroot->mNumMeshes )
		{
			(*ppoutroot)->mMeshes = new unsigned int[pinroot->mNumMeshes];
		}
		memcpy( (*ppoutroot)->mMeshes, pinroot->mMeshes, sizeof(unsigned int)*pinroot->mNumMeshes );
		if ( pinroot->mNumChildren )
		{
			(*ppoutroot)->mChildren = new aiNode*[pinroot->mNumChildren];
		}

		(*ppoutroot)->mParent = pparent;
		(*ppoutroot)->mName = aiString( pinroot->mName );
		(*ppoutroot)->mTransformation = pinroot->mTransformation;
		(*ppoutroot)->mNumChildren = pinroot->mNumChildren;
		(*ppoutroot)->mNumMeshes = pinroot->mNumMeshes;

		for ( u32 i = 0; i < (*ppoutroot)->mNumChildren; ++i )
		{
			(*ppoutroot)->mChildren[i] = NULL;
			CopyNodes( *ppoutroot, pinroot->mChildren[i], &((*ppoutroot)->mChildren[i]) );
		}
	}
}


	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////

	HSceneHandle CreateScene( const char* filename, u32 flags )
	{
		HSceneHandle ret = loadedScenes_.size();

		const aiScene* paiScene = aiImportFile( filename, flags );
		if ( !paiScene || (paiScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) == AI_SCENE_FLAGS_INCOMPLETE )
		{
			ret = INVALIDSCENEHANDLE;
			return ret;
		}

		loadedScenes_.resize( loadedScenes_.size() + 1 );
		SceneInstance* pScene = &loadedScenes_[ret];

		CopyNodes( NULL, paiScene->mRootNode, &pScene->pRoot_ );

		pScene->nLights_ = paiScene->mNumLights;
		pScene->pLights_ = new Light[pScene->nLights_];
		for ( u32 i = 0; i < pScene->nLights_; ++i )
		{
			pScene->pLights_[i].name_ = paiScene->mLights[i]->mName.data;
			pScene->pLights_[i].attenuationConstant_ = paiScene->mLights[i]->mAttenuationConstant;
			pScene->pLights_[i].attenuationLinear_ = paiScene->mLights[i]->mAttenuationLinear;
			pScene->pLights_[i].attenuationQuadratic_ = paiScene->mLights[i]->mAttenuationQuadratic;
			pScene->pLights_[i].angleInnerCone_ = paiScene->mLights[i]->mAngleInnerCone;
			pScene->pLights_[i].angleOuterCone_ = paiScene->mLights[i]->mAngleOuterCone;

			pScene->pLights_[i].colorDiffuse_.r = paiScene->mLights[i]->mColorDiffuse.r;
			pScene->pLights_[i].colorDiffuse_.g = paiScene->mLights[i]->mColorDiffuse.g;
			pScene->pLights_[i].colorDiffuse_.b = paiScene->mLights[i]->mColorDiffuse.b;
			pScene->pLights_[i].colorDiffuse_.a = 1.0f;

			pScene->pLights_[i].colorAmbient_.r = paiScene->mLights[i]->mColorAmbient.r;
			pScene->pLights_[i].colorAmbient_.g = paiScene->mLights[i]->mColorAmbient.g;
			pScene->pLights_[i].colorAmbient_.b = paiScene->mLights[i]->mColorAmbient.b;
			pScene->pLights_[i].colorAmbient_.a = 1.0f;

			pScene->pLights_[i].colorSpecular_.r = paiScene->mLights[i]->mColorSpecular.r;
			pScene->pLights_[i].colorSpecular_.g = paiScene->mLights[i]->mColorSpecular.g;
			pScene->pLights_[i].colorSpecular_.b = paiScene->mLights[i]->mColorSpecular.b;
			pScene->pLights_[i].colorSpecular_.a = 1.0f;

			pScene->pLights_[i].position_.x = paiScene->mLights[i]->mPosition.x;
			pScene->pLights_[i].position_.y = paiScene->mLights[i]->mPosition.y;
			pScene->pLights_[i].position_.z = paiScene->mLights[i]->mPosition.z;

			pScene->pLights_[i].direction_.x = paiScene->mLights[i]->mDirection.x;
			pScene->pLights_[i].direction_.y = paiScene->mLights[i]->mDirection.y;
			pScene->pLights_[i].direction_.z = paiScene->mLights[i]->mDirection.z;

			switch( paiScene->mLights[i]->mType )
			{
			case aiLightSource_DIRECTIONAL:
				pScene->pLights_[i].type_ = LIGHTSOURCETYPE_DIRECTION;
				break;
			case aiLightSource_POINT:
				pScene->pLights_[i].type_ = LIGHTSOURCETYPE_POINT;
				break;
			case aiLightSource_SPOT:
				pScene->pLights_[i].type_ = LIGHTSOURCETYPE_SPOT;
				break;
			}
		}

		pScene->nCameras_ = paiScene->mNumCameras;
		pScene->pCameras_ = new Camera[pScene->nCameras_];
		for ( u32 i = 0; i < pScene->nCameras_; ++i )
		{
			pScene->pCameras_[i].name_ = paiScene->mCameras[i]->mName.data;

			pScene->pCameras_[i].position_.x = paiScene->mCameras[i]->mPosition.x;
			pScene->pCameras_[i].position_.y = paiScene->mCameras[i]->mPosition.y;
			pScene->pCameras_[i].position_.z = paiScene->mCameras[i]->mPosition.z;

			pScene->pCameras_[i].up_.x = paiScene->mCameras[i]->mUp.x;
			pScene->pCameras_[i].up_.y = paiScene->mCameras[i]->mUp.y;
			pScene->pCameras_[i].up_.z = paiScene->mCameras[i]->mUp.z;

			pScene->pCameras_[i].lookAt_.x = paiScene->mCameras[i]->mLookAt.x;
			pScene->pCameras_[i].lookAt_.y = paiScene->mCameras[i]->mLookAt.y;
			pScene->pCameras_[i].lookAt_.z = paiScene->mCameras[i]->mLookAt.z;

			pScene->pCameras_[i].horizontalFOV_ = paiScene->mCameras[i]->mHorizontalFOV;
			pScene->pCameras_[i].aspect_ = paiScene->mCameras[i]->mAspect;
			pScene->pCameras_[i].far_ = paiScene->mCameras[i]->mClipPlaneFar;
			pScene->pCameras_[i].near_ = paiScene->mCameras[i]->mClipPlaneNear;
		}

// 		pScene->nMaterials_ = paiScene->mNumMaterials;
// 		pScene->pMaterials_ = new Material[pScene->nMaterials_];
// 		for ( u32 i = 0; i < pScene->nMaterials_; ++i )
// 		{
// 			BuildFromaiMaterial( ret, i, paiScene, paiScene->mMaterials[i], &pScene->pMaterials_[i] );
// 		}
// 
// 		pScene->nMeshes_ = paiScene->mNumMeshes;
// 		pScene->pMeshes_ = new Mesh[pScene->nMeshes_];
// 		for ( u32 i = 0; i < pScene->nMeshes_; ++i )
// 		{
// 			BuildFromaiMesh( ret, i, paiScene, paiScene->mMeshes[i], &pScene->pMeshes_[i] );
// 		}

		aiReleaseImport( paiScene );

		return ret;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	HTextureHandle CreateTexture( const char* filename )
	{
		HTextureHandle ret = loadedTextures_.size();

		//TODO: double check file exists

		//Add to the list
		loadedTextures_.resize( loadedTextures_.size() + 1 );
		TextureInstance* pTex = &loadedTextures_[ret];

		pTex->filename_ = filename;
		pTex->format_ = TextureFormat_RGBA8;

		return ret;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void EnumerateLoadedTextures( EnumerateTextureCallback fn, void* pUser )
	{
		u32 size = loadedTextures_.size();
		for ( u32 i = 0; i < size; ++i )
		{
			if ( fn( i, pUser ) != 0 )
			{
				return;
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void EnumerateLoadedScenes( EnumerateSceneCallback fn, void* pUser )
	{
		u32 size = loadedScenes_.size();
		for ( u32 i = 0; i < size; ++i )
		{
			if ( fn( i, pUser ) != 0 )
			{
				return;
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	aiNode* GetRootNode( HSceneHandle hscene )
	{
		return loadedScenes_[hscene].pRoot_;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////

	u32 GetMeshCount( HSceneHandle hScene )
	{
		return loadedScenes_[hScene].nMeshes_;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////

	Mesh* GetMeshFromScene( HSceneHandle hScene, u32 meshIndex )
	{
		if ( meshIndex < loadedScenes_[hScene].nMeshes_ )
		{
			return &loadedScenes_[hScene].pMeshes_[meshIndex];
		}
		else 
		{
			return NULL;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	u32 GetMaterialCount( HSceneHandle hScene )
	{
		return loadedScenes_[hScene].nMaterials_;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	Material* GetMaterial( HSceneHandle hScene, u32 materialIndex )
	{
		if ( materialIndex < loadedScenes_[hScene].nMaterials_ )
		{
			return &loadedScenes_[hScene].pMaterials_[materialIndex];
		}
		
		return NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	u32 GetLightCount( HSceneHandle hScene )
	{
		return loadedScenes_[hScene].nLights_;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	Light* GetLight( HSceneHandle hScene, u32 lightIndex )
	{
		if ( lightIndex < loadedScenes_[hScene].nLights_ )
		{
			return loadedScenes_[hScene].pLights_ + lightIndex;
		}
		return NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	u32 GetCameraCount( HSceneHandle hScene )
	{
		return loadedScenes_[hScene].nCameras_;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	Camera* GetCamera( HSceneHandle hScene, u32 cameraIndex )
	{
		if ( cameraIndex < loadedScenes_[hScene].nCameras_ )
		{
			return loadedScenes_[hScene].pCameras_ + cameraIndex;
		}

		return NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	const char* GetTextureFilename( HTextureHandle hTex )
	{
		return loadedTextures_[hTex].filename_.c_str();
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void ReassignTexture( HTextureHandle hTex, const char* newfilename )
	{
		//TODO: check file exists

		loadedTextures_[hTex].filename_ = newfilename;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	TextureFormat GetTextureFormat( HTextureHandle hTex )
	{
		return loadedTextures_[hTex].format_;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void SetTextureFormat( HTextureHandle hTex, TextureFormat fmt )
	{
		loadedTextures_[hTex].format_ = fmt;
	}

}
