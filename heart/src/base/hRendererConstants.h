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

    typedef void* (*hTempRenderMemAlloc)( hUint32 size );
    typedef void (*hTempRenderMemFree)( void* ptr );

    struct hConstantBlockDesc
    {
        hArray< hChar, 32 >     name_;
        hUint                   size_;
        hUint                   parameterCount_;
        hUint                   bindPoint_;
        hUint                   index_;
        hUint32                 hash_;
    };

    typedef hUint32 hShaderParameterID;

    struct hShaderParameter
    {
        hShaderParameter() 
            : size_(0)
            , cReg_(0)
            , cBufferBindPoint_(0)
        {
            hZeroMem(name_,sizeof(name_));
        }

        hArray< hChar, 32 >                 name_; 
        hUint32                             size_;
        hUint32                             cReg_;
        hUint32                             cBufferBindPoint_;
    };

    enum hShaderInputType {
        eShaderInputType_Buffer,
        eShaderInputType_Resource,
        eShaderInputType_Sampler,
        eShaderInputType_UAV,

        eShaderInputType_Invalid
    };

    struct hShaderInput
    {
        hShaderInput() 
            : type_(eShaderInputType_Invalid)
            , bindPoint_(0)
            , arraySize_(0)
        {
            hZeroMem(name_,sizeof(name_));
        }

        hArray< hChar, 32 > name_; 
        hShaderInputType    type_;
        hUint               bindPoint_;
        hUint               arraySize_;
    };

    struct hShaderDefine 
    {
        const hChar* define_;
        const hChar* value_;
    };

    class hIIncludeHandler
    {
    public:
        virtual ~hIIncludeHandler() {}
        virtual void findInclude(const hChar* includepath, const void** outdata, hUint* outlen) = 0;
    };

    enum hRenderCmdOpCode 
    {
        eRenderCmd_NOOP,
        eRenderCmd_Jump,
        eRenderCmd_Return,
        eRenderCmd_Draw,
        eRenderCmd_DrawIndex,
        eRenderCmd_DrawInstanced,
        eRenderCmd_DrawInstancedIndex,

        eRenderCmd_DeviceCmds, //NoOp - any enum higher is processed by the device level

        eRenderCmd_SetRenderStates,
        eRenderCmd_SetVertexShader,
        eRenderCmd_SetPixelShader,
        eRenderCmd_SetGeometeryShader,
        eRenderCmd_SetHullShader,
        eRenderCmd_SetDomainShader,
        eRenderCmd_SetComputeShader,

        eRenderCmd_SetVertexInputs,
        eRenderCmd_SetPixelInputs,
        eRenderCmd_SetGeometryInputs,
        eRenderCmd_SetHullInputs,
        eRenderCmd_SetDomainInputs,
        eRenderCmd_SetComputeInputs,

        eRenderCmd_SetInputStreams,

        eRenderCmd_End
    };

#define HEART_RENDER_CMD_APPEND (hErrorCode)

    struct hRCmd 
    {
        hRCmd(hRenderCmdOpCode oc, hSize_t size) 
            : opCode_(oc), size_((hByte)size) {}
        hRenderCmdOpCode    opCode_;
        hByte               size_;
    };

    struct hRCmdJump : public hRCmd
    {
        hRCmdJump(hRCmd* cmd) 
            : hRCmd(eRenderCmd_Jump, sizeof(hRCmdJump))
            , cmd_(cmd) {}
        hRCmd* cmd_;
    };

    struct hRCmdReturn : public hRCmd
    {
        hRCmdReturn() : hRCmd(eRenderCmd_Return, sizeof(hRCmdReturn)) {}
    };

    struct hRCmdNOOP : public hRCmd
    {
        hRCmdNOOP() : hRCmd(eRenderCmd_NOOP, sizeof(hRCmdNOOP)) {}
    };

    struct hRCmdDraw : public hRCmd
    {
        hRCmdDraw(hUint nPrims, hUint startvtx) 
            : hRCmd(eRenderCmd_Draw, sizeof(hRCmdDraw))
            , nPrimatives_(nPrims), startVertex_(startvtx) {}
        hUint nPrimatives_;
        hUint startVertex_;
    };

    struct hRCmdDrawIndex : public hRCmd
    {
        hRCmdDrawIndex(hUint nPrims, hUint startvtx) 
            : hRCmd(eRenderCmd_DrawIndex, sizeof(hRCmdDrawIndex))
            , nPrimatives_(nPrims), startVertex_(startvtx) {}
        hUint nPrimatives_;
        hUint startVertex_;
    };

    struct hRCmdDrawInstanced : public hRCmd
    {
        hRCmdDrawInstanced(hUint nPrims, hUint startvtx, hUint count) 
            : hRCmd(eRenderCmd_DrawInstanced, sizeof(hRCmdDrawInstanced))
            , nPrimatives_(nPrims), startVertex_(startvtx), instanceCount_(count) {}
        hUint nPrimatives_;
        hUint startVertex_;
        hUint instanceCount_;
    };

    struct hRCmdDrawInstancedIndex : public hRCmd
    {
        hRCmdDrawInstancedIndex(hUint nPrims, hUint startvtx, hUint count) 
            : hRCmd(eRenderCmd_DrawInstancedIndex, sizeof(hRCmdDrawInstancedIndex))
            , nPrimatives_(nPrims), startVertex_(startvtx), instanceCount_(count) {}
        hUint nPrimatives_;
        hUint startVertex_;
        hUint instanceCount_;
    };

    class hRenderCommands
    {
    public:
        hRenderCommands()
            : cmdSize_(0)
            , allocatedSize_(0)
            , cmds_(hNullptr)
        {

        }
        ~hRenderCommands()
        {
            release();
        }

        void    reset() { cmdSize_ = 0; }
        hBool   isEmpty() const { return cmdSize_ == 0; }
        hRCmd*  getFirst() { return cmds_; }
        hRCmd*  getEnd() { return (hRCmd*)((hByte*)cmds_+cmdSize_); }
        hRCmd*  getCommandAtOffset(hUint offset) { 
            hcAssert(offset < cmdSize_);
            return (hRCmd*)((hByte*)cmds_+offset);
        }
        void    release() {
            delete[] cmds_;
            cmds_ = nullptr;
        }

    private:

        hRenderCommands(const hRenderCommands& rhs);
        hRenderCommands& operator = (const hRenderCommands& rhs);

        friend class hRenderCommandGenerator;

        void    insertCommand(hUint where, const hRCmd* command, hBool overwrite);
        void    reserveSpace(hUint size);

        hUint  cmdSize_;
        hUint  allocatedSize_;
        hRCmd* cmds_;
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

    enum class hShaderProfile 
    {
        // Deprecated 
//         eShaderProfile_vs4_0,
//         eShaderProfile_vs4_1,
//         eShaderProfile_vs5_0,
// 
//         eShaderProfile_ps4_0,
//         eShaderProfile_ps4_1,
//         eShaderProfile_ps5_0,
// 
//         eShaderProfile_gs4_0,
//         eShaderProfile_gs4_1,
//         eShaderProfile_gs5_0,
// 
//         eShaderProfile_cs4_0,
//         eShaderProfile_cs4_1,
//         eShaderProfile_cs5_0,
// 
//         eShaderProfile_hs5_0,
//         eShaderProfile_ds5_0,
        // End - Deprecated

        ES2_vs, ES2_ps,
        WebGL_vs, WebGL_ps,
        ES3_vs, ES3_ps,
        FL10_vs, FL10_ps, FL10_gs, FL10_cs,
        FL11_vs, FL11_ps, FL11_gs, FL11_cs, FL11_hs, FL11_ds,

        // Nothing should support invalid
        Invalid,
        eShaderProfile_Max,
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

    struct hInputLayoutDesc
    {
    public:
        hInputLayoutDesc()
            : semanticName_(0)
            , semIndex_(0)
            , typeFormat_(eIF_FLOAT1)
            , inputStream_(0)
            , instanceDataRepeat_(0)
        {}
        hInputLayoutDesc(const hChar* semanticname, hByte semidx, hInputFormat fmt, hUint32 stream, hUint16 instancerepeat)
            : semanticName_(0)
            , semIndex_(semidx)
            , typeFormat_(fmt)
            , inputStream_(stream)
            , instanceDataRepeat_(instancerepeat)
        {
            setSemanticName(semanticname);
        }
        
        void         setSemanticName(const hChar* name);
        const hChar* getSemanticName() const;
        void         setSemanticIndex(hByte idx) { semIndex_=idx; }
        hByte        getSemanticIndex() const { return semIndex_; }
        void         setInputFormat(hInputFormat fmt) { typeFormat_=fmt; }
        hInputFormat getInputFormat() const { return typeFormat_; }
        void         setInputStream(hUint32 stream) { inputStream_=stream; }
        hUint32      getInputStream() const { return inputStream_; }
        void         setInstanceRepeat(hUint16 repeat) { instanceDataRepeat_=repeat; }
        hUint16      getInstanceRepeat() const { return instanceDataRepeat_; }
        
    private:

        hUint           semanticName_;
        hByte           semIndex_;
        hInputFormat    typeFormat_;
        hUint32         inputStream_;
        hUint16         instanceDataRepeat_;

        static std::vector<std::string> s_semanticNameMap;
    };

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

    enum hDebugShaderID
    {
        eDebugVertexPosOnly,
        eDebugPixelWhite,
        eConsoleVertex,
        eConsolePixel,
        eDebugFontVertex,
        eDebugFontPixel,
        eDebugVertexPosNormal,
        eDebugPixelWhiteViewLit,
        eDebugTexVertex,
        eDebugTexPixel,
        eDebugVertexPosCol,
        eDebugPixelPosCol,

        eDebugShaderMax
    };

#define hDebugShaderResourceID_VertexPosOnly     (Heart::hStringID("?builtin/vertex_pos_only"       ))
#define hDebugShaderResourceID_PixelWhite        (Heart::hStringID("?builtin/pixel_white"           ))
#define hDebugShaderResourceID_ConsoleVertex     (Heart::hStringID("?builtin/console_vertex"        ))
#define hDebugShaderResourceID_ConsolePixel      (Heart::hStringID("?builtin/console_pixel"         ))
#define hDebugShaderResourceID_FontVertex        (Heart::hStringID("?builtin/font_vertex"           ))
#define hDebugShaderResourceID_FontPixel         (Heart::hStringID("?builtin/font_pixel"            ))
#define hDebugShaderResourceID_VertexPosNormal   (Heart::hStringID("?builtin/vertex_pos_normal"     ))
#define hDebugShaderResourceID_PixelWhiteViewLit (Heart::hStringID("?builtin/pixel_white_view_lit"  ))
#define hDebugShaderResourceID_TexVertex         (Heart::hStringID("?builtin/texture_vertex"        ))
#define hDebugShaderResourceID_TexPixel          (Heart::hStringID("?builtin/texture_pixel"         ))
#define hDebugShaderResourceID_VertexPosCol      (Heart::hStringID("?builtin/vertex_pos_col"        ))
#define hDebugShaderResourceID_PixelPosCol       (Heart::hStringID("?builtin/pixel_pos_col"         ))


    struct hBufferSRV
    {
        hUint firstElement_;
        hUint elementOffset_;
        hUint numElements_;
        hUint elementWidth_;
    };
    typedef hBufferSRV hBufferRTV;
    typedef hBufferSRV hBufferDSV;

    struct hTex1DSRV
    {
        hUint topMip_;
        hUint mipLevels_;
    };
    typedef hTex1DSRV hTex1DRTV;
    typedef hTex1DSRV hTex1DDSV;

    struct hTex1DArraySRV
    {
        hUint topMip_;
        hUint mipLevels_;
        hUint arraySize_;
        hUint arrayStart_;
    };
    typedef hTex1DArraySRV hTex1DArrayRTV;
    typedef hTex1DArraySRV hTex1DArrayDSV;

    struct hTex2DSRV
    {
        hUint topMip_;
        hUint mipLevels_;
    };
    typedef hTex2DSRV hTex2DRTV;
    typedef hTex2DSRV hTex2DDSV;

    struct hTex2DArraySRV
    {
        hUint topMip_;
        hUint mipLevels_;
        hUint arraySize_;
        hUint arrayStart_;
    };
    typedef hTex2DArraySRV hTex2DArrayRTV;
    typedef hTex2DArraySRV hTex2DArrayDSV;

}//Heart

#include "base/hColour.h"
#include "render/hRenderStates.h"