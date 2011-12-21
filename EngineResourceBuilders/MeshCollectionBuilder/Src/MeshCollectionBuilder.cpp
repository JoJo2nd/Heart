/********************************************************************

	filename: 	MeshCollectionBuilder.cpp	
	
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

#include "MeshCollectionBuilder.h"
#include "ResourceFileSystem.h"

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

MeshCollectionBuilder::MeshCollectionBuilder()
{

}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

MeshCollectionBuilder::~MeshCollectionBuilder()
{

}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

hBool MeshCollectionBuilder::BuildResource()
{
	ResourceFile* pInFile = pResourceFileSystem()->OpenFile( inputFilename_.c_str(), FILEMODE_READ );//

	if ( !pInFile )
	{
		ThrowFatalError( "Couldn't open Input file for " __FUNCTION__ );
	}

	meshNode_.DefaultState();

	/*
	Read:
		Name (32 bytes)
		local matrix (16*float)
		global matrix (16*float)
	Mesh data:
		todo		
	*/
	pInFile->Read( &meshNode_.name_[0], Heart::Scene::SceneNodeBase::MAX_NAME_LEN );
	pInFile->Read( &meshNode_.xf_[0].matrix_, sizeof( meshNode_.xf_[0].matrix_ ) );
	pInFile->Read( &meshNode_.xf_[0].globalMatrix_, sizeof( meshNode_.xf_[0].globalMatrix_ ) );
	hUint32 castShadow;
	pInFile->Read( &castShadow, sizeof( castShadow ) );
	meshNode_.castsShadow_ = castShadow > 0;
	meshNode_.xf_[1].matrix_ = meshNode_.xf_[0].matrix_;
	meshNode_.xf_[1].globalMatrix_ = meshNode_.xf_[0].globalMatrix_;

	setBufferSize( 0, sizeof( Heart::Scene::SceneNodeMesh ) );
	addDataToBuffer( &meshNode_, sizeof( meshNode_ ) );

	return hTrue;
}