/********************************************************************
	created:	2010/07/03
	created:	3:7:2010   1:46
	filename: 	RendererConstants.h	
	author:		James
	
	purpose:	
*********************************************************************/
#ifndef RENDERERCONSTANTS_H__
#define RENDERERCONSTANTS_H__

#include "hTypes.h"

namespace Heart
{
	struct hRect 
	{
		hUint32 top_,left_,right_,bottom_;
	};

	struct hViewport
	{
        hViewport() {}
        hViewport( hUint32 x,hUint32 y,hUint32 w,hUint32 h )
            : x_(x), y_(y), width_(w), height_(h)
        {}
		hUint32 x_,y_,width_,height_;
	};

	typedef hRect ScissorRect;

	struct hColour
	{
		hColour() {}
		hColour( hFloat r, hFloat g, hFloat b, hFloat a ){ r_ = r;	b_ = b;	g_ = g;	a_ = a; }
		hFloat r_,g_,b_,a_;

		operator hUint32 () const
		{
			return ((hByte)( r_ * 255.0f )) << 16 |  ((hByte)( g_ * 255.0f )) << 8 | ((hByte)( b_ * 255.0f )) | ((hByte)( a_ * 255.0f )) << 24;
		}

		hColour operator * ( hFloat s ) const
		{
			return hColour( r_*s, g_*s, b_*s, a_*s );
		}

		hColour operator + ( hColour c ) const
		{
			return hColour( r_+c.r_, g_+c.g_, b_+c.b_, a_+c.a_ );
		}

		hColour operator * ( hColour c ) const
		{
			return hColour( r_*c.r_, g_*c.g_, b_*c.b_, a_*c.a_ );
		}
	};

	const hColour WHITE( 1.0f, 1.0f, 1.0f, 1.0f );
	const hColour BLACK( 0.0f, 0.0f, 0.0f, 1.0f );

#define HEART_VIEWPORT_CONSTANTS_REGISTIER (0)
#define HEART_INSTANCE_CONSTANTS_REGISTIER (1)

    typedef void* (*hTempRenderMemAlloc)( hUint32 size );
    typedef void (*hTempRenderMemFree)( void* ptr );

    struct hShaderParameter
    {
        hShaderParameter() 
            : name_(NULL)
            , size_(0)
            , cReg_(0)
            , cBuffer_(0)
        {

        }

        hChar*						        name_; 
        hUint32                             size_;
        hUint32                             cReg_;
        hUint32                             cBuffer_;
    };

    enum ShaderType
    {
        ShaderType_VERTEXPROG,
        ShaderType_FRAGMENTPROG,

        ShaderType_MAX,

        ShaderType_FORCE_DWORD = ~0U
    };

	enum hTextureFormat
	{
		TFORMAT_ARGB8,
		TFORMAT_XRGB8,
		TFORMAT_RGB8,
		TFORMAT_R16F,
		TFORMAT_GR16F,
		TFORMAT_ABGR16F,
		TFORMAT_R32F,
		TFORMAT_D32F,
		TFORMAT_D24S8F,
		TFORMAT_L8,
		TFORMAT_DXT5,
        TFORMAT_DXT3,
        TFORMAT_DXT1,

        TFORMAT_FORCE_DWORD = ~0U
	};

	enum RENDER_STATE
	{
		RS_CULL_MODE,
		RS_FILL_MODE,
		RS_DEPTH_BIAS,
		RS_SCISSOR_TEST,
		RS_Z_TEST,
		RS_Z_WRITE,
		RS_Z_COMPARE_FUNCTION,
		RS_STENCIL_TEST,
		RS_STENCIL_FAIL,
		RS_STENCIL_Z_FAIL,
		RS_STENCIL_PASS,
		RS_STENCIL_FUNCTION,
		RS_STENCIL_REF,
		RS_STENCIL_WRITE_MASK,
		RS_STENCIL_MASK,
		RS_ALPHA_BLEND,
		RS_SRC_ALPHA_BLEND,
		RS_DST_ALPHA_BLEND,
		RS_ALPHA_BLEND_FUNCTION,
		RS_COLOUR_WRITE_1,
		RS_COLOUR_WRITE_2,
		RS_COLOUR_WRITE_3,
		RS_COLOUR_WRITE_4,

		RS_MAX,

		RS_FORCE_DWORD = ~0U
	};

	enum RENDER_STATE_FLAG
	{
		RSV_COLOUR_WRITE_RED	= 1,
		RSV_COLOUR_WRITE_GREEN	= 1 << 1,
		RSV_COLOUR_WRITE_BLUE	= 1 << 2,
		RSV_COLOUR_WRITE_ALPHA	= 1 << 3,
		RSV_COLOUR_WRITE_FULL	= 0x0000000F,

        RSF_FORCE_DWORD = ~0U
	};

	enum RENDER_STATE_VALUE
	{
		//GENERAL
		RSV_ENABLE,
		RSV_DISABLE,
		//CULL MODE
		RSV_CULL_MODE_NONE,
		RSV_CULL_MODE_CW,
		RSV_CULL_MODE_CCW,
		//FILL MODE
		RSV_FILL_MODE_SOLID,
		RSV_FILL_MODE_WIREFRAME,
		//Z COMPARE FUNCTION
		RSV_Z_CMP_NEVER,
		RSV_Z_CMP_LESS,
		RSV_Z_CMP_EQUAL,
		RSV_Z_CMP_LESSEQUAL,
		RSV_Z_CMP_GREATER,
		RSV_Z_CMP_NOT_EQUAL,
		RSV_Z_CMP_GREATER_EQUAL,
		RSV_Z_CMP_ALWAYS,
		//STENCIL FUNCTION
		RSV_SF_CMP_NEVER,
		RSV_SF_CMP_LESS,
		RSV_SF_CMP_EQUAL,
		RSV_SF_CMP_LESSEQUAL,
		RSV_SF_CMP_GREATER,
		RSV_SF_CMP_NOT_EQUAL,
		RSV_SF_CMP_GREATER_EQUAL,
		RSV_SF_CMP_ALWAYS,
		//STENCIL OP
		RSV_SO_KEEP,
		RSV_SO_ZERO,
		RSV_SO_REPLACE,
		RSV_SO_INCRSAT,
		RSV_SO_DECRSAT,
		RSV_SO_INVERT,
		RSV_SO_INCR,
		RSV_SO_DECR,
		//BLEND FUNCTION
		RSV_BLEND_FUNC_ADD,
		RSV_BLEND_FUNC_SUB,
		RSV_BLEND_FUNC_MIN,
		RSV_BLEND_FUNC_MAX,
		//BLEND OP
		RSV_BLEND_OP_ZERO,
		RSV_BLEND_OP_ONE,
		RSV_BLEND_OP_SRC_COLOUR,
		RSV_BLEND_OP_INVSRC_COLOUR,
		RSV_BLEND_OP_DEST_COLOUR,
		RSV_BLEND_OP_INVDEST_COLOUR,
		RSV_BLEND_OP_SRC_ALPHA,
		RSV_BLEND_OP_INVSRC_ALPHA,
		RSV_BLEND_OP_DEST_ALPHA,
		RSV_BLEND_OP_INVDEST_ALPHA,

		RSV_FORCE_DWORD = ~0U
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

	enum ResourceFlags
	{
		RESOURCEFLAG_DYNAMIC				= 1 << 1,
        RESOURCEFLAG_RENDERTARGET   		= 1 << 2,
        RESOURCEFLAG_DEPTHTARGET    		= 1 << 3,
	};

	enum PrimitiveType
	{
		PRIMITIVETYPE_TRILIST,
		PRIMITIVETYPE_TRISTRIP,
		PRIMITIVETYPE_LINELIST,

		PRIMITIVETYPE_MAX
	};

	enum LightSourceType
	{
		LIGHTSOURCETYPE_DIRECTION,
		LIGHTSOURCETYPE_SPOT,
		LIGHTSOURCETYPE_POINT,

		LIGHTSOURCETYPE_MAX
	};

    struct hBlendStateDesc
    {
        hBlendStateDesc()
        {
            blendEnable_           = RSV_DISABLE;
            srcBlend_              = RSV_BLEND_OP_ONE;
            destBlend_             = RSV_BLEND_OP_ZERO;
            blendOp_               = RSV_BLEND_FUNC_ADD;
            srcBlendAlpha_         = RSV_BLEND_OP_ONE;
            destBlendAlpha_        = RSV_BLEND_OP_ZERO;
            blendOpAlpha_          = RSV_BLEND_FUNC_ADD;
            renderTargetWriteMask_ = RSV_COLOUR_WRITE_FULL;
        }

        RENDER_STATE_VALUE    blendEnable_;
        RENDER_STATE_VALUE    srcBlend_;
        RENDER_STATE_VALUE    destBlend_;
        RENDER_STATE_VALUE    blendOp_;
        RENDER_STATE_VALUE    srcBlendAlpha_;
        RENDER_STATE_VALUE    destBlendAlpha_;
        RENDER_STATE_VALUE    blendOpAlpha_;
        hUint32               renderTargetWriteMask_;
    };

    struct hDepthStencilStateDesc
    {
        hDepthStencilStateDesc()
        {
            depthEnable_        = RSV_DISABLE;
            depthWriteMask_     = RSV_DISABLE;
            depthFunc_          = RSV_Z_CMP_LESS;
            stencilEnable_      = RSV_DISABLE;
            stencilReadMask_    = ~0U;
            stencilWriteMask_   = ~0U;
            stencilFailOp_      = RSV_SO_KEEP;
            stencilDepthFailOp_ = RSV_SO_KEEP;
            stencilPassOp_      = RSV_SO_KEEP;
            stencilFunc_        = RSV_SF_CMP_ALWAYS;
            stencilRef_         = 0;  
        }

        RENDER_STATE_VALUE    depthEnable_;
        RENDER_STATE_VALUE    depthWriteMask_;
        RENDER_STATE_VALUE    depthFunc_;
        RENDER_STATE_VALUE    stencilEnable_;
        hUint32               stencilReadMask_;
        hUint32               stencilWriteMask_;
        RENDER_STATE_VALUE    stencilFailOp_;
        RENDER_STATE_VALUE    stencilDepthFailOp_;
        RENDER_STATE_VALUE    stencilPassOp_;
        RENDER_STATE_VALUE    stencilFunc_;
        hUint32               stencilRef_;
    };

    struct hRasterizerStateDesc
    {
        hRasterizerStateDesc()
        {
            fillMode_              = RSV_FILL_MODE_SOLID;
            cullMode_              = RSV_CULL_MODE_NONE;
            frontCounterClockwise_ = RSV_DISABLE;
            depthBias_             = 0;
            depthBiasClamp_        = 0.f;
            slopeScaledDepthBias_  = 0.f;
            depthClipEnable_       = RSV_ENABLE;
            scissorEnable_         = RSV_DISABLE;
        }

        RENDER_STATE_VALUE    fillMode_;
        RENDER_STATE_VALUE    cullMode_;
        RENDER_STATE_VALUE    frontCounterClockwise_;
        hInt32                depthBias_;
        hFloat                depthBiasClamp_;
        hFloat                slopeScaledDepthBias_;
        RENDER_STATE_VALUE    depthClipEnable_;
        RENDER_STATE_VALUE    scissorEnable_;
    };

    struct hSamplerStateDesc
    {
        hSamplerStateDesc()
        {
            filter_        = SSV_POINT;
            addressU_      = SSV_CLAMP;
            addressV_      = SSV_CLAMP;
            addressW_      = SSV_CLAMP;
            mipLODBias_    = 0;
            maxAnisotropy_ = 16;
            borderColour_  = WHITE;
            minLOD_        = -FLT_MAX;
            maxLOD_        = FLT_MAX;             
        }

        hSAMPLER_STATE_VALUE    filter_;
        hSAMPLER_STATE_VALUE    addressU_;
        hSAMPLER_STATE_VALUE    addressV_;
        hSAMPLER_STATE_VALUE    addressW_;
        hFloat                  mipLODBias_;
        hUint32                 maxAnisotropy_;
        hColour                 borderColour_;
        hFloat                  minLOD_;
        hFloat                  maxLOD_;
    };

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////

    template<>
    inline void SerialiseMethod< Heart::hColour >( Heart::hSerialiser* ser, const Heart::hColour& data )
    {
        SERIALISE_ELEMENT( data.r_ );
        SERIALISE_ELEMENT( data.g_ );
        SERIALISE_ELEMENT( data.b_ );
        SERIALISE_ELEMENT( data.a_ );
    }

    template<>
    inline void DeserialiseMethod< Heart::hColour >( Heart::hSerialiser* ser, Heart::hColour& data )
    {
        DESERIALISE_ELEMENT( data.r_ );
        DESERIALISE_ELEMENT( data.g_ );
        DESERIALISE_ELEMENT( data.b_ );
        DESERIALISE_ELEMENT( data.a_ );
    }

    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    
    template<>
    inline void SerialiseMethod< hBlendStateDesc >( hSerialiser* ser, const hBlendStateDesc& data )
    {
        SERIALISE_ELEMENT_ENUM_AS_INT( data.blendEnable_ );
        SERIALISE_ELEMENT_ENUM_AS_INT( data.srcBlend_ );
        SERIALISE_ELEMENT_ENUM_AS_INT( data.destBlend_ );
        SERIALISE_ELEMENT_ENUM_AS_INT( data.blendOp_ );
        SERIALISE_ELEMENT_ENUM_AS_INT( data.srcBlendAlpha_ );
        SERIALISE_ELEMENT_ENUM_AS_INT( data.destBlendAlpha_ );
        SERIALISE_ELEMENT_ENUM_AS_INT( data.blendOpAlpha_ );
        SERIALISE_ELEMENT( data.renderTargetWriteMask_ );
    }

    template<>
    inline void DeserialiseMethod< hBlendStateDesc >( hSerialiser* ser, hBlendStateDesc& data )
    {
        DESERIALISE_ELEMENT_INT_AS_ENUM( data.blendEnable_ );
        DESERIALISE_ELEMENT_INT_AS_ENUM( data.srcBlend_ );
        DESERIALISE_ELEMENT_INT_AS_ENUM( data.destBlend_ );
        DESERIALISE_ELEMENT_INT_AS_ENUM( data.blendOp_ );
        DESERIALISE_ELEMENT_INT_AS_ENUM( data.srcBlendAlpha_ );
        DESERIALISE_ELEMENT_INT_AS_ENUM( data.destBlendAlpha_ );
        DESERIALISE_ELEMENT_INT_AS_ENUM( data.blendOpAlpha_ );
        DESERIALISE_ELEMENT( data.renderTargetWriteMask_ );
    }

    template<>
    inline void SerialiseMethod< hDepthStencilStateDesc >( hSerialiser* ser, const hDepthStencilStateDesc& data )
    {
        SERIALISE_ELEMENT_ENUM_AS_INT( data.depthEnable_ );
        SERIALISE_ELEMENT_ENUM_AS_INT( data.depthWriteMask_ );
        SERIALISE_ELEMENT_ENUM_AS_INT( data.depthFunc_ );
        SERIALISE_ELEMENT_ENUM_AS_INT( data.stencilEnable_ );
        SERIALISE_ELEMENT( data.stencilReadMask_ );
        SERIALISE_ELEMENT( data.stencilWriteMask_ );
        SERIALISE_ELEMENT_ENUM_AS_INT( data.stencilFailOp_ );
        SERIALISE_ELEMENT_ENUM_AS_INT( data.stencilDepthFailOp_ );
        SERIALISE_ELEMENT_ENUM_AS_INT( data.stencilPassOp_ );
        SERIALISE_ELEMENT_ENUM_AS_INT( data.stencilFunc_ );
        SERIALISE_ELEMENT( data.stencilRef_ );
    }

    template<>
    inline void DeserialiseMethod< hDepthStencilStateDesc >( hSerialiser* ser, hDepthStencilStateDesc& data )
    {
        DESERIALISE_ELEMENT_INT_AS_ENUM( data.depthEnable_ );
        DESERIALISE_ELEMENT_INT_AS_ENUM( data.depthWriteMask_ );
        DESERIALISE_ELEMENT_INT_AS_ENUM( data.depthFunc_ );
        DESERIALISE_ELEMENT_INT_AS_ENUM( data.stencilEnable_ );
        DESERIALISE_ELEMENT( data.stencilReadMask_ );
        DESERIALISE_ELEMENT( data.stencilWriteMask_ );
        DESERIALISE_ELEMENT_INT_AS_ENUM( data.stencilFailOp_ );
        DESERIALISE_ELEMENT_INT_AS_ENUM( data.stencilDepthFailOp_ );
        DESERIALISE_ELEMENT_INT_AS_ENUM( data.stencilPassOp_ );
        DESERIALISE_ELEMENT_INT_AS_ENUM( data.stencilFunc_ );
        DESERIALISE_ELEMENT( data.stencilRef_ );
    }

    template<>
    inline void SerialiseMethod< hRasterizerStateDesc >( hSerialiser* ser, const hRasterizerStateDesc& data )
    {
        SERIALISE_ELEMENT_ENUM_AS_INT( data.fillMode_ );
        SERIALISE_ELEMENT_ENUM_AS_INT( data.cullMode_ );
        SERIALISE_ELEMENT_ENUM_AS_INT( data.frontCounterClockwise_ );
        SERIALISE_ELEMENT( data.depthBias_ );
        SERIALISE_ELEMENT( data.depthBiasClamp_ );
        SERIALISE_ELEMENT( data.slopeScaledDepthBias_ );
        SERIALISE_ELEMENT_ENUM_AS_INT( data.depthClipEnable_ );
        SERIALISE_ELEMENT_ENUM_AS_INT( data.scissorEnable_ );
    }

    template<>
    inline void DeserialiseMethod< hRasterizerStateDesc >( hSerialiser* ser, hRasterizerStateDesc& data )
    {
        DESERIALISE_ELEMENT_INT_AS_ENUM( data.fillMode_ );
        DESERIALISE_ELEMENT_INT_AS_ENUM( data.cullMode_ );
        DESERIALISE_ELEMENT_INT_AS_ENUM( data.frontCounterClockwise_ );
        DESERIALISE_ELEMENT( data.depthBias_ );
        DESERIALISE_ELEMENT( data.depthBiasClamp_ );
        DESERIALISE_ELEMENT( data.slopeScaledDepthBias_ );
        DESERIALISE_ELEMENT_INT_AS_ENUM( data.depthClipEnable_ );
        DESERIALISE_ELEMENT_INT_AS_ENUM( data.scissorEnable_ );
    }

    template<>
    inline void SerialiseMethod< hSamplerStateDesc >( hSerialiser* ser, const hSamplerStateDesc& data )
    {
        SERIALISE_ELEMENT_ENUM_AS_INT( data.filter_ );
        SERIALISE_ELEMENT_ENUM_AS_INT( data.addressU_ );
        SERIALISE_ELEMENT_ENUM_AS_INT( data.addressV_ );
        SERIALISE_ELEMENT_ENUM_AS_INT( data.addressW_ );
        SERIALISE_ELEMENT( data.mipLODBias_ );
        SERIALISE_ELEMENT( data.maxAnisotropy_ );
        SERIALISE_ELEMENT( data.borderColour_ );
        SERIALISE_ELEMENT( data.minLOD_ );
        SERIALISE_ELEMENT( data.maxLOD_ );
    }

    template<>
    inline void DeserialiseMethod< hSamplerStateDesc >( hSerialiser* ser, hSamplerStateDesc& data )
    {
        DESERIALISE_ELEMENT_INT_AS_ENUM( data.filter_ );
        DESERIALISE_ELEMENT_INT_AS_ENUM( data.addressU_ );
        DESERIALISE_ELEMENT_INT_AS_ENUM( data.addressV_ );
        DESERIALISE_ELEMENT_INT_AS_ENUM( data.addressW_ );
        DESERIALISE_ELEMENT( data.mipLODBias_ );
        DESERIALISE_ELEMENT( data.maxAnisotropy_ );
        DESERIALISE_ELEMENT( data.borderColour_ );
        DESERIALISE_ELEMENT( data.minLOD_ );
        DESERIALISE_ELEMENT( data.maxLOD_ );
    }

}//Heart

#endif // RENDERERCONSTANTS_H__