/********************************************************************

	filename: 	DataMaterial.cpp	
	
	Copyright (c) 20:2:2011 James Moran
	
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
#include "DataMaterial.h"
#include "aiScene.h"
#include "aiMaterial.h"
#include "SceneGraph.h"
#include "tinyxml.h"

namespace Data
{
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
namespace
{
	const char* gDefaultMaterialName = "DefaultMaterial";
	const char* gDefaultShader = 
"/////////////////////////////////////////////////////////////////////////////////\n"
"// Constants\n"
"	//New Style Matrix Names\n"
"	float4x4 WorldViewProjectionMatrix	: WORLDVIEWPROJMATRIX;\n"
"\n"
"////////////////////////////////////////////////////////////////////////////////\n"
"// Structures\n"
"\n"
"struct VS_OUTPUT\n"
"{\n"
"	float4 v4Position : POSITION;\n"
"	float2 uv		  : TEXCOORD0;\n"
"};\n"
"\n"
"struct VS_INPUT\n"
"{\n"
"	float4 v4Position   : POSITION;\n"
"	float2 uv			: TEXCOORD0;\n"
"};\n"
"\n"
"texture EffectTexture1 : DIFFUSETEXTURE0; \n"
"sampler DiffuseSampler = \n"
"sampler_state\n"
"	{\n"
"		Texture = <EffectTexture1>;\n"
"		MipFilter = LINEAR;\n"
"		MinFilter = LINEAR;\n"
"		MagFilter = LINEAR;\n"
"	};\n"
"\n"
"VS_OUTPUT VsStandardTex1( const VS_INPUT input )\n"
"{\n"
"	VS_OUTPUT output;\n"
"	output.v4Position  = mul( input.v4Position, WorldViewProjectionMatrix );\n"
"	output.uv		   = input.uv;\n"
"	return output;\n"
"}\n"
"\n"
"\n"
"float4 PsStandardTex1( VS_OUTPUT input ) : COLOR\n"
"{\n"
"	return saturate( tex2D( DiffuseSampler, input.uv ) );\n"
"}\n"
"\n"
"technique MeshTex1\n"
"{\n"
"	pass P0\n"
"	{\n"
"		PixelShader = compile ps_2_0 PsStandardTex1();\n"
"		VertexShader = compile vs_1_1 VsStandardTex1();\n"
"	}\n"
"}\n"
;
}

	BEGIN_ENUM_NAMES( ZTestState )
		ENUM_NAME( NEVER )
		ENUM_NAME( LESS )
		ENUM_NAME( EQUAL )
		ENUM_NAME( LESSEQUAL )
		ENUM_NAME( GREATER )
		ENUM_NAME( NOT_EQUAL )
		ENUM_NAME( GREATER_EQUAL )
		ENUM_NAME( ALWAYS )

		ENUM_NAME( MAX )
	END_ENUM_NAMES();

	BEGIN_ENUM_NAMES( BlendFunction )
		ENUM_NAME( NONE )
		ENUM_NAME( ADD )
		ENUM_NAME( SUB )
		ENUM_NAME( MIN )
		ENUM_NAME( MAX )
	END_ENUM_NAMES();

	BEGIN_ENUM_NAMES( BlendOP )
		ENUM_NAME( ZERO )
		ENUM_NAME( ONE )
		ENUM_NAME( SRC_COLOUR )
		ENUM_NAME( INVSRC_COLOUR )
		ENUM_NAME( DEST_COLOUR )
		ENUM_NAME( INVDEST_COLOUR )
		ENUM_NAME( SRC_ALPHA )
		ENUM_NAME( INVSRC_ALPHA )
		ENUM_NAME( DEST_ALPHA )
		ENUM_NAME( INVDEST_ALPHA )
	END_ENUM_NAMES();

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	Material::Material( HScene::SceneGraph* pScene ) :
		pScene_( pScene )
	{

	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	Material::~Material()
	{

	}

	//////////////////////////////////////////////////////////////////////////
	// 23:11:23 ////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	Data::TextureInstance* Material::GetDiffuseTexture( u32 idx ) const
	{
		if ( idx < diffuseTextures_.size() )
			return pScene_->GetTexture( diffuseTextures_[idx] );
		else
			return NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	// 21:22:54 ////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	bool Material::Serialise( TiXmlElement* toelement )
	{
		XML_LINK_STRING( name_, toelement );
		XML_LINK_INT( materialSceneIndex_, toelement );
		XML_LINK_INT_AS_STRING( "nDiffuseTextures_", diffuseTextures_.size(), toelement ); 
		for ( u32 i = 0; i < diffuseTextures_.size(); ++i )
		{
			XML_LINK_INT_ID( diffuseTextures_, i, toelement );
		}
		XML_LINK_INT( normalMap_, toelement );
		XML_LINK_INT( specularMap_, toelement );
		XML_LINK_INT( lightMap_, toelement );
		XML_LINK_STRING( hlslCode_.codeText_, toelement );
		XML_LINK_INT( twoSided_, toelement );
		XML_LINK_INT( wireframe_, toelement );
		XML_LINK_INT( zDepthTest_, toelement );
		XML_LINK_INT( zDepthWrite_, toelement );
		XML_LINK_ENUM( zTest_, ZTestState, toelement );
		XML_LINK_FLOAT( shininess_, toelement );
		XML_LINK_ENUM( blendFunc_, BlendFunction, toelement );
		XML_LINK_ENUM( blendOPSrc_, BlendOP, toelement );
		XML_LINK_ENUM( blendOPDst_, BlendOP, toelement );
		XML_LINK_COLOUR( diffuse_, toelement );
		XML_LINK_COLOUR( ambient_, toelement );
		XML_LINK_COLOUR( specular_, toelement );
		XML_LINK_COLOUR( emissive_, toelement );

		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	// 21:22:58 ////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	bool Material::Deserialise( TiXmlElement* fromelement )
	{
		XML_UNLINK_STRING( name_, fromelement );
		XML_UNLINK_INT( materialSceneIndex_, fromelement );
		u32 count;
		XML_UNLINK_INT_FROM_ELEMENT( "nDiffuseTextures_", count, fromelement );
		diffuseTextures_.resize( count, Data::INVALIDTEXTUREHANDLE );
		XML_UNLINK_INT_ID( diffuseTextures_, fromelement );
		XML_UNLINK_INT( normalMap_, fromelement );
		XML_UNLINK_INT( specularMap_, fromelement );
		XML_UNLINK_INT( lightMap_, fromelement );
		XML_UNLINK_STRING( hlslCode_.codeText_, fromelement );
		XML_UNLINK_INT( twoSided_, fromelement );
		XML_UNLINK_INT( wireframe_, fromelement );
		XML_UNLINK_INT( zDepthTest_, fromelement );
		XML_UNLINK_INT( zDepthWrite_, fromelement );
		XML_UNLINK_ENUM( zTest_, ZTestState, fromelement );
		XML_UNLINK_FLOAT( shininess_, fromelement );
		XML_UNLINK_ENUM( blendFunc_, BlendFunction, fromelement );
		XML_UNLINK_ENUM( blendOPSrc_, BlendOP, fromelement );
		XML_UNLINK_ENUM( blendOPDst_, BlendOP, fromelement );
		XML_UNLINK_COLOUR( diffuse_, fromelement );
		XML_UNLINK_COLOUR( ambient_, fromelement );
		XML_UNLINK_COLOUR( specular_, fromelement );
		XML_UNLINK_COLOUR( emissive_, fromelement );

		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void BuildFromaiMaterial( u32 materialSceneIdx, HScene::SceneGraph* pScene, aiMaterial* pinMat, Material* poutMat )
	{
		u32 nDiffuseTex = pinMat->GetTextureCount( aiTextureType_DIFFUSE );
		bool hasNormal = pinMat->GetTextureCount( aiTextureType_NORMALS ) > 0;
		bool hasSpec = pinMat->GetTextureCount( aiTextureType_SPECULAR ) > 0;
		bool hasLightMap = pinMat->GetTextureCount( aiTextureType_LIGHTMAP ) > 0;

		//initialise noraml, specular & lightmap values
		poutMat->normalMap_		= INVALIDTEXTUREHANDLE;
		poutMat->specularMap_	= INVALIDTEXTUREHANDLE;
		poutMat->lightMap_		= INVALIDTEXTUREHANDLE;

		poutMat->materialSceneIndex_ = materialSceneIdx;

		nDiffuseTex = nDiffuseTex > Material::MAX_DIFFUSE_TEXTURES ? Material::MAX_DIFFUSE_TEXTURES : nDiffuseTex;
		for ( u32 i = 0; i < nDiffuseTex; ++i )
		{
			aiString path;
			pinMat->GetTexture( aiTextureType_DIFFUSE, i, &path, NULL, NULL, NULL, NULL, NULL );
			if ( path.length > 0 )
			{
				poutMat->diffuseTextures_.push_back( pScene->ImportTextureToScene( path.data ) );
			}
		}

		if ( hasNormal )
		{
			aiString path;
			pinMat->GetTexture( aiTextureType_NORMALS, 0, &path, NULL, NULL, NULL, NULL, NULL );
			if ( path.length > 0 )
			{
				poutMat->normalMap_ = pScene->ImportTextureToScene( path.data );
			}
		}

		if ( hasSpec )
		{
			aiString path;
			pinMat->GetTexture( aiTextureType_NORMALS, 0, &path, NULL, NULL, NULL, NULL, NULL );
			if ( path.length > 0 )
			{
				poutMat->specularMap_ = pScene->ImportTextureToScene( path.data );
			}
		}

		if ( hasLightMap )
		{
			aiString path;
			pinMat->GetTexture( aiTextureType_NORMALS, 0, &path, NULL, NULL, NULL, NULL, NULL );
			if ( path.length > 0 )
			{
				poutMat->lightMap_ = pScene->ImportTextureToScene( path.data );
			}
		}

		aiString matName;
		if ( pinMat->Get( AI_MATKEY_NAME, matName ) == aiReturn_SUCCESS )
		{
			poutMat->name_ = matName.data;
		}
		else
		{
			poutMat->name_ = gDefaultMaterialName;
		}

		int twosided;
		if ( pinMat->Get( AI_MATKEY_TWOSIDED, twosided ) == aiReturn_SUCCESS )
		{
			poutMat->twoSided_ = twosided == 1 ? true : false;
		}
		else
		{
			poutMat->twoSided_ = false;
		}

		int wireframe;
		if ( pinMat->Get( AI_MATKEY_ENABLE_WIREFRAME, wireframe ) == aiReturn_SUCCESS )
		{
			poutMat->wireframe_ = wireframe == 1 ? true : false;
		}
		else
		{
			poutMat->wireframe_ = false;
		}

		poutMat->zDepthTest_ = true;
		poutMat->zDepthWrite_ = true;
		poutMat->zTest_ = MAT_Z_CMP_LESS;

		float shininess;
		if ( pinMat->Get( AI_MATKEY_SHININESS, shininess ) == aiReturn_SUCCESS )
		{
			poutMat->shininess_ = shininess;
		}
		else
		{
			poutMat->shininess_ = 0.0f;
		}

		int blendMode;
		if ( pinMat->Get( AI_MATKEY_BLEND_FUNC, blendMode ) == aiReturn_SUCCESS )
		{
			if ( blendMode == aiBlendMode_Additive )
			{
				poutMat->blendFunc_ = MAT_BLEND_FUNC_NONE;
				poutMat->blendOPSrc_ = MAT_BLEND_OP_ONE;
				poutMat->blendOPDst_ = MAT_BLEND_OP_ONE;
			}
			else if ( blendMode == aiBlendMode_Default )
			{
				poutMat->blendFunc_ = MAT_BLEND_FUNC_ADD;
				poutMat->blendOPSrc_ = MAT_BLEND_OP_SRC_ALPHA;
				poutMat->blendOPDst_ = MAT_BLEND_OP_INVSRC_ALPHA;
			}
			else
			{
				poutMat->blendFunc_ = MAT_BLEND_FUNC_NONE;
				poutMat->blendOPSrc_ = MAT_BLEND_OP_ONE;
				poutMat->blendOPDst_ = MAT_BLEND_OP_ONE;
			}
		}
		else
		{
			poutMat->blendFunc_ = MAT_BLEND_FUNC_NONE;
			poutMat->blendOPSrc_ = MAT_BLEND_OP_ONE;
			poutMat->blendOPDst_ = MAT_BLEND_OP_ONE;
		}

		aiColor4D diffuse;
		if ( pinMat->Get( AI_MATKEY_COLOR_DIFFUSE, diffuse ) == aiReturn_SUCCESS )
		{
			poutMat->diffuse_.r = diffuse.r;
			poutMat->diffuse_.g = diffuse.g;
			poutMat->diffuse_.b = diffuse.b;
			poutMat->diffuse_.a = diffuse.a;
		}
		else
		{
			poutMat->diffuse_.r = 1.0f;
			poutMat->diffuse_.g = 1.0f;
			poutMat->diffuse_.b = 1.0f;
			poutMat->diffuse_.a = 1.0f;
		}

		aiColor4D ambient;
		if ( pinMat->Get( AI_MATKEY_COLOR_AMBIENT, ambient ) == aiReturn_SUCCESS )
		{
			poutMat->ambient_.r = ambient.r;
			poutMat->ambient_.g = ambient.g;
			poutMat->ambient_.b = ambient.b;
			poutMat->ambient_.a = ambient.a;
		}
		else
		{
			poutMat->ambient_.r = 1.0f;
			poutMat->ambient_.g = 1.0f;
			poutMat->ambient_.b = 1.0f;
			poutMat->ambient_.a = 1.0f;
		}

		aiColor4D specular;
		if ( pinMat->Get( AI_MATKEY_COLOR_SPECULAR, specular ) == aiReturn_SUCCESS )
		{
			poutMat->specular_.r = specular.r;
			poutMat->specular_.g = specular.g;
			poutMat->specular_.b = specular.b;
			poutMat->specular_.a = specular.a;
		}
		else
		{
			poutMat->specular_.r = 1.0f;
			poutMat->specular_.g = 1.0f;
			poutMat->specular_.b = 1.0f;
			poutMat->specular_.a = 1.0f;
		}

		aiColor4D emissive;
		if ( pinMat->Get( AI_MATKEY_COLOR_EMISSIVE, emissive ) == aiReturn_SUCCESS )
		{
			poutMat->emissive_.r = emissive.r;
			poutMat->emissive_.g = emissive.g;
			poutMat->emissive_.b = emissive.b;
			poutMat->emissive_.a = emissive.a;
		}
		else
		{
			poutMat->emissive_.r = 1.0f;
			poutMat->emissive_.g = 1.0f;
			poutMat->emissive_.b = 1.0f;
			poutMat->emissive_.a = 1.0f;
		}

		poutMat->hlslCode_.codeText_ = gDefaultShader;
	}
}
