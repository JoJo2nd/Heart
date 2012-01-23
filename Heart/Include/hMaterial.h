/********************************************************************
	created:	2009/11/16
	created:	16:11:2009   22:21
	filename: 	hiMaterial.h	
	author:		James
	
	purpose:	
*********************************************************************/
#ifndef HIMATERIAL_H__
#define HIMATERIAL_H__

#include "hArray.h"
#include "hRendererConstants.h"
#include "hResource.h"
#include "hPtrImpl.h"

namespace Heart
{
	class hRenderer;
	class hRenderState;
	class hTexture;
    class hMaterialInstance;

    static const hUint32                    hMAX_SEMANTIC_LEN = 32;


    struct hShaderConstBuffer
    {
        hShaderConstBuffer() 
        {

        }
        
        hUint32 getSize() const; //in Floats
        hFloat* getBufferAddress() const;//
    };

    //Temp placement
    class hShaderProgram : public hPtrImpl< hdShaderProgram >,
                           public hResourceClassBase
    {
    public:
        hShaderProgram() 
            : vertexInputLayoutFlags_(0)
            , shaderProgramLength_(0)
            , shaderProgram_(NULL)
        {

        }
        ~hShaderProgram()
        {
            delete shaderProgram_;
        }

        hUint32                 GetParameterCount() const { return parameterCount_; }
        hUint32                 GetTotalParameterSize() const { return totalParameterSize_; }

#ifndef SHADERBUILDER_EXPORTS
    private:
#endif

        HEART_ALLOW_SERIALISE_FRIEND();

        friend class hRenderer;
        friend class ShaderProgramBuilder;

        hUint32                     vertexInputLayoutFlags_;
        hUint32                     shaderProgramLength_;    
        hByte*                      shaderProgram_;
        hUint32                     parameterCount_;
        hUint32                     totalParameterSize_;
        ShaderType                  shaderType_;
    };

    struct hSamplerParameter
    {
        hSamplerParameter()
            : nameLen_(0)
            , name_(NULL)
            , samplerReg_(~0U)
        {

        }
        void DefaultState();

        hUint32                             nameLen_;
        hChar*                              name_;
        hUint32                             samplerReg_;
        hTexture*                           boundTexture_;
        hSamplerStateDesc                   samplerDesc_;
        hdSamplerState*                     samplerState_;
    };

    typedef hVector< hSamplerParameter >  hSamplerArrayType;

    class hMaterialTechniquePass
    {
    public:

        hMaterialTechniquePass()
            : vertexProgram_(NULL)
            , fragmentProgram_(NULL)
            , blendState_(NULL)
            , depthStencilState_(NULL)
            , rasterizerState_(NULL)
        {

        }

        hShaderProgram*         GetVertexShader() { return vertexProgram_; }
        hShaderProgram*         GetPixelShader() { return fragmentProgram_; }
        hdBlendState*           GetBlendState() { return blendState_; }
        hdDepthStencilState*    GetDepthStencilState() { return depthStencilState_; }
        hdRasterizerState*      GetRasterizerState() { return rasterizerState_; }

    private:

        HEART_ALLOW_SERIALISE_FRIEND();

        friend class hRenderer;
        friend class MaterialEffectBuilder;

        hShaderProgram*                     vertexProgram_;
        hShaderProgram*                     fragmentProgram_;
        hBlendStateDesc                     blendStateDesc_;
        hdBlendState*                       blendState_;
        hDepthStencilStateDesc              depthStencilStateDesc_;
        hdDepthStencilState*                depthStencilState_;
        hRasterizerStateDesc                rasterizerStateDesc_;
        hdRasterizerState*                  rasterizerState_;
    };

    class hMaterialTechnique
    {
    public:

        hMaterialTechnique()
            : mask_(0)
        {
            hZeroMem( &name_[0], MAX_NAME_LEN );
        }
        hMaterialTechnique&             operator = ( const hMaterialTechnique& rhs )
        {
            name_ = rhs.name_;
            rhs.passes_.CopyTo( &passes_ );

            return *this;
        }

        const hChar*            GetName() const { return &name_[0]; }
        hUint32                 GetMask() const { return mask_; }
        hUint32                 GetPassCount() const { return passes_.GetSize(); }
        hMaterialTechniquePass* GetPass( hUint32 idx ) { return &passes_[idx]; }

    private:

        HEART_ALLOW_SERIALISE_FRIEND();

        friend class hRenderer;
        friend class MaterialEffectBuilder;

        static const hUint32 MAX_NAME_LEN = 32;
        typedef hVector< hMaterialTechniquePass > PassArrayType;

        hArray< hChar, MAX_NAME_LEN >   name_;
        PassArrayType                   passes_;
        hUint32                         mask_;//Set on load/create
    };

	class hMaterial : public hResourceClassBase
	{
	public:

		hMaterial( hRenderer* prenderer ) 
			: pRenderer_( prenderer )
		{
			
		}
		~hMaterial() 
		{
		}

        void                                FindOrAddShaderParameter( const hShaderParameter& newParam, const hFloat* defaultVal );
        void                                AddConstBufferDesc( const hChar* name, hUint32 reg, hUint32 size );
		hUint32								GetShaderParameterCount() const { return constParameters_.GetSize(); }
		const hShaderParameter*				GetShaderParameter( const hChar* name ) const;
		const hShaderParameter*				GetShaderParameterByIndex( hUint32 index ) const;
        hUint32                             GetConstantBufferCount() const { return constBufferDescs_.GetSize(); }
        hUint32                             GetConstantBufferSize( hUint32 idx ) const { return constBufferDescs_[idx].size_; }
        hUint32                             GetConstantBufferRegister( hUint32 idx ) const { return constBufferDescs_[idx].reg_; }
        const hFloat*                       GetConstantBufferDefaultData() const;
        hUint32                             GetTechniqueCount() const { return techniques_.GetSize(); }
        hMaterialTechnique*                 GetTechnique( hUint32 idx ) { hcAssert( techniques_.GetSize() ); return &techniques_[idx]; }
        hMaterialTechnique*                 GetTechniqueByName( const hChar* name );
        hMaterialTechnique*                 GetTechniqueByMask( hUint32 mask );
        const hSamplerArrayType&            GetSamplerArray() const { return samplers_; }

	private:

        HEART_ALLOW_SERIALISE_FRIEND();

		friend class hRenderer;
        friend class hRenderMaterialManager;
		friend class MaterialEffectBuilder;
        
        struct hConstBufferDesc
        {
            hConstBufferDesc() 
                : nameCRC_(0)
                , size_(0)
            {
            }
            hUint32     nameCRC_;
            hUint32     reg_;
            hUint32     size_;
        };

        typedef hVector< hMaterialTechnique > TechniqueArrayType;
        typedef hVector< hShaderParameter >   ParameterArrayType;
        typedef hVector< hConstBufferDesc >   ConstBufferDescArrayType;

        hUint32                             FindConstBufferIndexFromID( hUint32 id );
        hMaterialInstance*                  CreateMaterialInstance();
        void                                DestroyMaterialInstance( hMaterialInstance* inst );

		hRenderer*							pRenderer_;
        TechniqueArrayType                  techniques_;
        hSamplerArrayType                   samplers_;
        ParameterArrayType                  constParameters_;
        ConstBufferDescArrayType            constBufferDescs_;
	};

    class hMaterialInstance
    {
    public:

        hUint32                             GetTechniqueCount() const { return parentMaterial_->GetTechniqueCount(); }
        hMaterialTechnique*                 GetTechnique( hUint32 idx ) { parentMaterial_->GetTechnique( idx ); }
        hMaterialTechnique*                 GetTechniqueByName( const hChar* name ) { return parentMaterial_->GetTechniqueByName( name ); }
        hMaterialTechnique*                 GetTechniqueByMask( hUint32 mask ) { return parentMaterial_->GetTechniqueByMask( mask ); }
        hUint32								GetShaderParameterCount() const { return parentMaterial_->GetShaderParameterCount(); }
        const hShaderParameter*				GetShaderParameter( const hChar* name ) const { return parentMaterial_->GetShaderParameter( name ); }
        const hShaderParameter*				GetShaderParameterByIndex( hUint32 index ) const { return parentMaterial_->GetShaderParameterByIndex( index ); }
        void                                SetShaderParameter( const hShaderParameter* param, hFloat* val, hUint32 size );
        hUint32                             GetSamplerCount() const { return samplers_.GetSize(); }
        const hSamplerParameter*            GetSamplerParameter( hUint32 idx ) const { return &samplers_[idx]; }
        const hSamplerParameter*            GetSamplerParameterByName( const hChar* name );
        void                                SetSamplerParameter( const hSamplerParameter* param, hTexture* tex );
        hUint32                             GetConstantBufferCount() const { return parentMaterial_->GetConstantBufferCount(); }
        hdParameterConstantBlock*           GetConstantBlock( hUint32 idx ) { return constBuffers_ + idx; }
        const hMaterial*                    GetParentMaterial() const { return parentMaterial_; };

    private:

        friend class hMaterial;

        hMaterialInstance( hMaterial* parentMat, hRenderer* renderer );
        ~hMaterialInstance();

        hMaterialInstance( const hMaterialInstance& rhs );
        hMaterialInstance& operator = ( const hMaterialInstance& rhs );
        
        hRenderer*                          renderer_;
        hMaterial*                          parentMaterial_;
        hdParameterConstantBlock*           constBuffers_;
        hSamplerArrayType                   samplers_;
    };

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    template<>
    inline void SerialiseMethod< Heart::hMaterial >( Heart::hSerialiser* ser, const Heart::hMaterial& data )
    {
        SERIALISE_ELEMENT( data.techniques_ );
        SERIALISE_ELEMENT( data.samplers_ );
    }

    template<>
    inline void DeserialiseMethod< Heart::hMaterial >( Heart::hSerialiser* ser, Heart::hMaterial& data )
    {
        DESERIALISE_ELEMENT( data.techniques_ );
        DESERIALISE_ELEMENT( data.samplers_ );
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    template<>
    inline void SerialiseMethod< Heart::hMaterialTechnique >( Heart::hSerialiser* ser, const Heart::hMaterialTechnique& data )
    {
        SERIALISE_ELEMENT( data.name_ );
        SERIALISE_ELEMENT( data.passes_ );
    }

    template<>
    inline void DeserialiseMethod< Heart::hMaterialTechnique >( Heart::hSerialiser* ser, Heart::hMaterialTechnique& data )
    {
        DESERIALISE_ELEMENT( data.name_ );
        DESERIALISE_ELEMENT( data.passes_ );
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    template<>
    inline void SerialiseMethod< Heart::hMaterialTechniquePass >( Heart::hSerialiser* ser, const Heart::hMaterialTechniquePass& data )
    {
        SERIALISE_ELEMENT_RESOURCE_CRC( data.vertexProgram_ );
        SERIALISE_ELEMENT_RESOURCE_CRC( data.fragmentProgram_ );
        SERIALISE_ELEMENT( data.blendStateDesc_ );
        SERIALISE_ELEMENT( data.depthStencilStateDesc_ );
        SERIALISE_ELEMENT( data.rasterizerStateDesc_ );
    }

    template<>
    inline void DeserialiseMethod< Heart::hMaterialTechniquePass >( Heart::hSerialiser* ser, Heart::hMaterialTechniquePass& data )
    {
        DESERIALISE_ELEMENT_RESOURCE_CRC( data.vertexProgram_ );
        DESERIALISE_ELEMENT_RESOURCE_CRC( data.fragmentProgram_ );
        DESERIALISE_ELEMENT( data.blendStateDesc_ );
        DESERIALISE_ELEMENT( data.depthStencilStateDesc_ );
        DESERIALISE_ELEMENT( data.rasterizerStateDesc_ );
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    
    template<>
    inline void SerialiseMethod< Heart::hShaderProgram >( Heart::hSerialiser* ser, const Heart::hShaderProgram& data )
    {
        SERIALISE_ELEMENT( data.vertexInputLayoutFlags_ );
        SERIALISE_ELEMENT( data.shaderProgramLength_ );
        SERIALISE_ELEMENT_COUNT( data.shaderProgram_, data.shaderProgramLength_ );
        SERIALISE_ELEMENT( data.totalParameterSize_ );
        SERIALISE_ELEMENT( data.parameterCount_ );
        SERIALISE_ELEMENT_ENUM_AS_INT( data.shaderType_ );
    }

    template<>
    inline void DeserialiseMethod< Heart::hShaderProgram >( Heart::hSerialiser* ser, Heart::hShaderProgram& data )
    {
        DESERIALISE_ELEMENT( data.vertexInputLayoutFlags_ );
        DESERIALISE_ELEMENT( data.shaderProgramLength_ );
        DESERIALISE_ELEMENT( data.shaderProgram_ );
        DESERIALISE_ELEMENT( data.totalParameterSize_ );
        DESERIALISE_ELEMENT( data.parameterCount_ );
        DESERIALISE_ELEMENT_INT_AS_ENUM( data.shaderType_ );
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    template<>
    inline void SerialiseMethod< Heart::hSamplerParameter >( Heart::hSerialiser* ser, const Heart::hSamplerParameter& data )
    {
        SERIALISE_ELEMENT( data.nameLen_ );
        SERIALISE_ELEMENT_COUNT( data.name_, data.nameLen_+1 );
        SERIALISE_ELEMENT_RESOURCE_CRC( data.boundTexture_ );
        SERIALISE_ELEMENT( data.samplerDesc_ );
    }

    template<>
    inline void DeserialiseMethod< Heart::hSamplerParameter >( Heart::hSerialiser* ser, Heart::hSamplerParameter& data )
    {
        DESERIALISE_ELEMENT( data.nameLen_ );
        DESERIALISE_ELEMENT( data.name_ );
        DESERIALISE_ELEMENT_RESOURCE_CRC( data.boundTexture_ );
        DESERIALISE_ELEMENT( data.samplerDesc_ );
    }
}

#endif // HIMATERIAL_H__