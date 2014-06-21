/********************************************************************

    filename: 	hTexture.h	
    
    Copyright (c) 31:3:2012 James Moran
    
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

#define hiTexture_h__ //todo !!JM left for reference
#ifndef hiTexture_h__

class TextureBuilder;

#include "base/hTypes.h"
#include "base/hFunctor.h"
#include "base/hRendererConstants.h"
#include "base/hReferenceCounted.h"
#include "base/hProtobuf.h"
#include "components/hObjectFactory.h"

namespace Heart
{
    
    class hTexture;

    class  hTexture : public hdTexture,
                      public hIReferenceCounted
    {
    public:
        hObjectType(Heart::hTexture, Heart::proto::TextureResource);

        hFUNCTOR_TYPEDEF(void(*)(hTexture*), hZeroRefProc);

        hTexture() 
            : keepcpu_(hFalse)
            , singleAlloc_(hTrue)
            , levelDescs_(NULL)
            , type_(eRenderResourceType_Tex2D)
        {}
        hTexture(hZeroRefProc zeroproc, hRenderResourceType type) 
            : zeroProc_(zeroproc)
            , keepcpu_(hFalse)
            , singleAlloc_(hTrue)
            , levelDescs_(NULL)
            , type_(type)
        {}
        ~hTexture()
        {
            ReleaseCPUTextureData();
            delete[] levelDescs_;
            levelDescs_ = nullptr;
        }

        struct LevelDesc
        {
            hUint32 width_;
            hUint32 height_;
            hByte*  mipdata_;
            hUint32 mipdataSize_;
        };

        hUint32                 getWidth( hUint32 level = 0 ) const { hcAssert( level < nLevels_ ); return levelDescs_[ level ].width_; }
        hUint32                 getHeight( hUint32 level = 0 ) const { hcAssert( level < nLevels_ ); return levelDescs_[ level ].height_; }
        void                    ReleaseCPUTextureData();
        hColour                 ReadPixel(hUint32 x, hUint32 y);
        void                    SetKeepCPU(hBool val) { keepcpu_ = val; }
        hBool                   GetKeepCPU() const { return keepcpu_; }
        hTextureFormat          getTextureFormat() const { return format_; }
        hUint32                 getFlags() const { return flags_; }
        hRenderResourceType     getRenderType() const { return type_; }
        hUint                   getMipCount() const { return nLevels_; }
#pragma message ("TODO: fix array size on textures")
        hUint                   getArraySize() const { return 1; }

        static hUint32          GetDXTTextureSize( hBool dxt1, hUint32 width, hUint32 height );

    private:

        

        void OnZeroRef();

        hTexture( const hTexture& c );
        hTexture& operator = ( const hTexture& rhs );

        hZeroRefProc            zeroProc_;
        hRenderResourceType     type_;
        hTextureFormat          format_;
        hUint32                 nLevels_;
        LevelDesc*              levelDescs_;
        hUint32                 flags_;
        hBool                   keepcpu_;
        hBool                   singleAlloc_;
    };

    struct hTextureMapInfo
    {
        void*		        ptr_;
        hUint32		        level_;
        hUint32		        pitch_;
        hTexture*           tex_;
        hdMappedData    privateDeviceData_;
    };

}

#endif // hiTexture_h__