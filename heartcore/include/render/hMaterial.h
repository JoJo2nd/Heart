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

    static const hUint32                    hMAX_SEMANTIC_LEN = 32;


    struct HEARTCORE_SLIBEXPORT hShaderConstBuffer
    {
        hShaderConstBuffer() 
        {

        }
        
        hUint32 getSize() const; //in Floats
        hFloat* getBufferAddress() const;//
    };

    //Temp placement
    class HEARTCORE_SLIBEXPORT hShaderProgram : public hResourceClassBase,
                                                public hPtrImpl< hdShaderProgram >
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
            hDELETE_ARRAY_SAFE(GetGlobalHeap(), shaderProgram_);
        }

        void                    CopyShaderBlob(void* blob, hUint32 len);
        void*                   GetShaderBlob() const { return (void*)shaderProgram_; }
        hUint32                 GetShaderBlobLen() const { return shaderProgramLength_; }
        void                    SetParameterData(hUint32 paramCount, hUint32 paramBufferSize)
        {
            parameterCount_ = paramCount;
            totalParameterSize_ = paramBufferSize;
        }
        void                    SetVertexLayout(hUint32 layout) { vertexInputLayoutFlags_ = layout; }
        hUint32                 GetVertexLayout() const { return vertexInputLayoutFlags_; }
        hUint32                 GetParameterCount() const { return parameterCount_; }
        hUint32                 GetTotalParameterSize() const { return totalParameterSize_; }
        ShaderType              GetShaderType() const { return shaderType_; } 
        void                    SetShaderType(ShaderType shadertype) { shaderType_ = shadertype; }


    private:

        HEART_ALLOW_SERIALISE_FRIEND();

        friend class hRenderer;
        friend class ::ShaderProgramBuilder;

        hUint32                     vertexInputLayoutFlags_;
        hUint32                     shaderProgramLength_;    
        hByte*                      shaderProgram_;
        hUint32                     parameterCount_;
        hUint32                     totalParameterSize_;
        ShaderType                  shaderType_;
    };

    struct HEARTCORE_SLIBEXPORT hSamplerParameter
    {
        hSamplerParameter()
            : nameLen_(0)
            , boundTexture_(NULL)
            , defaultTextureID_(0)
            , samplerReg_(~0U)
            , samplerState_(NULL)
        {
            hZeroMem(name_.GetBuffer(),name_.GetMaxSize());
        }
        ~hSamplerParameter()
        {
        }

        void DefaultState();

        hUint32                             nameLen_;
        hArray<hChar, 32>                   name_;
        hUint32                             samplerReg_;
        hResourceID                         defaultTextureID_;
        hTexture*                           boundTexture_;
        hSamplerStateDesc                   samplerDesc_;
        hdSamplerState*                     samplerState_;
        
        //HEART_PRIVATE_COPY(hSamplerParameter);
    };

    typedef hVector< hSamplerParameter >  hSamplerArrayType;

    class HEARTCORE_SLIBEXPORT hMaterialTechniquePass
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
        void                    SetVertexShaderResID(hResourceID id) { vertexProgramID_ = id; }
        hShaderProgram*         GetFragmentShader() { return fragmentProgram_; }
        void                    SetFragmentShaderResID(hResourceID id) { fragmentProgramID_ = id; }
        hdBlendState*           GetBlendState() { return blendState_; }
        void                    SetBlendStateDesc(const hBlendStateDesc& blendState) { blendStateDesc_ = blendState; }
        hdDepthStencilState*    GetDepthStencilState() { return depthStencilState_; }
        void                    SetDepthStencilStateDesc(const hDepthStencilStateDesc& depthStencilStateDesc) { depthStencilStateDesc_ = depthStencilStateDesc; }
        hdRasterizerState*      GetRasterizerState() { return rasterizerState_; }
        void                    SetRasterizerStateDesc(const hRasterizerStateDesc& rasterizerStateDesc) { rasterizerStateDesc_ = rasterizerStateDesc; }
        hBool                   Link(hResourceManager* resManager, hRenderer* renderer, hRenderMaterialManager* matManager);
        void                    ReleaseResources(hRenderer* renderer);

    private:

        HEART_ALLOW_SERIALISE_FRIEND();

        friend class hRenderer;
        friend class ::MaterialEffectBuilder;

        hResourceID                         vertexProgramID_;
        hShaderProgram*                     vertexProgram_;
        hResourceID                         fragmentProgramID_;
        hShaderProgram*                     fragmentProgram_;
        hBlendStateDesc                     blendStateDesc_;
        hdBlendState*                       blendState_;
        hDepthStencilStateDesc              depthStencilStateDesc_;
        hdDepthStencilState*                depthStencilState_;
        hRasterizerStateDesc                rasterizerStateDesc_;
        hdRasterizerState*                  rasterizerState_;
    };

    class HEARTCORE_SLIBEXPORT hMaterialTechnique
    {
    public:

        hMaterialTechnique()
            : mask_(0)
        {
            hZeroMem( name_, MAX_NAME_LEN );
        }
        hMaterialTechnique&             operator = ( const hMaterialTechnique& rhs )
        {
            name_ = rhs.name_;
            transparent_ = rhs.transparent_;
            layer_ = rhs.layer_;
            rhs.passes_.CopyTo( &passes_ );

            return *this;
        }
        ~hMaterialTechnique()
        {
        
        }

        const hChar*            GetName() const { return &name_[0]; }
        void                    SetName(const hChar* name) { hStrCopy(name_.GetBuffer(), MAX_NAME_LEN, name); }
        hUint32                 GetMask() const { return mask_; }
        hUint32                 GetPassCount() const { return passes_.GetSize(); }
        hMaterialTechniquePass* GetPass( hUint32 idx ) { return &passes_[idx]; }
        void                    SetSortAsTransparent(hBool val) { transparent_ = val; }
        hBool                   GetSortAsTransparent() const { return transparent_; }
        void                    SetLayer(hByte layer) { layer_ = layer; }
        hByte                   GetLayer(hByte layer) { return layer_; }
        void                    AppendPass(const hMaterialTechniquePass& pass);
        hBool                   Link(hResourceManager* resManager, hRenderer* renderer, hRenderMaterialManager* matManager);

    private:

        HEART_ALLOW_SERIALISE_FRIEND();

        friend class hRenderer;
        friend class ::MaterialEffectBuilder;

        static const hUint32 MAX_NAME_LEN = 32;
        typedef hVector< hMaterialTechniquePass > PassArrayType;

        hArray< hChar, MAX_NAME_LEN >   name_;
        PassArrayType                   passes_;
        hUint32                         mask_;//Set on load/create
        hBool                           transparent_;
        hByte                           layer_;
    };

    struct HEARTCORE_SLIBEXPORT hMaterialGroup
    {
        static const hUint32 MAX_NAME_LEN = 32;
        typedef hVector< hMaterialTechnique > TechniqueArrayType;

        hMaterialGroup& operator = ( const hMaterialGroup& rhs )
        {
            name_ = rhs.name_;
            rhs.techniques_.CopyTo(&techniques_);
            return *this;
        }

        hArray< hChar, MAX_NAME_LEN >   name_;
        TechniqueArrayType              techniques_;
    };

	class HEARTCORE_SLIBEXPORT hMaterial : public hResourceClassBase
	{
	public:

		hMaterial() 
			: renderer_(NULL)
            , activeTechniques_(NULL)
            , manager_(NULL)
		{
			
		}
		~hMaterial();

        void                                FindOrAddShaderParameter( const hShaderParameter& newParam, const hFloat* defaultVal );
        void                                AddConstBufferDesc( const hChar* name, hUint32 reg, hUint32 size );
		hUint32								GetShaderParameterCount() const { return constParameters_.GetSize(); }
		const hShaderParameter*				GetShaderParameter( const hChar* name ) const;
		const hShaderParameter*				GetShaderParameterByIndex( hUint32 index ) const;
        hUint32                             GetConstantBufferCount() const { return constBufferDescs_.GetSize(); }
        hUint32                             GetConstantBufferSize( hUint32 idx ) const { return constBufferDescs_[idx].size_; }
        hUint32                             GetConstantBufferRegister( hUint32 idx ) const { return constBufferDescs_[idx].reg_; }
        const hFloat*                       GetConstantBufferDefaultData() const;
        hUint32                             GetTechniqueCount() const { return activeTechniques_->GetSize(); }
        hMaterialTechnique*                 GetTechnique( hUint32 idx ) { hcAssert( activeTechniques_->GetSize() ); return &(*activeTechniques_)[idx]; }
        hMaterialTechnique*                 GetTechniqueByName( const hChar* name );
        hMaterialTechnique*                 GetTechniqueByMask( hUint32 mask );
        const hSamplerArrayType&            GetSamplerArray() const { return samplers_; }
        hRenderMaterialManager*             GetManager() const { return manager_; }
        void                                SetManager(hRenderMaterialManager* val) { manager_ = val; }
        hMaterialInstance*                  CreateMaterialInstance();
        void                                DestroyMaterialInstance(hMaterialInstance* inst);
        hMaterialGroup*                     AddGroup(const hChar* name);
        void                                SetActiveGroup(const hChar* name);
        void                                AddSamplerParameter(const hSamplerParameter& sampler);
        hBool                               Link(hResourceManager* resManager, hRenderer* renderer, hRenderMaterialManager* matManager);
        hUint32                             GetMatKey() const { return uniqueKey_; }

	private:

        HEART_ALLOW_SERIALISE_FRIEND();

		friend class hRenderer;
        friend class hRenderMaterialManager;
        friend class ::MaterialEffectBuilder;
        
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

        typedef hVector< hMaterialGroup >     GroupArrayType;
        typedef hVector< hMaterialTechnique > TechniqueArrayType;
        typedef hVector< hShaderParameter >   ParameterArrayType;
        typedef hVector< hConstBufferDesc >   ConstBufferDescArrayType;

        hUint32                             FindConstBufferIndexFromID( hUint32 id );

        hUint32                             uniqueKey_;
		hRenderer*							renderer_;
        hRenderMaterialManager*             manager_;
        GroupArrayType                      groups_;
        TechniqueArrayType*                 activeTechniques_;
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
        hdParameterConstantBlock*           constBuffers_;
        hSamplerArrayType                   samplers_;
    };

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    template<>
    inline void SerialiseMethod< Heart::hMaterial >( Heart::hSerialiser* ser, const Heart::hMaterial& data )
    {
        SERIALISE_ELEMENT( data.groups_ );
        SERIALISE_ELEMENT( data.samplers_ );
    }

    template<>
    inline void DeserialiseMethod< Heart::hMaterial >( Heart::hSerialiser* ser, Heart::hMaterial& data )
    {
        DESERIALISE_ELEMENT( data.groups_ );
        DESERIALISE_ELEMENT( data.samplers_ );
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    template<>
    inline void SerialiseMethod< Heart::hMaterialGroup >( Heart::hSerialiser* ser, const Heart::hMaterialGroup& data )
    {
        SERIALISE_ELEMENT( data.name_ );
        SERIALISE_ELEMENT( data.techniques_ );
    }

    template<>
    inline void DeserialiseMethod< Heart::hMaterialGroup >( Heart::hSerialiser* ser, Heart::hMaterialGroup& data )
    {
        DESERIALISE_ELEMENT( data.name_ );
        DESERIALISE_ELEMENT( data.techniques_ );
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    template<>
    inline void SerialiseMethod< Heart::hMaterialTechnique >( Heart::hSerialiser* ser, const Heart::hMaterialTechnique& data )
    {
        SERIALISE_ELEMENT( data.name_ );
        SERIALISE_ELEMENT( data.passes_ );
        SERIALISE_ELEMENT( data.layer_ );
        SERIALISE_ELEMENT( data.transparent_ );
    }

    template<>
    inline void DeserialiseMethod< Heart::hMaterialTechnique >( Heart::hSerialiser* ser, Heart::hMaterialTechnique& data )
    {
        DESERIALISE_ELEMENT( data.name_ );
        DESERIALISE_ELEMENT( data.passes_ );
        DESERIALISE_ELEMENT( data.layer_ );
        DESERIALISE_ELEMENT( data.transparent_ );
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    template<>
    inline void SerialiseMethod< Heart::hMaterialTechniquePass >( Heart::hSerialiser* ser, const Heart::hMaterialTechniquePass& data )
    {
        SERIALISE_ELEMENT( data.vertexProgramID_ );
        SERIALISE_ELEMENT( data.fragmentProgramID_ );
        SERIALISE_ELEMENT( data.blendStateDesc_ );
        SERIALISE_ELEMENT( data.depthStencilStateDesc_ );
        SERIALISE_ELEMENT( data.rasterizerStateDesc_ );
    }

    template<>
    inline void DeserialiseMethod< Heart::hMaterialTechniquePass >( Heart::hSerialiser* ser, Heart::hMaterialTechniquePass& data )
    {
        DESERIALISE_ELEMENT( data.vertexProgramID_ );
        DESERIALISE_ELEMENT( data.fragmentProgramID_ );
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
        SERIALISE_ELEMENT( data.name_ );
        SERIALISE_ELEMENT( data.defaultTextureID_ );
        SERIALISE_ELEMENT( data.samplerDesc_ );
    }

    template<>
    inline void DeserialiseMethod< Heart::hSamplerParameter >( Heart::hSerialiser* ser, Heart::hSamplerParameter& data )
    {
        DESERIALISE_ELEMENT( data.nameLen_ );
        DESERIALISE_ELEMENT( data.name_ );
        DESERIALISE_ELEMENT( data.defaultTextureID_ );
        DESERIALISE_ELEMENT( data.samplerDesc_ );
    }
}

#endif // HIMATERIAL_H__