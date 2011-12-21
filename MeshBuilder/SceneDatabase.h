#include "DataTexture.h"
/********************************************************************

	filename: 	Scene.h
	
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

#ifndef _SCENEDATABASE_H__
#define _SCENEDATABASE_H__

struct aiNode;

namespace Data 
{
	struct Colour
	{
		float r,g,b,a;
	};

	class Mesh;
	class Material;
	class Light;
	class Camera;

	typedef u32 HSceneHandle;
	typedef u32 HTextureHandle;

	static const u32			INVALIDHANDLE			= 0xFFFFFFFF;
	static const HSceneHandle	INVALIDSCENEHANDLE		= INVALIDHANDLE;
	static const HTextureHandle	INVALIDTEXTUREHANDLE	= INVALIDHANDLE;

	void			Initialise();
	void			Destroy();
	HSceneHandle	CreateScene( const char* filename, u32 flags );
	HTextureHandle	CreateTexture( const char* filename );
	aiNode*			GetRootNode( HSceneHandle hscene );
	u32				GetMeshCount( HSceneHandle hscene );
	Mesh*			GetMeshFromScene( HSceneHandle hscene, u32 meshIndex );
	u32				GetMaterialCount( HSceneHandle hScene );
	Material*		GetMaterial( HSceneHandle hScene, u32 materialIndex );
 	u32				GetLightCount( HSceneHandle hScene );
 	Light*			GetLight( HSceneHandle hScene, u32 lightIndex );
 	u32				GetCameraCount( HSceneHandle hScene );
 	Camera*			GetCamera( HSceneHandle hScene, u32 cameraIndex );

	const char*		GetTextureFilename( HTextureHandle hTex );
	void			ReassignTexture( HTextureHandle hTex, const char* newfilename );
	TextureFormat	GetTextureFormat( HTextureHandle hTex );
	void			SetTextureFormat( HTextureHandle hTex, TextureFormat fmt );

	typedef u32 (*EnumerateSceneCallback)(HSceneHandle,void*);
	void			EnumerateLoadedScenes( EnumerateSceneCallback, void* pUser );

	typedef u32 (*EnumerateTextureCallback)(HTextureHandle,void*);
	void			EnumerateLoadedTextures( EnumerateTextureCallback, void* pUser );

}

#endif // _SCENEDATABASE_H__