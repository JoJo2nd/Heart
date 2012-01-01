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
#include "hTextureBase.h"

namespace Heart
{
	class hRenderer;
	class hRenderState;
	class hTexture;

    static const hUint32                    hMAX_SEMANTIC_LEN = 32;


    struct hShaderConstBuffer
    {
        hShaderConstBuffer() 
            : size_(0)
            , buffer_(NULL)
        {

        }
        hUint32     size_;//in multiples of 16 (float4)
        hFloat*     buffer_;
    };

    //Temp placement
    class hShaderProgram : public pimpl< hdShaderProgram >,
                           public hResourceClassBase
    {
    public:
        hShaderProgram() 
            : shaderProgramLength_(0)
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

        hUint32                     shaderProgramLength_;    
        hByte*                      shaderProgram_;
        hUint32                     parameterCount_;
        hUint32                     totalParameterSize_;
        ShaderType                  shaderType_;
    };

    struct hSamplerParameter
    {
        void DefaultState();

        EffectSemantics::EffectSemanticID   semanticID_; //EFFECTSEMANTICID_MAX if not valid
        hChar                               semanticName_[hMAX_SEMANTIC_LEN];
        hTexture*                           boundTexture_;
        hColour                             borderColour_;
        hSAMPLER_STATE_VALUE                minFilter_;
        hSAMPLER_STATE_VALUE                magFilter_;
        hSAMPLER_STATE_VALUE                mipFilter_;
        hSAMPLER_STATE_VALUE                addressU_;
        hSAMPLER_STATE_VALUE                addressV_;
        hSAMPLER_STATE_VALUE                addressW_;
        hFloat                              mipLODBias_;
        hUint32                             maxAnisotropy_;
        void*                               deviceSampler_;
    };

    class hMaterialTechniquePass
    {
    public:

        void DefaultState();

    private:

        HEART_ALLOW_SERIALISE_FRIEND();

        friend class hRenderer;
        friend class MaterialEffectBuilder;

        hShaderProgram*                     vertexProgram_;
        hShaderProgram*                     fragmentProgram_;
        RENDER_STATE_VALUE                  alphaBlendEnable_;
        RENDER_STATE_VALUE                  alphaSrcBlend_;
        RENDER_STATE_VALUE                  alphaDstBlend_;
        RENDER_STATE_VALUE                  alphaBlendFunction_;
        RENDER_STATE_VALUE                  zTestEnable_;
        RENDER_STATE_VALUE                  zWriteEnable_;
        RENDER_STATE_VALUE                  zCompareFunction_;
        RENDER_STATE_VALUE                  cullMode_;
        RENDER_STATE_VALUE                  fillMode_;
        RENDER_STATE_VALUE                  stencilTest_;
        RENDER_STATE_VALUE                  stencilFail_;
        RENDER_STATE_VALUE                  stencilZFail_;
        RENDER_STATE_VALUE                  stencilPass_;
        RENDER_STATE_VALUE                  stencilFunc_;
        hUint32                             stencilRef_;
        hUint32                             stencilWriteMask_;
        hUint32                             stencilMask_;
        hUint32                             colourMask1_;
        hUint32                             colourMask2_;
        hUint32                             colourMask3_;
        hUint32                             colourMask4_;
    };

    class hMaterialTechnique
    {
    public:

        void                            Serialise( hSerialiser* ser ) const;
        void                            Deserialise( hSerialiser* ser );

        hMaterialTechnique&             operator = ( const hMaterialTechnique& rhs )
        {
            name_ = rhs.name_;
            rhs.passes_.CopyTo( &passes_ );

            return *this;
        }

    private:

        friend class hRenderer;
        friend class MaterialEffectBuilder;

        static const hUint32 MAX_NAME_LEN = 32;
        typedef hVector< hMaterialTechniquePass > PassArrayType;

        hArray< hChar, MAX_NAME_LEN >   name_;
        PassArrayType                   passes_;
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

        void                                FindOrAddShaderParameter( const hShaderParameter& newParam );
		void								SetFloatArrayParameter( const hShaderParameter* param, hFloat* val, hUint32 nCount );
		void								BindTextureParameter( const hShaderParameter* param, const hResourceHandle< hTextureBase >& tex );
		hUint32								GetShaderParameterCount() const { return 0/*nShaderParameters_*/; }
		const hShaderParameter*				GetShaderParameter( const hChar* name ) const;
		const hShaderParameter*				GetShaderParameter( EffectSemantics::EffectSemanticID id ) const;
		const hShaderParameter*				GetShaderParameter( hUint32 index ) const;
		//////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////
		void								Release();
        void                                Serialise( hSerialiser* ser ) const;
        void                                Deserialise( hSerialiser* ser );

	private:

		friend class hRenderer;
		friend class MaterialEffectBuilder;
        

        typedef hVector< hMaterialTechnique > TechniqueArrayType;
        typedef hVector< hSamplerParameter >  SamplerArrayType;
        typedef hVector< hShaderParameter >   ParameterArrayType;

		hRenderer*							pRenderer_;
        TechniqueArrayType                  techniques_;
        SamplerArrayType                    samplers_;
        ParameterArrayType                  constParameters_;
	};


    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    template<>
    inline void SerialiseMethod< Heart::hMaterial >( Heart::hSerialiser* ser, const Heart::hMaterial& data )
    {
        data.Serialise( ser );
    }

    template<>
    inline void DeserialiseMethod< Heart::hMaterial >( Heart::hSerialiser* ser, Heart::hMaterial& data )
    {
        data.Deserialise( ser );
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    template<>
    inline void SerialiseMethod< Heart::hMaterialTechnique >( Heart::hSerialiser* ser, const Heart::hMaterialTechnique& data )
    {
        data.Serialise( ser );
    }

    template<>
    inline void DeserialiseMethod< Heart::hMaterialTechnique >( Heart::hSerialiser* ser, Heart::hMaterialTechnique& data )
    {
        data.Deserialise( ser );
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    template<>
    inline void SerialiseMethod< Heart::hMaterialTechniquePass >( Heart::hSerialiser* ser, const Heart::hMaterialTechniquePass& data )
    {
        SERIALISE_ELEMENT_PTR_AS_INT( data.vertexProgram_ );
        SERIALISE_ELEMENT_PTR_AS_INT( data.fragmentProgram_ );
        SERIALISE_ELEMENT_ENUM_AS_INT( data.alphaBlendEnable_ );
        SERIALISE_ELEMENT_ENUM_AS_INT( data.alphaSrcBlend_ );
        SERIALISE_ELEMENT_ENUM_AS_INT( data.alphaDstBlend_ );
        SERIALISE_ELEMENT_ENUM_AS_INT( data.alphaBlendFunction_ );
        SERIALISE_ELEMENT_ENUM_AS_INT( data.zTestEnable_ );
        SERIALISE_ELEMENT_ENUM_AS_INT( data.zWriteEnable_ );
        SERIALISE_ELEMENT_ENUM_AS_INT( data.zCompareFunction_ );
        SERIALISE_ELEMENT_ENUM_AS_INT( data.cullMode_ );
        SERIALISE_ELEMENT_ENUM_AS_INT( data.fillMode_ );
        SERIALISE_ELEMENT_ENUM_AS_INT( data.stencilTest_ );
        SERIALISE_ELEMENT_ENUM_AS_INT( data.stencilFail_ );
        SERIALISE_ELEMENT_ENUM_AS_INT( data.stencilZFail_ );
        SERIALISE_ELEMENT_ENUM_AS_INT( data.stencilPass_ );
        SERIALISE_ELEMENT_ENUM_AS_INT( data.stencilFunc_ );
        SERIALISE_ELEMENT( data.stencilRef_ );
        SERIALISE_ELEMENT( data.stencilWriteMask_ );
        SERIALISE_ELEMENT( data.stencilMask_ );
        SERIALISE_ELEMENT( data.colourMask1_ );
        SERIALISE_ELEMENT( data.colourMask2_ );
        SERIALISE_ELEMENT( data.colourMask3_ );
        SERIALISE_ELEMENT( data.colourMask4_ );
    }

    template<>
    inline void DeserialiseMethod< Heart::hMaterialTechniquePass >( Heart::hSerialiser* ser, Heart::hMaterialTechniquePass& data )
    {
        DESERIALISE_ELEMENT_INT_AS_PTR( data.vertexProgram_ );
        DESERIALISE_ELEMENT_INT_AS_PTR( data.fragmentProgram_ );
        DESERIALISE_ELEMENT_INT_AS_ENUM( data.alphaBlendEnable_ );
        DESERIALISE_ELEMENT_INT_AS_ENUM( data.alphaSrcBlend_ );
        DESERIALISE_ELEMENT_INT_AS_ENUM( data.alphaDstBlend_ );
        DESERIALISE_ELEMENT_INT_AS_ENUM( data.alphaBlendFunction_ );
        DESERIALISE_ELEMENT_INT_AS_ENUM( data.zTestEnable_ );
        DESERIALISE_ELEMENT_INT_AS_ENUM( data.zWriteEnable_ );
        DESERIALISE_ELEMENT_INT_AS_ENUM( data.zCompareFunction_ );
        DESERIALISE_ELEMENT_INT_AS_ENUM( data.cullMode_ );
        DESERIALISE_ELEMENT_INT_AS_ENUM( data.fillMode_ );
        DESERIALISE_ELEMENT_INT_AS_ENUM( data.stencilTest_ );
        DESERIALISE_ELEMENT_INT_AS_ENUM( data.stencilFail_ );
        DESERIALISE_ELEMENT_INT_AS_ENUM( data.stencilZFail_ );
        DESERIALISE_ELEMENT_INT_AS_ENUM( data.stencilPass_ );
        DESERIALISE_ELEMENT_INT_AS_ENUM( data.stencilFunc_ );
        DESERIALISE_ELEMENT( data.stencilRef_ );
        DESERIALISE_ELEMENT( data.stencilWriteMask_ );
        DESERIALISE_ELEMENT( data.stencilMask_ );
        DESERIALISE_ELEMENT( data.colourMask1_ );
        DESERIALISE_ELEMENT( data.colourMask2_ );
        DESERIALISE_ELEMENT( data.colourMask3_ );
        DESERIALISE_ELEMENT( data.colourMask4_ );
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    
    template<>
    inline void SerialiseMethod< Heart::hShaderProgram >( Heart::hSerialiser* ser, const Heart::hShaderProgram& data )
    {
        SERIALISE_ELEMENT( data.shaderProgramLength_ );
        SERIALISE_ELEMENT_COUNT( data.shaderProgram_, data.shaderProgramLength_ );
        SERIALISE_ELEMENT( data.totalParameterSize_ );
        SERIALISE_ELEMENT( data.parameterCount_ );
        SERIALISE_ELEMENT_ENUM_AS_INT( data.shaderType_ );
    }

    template<>
    inline void DeserialiseMethod< Heart::hShaderProgram >( Heart::hSerialiser* ser, Heart::hShaderProgram& data )
    {
        DESERIALISE_ELEMENT( data.shaderProgramLength_ );
        DESERIALISE_ELEMENT( data.shaderProgram_ );
        DESERIALISE_ELEMENT( data.totalParameterSize_ );
        DESERIALISE_ELEMENT( data.parameterCount_ );
        DESERIALISE_ELEMENT_INT_AS_ENUM( data.shaderType_ );
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    
//     template<>
//     inline void SerialiseMethod< Heart::hShaderParameter >( Heart::hSerialiser* ser, const Heart::hShaderParameter& data )
//     {
//         SERIALISE_ELEMENT_ENUM_AS_INT( data.semanticID_ ); 
//         SERIALISE_ELEMENT( data.semanticName_ );
//         SERIALISE_ELEMENT_COUNT( data.name_, data.nameLength_ ); 
//     }
// 
//     template<>
//     inline void DeserialiseMethod< Heart::hShaderParameter >( Heart::hSerialiser* ser, Heart::hShaderParameter& data )
//     {
//         DESERIALISE_ELEMENT_INT_AS_ENUM( data.semanticID_ ); 
//         DESERIALISE_ELEMENT( data.semanticName_ );
//         DESERIALISE_ELEMENT( data.name_); 
//     }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    template<>
    inline void SerialiseMethod< Heart::hShaderConstBuffer >( Heart::hSerialiser* ser, const Heart::hShaderConstBuffer& data )
    {
        SERIALISE_ELEMENT( data.size_ );
        SERIALISE_ELEMENT_COUNT( data.buffer_, data.size_ ); 
    }

    template<>
    inline void DeserialiseMethod< Heart::hShaderConstBuffer >( Heart::hSerialiser* ser, Heart::hShaderConstBuffer& data )
    {
        DESERIALISE_ELEMENT( data.size_ );
        DESERIALISE_ELEMENT( data.buffer_ ); 
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    template<>
    inline void SerialiseMethod< Heart::hSamplerParameter >( Heart::hSerialiser* ser, const Heart::hSamplerParameter& data )
    {
        SERIALISE_ELEMENT( data.semanticName_ );
        SERIALISE_ELEMENT_PTR_AS_INT( data.boundTexture_ );
        SERIALISE_ELEMENT( data.borderColour_ );
        SERIALISE_ELEMENT_ENUM_AS_INT( data.minFilter_ );
        SERIALISE_ELEMENT_ENUM_AS_INT( data.magFilter_ );
        SERIALISE_ELEMENT_ENUM_AS_INT( data.mipFilter_ );
        SERIALISE_ELEMENT_ENUM_AS_INT( data.addressU_ );
        SERIALISE_ELEMENT_ENUM_AS_INT( data.addressV_ );
        SERIALISE_ELEMENT_ENUM_AS_INT( data.addressW_ );
        SERIALISE_ELEMENT( data.mipLODBias_ );
        SERIALISE_ELEMENT( data.maxAnisotropy_ );
    }

    template<>
    inline void DeserialiseMethod< Heart::hSamplerParameter >( Heart::hSerialiser* ser, Heart::hSamplerParameter& data )
    {
        DESERIALISE_ELEMENT( data.semanticName_ );
        DESERIALISE_ELEMENT_INT_AS_PTR( data.boundTexture_ );
        DESERIALISE_ELEMENT( data.borderColour_ );
        DESERIALISE_ELEMENT_INT_AS_ENUM( data.minFilter_ );
        DESERIALISE_ELEMENT_INT_AS_ENUM( data.magFilter_ );
        DESERIALISE_ELEMENT_INT_AS_ENUM( data.mipFilter_ );
        DESERIALISE_ELEMENT_INT_AS_ENUM( data.addressU_ );
        DESERIALISE_ELEMENT_INT_AS_ENUM( data.addressV_ );
        DESERIALISE_ELEMENT_INT_AS_ENUM( data.addressW_ );
        DESERIALISE_ELEMENT( data.mipLODBias_ );
        DESERIALISE_ELEMENT( data.maxAnisotropy_ );
    }
}

#endif // HIMATERIAL_H__