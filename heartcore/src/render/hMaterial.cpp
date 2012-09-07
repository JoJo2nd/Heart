/********************************************************************

	filename: 	hMaterial.cpp	
	
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


namespace Heart
{

    HEARTCORE_SLIBEXPORT
    hUint32 HEART_API hFloatToFixed(hFloat input, hUint32 totalbits, hUint32 fixedpointbits)
    {
        hUint32 factor = 1 << fixedpointbits;
        hUint32 fp = (hUint32)(input*factor);
        
        //discard the LSB, this means that fixedpointbits will be reduced
        return fp >> (32-totalbits);
    }

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
     *  
     *  NOTE:
     *  When transparent material ID & pass are swapped with depth.
     */
    typedef hUint64 hMaterialSortKey;

    HEARTCORE_SLIBEXPORT
    hMaterialSortKey HEART_API hBuildRenderSortKey(hByte cameraID, hByte sortLayer, hBool transparent, hFloat viewSpaceDepth, hUint32 materialID, hByte pass)
    {
        hUint64 msb1  = (hUint64)(cameraID&0xF)  << 60;
        hUint64 msb2  = (hUint64)(sortLayer&0xF) << 56;
        hUint32 msb3  = (hUint64)(transparent&0x1) << 55;
        hUint64 msb4  = (hUint64)(hFloatToFixed(viewSpaceDepth,32,16)&0xFFFFFFFF) << 19;
        hUint64 msb4t = (hUint64)((~hFloatToFixed(viewSpaceDepth,32,16))&0xFFFFFFFF) << 19;
        hUint64 msb5  = (hUint64)(materialID&0x3FFFF) << 4;
        hUint64 msb6  = (hUint64)(pass&0xF);
        return transparent ? (msb1 | msb2 | msb3 | msb4 | msb5 | msb6) : (msb1 | msb2 | msb3 | msb5 | msb6 | msb4t);
    }

    static const hUint32 VIEWPORT_CONST_BUFFER_ID = hCRC32::StringCRC( "ViewportConstants" );
    static const hUint32 INSTANCE_CONST_BUFFER_ID = hCRC32::StringCRC( "InstanceConstants" );


    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hMaterial::~hMaterial()
    {
        if (renderer_)
        {
            /* - TODO: FIX ME
            for (hUint32 i = 0, c = samplers_.GetSize(); i < c; ++i)
            {
                renderer_->DestroySamplerState(samplers_[i].samplerState_);
                samplers_[i].samplerState_ = NULL;

                 if (samplers_[i].boundTexture_)
                 {
                     samplers_[i].boundTexture_->DecRef();
                 }
            }
            }*/
        }
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hMaterialTechnique* hMaterial::GetTechniqueByName( const hChar* name )
    {
        for ( hUint32 i = 0; i < activeTechniques_->GetSize(); ++i )
        {
            if ( hStrCmp( (*activeTechniques_)[i].GetName(), name ) == 0 )
            {
                return &(*activeTechniques_)[i];
            }
        }

        return NULL;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hMaterialTechnique* hMaterial::GetTechniqueByMask( hUint32 mask )
    {
        for ( hUint32 i = 0; i < activeTechniques_->GetSize(); ++i )
        {
            if ( (*activeTechniques_)[i].GetMask() == mask )
            {
                return &(*activeTechniques_)[i];
            }
        }

        return NULL;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hMaterialInstance* hMaterial::CreateMaterialInstance()
    {
        return hNEW(GetGlobalHeap()/*!heap*/, hMaterialInstance)(this, renderer_);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hMaterial::DestroyMaterialInstance( hMaterialInstance* inst )
    {
        hDELETE(GetGlobalHeap()/*!heap*/, inst);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hMaterialGroup* hMaterial::AddGroup( const hChar* name )
    {
        hMaterialGroup newGroup;
        hStrCopy(newGroup.name_.GetBuffer(), newGroup.name_.GetMaxSize(), name);
        groups_.PushBack(newGroup);
        return &groups_[groups_.GetSize()-1];
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hMaterial::SetActiveGroup( const hChar* name )
    {
        hUint32 groups = groups_.GetSize();
        for ( hUint32 i = 0; i < groups; ++i)
        {
            if (hStrICmp(name, groups_[i].name_) == 0)
            {
                activeTechniques_ = &groups_[i].techniques_;
                return;
            }
        }

        hcAssertFailMsg("Couldn't find group %s", name);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hBool hMaterial::Link(hResourceManager* resManager, hRenderer* renderer, hRenderMaterialManager* matManager)
    {
        for (hUint32 i = 0; i < )
        return hTrue;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hMaterial::SetParameterInputOutputReserves( hUint32 totalIn, hUint32 totalOut )
    {
        materialParameters_.Reserve(totalIn);
        programOutputs_.Reserve(totalOut);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hMaterialParameterID hMaterial::AddMaterialParameter( hChar* name, hParameterType type )
    {
        //Don't want to grow.
        hcAssert(materialParameters_.GetSize() < materialParameters_.GetReserve());
        hMaterialParameterID id = materialParameters_.GetSize();
        hMaterialParameter param;
        hStrCopy(param.name_, hMAX_PARAMETER_NAME_LEN, name);
        param.type_ = type;
        param.dataOffset_ = totalParameterDataSize_;
        switch(type)
        {
        case ePTFloat1:
        case ePTFloat2:
        case ePTFloat3:
        case ePTFloat4:
            totalParameterDataSize_ += sizeof(hFloat)*4;
            break;
        case ePTFloat3x3:
        case ePTFloat4x4:
            totalParameterDataSize_ += sizeof(hFloat)*16;
            break;
        }

        materialParameters_.PushBack(param);

        return id;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hMaterial::AddProgramOutput( hChar* name, hMaterialParameterID parameterID )
    {
        hcAssert(programOutputs_.GetSize() < programOutputs_.GetReserve());

        hProgramOutput outp;
        hStrCopy(outp.name_, hMAX_PARAMETER_NAME_LEN, name);
        outp.parameterID_ = parameterID;

        programOutputs_.PushBack(outp);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hMaterial::SetSamplerCount( hUint32 val )
    {
        samplers_.Reserve(val);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hMaterial::AddSamplerParameter( const hSamplerParameter& sampler )
    {
        samplers_.PushBack(sampler);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hMaterialInstance::hMaterialInstance( hMaterial* parentMat, hRenderer* renderer ) 
        : renderer_(renderer)
        , parentMaterial_(parentMat)
    {
        hcAssert( parentMat );
/*
        hUint32 cbCount = parentMat->GetConstantBufferCount();
        hUint32* sizes = (hUint32*)hAlloca( cbCount*sizeof(hUint32) );
        hUint32* regs = (hUint32*)hAlloca( cbCount*sizeof(hUint32) );
        for ( hUint32 i = 0; i < cbCount; ++i )
        {
            sizes[i] = parentMat->GetConstantBufferSize( i );
            regs[i] = parentMat->GetConstantBufferRegister( i );
        }
        constBuffers_ = renderer->CreateConstantBlocks( sizes, regs, parentMat->GetConstantBufferCount() );

        parentMaterial_->GetSamplerArray().CopyTo( &samplers_ );
*/
        matKey_ = parentMaterial_->GetMatKey();
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    hMaterialInstance::~hMaterialInstance()
    {
        renderer_->DestroyConstantBlocks(constBuffers_, constBufferCount_);
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
/*
    void hMaterialInstance::SetShaderParameter( const hShaderParameter* param, hFloat* val, hUint32 size )
    {
//         hcAssert( param && val && size );
//         hcAssert( param >= parentMaterial_->GetShaderParameterByIndex(0) && 
//                   param <= parentMaterial_->GetShaderParameterByIndex(parentMaterial_->GetShaderParameterCount()-1) );
// 
//         hdParameterConstantBlock& cb = constBuffers_[param->cBuffer_];
//         hFloat* dst = cb.GetBufferAddress() + param->cReg_;
//         for ( hUint32 i = 0; i < size && i < param->size_; ++i )
//         {
//             dst[i] = val[i];
//         }
    }
*/
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    const hSamplerParameter* hMaterialInstance::GetSamplerParameterByName( const hChar* name )
    {
        hUint32 count = samplers_.GetSize();
        for ( hUint32 i = 0; i < count; ++i )
        {
            if ( hStrCmp( samplers_[i].name_, name ) == 0 )
            {
                return &samplers_[i];
            }
        }

        return NULL;
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    void hMaterialInstance::SetSamplerParameter( const hSamplerParameter* param, hTexture* tex )
    {
        hSamplerParameter* p = const_cast< hSamplerParameter* >( param );
        p->boundTexture_ = tex;
    }

}//Heart
