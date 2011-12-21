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

#include "LocatorBuilder.h"
#include "ResourceFileSystem.h"

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

LocatorBuilder::LocatorBuilder()
{

}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

LocatorBuilder::~LocatorBuilder()
{

}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

hBool LocatorBuilder::BuildResource()
{
	ResourceFile* pInFile = pResourceFileSystem()->OpenFile( inputFilename_.c_str(), FILEMODE_READ );//

	if ( !pInFile )
	{
		ThrowFatalError( "Couldn't open Input file for " __FUNCTION__ );
	}

	locatorNode_.DefaultState();

	/*
	Read:
		Name (32 bytes)
		local matrix (16*float)
		global matrix (16*float)
		castshadow (4 bytes)
	*/
	pInFile->Read( &locatorNode_.name_[0], Heart::Scene::SceneNodeBase::MAX_NAME_LEN );
	pInFile->Read( &locatorNode_.xf_[0].matrix_, sizeof( locatorNode_.xf_[0].matrix_ ) );
	pInFile->Read( &locatorNode_.xf_[0].globalMatrix_, sizeof( locatorNode_.xf_[0].globalMatrix_ ) );
	hUint32 castShadow;
	pInFile->Read( &castShadow, sizeof( castShadow ) );
	locatorNode_.castsShadow_ = castShadow > 0;
	locatorNode_.xf_[1].matrix_ = locatorNode_.xf_[0].matrix_;
	locatorNode_.xf_[1].globalMatrix_ = locatorNode_.xf_[0].globalMatrix_;

	setBufferSize( 0, sizeof( Heart::Scene::SceneNodeLocator ) );
	addDataToBuffer( &locatorNode_, sizeof( locatorNode_ ) );

	return hTrue;
}
