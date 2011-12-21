/********************************************************************

	filename: 	DataMaterial.h	
	
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

#ifndef DATAMATERIAL_H__
#define DATAMATERIAL_H__

#include <string>
#include "SceneDatabase.h"
#include "Serialiser.h"

struct aiScene;
struct aiMaterial;

namespace Direct3D
{
	class Device;
}

namespace HScene
{
	class SceneGraph;
}

namespace Data
{

	void BuildFromaiMaterial( u32 materialSceneIdx, HScene::SceneGraph* pScene, aiMaterial* pinMat, Material* poutMat );

	enum ZTestState
	{
		//Z COMPARE FUNCTION
		MAT_Z_CMP_NEVER,
		MAT_Z_CMP_LESS,
		MAT_Z_CMP_EQUAL,
		MAT_Z_CMP_LESSEQUAL,
		MAT_Z_CMP_GREATER,
		MAT_Z_CMP_NOT_EQUAL,
		MAT_Z_CMP_GREATER_EQUAL,
		MAT_Z_CMP_ALWAYS,

		MAT_Z_MAX
	};

	DECLARE_ENUM_NAMES( ZTestState );

	enum BlendFunction
	{
		MAT_BLEND_FUNC_NONE,
		MAT_BLEND_FUNC_ADD,
		MAT_BLEND_FUNC_SUB,
		MAT_BLEND_FUNC_MIN,
		MAT_BLEND_FUNC_MAX,
	};

	DECLARE_ENUM_NAMES( BlendFunction );

	enum BlendOP
	{
		MAT_BLEND_OP_ZERO,
		MAT_BLEND_OP_ONE,
		MAT_BLEND_OP_SRC_COLOUR,
		MAT_BLEND_OP_INVSRC_COLOUR,
		MAT_BLEND_OP_DEST_COLOUR,
		MAT_BLEND_OP_INVDEST_COLOUR,
		MAT_BLEND_OP_SRC_ALPHA,
		MAT_BLEND_OP_INVSRC_ALPHA,
		MAT_BLEND_OP_DEST_ALPHA,
		MAT_BLEND_OP_INVDEST_ALPHA,
	};

	DECLARE_ENUM_NAMES( BlendOP );

	class Shader
	{
	public:
		Shader()
		{
		}
		~Shader()
		{
		}
	

		std::string		codeText_;
	};

	class Material : public Serialiser
	{
	public:
		Material( HScene::SceneGraph* pScene );
		~Material();

		static const u32		MAX_DIFFUSE_TEXTURES = 4;

		const char*				GetName() const { return name_.c_str(); }
		void					SetName( const char* val ) { name_ = val; }
		const char*				GetShaderCode() const { return hlslCode_.codeText_.c_str(); }
		void					SetShaderCode( const char* val ) { hlslCode_.codeText_ = val; }
		bool					GetTwoSided() const { return twoSided_ > 0; }
		void					SetTwoSided( bool val ) { twoSided_ = val; }
		bool					GetWireframe() const { return wireframe_ > 0; }
		void					SetWireframe( bool val ) { wireframe_ = val; }
		bool					GetZDepthTest() const { return zDepthTest_ > 0; }
		void					SetZDepthTest( bool val ) { zDepthTest_ = val; }
		bool					GetZWrite() const { return zDepthWrite_ > 0; }
		void					SetZWrite( bool val ) { zDepthWrite_ = val; }
		ZTestState				GetZTestState() const { return zTest_; }
		void					SetZTestState( ZTestState val ) { zTest_ = val; }
		float					GetShininess() const { return shininess_; }
		void					SetShininess( float val ) { shininess_ = val; }
		BlendFunction			GetBlendFunction() const { return blendFunc_; }
		void					SetBlendFunction( BlendFunction val ) { blendFunc_ = val; }
		BlendOP					GetBlendOPSrc() const { return blendOPSrc_; }
		void					SetBlendOPSrc( BlendOP val ) { blendOPSrc_ = val; }
		BlendOP					GetBlendOPDst() const { return blendOPDst_; }
		void					SetBlendOPDst( BlendOP val ) { blendOPDst_ = val; }
		Colour					GetDiffuse() const { return diffuse_; }
		void					SetDiffuse( const Colour& val ) { diffuse_ = val; }
		Colour					GetAmbient() const { return ambient_; }
		void					SetAmbient( const Colour& val ) { ambient_ = val; }
		Colour					GetSpecular() const { return specular_; }
		void					SetSpecular( const Colour& val ) { specular_ = val; }
		Colour					GetEmissive() const { return emissive_; }
		void					SetEmissive( const Colour& val ) { emissive_ = val; }
		Data::TextureInstance*	GetDiffuseTexture( u32 idx ) const;
		u32						GetDiffuseTextureHandle( u32 idx ) const
		{
			if ( idx < diffuseTextures_.size() )
				return diffuseTextures_[idx];
			else
				return Data::INVALIDTEXTUREHANDLE;
		}
		void					SetDiffuseTextureHandle( u32 val, u32 idx )
		{
			if ( idx < diffuseTextures_.size() )
				diffuseTextures_[idx] = val;
			else if ( idx < MAX_DIFFUSE_TEXTURES )
			{
				diffuseTextures_.resize( idx+1, Data::INVALIDTEXTUREHANDLE );
				diffuseTextures_[idx] = val;
			}
				
		}
		u32						GetNormalTextureHandle() const { return normalMap_; }
		void					SetNormalTextureHandle( u32 val ) { normalMap_ = val; }
		u32						GetSpecularTextureHandle() const { return specularMap_; }
		void					SetSpecularTextureHandle( u32 val ) { specularMap_ = val; }
		u32						GetLightMapTextureHandle() const { return lightMap_; }
		void					SetLightMapTextureHandle( u32 val ) { lightMap_ = val; }
		
		u32						GetMaterialSceneIndex() const { return materialSceneIndex_; }

		bool					Serialise( TiXmlElement* toelement );
		bool					Deserialise( TiXmlElement* fromelement );

	private:

		typedef std::vector< u32 > TextureVectorType;

		friend void BuildFromaiMaterial( u32 materialSceneIdx, HScene::SceneGraph* pScene, aiMaterial* pinMat, Material* poutMat );

		u32						materialSceneIndex_;
		HScene::SceneGraph*		pScene_;
		std::string				name_;
		TextureVectorType		diffuseTextures_;
		u32						normalMap_;
		u32						specularMap_;
		u32						lightMap_;
		Shader					hlslCode_;
		u32						twoSided_;
		u32						wireframe_;
		u32						zDepthTest_;
		u32						zDepthWrite_;
		ZTestState				zTest_;
		float					shininess_;
		BlendFunction			blendFunc_;
		BlendOP					blendOPSrc_;
		BlendOP					blendOPDst_;
		Colour					diffuse_;
		Colour					ambient_;
		Colour					specular_;
		Colour					emissive_;
	};
}

#endif // DATAMATERIAL_H__