/********************************************************************

	filename: 	hMaterial.h	
	
	Copyright (c) 1:4:2012 James Moran
	
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

#ifndef HIMATERIAL_H__
#define HIMATERIAL_H__

class MaterialEffectBuilder;
class ShaderProgramBuilder;

namespace Heart
{
	class hRenderer;
	class hRenderState;
	class hTexture;
    class hMaterialInstance;

    HEARTCORE_SLIBEXPORT
    hUint32 HEART_API hFloatToFixed(hFloat input, hUint32 totalbits, hUint32 fixedpointbits);

    /*
     * See - http://realtimecollisiondetection.net/blog/?p=86/
     * MSB is our leading sort priority, LSB is last sort priority.
     * Our key is using 
     * 63 - 60 = camera / viewport / render target (0-15) - 4 bits = 64
     * 59 - 54 = sort layer (0-15) - 4 bits = 60
     * 53      = transparent flag - force transparent bits to end - 1 bits = 56
     * 52 - 29 = depth - 32 bits as 16:16 fixed point number - 32 bits = 55
     * 28 - 4  = material ID            - 19 bits = 23
     *  3 - 0  = material pass ID (0-15) - 4 bits = 4
     */
    typedef hUint64 hMaterialSortKey;

    HEARTCORE_SLIBEXPORT
    hMaterialSortKey HEART_API hBuildRenderSortKey(hByte cameraID, hByte sortLayer, hBool transparent, hFloat viewSpaceDepth, hUint32 materialID, hByte pass);

    static const hUint32                    hMAX_PARAMETER_NAME_LEN = 32;
    static const hUint32                    hMAX_SEMANTIC_LEN = 32;

    struct HEARTCORE_SLIBEXPORT hSamplerParameter
    {
        hSamplerParameter()
            : boundTexture_(NULL)
            , defaultTextureID_(0)
            , samplerState_(NULL)
            , samplerReg_(hErrorCode)
        {
            hZeroMem(name_.GetBuffer(),name_.GetMaxSize());
        }
        ~hSamplerParameter()
        {
        }

        hArray<hChar, 32>                   name_;
        hUint32                             samplerReg_;
        hResourceID                         defaultTextureID_;
        hTexture*                           boundTexture_;
        hdSamplerState*                     samplerState_;
    };

    typedef hVector< hSamplerParameter >  hSamplerArrayType;

    typedef hUint16 hMaterialParameterID;

    struct hMaterialParameter
    {
        hChar           name_[hMAX_PARAMETER_NAME_LEN];
        hParameterType  type_;
        hUint16         dataOffset_;
    };

    struct hParameterMapping
    {
        // Data travels from cpu -> gpu
        hUint32         sizeBytes_;
        hFloat*         cpuData_;
        hUint32         cReg_;
        hUint32         cOffset_;
    };

    struct hProgramOutput
    {
        hChar                   name_[hMAX_PARAMETER_NAME_LEN];
        hMaterialParameterID    parameterID_;
    };

	class HEARTCORE_SLIBEXPORT hMaterial : public hResourceClassBase
	{
	public:

		hMaterial() 
			: renderer_(NULL)
            , activeTechniques_(NULL)
            , manager_(NULL)
            , totalParameterDataSize_(0)
		{
			
		}
		~hMaterial();

        hUint32                             GetTechniqueCount() const { return activeTechniques_->GetSize(); }
        hMaterialTechnique*                 GetTechnique( hUint32 idx ) { hcAssert( activeTechniques_->GetSize() ); return &(*activeTechniques_)[idx]; }
        hMaterialTechnique*                 GetTechniqueByName( const hChar* name );
        hMaterialTechnique*                 GetTechniqueByMask( hUint32 mask );
        hRenderMaterialManager*             GetManager() const { return manager_; }
        void                                SetManager(hRenderMaterialManager* val) { manager_ = val; }
        hMaterialGroup*                     AddGroup(const hChar* name);
        void                                SetActiveGroup(const hChar* name);
        void                                SetSamplerCount(hUint32 val);
        void                                AddSamplerParameter(const hSamplerParameter& sampler);
        hBool                               Link(hResourceManager* resManager, hRenderer* renderer, hRenderMaterialManager* matManager);
        hUint32                             GetMatKey() const { return uniqueKey_; }
        void                                SetParameterInputOutputReserves(hUint32 totalIn, hUint32 totalOut);
        hMaterialParameterID                AddMaterialParameter(hChar* name, hParameterType type);
        void                                AddProgramOutput(hChar* name, hMaterialParameterID parameterID);

        hMaterialInstance*                  CreateMaterialInstance();
        void                                DestroyMaterialInstance(hMaterialInstance* inst);

	private:

        HEART_ALLOW_SERIALISE_FRIEND();

		friend class hRenderer;
        friend class hRenderMaterialManager;
        friend class ::MaterialEffectBuilder;
       
        typedef hVector< hMaterialGroup >     GroupArrayType;
        typedef hVector< hMaterialTechnique > TechniqueArrayType;
        typedef hVector< hSamplerParameter >  SamplerArrayType;
        typedef hVector< hMaterialParameter > MaterialParametersArrayType;
        typedef hVector< hProgramOutput >     ProgramOutputArrayType;

        hUint32                             uniqueKey_;
		hRenderer*							renderer_;
        hRenderMaterialManager*             manager_;
        GroupArrayType                      groups_;
        TechniqueArrayType*                 activeTechniques_;
        SamplerArrayType                    samplers_;
        MaterialParametersArrayType         materialParameters_;
        ProgramOutputArrayType              programOutputs_;
        hUint32                             totalParameterDataSize_;
	};

    class hMaterialInstance
    {
    public:

        hUint32                             GetTechniqueCount() const { return parentMaterial_->GetTechniqueCount(); }
        hMaterialTechnique*                 GetTechnique( hUint32 idx ) { parentMaterial_->GetTechnique( idx ); }
        hMaterialTechnique*                 GetTechniqueByName( const hChar* name ) { return parentMaterial_->GetTechniqueByName( name ); }
        hMaterialTechnique*                 GetTechniqueByMask( hUint32 mask ) { return parentMaterial_->GetTechniqueByMask( mask ); }
        //hUint32								GetShaderParameterCount() const { return parentMaterial_->GetShaderParameterCount(); }
        //void                                SetShaderParameter( const hShaderParameter* param, hFloat* val, hUint32 size );
        hUint32                             GetSamplerCount() const { return samplers_.GetSize(); }
        const hSamplerParameter*            GetSamplerParameter( hUint32 idx ) const { return &samplers_[idx]; }
        const hSamplerParameter*            GetSamplerParameterByName( const hChar* name );
        void                                SetSamplerParameter( const hSamplerParameter* param, hTexture* tex );
        //hUint32                             GetConstantBufferCount() const { return parentMaterial_->GetConstantBufferCount(); }
        hdParameterConstantBlock*           GetConstantBlock( hUint32 idx ) { return constBuffers_ + idx; }
        const hMaterial*                    GetParentMaterial() const { return parentMaterial_; }
        hUint32                             GetMatKey() const { return matKey_; }

    private:

        friend class hMaterial;

        hPRIVATE_DESTRUCTOR();

        hMaterialInstance( hMaterial* parentMat, hRenderer* renderer );
        ~hMaterialInstance();

        hMaterialInstance( const hMaterialInstance& rhs );
        hMaterialInstance& operator = ( const hMaterialInstance& rhs );
        
        hRenderer*                          renderer_;
        hUint32                             matKey_;
        hMaterial*                          parentMaterial_;
        hUint32                             constBufferCount_;
        hdParameterConstantBlock*           constBuffers_;
        hSamplerArrayType                   samplers_;
        hUint32                             parameterMappingCount_;
        hParameterMapping*                  parameterMappings_;
        hUint32                             cpuDataSizeBytes_;
        hFloat*                             cpuData_;
    };

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    template<>
    inline void SerialiseMethod< Heart::hMaterial >( Heart::hSerialiser* ser, const Heart::hMaterial& data )
    {
//        SERIALISE_ELEMENT( data.groups_ );
    }

    template<>
    inline void DeserialiseMethod< Heart::hMaterial >( Heart::hSerialiser* ser, Heart::hMaterial& data )
    {
//        DESERIALISE_ELEMENT( data.groups_ );
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    template<>
    inline void SerialiseMethod< Heart::hMaterialGroup >( Heart::hSerialiser* ser, const Heart::hMaterialGroup& data )
    {
//         SERIALISE_ELEMENT( data.name_ );
//         SERIALISE_ELEMENT( data.techniques_ );
    }

    template<>
    inline void DeserialiseMethod< Heart::hMaterialGroup >( Heart::hSerialiser* ser, Heart::hMaterialGroup& data )
    {
//         DESERIALISE_ELEMENT( data.name_ );
//         DESERIALISE_ELEMENT( data.techniques_ );
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    template<>
    inline void SerialiseMethod< Heart::hMaterialTechnique >( Heart::hSerialiser* ser, const Heart::hMaterialTechnique& data )
    {
//         SERIALISE_ELEMENT( data.name_ );
//         SERIALISE_ELEMENT( data.passes_ );
//         SERIALISE_ELEMENT( data.layer_ );
//         SERIALISE_ELEMENT( data.transparent_ );
    }

    template<>
    inline void DeserialiseMethod< Heart::hMaterialTechnique >( Heart::hSerialiser* ser, Heart::hMaterialTechnique& data )
    {
//         DESERIALISE_ELEMENT( data.name_ );
//         DESERIALISE_ELEMENT( data.passes_ );
//         DESERIALISE_ELEMENT( data.layer_ );
//         DESERIALISE_ELEMENT( data.transparent_ );
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    template<>
    inline void SerialiseMethod< Heart::hMaterialTechniquePass >( Heart::hSerialiser* ser, const Heart::hMaterialTechniquePass& data )
    {
//         SERIALISE_ELEMENT( data.vertexProgramID_ );
//         SERIALISE_ELEMENT( data.fragmentProgramID_ );
    }

    template<>
    inline void DeserialiseMethod< Heart::hMaterialTechniquePass >( Heart::hSerialiser* ser, Heart::hMaterialTechniquePass& data )
    {
//         DESERIALISE_ELEMENT( data.vertexProgramID_ );
//         DESERIALISE_ELEMENT( data.fragmentProgramID_ );
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    
    template<>
    inline void SerialiseMethod< Heart::hShaderProgram >( Heart::hSerialiser* ser, const Heart::hShaderProgram& data )
    {
//         SERIALISE_ELEMENT( data.vertexInputLayoutFlags_ );
//         SERIALISE_ELEMENT( data.shaderProgramLength_ );
//         SERIALISE_ELEMENT_COUNT( data.shaderProgram_, data.shaderProgramLength_ );
//         SERIALISE_ELEMENT_ENUM_AS_INT( data.shaderType_ );
    }

    template<>
    inline void DeserialiseMethod< Heart::hShaderProgram >( Heart::hSerialiser* ser, Heart::hShaderProgram& data )
    {
//         DESERIALISE_ELEMENT( data.vertexInputLayoutFlags_ );
//         DESERIALISE_ELEMENT( data.shaderProgramLength_ );
//         DESERIALISE_ELEMENT( data.shaderProgram_ );
//         DESERIALISE_ELEMENT_INT_AS_ENUM( data.shaderType_ );
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    template<>
    inline void SerialiseMethod< Heart::hSamplerParameter >( Heart::hSerialiser* ser, const Heart::hSamplerParameter& data )
    {
//         SERIALISE_ELEMENT( data.name_ );
//         SERIALISE_ELEMENT( data.defaultTextureID_ );
    }

    template<>
    inline void DeserialiseMethod< Heart::hSamplerParameter >( Heart::hSerialiser* ser, Heart::hSamplerParameter& data )
    {
//         DESERIALISE_ELEMENT( data.name_ );
//         DESERIALISE_ELEMENT( data.defaultTextureID_ );
    }
}

#endif // HIMATERIAL_H__