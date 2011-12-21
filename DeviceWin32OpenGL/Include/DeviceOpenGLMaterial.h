/********************************************************************

	filename: 	DeviceOpenGLMaterial.h	
	
	Copyright (c) 26:7:2011 James Moran
	
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
#ifndef DEVICEOPENGLMATERIAL_H__
#define DEVICEOPENGLMATERIAL_H__

namespace Heart
{
	class hdMaterialParam
	{
	public:
		hdMaterialParam() {}
	private:
		friend class hdOGLMaterial;
		friend bool operator == ( const hdMaterialParam&, const hdMaterialParam& );

		hdMaterialParam( const hdMaterialParam& ) {}
	};

	//////////////////////////////////////////////////////////////////////////
	// Class to cleanly pass around a compiled shader resource. This class ///
	// may become managed by the DeviceRenderer to prevent multiple copies ///
	// of this class existing ////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	class CompiledShaderResource
	{
	public:

	private:

		friend class hdOGLMaterial;
		friend class MaterialBuilder;
	};

	class hdOGLMaterial
	{
	public:
		hdOGLMaterial() {}
		~hdOGLMaterial() {}

		const hFloat*						DiffuseColour() const { return NULL; };
		const hFloat*						SpecularColour() const { return NULL; };
		const hFloat*						AmbientColour() const { return NULL; };
		const hFloat*						EmissiveColour() const { return NULL; };
		hFloat								SpecularPower() const { return 0; };
		void								ApplyEffect() {}
		void								EndEffect() {}
		void								SetFloatArrayParameter( const hdMaterialParam* param, const hFloat* val, hUint32 count ) {}
		void								SetTextureParameter( const hdMaterialParam* param, const hdOGLTexture* pITexture ) {}
		virtual void						SetFloatArrayParameter( hUint32 index, const hFloat* val, hUint32 count ) {}
		virtual void						SetTextureParameter( hUint32 index, const hdOGLTexture* pTexture ) {}
		const hdMaterialParam*				GetInvalidParamGUID() const { return NULL; }
		const hdMaterialParam*				GetParameterGUID( const char* paramName ) const { return NULL; }
		const hdMaterialParam*				GetParameterBySemantic( const hChar* semantic ) const { return NULL; }
		const hdMaterialParam*				GetParameterGUID( hUint32 index ) const { return NULL; }
		const hChar*						GetParameterSemantic( hUint32 index ) const { return NULL; }
		const hChar*						GetParameterName( hUint32 index ) const { return NULL; }
		void								CommitChanges() {}
		//void								Validate() const;

	private:

	};
}

#endif // DEVICEOPENGLMATERIAL_H__