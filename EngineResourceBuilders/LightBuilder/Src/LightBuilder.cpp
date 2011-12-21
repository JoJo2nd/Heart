/********************************************************************

	filename: 	LightBuilder.cpp	
	
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

#include "LightBuilder.h"
#include "ResourceFileSystem.h"

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

LightBuilder::LightBuilder()
{

}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

LightBuilder::~LightBuilder()
{

}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

hBool LightBuilder::BuildResource()
{
	ResourceFile* pInFile = pResourceFileSystem()->OpenFile( inputFilename_.c_str(), FILEMODE_READ );//

	if ( !pInFile )
	{
		ThrowFatalError( "Couldn't open Input file for " __FUNCTION__ );
	}

	lightNode_.DefaultState();

	/*
	Read:
		Name (32 bytes)
		local matrix (16*float)
		global matrix (16*float)
	Light data:
		todo					
	*/
	pInFile->Read( &lightNode_.name_[0], Heart::Scene::SceneNodeBase::MAX_NAME_LEN );
	pInFile->Read( &lightNode_.xf_[0].matrix_, sizeof( lightNode_.xf_[0].matrix_ ) );
	pInFile->Read( &lightNode_.xf_[0].globalMatrix_, sizeof( lightNode_.xf_[0].globalMatrix_ ) );
	hUint32 castShadow;
	pInFile->Read( &castShadow, sizeof( castShadow ) );

	hFloat ftmp;
	hUint32 itmp;
	Heart::Render::Colour ctmp;
	pInFile->Read( &lightNode_.type_,			sizeof( lightNode_.type_ ) );
	pInFile->Read( &itmp,						sizeof( itmp ) );// lightNode_.constAttenuation_ ) );
	pInFile->Read( &itmp,						sizeof( itmp ) );// lightNode_.linearAttenuation_ ) );
	pInFile->Read( &itmp,						sizeof( itmp ) );// lightNode_.quadraticAttenuation_ ) );
	if( lightNode_.type_ == Heart::Scene::LightType_SPOT )
	{
		pInFile->Read( &lightNode_.innerAngle_,		sizeof( lightNode_.innerAngle_ ) );// lightNode_.innerCone_ ) );
		pInFile->Read( &lightNode_.outerAngle_,		sizeof( lightNode_.outerAngle_ ) );// lightNode_.outerCone_ ) );
		pInFile->Read( &lightNode_.diffuse_,		sizeof( lightNode_.diffuse_ ) );
		pInFile->Read( &lightNode_.specular_,		sizeof( lightNode_.specular_ ) );
		pInFile->Read( &ctmp,						sizeof( ctmp ) );// lightNode_.ambient_ ) );
		pInFile->Read( &lightNode_.power_,			sizeof( lightNode_.power_ ) );
		pInFile->Read( &ftmp,						sizeof( lightNode_.minRadius_ ) );
		pInFile->Read( &ftmp,						sizeof( lightNode_.maxRadius_ ) );
		pInFile->Read( &lightNode_.falloffDist_,	sizeof( lightNode_.falloffDist_ )  );


		//Currently outerAngle is wrong, only innerAngle is used
		lightNode_.outerAngle_ = lightNode_.innerAngle_;

		lightNode_.innerRadius_ = tan(lightNode_.innerAngle_)*lightNode_.falloffDist_;
		lightNode_.outerRadius_ = tan(lightNode_.outerAngle_)*lightNode_.falloffDist_;
	}
	else if ( lightNode_.type_ == Heart::Scene::LightType_POINT )
	{
		pInFile->Read( &ftmp,						sizeof( lightNode_.innerAngle_ ) );// lightNode_.innerCone_ ) );
		pInFile->Read( &ftmp,						sizeof( lightNode_.outerAngle_ ) );// lightNode_.outerCone_ ) );
		pInFile->Read( &lightNode_.diffuse_,		sizeof( lightNode_.diffuse_ ) );
		pInFile->Read( &lightNode_.specular_,		sizeof( lightNode_.specular_ ) );
		pInFile->Read( &ctmp,						sizeof( ctmp ) );// lightNode_.ambient_ ) );
		pInFile->Read( &lightNode_.power_,			sizeof( lightNode_.power_ ) );
		pInFile->Read( &lightNode_.minRadius_,		sizeof( lightNode_.minRadius_ ) );
		pInFile->Read( &lightNode_.maxRadius_,		sizeof( lightNode_.maxRadius_ ) );
		pInFile->Read( &ftmp,						sizeof( lightNode_.falloffDist_ )  );
	}
	else if ( lightNode_.type_ == Heart::Scene::LightType_DIRECTION )
	{
		pInFile->Read( &ftmp,						sizeof( lightNode_.innerAngle_ ) );// lightNode_.innerCone_ ) );
		pInFile->Read( &ftmp,						sizeof( lightNode_.outerAngle_ ) );// lightNode_.outerCone_ ) );
		pInFile->Read( &lightNode_.diffuse_,		sizeof( lightNode_.diffuse_ ) );
		pInFile->Read( &lightNode_.specular_,		sizeof( lightNode_.specular_ ) );
		pInFile->Read( &ctmp,						sizeof( ctmp ) );// lightNode_.ambient_ ) );
		pInFile->Read( &lightNode_.power_,			sizeof( lightNode_.power_ ) );
		pInFile->Read( &ftmp,						sizeof( lightNode_.minRadius_ ) );
		pInFile->Read( &ftmp,						sizeof( lightNode_.maxRadius_ ) );
		pInFile->Read( &ftmp,						sizeof( lightNode_.falloffDist_ )  );
	}


	lightNode_.castsShadow_ = castShadow > 0;
	lightNode_.xf_[1].matrix_ = lightNode_.xf_[0].matrix_;
	lightNode_.xf_[1].globalMatrix_ = lightNode_.xf_[0].globalMatrix_;

	lightNode_.specExponent_ = 255.0f;

	setBufferSize( 0, sizeof( Heart::Scene::SceneNodeLight ) );
	addDataToBuffer( &lightNode_, sizeof( lightNode_ ) );

	return hTrue;
}