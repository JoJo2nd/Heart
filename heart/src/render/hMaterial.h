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

#include "base/hTypes.h"
#include "base/hArray.h"
#include "base/hRendererConstants.h"
#include "base/hStringUtil.h"
#include "base/hCRC32.h"
#include "core/hResource.h"
#include "render/hRenderStateBlock.h"
#include "render/hMaterialTypes.h"
#include "base/hProtobuf.h"
#include "components/hObjectFactory.h"

namespace Heart
{
    class hRenderer;
    class hRenderState;
    class hTexture;
    class hMaterial;
    class hMaterialInstance;
    class hRenderMaterialManager;
    class hRenderSubmissionCtx;

    
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
    public:
        hObjectType(Heart::hMaterial, Heart::proto::MaterialResource);

        hMaterial() {}
        hMaterial(hRenderer* renderer) {}
        ~hMaterial() {}
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

#endif // HIMATERIAL_H__