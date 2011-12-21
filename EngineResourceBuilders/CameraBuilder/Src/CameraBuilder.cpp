/********************************************************************

	filename: 	LocatorBuilder.cpp	
	
	Copyright (c) 26:3:2011 James Moran
	
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

#include "CameraBuilder.h"
#include "ResourceFileSystem.h"

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

CameraBuilder::CameraBuilder()
{

}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

CameraBuilder::~CameraBuilder()
{

}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

hBool CameraBuilder::BuildResource()
{
	ResourceFile* pInFile = pResourceFileSystem()->OpenFile( inputFilename_.c_str(), FILEMODE_READ );//

	if ( !pInFile )
	{
		ThrowFatalError( "Couldn't open Input file for " __FUNCTION__ );
	}

	cameraNode_.DefaultState();

	/*
	Read:
		Name (32 bytes)
		local matrix (16*float)
		global matrix (16*float)
	Camera data:
		position_,		Heart::Math::Vec3
		at_,			Heart::Math::Vec3
		up_,			Heart::Math::Vec3
		fov_,			hFloat
		near_,			hFloat
		far_,			hFloat
		aspect_,		hFloat						
	*/
	pInFile->Read( &cameraNode_.name_[0], Heart::Scene::SceneNodeBase::MAX_NAME_LEN );
	pInFile->Read( &cameraNode_.xf_[0].matrix_, sizeof( cameraNode_.xf_[0].matrix_ ) );
	pInFile->Read( &cameraNode_.xf_[0].globalMatrix_, sizeof( cameraNode_.xf_[0].globalMatrix_ ) );
	hUint32 castShadow;
	pInFile->Read( &castShadow, sizeof( castShadow ) );
	cameraNode_.castsShadow_ = castShadow > 0;
	pInFile->Read( &cameraNode_.camPos_, sizeof( cameraNode_.camPos_ ) );
	pInFile->Read( &cameraNode_.camLookAt_, sizeof( cameraNode_.camLookAt_ ) );
	pInFile->Read( &cameraNode_.camUp_, sizeof( cameraNode_.camUp_ ) );
	pInFile->Read( &cameraNode_.fovYDegrees_, sizeof( cameraNode_.fovYDegrees_ ) );
	pInFile->Read( &cameraNode_.near_, sizeof( cameraNode_.near_ ) );
	pInFile->Read( &cameraNode_.far_, sizeof( cameraNode_.far_ ) );
	pInFile->Read( &cameraNode_.aspect_, sizeof( cameraNode_.aspect_ ) );

	cameraNode_.xf_[1].matrix_ = cameraNode_.xf_[0].matrix_;
	cameraNode_.xf_[1].globalMatrix_ = cameraNode_.xf_[0].globalMatrix_;

	setBufferSize( 0, sizeof( Heart::Scene::SceneNodeCamera ) );
	addDataToBuffer( &cameraNode_, sizeof( cameraNode_ ) );

	return hTrue;
}
