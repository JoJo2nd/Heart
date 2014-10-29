/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#pragma once

#include "base/hTypes.h"
#include "base/hArray.h"
#include "base/hRendererConstants.h"
#include "base/hStringUtil.h"
#include "base/hCRC32.h"
#include "core/hResource.h"
#include "render/hRenderStateBlock.h"
#include "render/hMaterialTypes.h"
#include "render/hRenderCallDesc.h"
#include "base/hProtobuf.h"
#include "components/hObjectFactory.h"

#include "cryptoMurmurHash.h"

#include <unordered_map>
#include <memory>

namespace Heart {
    
    class hRenderState;
    class hTexture;
    class hMaterial;
    class hMaterialInstance;
    class hRenderMaterialManager;
    class hRenderSubmissionCtx;

    struct hPassKey {
        hPassKey(hStringID g, hStringID t, hUint p) 
            : group_(g), tech_(t), pass_(p) {}
        hStringID group_;
        hStringID tech_;
        hUint     pass_;
    };
}
namespace std {
    template<>
    struct hash<Heart::hPassKey> {
        size_t operator () (const Heart::hPassKey& rhs) const { 
            hUint32 r;
            hUint32 a[3] = {rhs.group_.hash(), rhs.tech_.hash(), rhs.pass_};
            cyMurmurHash3_x86_32(a, sizeof(a), hGetMurmurHashSeed(), &r);
            return r;
        }
    };

}
namespace Heart {
    
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

    
    hMaterialSortKey HEART_API hBuildRenderSortKey(hByte cameraID, hByte sortLayer, hBool transparent, hFloat viewSpaceDepth, hUint32 materialID, hByte pass);

    class  hMaterial
    {
        typedef std::unordered_map<hPassKey, hRenderer::hRenderCallDesc*> hPassHashTable;

        hPassHashTable                              passTable_;
        std::unique_ptr<hRenderer::hRenderCallDesc> rcDescs_;
        std::vector<hStringID>                      groupNames_;
        std::vector<hStringID>                      techniqueNames_;
        hUint                                       totalPasses_;

    public:
        hObjectType(Heart::hMaterial, Heart::proto::MaterialResource);

        hMaterial() {}
        ~hMaterial() {}

        hUint                               getGroupCount() const;
        hStringID                           getGroupName(hUint index) const;
        hUint                               getTechniqueCount(hStringID group) const;
        hStringID                           getTechniqueName(hStringID group, hUint index) const;
        const hRenderer::hRenderCallDesc&   getPass(hStringID group, hStringID tech, hUint pass);
#if 0
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
        void                    addDefaultParameterValue(const hChar* name, hStringID resid);
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

        void listenToResourceEvents();
        void stopListeningToResourceEvents();

    private:

        hMaterial(const hMaterial&);
        hMaterial& operator = (hMaterial);

        
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
        hBool resourceUpdate(hStringID resourceid, hResurceEvent event);

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
#endif
    };
}