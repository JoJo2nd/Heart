/********************************************************************
	created:	2008/10/11
	filename: 	DeviceD3D.h
	author:		James Moran
	
	purpose:	
*********************************************************************/
#ifndef DEVICED3D9MATERIAL_H__
#define DEVICED3D9MATERIAL_H__

namespace Heart
{
	// forwards
	class hdD3D9Renderer;
	class hdD3D9Texture;

	enum ShaderType
	{
		ShaderType_VERTEXPROG,
		ShaderType_FRAGMENTPROG,

		ShaderType_MAX,
	};

	class hdMaterialParam
	{
	public:
		hdMaterialParam() {}
	private:
		friend class hdD3D9Material;
		
		hdMaterialParam( const hdMaterialParam& ) {}

		D3DXHANDLE				handle_;
		D3DXCONSTANT_DESC		desc_;
		hdMaterialParam*		next_;// if the parameter name is shared between two the vertex & pixel shaders, next becomes valid
	};

	class hdD3D9Shader
	{
	public:
		hdD3D9Shader()
			: type_( ShaderType_MAX )
			, fShader_( NULL )
			, constTable_( NULL )
		{}

		ShaderType					type_;
		union
		{
			LPDIRECT3DPIXELSHADER9	fShader_;
			LPDIRECT3DVERTEXSHADER9 vShader_;
		};
		LPD3DXCONSTANTTABLE			constTable_; 
	};

	class hdD3D9Material
	{
	public:
		hdD3D9Material()
		{
			
		}
		virtual ~hdD3D9Material();

		const hFloat*						DiffuseColour() const { return (hFloat*)&materialParams_.Diffuse; };
		const hFloat*						SpecularColour() const { return (hFloat*)&materialParams_.Specular; };
		const hFloat*						AmbientColour() const { return (hFloat*)&materialParams_.Ambient; };
		const hFloat*						EmissiveColour() const { return (hFloat*)&materialParams_.Emissive; };
		hFloat								SpecularPower() const { return materialParams_.Power; };
		void								ApplyEffect();
		void								EndEffect();
		void								SetFloatArrayParameter( const hdMaterialParam* param, const hFloat* val, hUint32 count );
		void								SetTextureParameter( const hdMaterialParam* param, const hdD3D9Texture* pITexture );
		virtual void						SetFloatArrayParameter( hUint32 index, const hFloat* val, hUint32 count );
		virtual void						SetTextureParameter( hUint32 index, const hdD3D9Texture* pTexture );
		const hdMaterialParam*				GetInvalidParamGUID() const;
		const hdMaterialParam*				GetParameterGUID( const char* paramName ) const;
		const hdMaterialParam*				GetParameterGUID( hUint32 index ) const;
		void								CommitChanges();

	private:

		void								InitialiseEffectParameters( hUint32 paramCount );

		friend class hdD3D9Renderer;
		friend class MaterialBuilder;

		static hdMaterialParam			gInvalidParameter_;

		D3DMATERIAL9					materialParams_;
		hdD3D9Shader*					vertexShader_;
		hdD3D9Shader*					fragmentShader_;
		hUint32							nParameters_;
		hdMaterialParam*				pParameters_;
	};
}

#endif // _DEVICED3D9MATERIAL_H__