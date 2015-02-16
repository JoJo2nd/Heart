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
#include "render/hRenderCallDesc.h"
#include "render/hMaterialParamValue.h"
#include "base/hProtobuf.h"
#include "components/hObjectFactory.h"

#include "cryptoMurmurHash.h"

#include <unordered_map>
#include <memory>

namespace Heart {
namespace hRenderer {
    struct hProgramReflectionInfo;
}

    struct hPassKey {
        hPassKey(hStringID g, hStringID t, hUint p) 
            : group_(g), tech_(t), pass_(p) {}
        hStringID group_;
        hStringID tech_;
        hUint     pass_;

        hBool operator == (const hPassKey& rhs) const {
            return group_ == rhs.group_ && tech_ == rhs.tech_ && pass_ == rhs.pass_;
        }
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
        struct hPassDesc {
            hPassDesc() 
                : rcd(nullptr)
                , progIDs(nullptr)
                , reflection(nullptr) {

            }

            hRenderer::hRenderCallDesc*         rcd;
            hStringID*                          progIDs;
            hShaderProfile*                     progProfiles;
            hRenderer::hProgramReflectionInfo*  reflection;
        };
        typedef std::unordered_map<hPassKey, hPassDesc> hPassHashTable;

        static const hUint maxShaderCount = 5;

        hPassHashTable                                              passTable_;
        std::unique_ptr<hRenderer::hRenderCallDesc[]>               rcDescs_;
        std::unique_ptr<hByte[]>                                    defaultParamData_;
        std::vector<hStringID>                                      groupNames_;
        std::vector<hStringID>                                      techniqueNames_;
        std::vector<hRenderer::hRenderCallDesc::hSamplerStateDesc>  samplerStates_;
        std::vector<hMaterialParamValue>                            parameters_;
        hUint                                                       totalPasses_;
        std::unique_ptr<hStringID[]>                                passProgIDs_;

    public:
        hObjectType(Heart::hMaterial, Heart::proto::MaterialResource);

        hMaterial() {}
        ~hMaterial() {}

        hUint                               getGroupCount() const;
        hStringID                           getGroupName(hUint index) const;
        hUint                               getTechniqueCount(hStringID group) const;
        hStringID                           getTechniqueName(hStringID group, hUint index) const;
        const hRenderer::hRenderCallDesc&   getPass(hStringID group, hStringID tech, hUint pass);

    };
}