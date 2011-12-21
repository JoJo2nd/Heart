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

	static const hUint32 NUM_RENDER_BUFFERS = 3;
	const hColour WHITE( 1.0f, 1.0f, 1.0f, 1.0f );
	const hColour BLACK( 0.0f, 0.0f, 0.0f, 1.0f );

    struct hShaderParameter
    {
        hShaderParameter() 
            : semanticID_(~0U)
            , name_(NULL)
            , size_(0)
            , cReg_(0)
            , cBuffer_(0)
        {

        }

        hUint32	                            semanticID_; //EFFECTSEMANTICID_MAX(~0U) if not valid
        hChar*						        name_; 
        hUint32                             size_;
        hUint32                             cReg_ : 24;
        hUint32                             cBuffer_ : 8;
    };

    enum ShaderType
    {
        ShaderType_VERTEXPROG,
        ShaderType_FRAGMENTPROG,

        ShaderType_MAX,
    };

	enum TextureFormat
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
        TFORMAT_DXT1
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

		RS_FORCE_DWORD = 0xFFFFFFFF
	};

	enum RENDER_STATE_FLAG
	{
		RSV_COLOUR_WRITE_RED	= 1,
		RSV_COLOUR_WRITE_GREEN	= 1 << 1,
		RSV_COLOUR_WRITE_BLUE	= 1 << 2,
		RSV_COLOUR_WRITE_ALPHA	= 1 << 3,
		RSV_COLOUR_WRITE_FULL	= 0x0000000F,
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

		hrVE_MAX			
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

	enum BufferFlags
	{
		DISCARD_ON_LOCK		= 1,
		DYNAMIC				= 1 << 1,
	};

	enum PrimitiveType
	{
		PRIMITIVETYPE_TRILIST,
		PRIMITIVETYPE_TRISTRIP,
		PRIMITIVETYPE_TRIFAN,
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
    };

    struct hRasterizerStateDesc
    {
        RENDER_STATE_VALUE    fillMode_;
        RENDER_STATE_VALUE    cullMode_;
        RENDER_STATE_VALUE    frontCounterClockwise_;
        hUint32               depthBias_;
        hFloat                depthBiasClamp_;
        hFloat                slopeScaledDepthBias_;
        RENDER_STATE_VALUE    depthClipEnable_;
        RENDER_STATE_VALUE    scissorEnable_;
    };

    struct hSamplerStateDesc
    {
        hSAMPLER_STATE_VALUE    filter_;
        hSAMPLER_STATE_VALUE    addressU_;
        hSAMPLER_STATE_VALUE    addressV_;
        hSAMPLER_STATE_VALUE    addressW_;
        hFloat                  mipLODBias_;
        hUint32                 maxAnisotropy_;
        hColour                 borderColor_;
        hFloat                  minLOD_;
        hFloat                  maxLOD_;
    };

namespace EffectSemantics
{
	enum EffectSemanticID
	{
		EFFECTSEMANTICID_SCREENPARAMETERS,
		EFFECTSEMANTICID_WORLDMATRIX,
		EFFECTSEMANTICID_INVERSEWORLDMATRIX,
		EFFECTSEMANTICID_WORLDINVERSETRANSPOSEMATRIX,
		EFFECTSEMANTICID_VIEWMATRIX,
		EFFECTSEMANTICID_INVERSEVIEWMATRIX,
		EFFECTSEMANTICID_INVERSETRANSPOSEVIEWMATRIX,
		EFFECTSEMANTICID_PROJECTIONMATRIX,
		EFFECTSEMANTICID_INVERSEPROJECTIONMATRIX,
		EFFECTSEMANTICID_VIEWPROJECTIONMATRIX,
		EFFECTSEMANTICID_INVERSEVIEWPROJECTIONMATRIX,
		EFFECTSEMANTICID_WORLDVIEWMATRIX,
		EFFECTSEMANTICID_WORLDVIEWINVERSEMATRIX,
		EFFECTSEMANTICID_WORLDINVERSETRANSPOSEVIEWMATRIX,
		EFFECTSEMANTICID_WORLDVIEWINVERSETRANSPOSEMATRIX,
		EFFECTSEMANTICID_WORLDVIEWPROJECTIONMATRIX,
		EFFECTSEMANTICID_MATERIALAMBIENT,
		EFFECTSEMANTICID_MATERIALDIFFUSE,
		EFFECTSEMANTICID_MATERIALEMISSIVE,
		EFFECTSEMANTICID_MATERIALSPECULAR,
		EFFECTSEMANTICID_MATERIALSPECPOWER,
		EFFECTSEMANTICID_FOGSTART,
		EFFECTSEMANTICID_FOGRANGE,
		EFFECTSEMANTICID_FOGCOLOUR,
		EFFECTSEMANTICID_LIGHTBRIGHTNESS,
		EFFECTSEMANTICID_LIGHTPOSITION,
		EFFECTSEMANTICID_LIGHTDIRECTION,
		EFFECTSEMANTICID_LIGHTDIFFUSE,
		EFFECTSEMANTICID_LIGHTSPECULAR,
		EFFECTSEMANTICID_LIGHTMINRADIUS,
		EFFECTSEMANTICID_LIGHTMAXRADIUS,
		EFFECTSEMANTICID_LIGHTEXPONENT,
		EFFECTSEMANTICID_LIGHTFALLOFF,
		EFFECTSEMANTICID_DIFFUSETEXTURE0,
		EFFECTSEMANTICID_DIFFUSETEXTURE1,
		EFFECTSEMANTICID_DIFFUSETEXTURE2,
		EFFECTSEMANTICID_DIFFUSETEXTURE3,
		EFFECTSEMANTICID_NORMALTEXTURE,
		EFFECTSEMANTICID_SPECULARTEXTURE,
		EFFECTSEMANTICID_LIGHTMAPTEXTURE,
		EFFECTSEMANTICID_SHADOWMAPTEXTURE,

		EFFECTSEMANTICID_MAX
	};

	static const hUint32 DIFFISE_TEXTURE_SEMANTIC_COUNT = EFFECTSEMANTICID_DIFFUSETEXTURE3 - EFFECTSEMANTICID_DIFFUSETEXTURE0;
	static const hUint32 TEXTURE_SEMANTIC_COUNT = EFFECTSEMANTICID_SHADOWMAPTEXTURE - EFFECTSEMANTICID_DIFFUSETEXTURE0;

	extern const hChar* SemanticNames[];
}//EffectSemantics

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
}//Heart

#endif // RENDERERCONSTANTS_H__