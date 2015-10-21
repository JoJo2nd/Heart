/********************************************************************
    Written by James Moran
    Please see the file HEART_LICENSE.txt in the source's root directory.
*********************************************************************/

#pragma once

#include "base/hTypes.h"
#include "base/hArray.h"
#include "base/hMemoryUtil.h"
#include <vector>

namespace Heart {
    struct hRect 
    {
        hUint32 top_,left_,right_,bottom_;

        hBool operator == (const hRect& rhs)
        {
            return left_ == rhs.left_ && right_ == rhs.right_ && top_ == rhs.top_ && bottom_ == rhs.bottom_;
        }

        hBool operator != (const hRect& rhs)
        {
            return !(*this == rhs);
        }
    };

    struct hViewport
    {
        hViewport() {}
        hViewport( hUint32 x,hUint32 y,hUint32 w,hUint32 h )
            : x_(x), y_(y), width_(w), height_(h)
        {}
        hViewport(hViewport&& rhs) {
            hPLACEMENT_NEW(this) hViewport();
            swap(*this, rhs);
        }
        const hViewport& operator = (hViewport rhs) {
            swap(*this, rhs);
            return *this;
        }
        void swap(hViewport& lhs, hViewport& rhs) {
            std::swap(lhs.x_, rhs.x_);
            std::swap(lhs.y_, rhs.y_);
            std::swap(lhs.width_, rhs.width_);
            std::swap(lhs.height_, rhs.height_);
        }
        hUint32 x_,y_,width_,height_;
    };
    struct hRelativeViewport 
    {
        hRelativeViewport() {}
        hRelativeViewport(hFloat fx, hFloat fy, hFloat fw, hFloat fh)
            : x(fx), y(fy), w(fw), h(fh)
        {}
        hFloat x, y, w, h;
    };

    typedef hRect hScissorRect;

    struct hRelativeScissorRect
    {
        hFloat left,top,right,bottom;

        hBool operator == (const hRelativeScissorRect& rhs)
        {
            return left == rhs.left && right == rhs.right && top == rhs.top && bottom == rhs.bottom;
        }

        hBool operator != (const hRelativeScissorRect& rhs)
        {
            return !(*this == rhs);
        }
    };

    struct hRenderFrameStats
    {
        hFloat          frametime_;
        hFloat          gpuTime_;
        hUint32			nPrims_;
        hUint32			nDeviceCalls_;
        hUint32			nDrawCalls_;
    };

#define HEART_MAX_RESOURCE_INPUTS                       (16) // D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT == 128
#define HEART_MAX_UAV_INPUTS                            (7) // D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT == 128
#define HEART_MAX_CONSTANT_BLOCKS                       (14) // == D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT 
#define HEART_VIEWPORT_CONSTANTS_REGISTIER              (HEART_MAX_CONSTANT_BLOCKS-1)
#define HEART_INSTANCE_CONSTANTS_REGISTIER              (HEART_MAX_CONSTANT_BLOCKS-2)
#define HEART_RESERVED_CUSTOM_CONSTANTS_REGISTIER_1     (HEART_MAX_CONSTANT_BLOCKS-3)
#define HEART_RESERVED_CUSTOM_CONSTANTS_REGISTIER_2     (HEART_MAX_CONSTANT_BLOCKS-4)
#define HEART_MIN_RESERVED_CONSTANT_BLOCK               (HEART_RESERVED_CUSTOM_CONSTANTS_REGISTIER_2)
#define HEART_MAX_INPUT_STREAMS                         (15) //Add more if needed
#define HEART_MAX_RENDER_CAMERAS                        (15)
#define HEART_MAX_GLOBAL_CONST_BLOCK_NAME_LEN           (64)
#define HEART_MAX_SIMULTANEOUS_RENDER_TARGETS           (8)


    struct hConstantBlockDesc
    {
        hArray< hChar, 32 >     name_;
        hUint                   size_;
        hUint                   parameterCount_;
        hUint                   bindPoint_;
        hUint                   index_;
        hUint32                 hash_;
    };

    enum hShaderType
    {
        ShaderType_VERTEXPROG,
        ShaderType_FRAGMENTPROG,
        ShaderType_GEOMETRYPROG,
        ShaderType_HULLPROG,
        ShaderType_DOMAINPROG,

        ShaderType_COMPUTEPROG,

        ShaderType_MAX,
        ShaderType_FORCE_DWORD = ~0U
    };

    enum hRenderResourceType 
    {
        eRenderResourceType_Buffer,
        eRenderResourceType_Tex1D,
        eRenderResourceType_Tex2D,
        eRenderResourceType_Tex3D,
        eRenderResourceType_TexCube,
        eRenderResourceType_ArrayMask = 0x70000000,
        eRenderResourceType_Tex1DArray   = eRenderResourceType_Tex1D    | eRenderResourceType_ArrayMask,
        eRenderResourceType_Tex2DArray   = eRenderResourceType_Tex2D    | eRenderResourceType_ArrayMask,
        eRenderResourceType_TexCubeArray = eRenderResourceType_TexCube  | eRenderResourceType_ArrayMask,
        eRenderResourceType_Invalid = ~0
    };

    enum class hShaderProfile {
        ES2_vs, ES2_ps,
        WebGL_vs, WebGL_ps,
        ES3_vs, ES3_ps,
        D3D_9c_vs, D3D_9c_ps,
        FL10_vs, FL10_ps, FL10_gs, FL10_cs,
        FL11_vs, FL11_ps, FL11_gs, FL11_cs, FL11_hs, FL11_ds,

        // Nothing should support invalid
        Invalid,
        eShaderProfile_Max,
    };

    enum class hShaderFrequency {
        Vertex, Pixel, Hull, Domain, Compute
    };

    enum hSAMPLER_STATE_VALUE
    {
        SSV_NONE,
        SSV_POINT,
        SSV_LINEAR,
        SSV_ANISOTROPIC,
        SSV_WRAP,
        SSV_MIRROR,
        SSV_CLAMP,
        SSV_BORDER,

        SSV_FORCE_DWORD = ~0U
    };
/*
    //////////////////////////////////////////////////////////////////////////
    // These may want to move into the device layer //////////////////////////
    //////////////////////////////////////////////////////////////////////////
    enum VertexElement
    {
        hrVE_XYZ,	
        hrVE_XYZW,
        hrVE_NORMAL,			
        hrVE_1UV,			
        hrVE_2UV,			
        hrVE_3UV,			
        hrVE_4UV,			
        hrVE_5UV,			
        hrVE_6UV,			
        hrVE_7UV,			
        hrVE_8UV,			
        hrVE_BLEND1,			
        hrVE_BLEND2,		
        hrVE_BLEND3,			
        hrVE_BLEND4,			
        hrVE_COLOR,			
        hrVE_TANGENT,		
        hrVE_BINORMAL,		

        hrVE_MAX,

        hrVE_FORCE_DWORD = ~0U
    };

    enum VertexFlags
    {
        hrVF_XYZ				= 1 << hrVE_XYZ,
        hrVF_XYZW				= 1 << hrVE_XYZW,
        hrVF_NORMAL				= 1 << hrVE_NORMAL,
        hrVF_1UV				= 1 << hrVE_1UV,	
        hrVF_2UV				= 1 << hrVE_2UV,	
        hrVF_3UV				= 1 << hrVE_3UV,	
        hrVF_4UV				= 1 << hrVE_4UV,	
        hrVF_5UV				= 1 << hrVE_5UV,	
        hrVF_6UV				= 1 << hrVE_6UV,	
        hrVF_7UV				= 1 << hrVE_7UV,	
        hrVF_8UV				= 1 << hrVE_8UV,	
        hrVF_BLEND1				= 1 << hrVE_BLEND1,	
        hrVF_BLEND2				= 1 << hrVE_BLEND2,	
        hrVF_BLEND3				= 1 << hrVE_BLEND3,	
        hrVF_BLEND4				= 1 << hrVE_BLEND4,	
        hrVF_COLOR				= 1 << hrVE_COLOR,	
        hrVF_TANGENT			= 1 << hrVE_TANGENT,
        hrVF_BINORMAL			= 1 << hrVE_BINORMAL,

        hrVF_MAX				= 17,

        hrVF_UVMASK				= hrVF_1UV | hrVF_2UV | hrVF_3UV | hrVF_4UV | hrVF_5UV | hrVF_6UV | hrVF_7UV | hrVF_8UV,
        hrVF_BLENDMASK			= hrVF_BLEND1 | hrVF_BLEND2 | hrVF_BLEND3 | hrVF_BLEND4,
    };

    enum hInputSemantic
    {
        eIS_POSITION,
        eIS_NORMAL,
        eIS_TEXCOORD,
        eIS_COLOUR,
        eIS_TANGENT,
        eIS_BITANGENT,
        eIS_INSTANCE,       // generic id for instance data
        //TODO: Blend weights?

        eIS_FORCEDWORD = 0xFFFFFFFF
    };

    enum hInputFormat
    {
        eIF_FLOAT1,
        eIF_FLOAT2,
        eIF_FLOAT3,
        eIF_FLOAT4,
        eIF_UBYTE4_UNORM, // 0.f - 1.f
        eIF_UBYTE4_SNORM, // -1.f - 1.f
        //TODO: more?
    
        eIF_FORCEDWORD = 0xFFFFFFFF
    };
*/
    enum ResourceFlags
    {
        RESOURCEFLAG_DYNAMIC         = 1 << 1,
        RESOURCEFLAG_RENDERTARGET    = 1 << 2,
        RESOURCEFLAG_DEPTHTARGET     = 1 << 3,
        RESOURCEFLAG_KEEPCPUDATA     = 1 << 4,
        RESOURCEFLAG_DONTOWNCPUDATA  = 1 << 5,
        RESOURCEFLAG_UNORDEREDACCESS = 1 << 6,
        eResourceFlag_ConstantBuffer = 1 << 7,
        eResourceFlag_ShaderResource = 1 << 8,
        eResourceFlag_StreamOut      = 1 << 9,
        eResourceFlag_StructuredBuffer = 1 << 10,
    };

    enum PrimitiveType
    {
        PRIMITIVETYPE_TRILIST,
        PRIMITIVETYPE_TRISTRIP,
        PRIMITIVETYPE_LINELIST,

        PRIMITIVETYPE_MAX
    };

    enum hIndexBufferType {
        hIndexBufferType_Index16,
        hIndexBufferType_Index32,

        hIndexBufferType_Max
    };

}//Heart

#include "base/hColour.h"
#include "render/hRenderStates.h"
