/********************************************************************
	created:	2010/03/22
	created:	22:3:2010   23:19
	filename: 	FBXBuilderBase.h	
	author:		James
	
	purpose:	
*********************************************************************/
#ifndef FBXBUILDERBASE_H__
#define FBXBUILDERBASE_H__

#include "ResourceBuilder.h"
#include <string>
#include <list>

struct PassDef
{
	PassDef( const char* passname, const char* file, const char* tech )
	{
		hlslPassName_ = passname;
		hlslShaderFileName_ = file;
		hlslShaderTechName_ = tech;
	}

	std::string		hlslPassName_;//shadow pass, skinned pass, etc
	std::string		hlslShaderFileName_;
	std::string		hlslShaderTechName_;
};

struct MaterialLink
{
	std::string					materialName_;
	std::list< PassDef >		passes_;
};

class MaterialDefFile
{
public:

	std::list< MaterialLink >		materialDefs_;
};

class FBXBuilderCommon : public ResourceBuilder
{
public:
	FBXBuilderCommon( std::string typeName ) :
		ResourceBuilder( typeName )
	{}
	virtual ~FBXBuilderCommon() {}

protected:

	hBool				BuildXMLMaterialDefFile( const std::string& filename, MaterialDefFile& matDef );
};

#endif // FBXBUILDERBASE_H__