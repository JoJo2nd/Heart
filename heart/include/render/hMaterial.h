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
    class hMaterial;
    class hMaterialInstance;
    class hRenderSubmissionCtx;

    HEART_DLLEXPORT
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

    HEART_DLLEXPORT
    hMaterialSortKey HEART_API hBuildRenderSortKey(hByte cameraID, hByte sortLayer, hBool transparent, hFloat viewSpaceDepth, hUint32 materialID, hByte pass);

    static const hUint32                    hMAX_PARAMETER_NAME_LEN = 32;
    static const hUint32                    hMAX_SEMANTIC_LEN = 32;

    struct HEART_DLLEXPORT hSamplerParameter
    {
        hSamplerParameter()
            : boundTexture_(NULL)
            , defaultTextureID_(0)
            , samplerState_(NULL)
        {
            hZeroMem(name_.GetBuffer(),name_.GetMaxSize());
        }
        ~hSamplerParameter()
        {
        }

        hArray<hChar, hMAX_PARAMETER_NAME_LEN> name_;
        hResourceID                         defaultTextureID_;
        hTexture*                           boundTexture_;
        hSamplerState*                      samplerState_;
        hShaderResourceView*                resView_;
    };

    typedef hVector< hSamplerParameter >  hSamplerArrayType;

    struct hBoundTexture
    {
        hShaderParameterID paramid;
        hTexture* texture;
        hSamplerState* state;
    };

    struct hBoundSampler 
    {
        hShaderParameterID paramid;
        hSamplerState* state;
    };

    struct hBoundResource
    {
        hShaderParameterID paramid;
        hShaderResourceView* srv;
    };

    struct hBoundConstBlock
    {
        hShaderParameterID paramid;
        hParameterConstantBlock* constBlock;
    };

    struct hDefaultParameterValue
    {
        hUint16         dataOffset;
        hUint16         dataSize;
        hUint32         parameterNameHash;
        hChar           parameterName[hMAX_PARAMETER_NAME_LEN];
    };

    class HEART_DLLEXPORT hMaterialInstance
    {
    public:

        hUint32             getMaterialKey() const;
        hUint32             GetTechniqueCount() const { return techniques_.GetSize(); }
        hMaterialTechnique* GetTechnique( hUint32 idx ) { hcAssert( techniques_.GetSize() ); return &techniques_[idx]; }
        hMaterialTechnique* GetTechniqueByName( const hChar* name );
        hMaterialTechnique* GetTechniqueByMask( hUint32 mask );
        /* Bind interface - return false if not set on any programs */
        hBool bindConstanstBuffer(hShaderParameterID id, hParameterConstantBlock* cb);
        hBool bindResource(hShaderParameterID id, hShaderResourceView* view);
        hBool bindSampler(hShaderParameterID id, hSamplerState* samplerState);
        hBool bindInputStreams(PrimitiveType type, hIndexBuffer* idx, hVertexBuffer** vtxs, hUint streamCnt);
        hBool bindVertexStream(hUint inputSlot, hVertexBuffer* vtxBuf);
        /* Allow access to parameter blocks and updating of parameters */
        hParameterConstantBlock* GetParameterConstBlock(hShaderParameterID cbid);
        static void destroyMaterialInstance(hMaterialInstance* inst);

    private:

        friend class hMaterial;
        hPRIVATE_DESTRUCTOR();

        typedef hVector< hMaterialTechnique > TechniqueArrayType;
        typedef hVector< hBoundConstBlock > BoundConstBlockArrayType;
        typedef hVector< hBoundResource > BoundResourceArrayType;
        typedef hVector< hBoundSampler > BoundSamplerArrayType;

        hMaterialInstance(hMemoryHeapBase* heap, hMaterial* parent)
            : memHeap_(heap)
            , material_(parent)
            , manager_(NULL)
            , techniques_(heap)
            , constBlocks_(heap)
            , boundResources_(heap)
            , boundSamplers_(heap)
        {
        }
        HEART_PRIVATE_COPY(hMaterialInstance);
        ~hMaterialInstance()
        {}

        hMemoryHeapBase*            memHeap_;
        hMaterial*                  material_;
        hRenderMaterialManager*     manager_;
        TechniqueArrayType          techniques_;
        BoundConstBlockArrayType    constBlocks_;
        BoundResourceArrayType      boundResources_;
        BoundSamplerArrayType       boundSamplers_;
        hUint32                     flags_;
    };

    class HEART_DLLEXPORT hMaterial : public hResourceClassBase
    {
    public:

        hMaterial(hMemoryHeapBase* heap, hRenderer* renderer);
        ~hMaterial();

        hUint32                 GetTechniqueCount() const { hcAssert(activeTechniques_); return activeTechniques_->GetSize(); }
        hMaterialTechnique*     GetTechnique( hUint32 idx ) { hcAssert(activeTechniques_ && activeTechniques_->GetSize() ); return &(*activeTechniques_)[idx]; }
        hMaterialTechnique*     GetTechniqueByName( const hChar* name );
        hMaterialTechnique*     GetTechniqueByMask( hUint32 mask );
        hRenderMaterialManager* GetManager() const { return manager_; }
        void                    SetManager(hRenderMaterialManager* val) { manager_ = val; }
        hMaterialGroup*         AddGroup(const hChar* name);
        void                    SetActiveGroup(const hChar* name);
        hBool                   Link(hResourceManager* resManager, hRenderer* renderer, hRenderMaterialManager* matManager);
        hUint32                 GetMatKey() const { return uniqueKey_; }
        void                    AddSamplerParameter(const hSamplerParameter& samp);
        void                    addDefaultParameterValue(const hChar* paramName, void* data, hUint size);
        
        /* Create Create/DestroyMaterialOverrides()*/
        hMaterialInstance*  createMaterialInstance(hUint32 flags);
        void                destroyMaterialInstance(hMaterialInstance*);

        /* Bind interface - return false if not set on any programs */
        hBool bindConstanstBuffer(hShaderParameterID id, hParameterConstantBlock* cb);
        hBool bindMaterial(hRenderMaterialManager* matManager);
        hBool bindResource(hShaderParameterID id, hShaderResourceView* srv);
        hBool bindSampler(hShaderParameterID id, hSamplerState* samplerState);
        void  unbind();

        /* Allow access to parameter blocks and updating of parameters */
        hParameterConstantBlock* GetParameterConstBlock(hShaderParameterID cbid);

    private:

        HEART_ALLOW_SERIALISE_FRIEND();
        friend class hRenderer;
        friend class hRenderMaterialManager;

        typedef hVector< hMaterialGroup >           GroupArrayType;
        typedef hVector< hMaterialTechnique >       TechniqueArrayType;
        typedef hVector< hDefaultParameterValue >   DefaultValueArrayType;
        typedef hVector< hBoundConstBlock >         BoundConstBlockArrayType;
        typedef hVector< hBoundResource >           BoundResourceArrayType;
        typedef hVector< hBoundSampler >            BoundSamplerArrayType;

        void initConstBlockBufferData(
            const hShaderProgram* prog, const hConstantBlockDesc& desc, void* outinitdata, hUint totaloutsize) const;

        hAtomicInt                          instanceCount_;
        hSamplerArrayType                   defaultSamplers_;
        hMemoryHeapBase*                    memHeap_;
        hUint32                             uniqueKey_;
        hRenderer*                          renderer_;
        hRenderMaterialManager*             manager_;
        GroupArrayType                      groups_;
        TechniqueArrayType*                 activeTechniques_;

        hUint                               defaultDataSize_;
        hUint8*                             defaultData_;
        DefaultValueArrayType               defaultValues_;
        BoundConstBlockArrayType            constBlocks_;
        BoundResourceArrayType              boundResources_;
        BoundSamplerArrayType               boundSamplers_;
    };
}

#endif // HIMATERIAL_H__