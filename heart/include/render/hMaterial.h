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
            : samplerState_(NULL)
        {
            hZeroMem(name_.GetBuffer(),name_.GetMaxSize());
        }
        ~hSamplerParameter()
        {
        }
        void init(const hChar* name, hSamplerState* ss) {
            hStrCopy(name_, name_.GetMaxSize(), name);
            paramID_=hCRC32::StringCRC(name_);
            samplerState_=ss;
        }

        hArray<hChar, hMAX_PARAMETER_NAME_LEN>  name_;
        hShaderParameterID                      paramID_;
        hSamplerState*                          samplerState_;
    };

    typedef hVector< hSamplerParameter >  hSamplerArrayType;

    class hMaterialCmdLookUpHelper
    {
    public:
        hMaterialCmdLookUpHelper() 
            : count_(0)
            , group_(hNullptr)
            , tech_(hNullptr)
            , pass_(hNullptr)
        {}
        hMaterialCmdLookUpHelper(hMaterialCmdLookUpHelper&& rhs) {
            hPLACEMENT_NEW(this) hMaterialCmdLookUpHelper();
            swap(this, &rhs);
        }
        hMaterialCmdLookUpHelper& operator = (hMaterialCmdLookUpHelper&& rhs) {
            swap(this, &rhs);
            return *this;
        }
        ~hMaterialCmdLookUpHelper() {
            destroy();
        }

        void  init(hMaterial* material);
        void  destroy();
        void  setCommand(hUint group, hUint tech, hUint pass, hUint cmdoffset) {
            hcAssert(&pass_[tech_[group_[group]]+pass] < group_+count_);
            pass_[tech_[group_[group]]+pass]=cmdoffset;
        }
        hUint getCommand(hUint group, hUint tech, hUint pass) const {
            hcAssert(&pass_[tech_[group_[group]]+pass] < group_+count_);
            return pass_[tech_[group_[group]]+pass];
        }

    private:
        HEART_PRIVATE_COPY(hMaterialCmdLookUpHelper);
        static void swap(hMaterialCmdLookUpHelper* lhs, hMaterialCmdLookUpHelper* rhs) {
            std::swap(lhs->count_, rhs->count_);
            std::swap(lhs->group_, rhs->group_);
            std::swap(lhs->tech_,  rhs->tech_);
            std::swap(lhs->pass_,  rhs->pass_);
        }

        hUint  count_;
        hUint* group_;
        hUint* tech_; 
        hUint* pass_; 
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
        hRenderBuffer* constBlock;
    };

    struct hDefaultParameterValue
    {
        hDefaultParameterValue() 
            : type(ePTNone)
            , count(0)
            , paramid(0)
            //, resourcePtr(hNullptr) 
            , dataOffset(0)
        {}
        hParameterType      type;
        hUint               count;
        hShaderParameterID  paramid;
        hResourceID         resourceID;
        hResourceHandle     resourcePtr; // Needs to be merged with resourceID. Resource Handles can be loaded from disk safely now...
        hUint16             dataOffset;
    };

    class HEART_DLLEXPORT hMaterial : public hResourceClassBase
    {
    public:

        hMaterial(hRenderer* renderer);
        ~hMaterial();

        hRenderMaterialManager* GetManager() const { return manager_; }
        void                    SetManager(hRenderMaterialManager* val) { manager_ = val; }
        hMaterialGroup*         addGroup(const hChar* name);
        hUint32                 getGroupCount() const { return (hUint)groups_.size(); }
        hMaterialGroup*         getGroup(hUint idx) { return &groups_[idx]; }
        hMaterialGroup*         getGroupByName(const hChar* name);
        hUint                   getTotalTechniqueCount() const { return totalTechniqueCount_; }
        hUint                   getTotalPassCount() const { return totalPassCount_; }
        hUint32                 GetMatKey() const { return uniqueKey_; }
        void                    addSamplerParameter(const hSamplerParameter& samp);
        void                    addDefaultParameterValue(const hChar* name, const hResourceID& resid);
        void                    addDefaultParameterValue(const hChar* name, const hInt32* data, hUint count);
        void                    addDefaultParameterValue(const hChar* name, const hFloat* data, hUint count);
        void                    addDefaultParameterValue(const hChar* name, const hColour& resid);

        hRCmd* getRenderCommandsBegin(hUint group, hUint tech, hUint pass) {
            return renderCmds_.getCommandAtOffset(passCmds_[techCmds_[groupCmds_[group]]+pass]);
        }
        hRCmd* getRenderCommandsEnd(hUint group, hUint tech, hUint pass) {
            hUint* idx=&passCmds_[techCmds_[groupCmds_[group]]+pass+1];
            return idx < groupCmds_+selectorCount_ ? renderCmds_.getCommandAtOffset(*idx) : renderCmds_.getEnd();
        }

        /* Bind interface - return false if not set on any programs */
        hBool bindConstanstBuffer(hShaderParameterID id, hRenderBuffer* cb);
        hBool bindResource(hShaderParameterID id, hShaderResourceView* srv);
        hBool bindSampler(hShaderParameterID id, hSamplerState* samplerState);
        hBool bind();
        void  unbind();

        /* Allow access to parameter blocks and updating of parameters */
        hRenderBuffer* GetParameterConstBlock(hShaderParameterID cbid);

        void listenToResourceEvents(hResourceManager* resmanager);
        void stopListeningToResourceEvents();

    private:

        hMaterial(const hMaterial&);
        hMaterial& operator = (hMaterial);

        friend class hRenderer;
        friend class hRenderMaterialManager;

        typedef std::vector< hMaterialGroup >           GroupArrayType;
        typedef hVector< hMaterialTechnique >       TechniqueArrayType;
        typedef hVector< hDefaultParameterValue >   DefaultValueArrayType;
        typedef hVector< hBoundConstBlock >         BoundConstBlockArrayType;
        typedef hVector< hBoundResource >           BoundResourceArrayType;
        typedef hVector< hBoundSampler >            BoundSamplerArrayType;

        void initConstBlockBufferData(const hConstantBlockDesc& desc, const hShaderProgram* prog, void* outdata);
        void generateRenderCommands();
        void releaseRenderCommands();
        void updateRenderCommands();
        void cleanup();

        hBool linkDependeeResources();
        hBool resourceUpdate(hResourceID resourceid, hResurceEvent event, hResourceManager* resManager, hResourceClassBase* resource);

        hUint32                             uniqueKey_;
        hRenderer*                          renderer_;
        hRenderMaterialManager*             manager_;
        GroupArrayType                      groups_;

        hUint                               totalTechniqueCount_;
        hUint                               totalPassCount_;
        hUint                               defaultDataSize_;
        hUint8*                             defaultData_;
        DefaultValueArrayType               defaultValues_;
        hSamplerArrayType                   defaultSamplers_;
        BoundConstBlockArrayType            constBlocks_;
        BoundResourceArrayType              boundResources_;
        BoundSamplerArrayType               boundSamplers_;

        hRenderCommands             renderCmds_;
        hUint                       selectorCount_;
        hUint*                      groupCmds_; // pointer within selectorIDs array
        hUint*                      techCmds_;  // pointer within selectorIDs array
        hUint*                      passCmds_;  // pointer within selectorIDs array
    };
}

#endif // HIMATERIAL_H__