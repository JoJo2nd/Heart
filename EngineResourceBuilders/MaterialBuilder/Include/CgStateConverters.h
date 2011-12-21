/********************************************************************

	filename: 	CgStateConverters.h	
	
	Copyright (c) 10:12:2011 James Moran
	
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
#ifndef CGSTATECONVERTERS_H__
#define CGSTATECONVERTERS_H__


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#define VALIDATE_CG_STATE( cgst, type ) \
    if ( cgGetStateType( cgst ) != type ) \
    ThrowFatalError( "CG Pass state %s, got state type %s expected %s", \
    cgGetStateName( cgst ), \
    cgGetTypeString( cgGetStateType( cgst ) ), \
    cgGetTypeString( type ) );

#define REMAP_ENUM_VALUE( lhs, st, val, remap, found ) \
{ \
    found = false; \
    const char* vname = cgGetStateEnumerantName( st, val ); \
    for ( hUint32 i = 0; i < sizeof(remap)/sizeof(remap[0]); ++i ) \
    { \
    if ( Heart::hStrCmp( remap[i].name, vname ) == 0 ) \
        { \
        lhs = remap[i].value; found = true; \
        } \
    } \
    if ( !found ) ThrowFatalError( "Unexpected enum value %s", vname ); \
}

#define CONVERT_CG_BOOL_STATE( stname, lhs, cgstas, found ) \
{\
    CGstate cgst = cgGetStateAssignmentState( cgstas ); \
    if ( Heart::hStrCmp( cgGetStateName( cgst ), stname ) == 0 ) \
    { \
    VALIDATE_CG_STATE( cgst, CG_BOOL ); \
    int nvalues; \
    const CGbool* bvals = cgGetBoolStateAssignmentValues( cgstas, &nvalues ); \
    lhs = bvals[0] ? Heart::RSV_ENABLE : Heart::RSV_DISABLE; \
    found = true; \
    } \
}

#define CONVERT_CG_BOOLx4_STATE( stname, lhs1, lhs2, lhs3, lhs4, cgstas, found ) \
{\
    CGstate cgst = cgGetStateAssignmentState( cgstas ); \
    if ( Heart::hStrCmp( cgGetStateName( cgst ), stname ) == 0 ) \
    { \
    VALIDATE_CG_STATE( cgst, CG_BOOL ); \
    int nvalues; \
    const CGbool* bvals = cgGetBoolStateAssignmentValues( cgstas, &nvalues ); \
    lhs1 = bvals[0] ? Heart::RSV_ENABLE : Heart::RSV_DISABLE; \
    lhs2 = bvals[1] ? Heart::RSV_ENABLE : Heart::RSV_DISABLE; \
    lhs3 = bvals[2] ? Heart::RSV_ENABLE : Heart::RSV_DISABLE; \
    lhs4 = bvals[3] ? Heart::RSV_ENABLE : Heart::RSV_DISABLE; \
    found = true \
    } \
}

#define CONVERT_CG_BOOL_COLOURWRITE_STATE( stname, lhs, r, g, b, a, cgstas, found ) \
{ \
    CGstate cgst = cgGetStateAssignmentState( cgstas ); \
    if ( Heart::hStrCmp( cgGetStateName( cgst ), stname ) == 0 ) \
    { \
    VALIDATE_CG_STATE( cgst, CG_BOOL ); \
    int nvalues; \
    const CGbool* bvals = cgGetBoolStateAssignmentValues( cgstas, &nvalues ); \
    lhs = 0; \
    lhs &= bvals[0] ? r : 0; \
    lhs &= bvals[1] ? b : 0; \
    lhs &= bvals[2] ? g : 0; \
    lhs &= bvals[3] ? a : 0; \
    found = true; \
    } \
}

#define CONVERT_CG_INT_ENUM_STATE( stname, lhs, cgstas, enumremap, found ) \
{\
    CGstate cgst = cgGetStateAssignmentState( cgstas ); \
    if ( Heart::hStrCmp( cgGetStateName( cgst ), stname ) == 0 ) \
    { \
        VALIDATE_CG_STATE( cgst, CG_INT ); \
        int nvalues; \
        const int* ivals = cgGetIntStateAssignmentValues( cgstas, &nvalues ); \
        REMAP_ENUM_VALUE( lhs, cgst, ivals[0], enumremap, found ); \
    } \
}

#define CONVERT_CG_INTx2_ENUM_STATE( stname, lhs1, lhs2, cgstas, enumremap, found ) \
{\
    CGstate cgst = cgGetStateAssignmentState( cgstas ); \
    if ( Heart::hStrCmp( cgGetStateName( cgst ), stname ) == 0 ) \
    { \
    VALIDATE_CG_STATE( cgst, CG_INT2 ); \
    int nvalues; \
    const int* ivals = cgGetIntStateAssignmentValues( cgstas, &nvalues ); \
    REMAP_ENUM_VALUE( lhs1, cgst, ivals[0], enumremap, found ); \
    REMAP_ENUM_VALUE( lhs2, cgst, ivals[1], enumremap, found ); \
    } \
}

#define CONVERT_CG_INT_STATE( stname, lhs, cgstas, found ) \
{\
    CGstate cgst = cgGetStateAssignmentState( cgstas ); \
    if ( Heart::hStrCmp( cgGetStateName( cgst ), stname ) == 0 ) \
    { \
    VALIDATE_CG_STATE( cgst, CG_INT ); \
    int nvalues; \
    const int* ivals = cgGetIntStateAssignmentValues( cgstas, &nvalues ); \
    lhs = ivals[0]; \
    found = true; \
    } \
}

#define CONVERT_CG_FLOAT_STATE( stname, lhs, cgstas, found ) \
{\
    CGstate cgst = cgGetStateAssignmentState( cgstas ); \
    if ( Heart::hStrCmp( cgGetStateName( cgst ), stname ) == 0 ) \
    { \
        VALIDATE_CG_STATE( cgst, CG_FLOAT ); \
        int nvalues; \
        const float* ivals = cgGetFloatStateAssignmentValues( cgstas, &nvalues ); \
        lhs = ivals[0]; \
        found = true; \
    } \
}

#define CONVERT_CG_COLOUR_STATE( stname, lhs, cgstas, found ) \
{\
    CGstate cgst = cgGetStateAssignmentState( cgstas ); \
    if ( Heart::hStrCmp( cgGetStateName( cgst ), stname ) == 0 ) \
    { \
        VALIDATE_CG_STATE( cgst, CG_FLOAT4 ); \
        int nvalues; \
        const float* ivals = cgGetFloatStateAssignmentValues( cgstas, &nvalues ); \
        lhs.r_ = ivals[0]; \
        lhs.g_ = ivals[1]; \
        lhs.b_ = ivals[2]; \
        lhs.a_ = ivals[3]; \
        found = true; \
    } \
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////// 

#define CONVERT_CG_INT_ENUM_STATE_SAMP( stname, lhs, cgstas, enumremap, found ) \
{\
    CGstate cgst = cgGetSamplerStateAssignmentState( cgstas ); \
    if ( Heart::hStrCmp( cgGetStateName( cgst ), stname ) == 0 ) \
    { \
    VALIDATE_CG_STATE( cgst, CG_INT ); \
    int nvalues; \
    const int* ivals = cgGetIntStateAssignmentValues( cgstas, &nvalues ); \
    REMAP_ENUM_VALUE( lhs, cgst, ivals[0], enumremap, found ); \
    } \
}

#define CONVERT_CG_INTx2_ENUM_STATE_SAMP( stname, lhs1, lhs2, cgstas, enumremap, found ) \
{\
    CGstate cgst = cgGetSamplerStateAssignmentState( cgstas ); \
    if ( Heart::hStrCmp( cgGetStateName( cgst ), stname ) == 0 ) \
    { \
    VALIDATE_CG_STATE( cgst, CG_INT2 ); \
    int nvalues; \
    const int* ivals = cgGetIntStateAssignmentValues( cgstas, &nvalues ); \
    REMAP_ENUM_VALUE( lhs1, cgst, ivals[0], enumremap, found ); \
    REMAP_ENUM_VALUE( lhs2, cgst, ivals[1], enumremap, found ); \
    } \
}

#define CONVERT_CG_INT_STATE_SAMP( stname, lhs, cgstas, found ) \
{\
    CGstate cgst = cgGetSamplerStateAssignmentState( cgstas ); \
    if ( Heart::hStrCmp( cgGetStateName( cgst ), stname ) == 0 ) \
    { \
    VALIDATE_CG_STATE( cgst, CG_INT ); \
    int nvalues; \
    const int* ivals = cgGetIntStateAssignmentValues( cgstas, &nvalues ); \
    lhs = ivals[0]; \
    found = true; \
    } \
}

#define CONVERT_CG_FLOAT_STATE_SAMP( stname, lhs, cgstas, found ) \
{\
    CGstate cgst = cgGetSamplerStateAssignmentState( cgstas ); \
    if ( Heart::hStrCmp( cgGetStateName( cgst ), stname ) == 0 ) \
    { \
    VALIDATE_CG_STATE( cgst, CG_FLOAT ); \
    int nvalues; \
    const float* ivals = cgGetFloatStateAssignmentValues( cgstas, &nvalues ); \
    lhs = ivals[0]; \
    found = true; \
    } \
}

#define CONVERT_CG_COLOUR_STATE_SAMP( stname, lhs, cgstas, found ) \
{\
    CGstate cgst = cgGetSamplerStateAssignmentState( cgstas ); \
    if ( Heart::hStrCmp( cgGetStateName( cgst ), stname ) == 0 ) \
    { \
    VALIDATE_CG_STATE( cgst, CG_FLOAT4 ); \
    int nvalues; \
    const float* ivals = cgGetFloatStateAssignmentValues( cgstas, &nvalues ); \
    lhs.r_ = ivals[0]; \
    lhs.g_ = ivals[1]; \
    lhs.b_ = ivals[2]; \
    lhs.a_ = ivals[3]; \
    found = true; \
    } \
}

#endif // CGSTATECONVERTERS_H__

/*

AlphaBlendEnable (Type:bool)
AlphaFunc (Type:float2)
AlphaRef (Type:float)
BlendOp (Type:int)
{
    { "funcadd" ,  },
    { "funcsubtract" ,  },
    { "funcreversesubtract" ,  },
    { "add" ,  },
    { "subtract" ,  },
    { "reversesubtract" ,  },
    { "min" ,  },
    { "max" ,  },
    { "logicop" ,  },
};
BlendEquation (Type:int)
{
    { "funcadd" ,  },
    { "funcsubtract" ,  },
    { "funcreversesubtract" ,  },
    { "add" ,  },
    { "subtract" ,  },
    { "reversesubtract" ,  },
    { "min" ,  },
    { "max" ,  },
    { "logicop" ,  },
};
BlendFunc (Type:int2)
{
    { "zero" ,  },
    { "one" ,  },
    { "destcolor" ,  },
    { "oneminusdestcolor" ,  },
    { "invdestcolor" ,  },
    { "srcalpha" ,  },
    { "oneminussrcalpha" ,  },
    { "invsrcalpha" ,  },
    { "dstalpha" ,  },
    { "oneminusdstalpha" ,  },
    { "invdestalpha" ,  },
    { "srcalphasaturate" ,  },
    { "srcalphasat" ,  },
    { "srccolor" ,  },
    { "oneminussrccolor" ,  },
    { "invsrccolor" ,  },
    { "constantcolor" ,  },
    { "blendfactor" ,  },
    { "oneminusconstantcolor" ,  },
    { "invblendfactor" ,  },
    { "constantalpha" ,  },
    { "oneminusconstantalpha" ,  },
};
BlendFuncSeparate (Type:int4)
{
    { "zero" ,  },
    { "one" ,  },
    { "destcolor" ,  },
    { "oneminusdestcolor" ,  },
    { "invdestcolor" ,  },
    { "srcalpha" ,  },
    { "oneminussrcalpha" ,  },
    { "invsrcalpha" ,  },
    { "dstalpha" ,  },
    { "oneminusdstalpha" ,  },
    { "invdestalpha" ,  },
    { "srcalphasaturate" ,  },
    { "srcalphasat" ,  },
    { "srccolor" ,  },
    { "oneminussrccolor" ,  },
    { "invsrccolor" ,  },
    { "constantcolor" ,  },
    { "blendfactor" ,  },
    { "oneminusconstantcolor" ,  },
    { "invblendfactor" ,  },
    { "constantalpha" ,  },
    { "oneminusconstantalpha" ,  },
};
BlendEquationSeparate (Type:int2)
{
    { "funcadd" ,  },
    { "funcsubtract" ,  },
    { "min" ,  },
    { "max" ,  },
    { "add" ,  },
    { "subtract" ,  },
    { "funcreversesubtract" ,  },
    { "reversesubtract" ,  },
    { "logicop" ,  },
};
BlendColor (Type:float4)
ClearColor (Type:float4)
ClearStencil (Type:int)
{
};
ClearDepth (Type:float)
ClipPlane (Type:float4)
ClipPlaneEnable (Type:int)
{
    { "d3dclipplane0" ,  },
    { "d3dclipplane1" ,  },
    { "d3dclipplane2" ,  },
    { "d3dclipplane3" ,  },
    { "d3dclipplane4" ,  },
    { "d3dclipplane5" ,  },
};
ColorWriteEnable (Type:bool4)
ColorMask (Type:bool4)
ColorVertex (Type:bool)
ColorMaterial (Type:int2)
{
    { "emission" ,  },
    { "emissive" ,  },
    { "ambient" ,  },
    { "diffuse" ,  },
    { "specular" ,  },
    { "front" ,  },
    { "back" ,  },
    { "frontandback" ,  },
    { "ambientanddiffuse" ,  },
};
ColorMatrix (Type:float4x4)
ColorTransform (Type:float4x4)
CullMode (Type:int)
{
    { "none" ,  },
    { "front" ,  },
    { "back" ,  },
    { "cw" ,  },
    { "ccw" ,  },
    { "frontandback" ,  },
};
DepthBounds (Type:float2)
DepthBias (Type:float)
DepthFunc (Type:int)
{
    { "never" ,  },
    { "less" ,  },
    { "lequal" ,  },
    { "lessequal" ,  },
    { "equal" ,  },
    { "greater" ,  },
    { "notequal" ,  },
    { "gequal" ,  },
    { "greaterequal" ,  },
    { "always" ,  },
};
ZFunc (Type:int)
{
    { "never" ,  },
    { "less" ,  },
    { "lequal" ,  },
    { "lessequal" ,  },
    { "equal" ,  },
    { "greater" ,  },
    { "notequal" ,  },
    { "gequal" ,  },
    { "greaterequal" ,  },
    { "always" ,  },
};
DepthMask (Type:bool)
ZWriteEnable (Type:bool)
DepthRange (Type:float2)
FogDistanceMode (Type:int)
{
};
FogMode (Type:int)
{
    { "linear" ,  },
    { "exp" ,  },
    { "exp2" ,  },
};
FogTableMode (Type:int)
{
    { "none" ,  },
    { "linear" ,  },
    { "exp" ,  },
    { "exp2" ,  },
};
IndexedVertexBlendEnable (Type:bool)
FogDensity (Type:float)
FogStart (Type:float)
FogEnd (Type:float)
FogColor (Type:float4)
FragmentEnvParameter (Type:float4)
FragmentLocalParameter (Type:float4)
FogVertexMode (Type:int)
{
    { "none" ,  },
    { "exp" ,  },
    { "exp2" ,  },
    { "linear" ,  },
};
LightModelAmbient (Type:float4)
Ambient (Type:float4)
LightingEnable (Type:bool)
Lighting (Type:bool)
LightEnable (Type:bool)
LightAmbient (Type:float4)
LightConstantAttenuation (Type:float)
LightAttenuation0 (Type:float)
LightDiffuse (Type:float4)
LightLinearAttenuation (Type:float)
LightAttenuation1 (Type:float)
LightPosition (Type:float4)
LightQuadraticAttenuation (Type:float)
LightAttenuation2 (Type:float)
LightSpecular (Type:float4)
LightSpotCutoff (Type:float)
LightFalloff (Type:float)
LightSpotDirection (Type:float4)
LightDirection (Type:float4)
LightPhi (Type:float)
LightRange (Type:float)
LightTheta (Type:float)
LightType (Type:int)
{
};
LocalViewer (Type:bool)
MultiSampleAntialias (Type:bool)
MultiSampleMask (Type:int)
{
};
PatchSegments (Type:float)
PointScale_A (Type:float)
PointScale_B (Type:float)
PointScale_C (Type:float)
PointScaleEnable (Type:bool)
RangeFogEnable (Type:bool)
SpecularEnable (Type:bool)
TweenFactor (Type:float)
VertexBlend (Type:int)
{
};
AmbientMaterialSource (Type:int)
{
};
DiffuseMaterialSource (Type:int)
{
};
EmissiveMaterialSource (Type:int)
{
};
SpecularMaterialSource (Type:int)
{
};
Clipping (Type:bool)
LightModelColorControl (Type:int)
{
    { "singlecolor" ,  },
    { "separatespecular" ,  },
};
LineStipple (Type:int2)
{
};
LineWidth (Type:float)
LogicOp (Type:int)
{
    { "clear" ,  },
    { "and" ,  },
    { "andreverse" ,  },
    { "copy" ,  },
    { "andinverted" ,  },
    { "noop" ,  },
    { "xor" ,  },
    { "or" ,  },
    { "nor" ,  },
    { "equiv" ,  },
    { "invert" ,  },
    { "orreverse" ,  },
    { "copyinverted" ,  },
    { "nand" ,  },
    { "set" ,  },
    { "lowerleft" ,  },
    { "upperleft" ,  },
    { "zero" ,  },
    { "s" ,  },
    { "r" ,  },
};
MaterialAmbient (Type:float4)
MaterialDiffuse (Type:float4)
MaterialEmission (Type:float4)
MaterialEmissive (Type:float4)
MaterialShininess (Type:float)
MaterialPower (Type:float)
MaterialSpecular (Type:float4)
ModelViewMatrix (Type:float4x4)
ModelViewTransform (Type:float4x4)
ViewTransform (Type:float4x4)
WorldTransform (Type:float4x4)
PointDistanceAttenuation (Type:float3)
PointFadeThresholdSize (Type:float)
PointSize (Type:float)
PointSizeMin (Type:float)
PointSizeMax (Type:float)
PointSpriteCoordOrigin (Type:int)
{
};
PointSpriteCoordReplace (Type:bool)
PointSpriteRMode (Type:int)
{
};
PolygonMode (Type:int2)
{
    { "front" ,  },
    { "back" ,  },
    { "frontandback" ,  },
    { "point" ,  },
    { "line" ,  },
    { "fill" ,  },
    { "solid" ,  },
    { "wireframe" ,  },
};
FillMode (Type:int2)
{
    { "solid" ,  },
    { "wireframe" ,  },
    { "point" ,  },
};
LastPixel (Type:bool)
PolygonOffset (Type:float2)
ProjectionMatrix (Type:float4x4)
ProjectionTransform (Type:float4x4)
Scissor (Type:int4)
{
};
ShadeModel (Type:int)
{
    { "flat" ,  },
    { "smooth" ,  },
};
ShadeMode (Type:int)
{
    { "flat" ,  },
    { "smooth" ,  },
    { "gouraud" ,  },
    { "phong" ,  },
};
SlopScaleDepthBias (Type:float)
DestBlend (Type:int)
{
    { "zero" ,  },
    { "one" ,  },
    { "destcolor" ,  },
    { "invdestcolor" ,  },
    { "srcalpha" ,  },
    { "invsrcalpha" ,  },
    { "dstalpha" ,  },
    { "invdestalpha" ,  },
    { "srcalphasat" ,  },
    { "srccolor" ,  },
    { "invsrccolor" ,  },
    { "blendfactor" ,  },
    { "invblendfactor" ,  },
};
SrcBlend (Type:int)
{
    { "zero" ,  },
    { "one" ,  },
    { "destcolor" ,  },
    { "invdestcolor" ,  },
    { "srcalpha" ,  },
    { "invsrcalpha" ,  },
    { "dstalpha" ,  },
    { "invdestalpha" ,  },
    { "srcalphasat" ,  },
    { "srccolor" ,  },
    { "invsrccolor" ,  },
    { "blendfactor" ,  },
    { "invblendfactor" ,  },
};
StencilFunc (Type:int3)
{
    { "never" ,  },
    { "less" ,  },
    { "lequal" ,  },
    { "lessequal" ,  },
    { "equal" ,  },
    { "greater" ,  },
    { "notequal" ,  },
    { "gequal" ,  },
    { "greaterequal" ,  },
    { "always" ,  },
};
StencilMask (Type:int)
{
};
StencilPass (Type:int)
{
};
StencilRef (Type:int)
{
};
StencilWriteMask (Type:int)
{
};
StencilZFail (Type:int)
{
};
TextureFactor (Type:int)
{
};
StencilOp (Type:int3)
{
    { "keep" ,  },
    { "zero" ,  },
    { "replace" ,  },
    { "incr" ,  },
    { "decr" ,  },
    { "invert" ,  },
    { "incrwrap" ,  },
    { "decrwrap" ,  },
    { "incrsat" ,  },
    { "decrsat" ,  },
};
StencilFuncSeparate (Type:int4)
{
    { "never" ,  },
    { "less" ,  },
    { "lequal" ,  },
    { "equal" ,  },
    { "greater" ,  },
    { "notequal" ,  },
    { "gequal" ,  },
    { "always" ,  },
    { "lessequal" ,  },
    { "greaterequal" ,  },
    { "front" ,  },
    { "back" ,  },
    { "frontandback" ,  },
};
StencilMaskSeparate (Type:int2)
{
    { "front" ,  },
    { "back" ,  },
    { "frontandback" ,  },
};
StencilOpSeparate (Type:int4)
{
    { "front" ,  },
    { "back" ,  },
    { "frontandback" ,  },
    { "keep" ,  },
    { "zero" ,  },
    { "replace" ,  },
    { "incr" ,  },
    { "decr" ,  },
    { "invert" ,  },
    { "incrwrap" ,  },
    { "decrwrap" ,  },
    { "incrsat" ,  },
    { "decrsat" ,  },
};
TexGenSMode (Type:int)
{
    { "objectlinear" ,  },
    { "eyelinear" ,  },
    { "spheremap" ,  },
    { "reflectionmap" ,  },
    { "normalmap" ,  },
};
TexGenSObjectPlane (Type:float4)
TexGenSEyePlane (Type:float4)
TexGenTMode (Type:int)
{
    { "objectlinear" ,  },
    { "eyelinear" ,  },
    { "spheremap" ,  },
    { "reflectionmap" ,  },
    { "normalmap" ,  },
};
TexGenTObjectPlane (Type:float4)
TexGenTEyePlane (Type:float4)
TexGenRMode (Type:int)
{
    { "objectlinear" ,  },
    { "eyelinear" ,  },
    { "spheremap" ,  },
    { "reflectionmap" ,  },
    { "normalmap" ,  },
};
TexGenRObjectPlane (Type:float4)
TexGenREyePlane (Type:float4)
TexGenQMode (Type:int)
{
    { "objectlinear" ,  },
    { "eyelinear" ,  },
    { "spheremap" ,  },
    { "reflectionmap" ,  },
    { "normalmap" ,  },
};
TexGenQObjectPlane (Type:float4)
TexGenQEyePlane (Type:float4)
TextureEnvColor (Type:float4)
TextureEnvMode (Type:int)
{
    { "modulate" ,  },
    { "decal" ,  },
    { "blend" ,  },
    { "replace" ,  },
    { "add" ,  },
};
Texture2D (Type:sampler2D)
Texture3D (Type:sampler3D)
TextureRectangle (Type:samplerRECT)
TextureCubeMap (Type:samplerCUBE)
Texture1DEnable (Type:bool)
Texture2DEnable (Type:bool)
Texture3DEnable (Type:bool)
TextureRectangleEnable (Type:bool)
TextureCubeMapEnable (Type:bool)
TextureTransform (Type:float4x4)
TextureMatrix (Type:float4x4)
VertexEnvParameter (Type:float4)
VertexLocalParameter (Type:float4)
AlphaTestEnable (Type:bool)
AutoNormalEnable (Type:bool)
BlendEnable (Type:bool)
ColorLogicOpEnable (Type:bool)
CullFaceEnable (Type:bool)
DepthBoundsEnable (Type:bool)
DepthClampEnable (Type:bool)
DepthTestEnable (Type:bool)
ZEnable (Type:bool)
DitherEnable (Type:bool)
FogEnable (Type:bool)
LightModelLocalViewerEnable (Type:bool)
LightModelTwoSideEnable (Type:bool)
LineSmoothEnable (Type:bool)
LineStippleEnable (Type:bool)
LogicOpEnable (Type:bool)
MultisampleEnable (Type:bool)
NormalizeEnable (Type:bool)
NormalizeNormals (Type:bool)
PointSmoothEnable (Type:bool)
PointSpriteEnable (Type:bool)
PolygonOffsetFillEnable (Type:bool)
PolygonOffsetLineEnable (Type:bool)
PolygonOffsetPointEnable (Type:bool)
PolygonSmoothEnable (Type:bool)
PolygonStippleEnable (Type:bool)
RescaleNormalEnable (Type:bool)
SampleAlphaToCoverageEnable (Type:bool)
SampleAlphaToOneEnable (Type:bool)
SampleCoverageEnable (Type:bool)
ScissorTestEnable (Type:bool)
StencilTestEnable (Type:bool)
StencilEnable (Type:bool)
StencilTestTwoSideEnable (Type:bool)
StencilFail (Type:int)
{
};
TexGenSEnable (Type:bool)
TexGenTEnable (Type:bool)
TexGenREnable (Type:bool)
TexGenQEnable (Type:bool)
Wrap0 (Type:int)
{
};
Wrap1 (Type:int)
{
};
Wrap2 (Type:int)
{
};
Wrap3 (Type:int)
{
};
Wrap4 (Type:int)
{
};
Wrap5 (Type:int)
{
};
Wrap6 (Type:int)
{
};
Wrap7 (Type:int)
{
};
Wrap8 (Type:int)
{
};
Wrap9 (Type:int)
{
};
Wrap10 (Type:int)
{
};
Wrap11 (Type:int)
{
};
Wrap12 (Type:int)
{
};
Wrap13 (Type:int)
{
};
Wrap14 (Type:int)
{
};
Wrap15 (Type:int)
{
};
VertexProgram (Type:program)
FragmentProgram (Type:program)
VertexShader (Type:program)
PixelShader (Type:program)
PixelShaderConstant (Type:float4x4)
VertexShaderConstant (Type:float4x4)
PixelShaderConstant1 (Type:float4)
PixelShaderConstant2 (Type:float4x2)
PixelShaderConstant3 (Type:float4x3)
PixelShaderConstant4 (Type:float4x4)
VertexShaderConstant1 (Type:float4)
VertexShaderConstant2 (Type:float4x2)
VertexShaderConstant3 (Type:float4x3)
vertexShaderConstant4 (Type:float4x4)
PixelShaderConstantB (Type:bool4)
PixelShaderConstantI (Type:int4)
{
};
PixelShaderConstantF (Type:float4)
VertexShaderConstantB (Type:bool4)
VertexShaderConstantI (Type:int4)
{
};
VertexShaderConstantF (Type:float4)
Texture (Type:texture)
Sampler (Type:sampler)
AddressU (Type:int)
{
    { "repeat" ,  },
    { "wrap" ,  },
    { "clamp" ,  },
    { "clamptoedge" ,  },
    { "clamptoborder" ,  },
    { "border" ,  },
    { "mirroredrepeat" ,  },
    { "mirror" ,  },
    { "mirrorclamp" ,  },
    { "mirrorclamptoedge" ,  },
    { "mirrorclamptoborder" ,  },
    { "mirroronce" ,  },
};
AddressV (Type:int)
{
    { "repeat" ,  },
    { "wrap" ,  },
    { "clamp" ,  },
    { "clamptoedge" ,  },
    { "clamptoborder" ,  },
    { "border" ,  },
    { "mirroredrepeat" ,  },
    { "mirror" ,  },
    { "mirrorclamp" ,  },
    { "mirrorclamptoedge" ,  },
    { "mirrorclamptoborder" ,  },
    { "mirroronce" ,  },
};
AddressW (Type:int)
{
    { "repeat" ,  },
    { "wrap" ,  },
    { "clamp" ,  },
    { "clamptoedge" ,  },
    { "clamptoborder" ,  },
    { "border" ,  },
    { "mirroredrepeat" ,  },
    { "mirror" ,  },
    { "mirrorclamp" ,  },
    { "mirrorclamptoedge" ,  },
    { "mirrorclamptoborder" ,  },
    { "mirroronce" ,  },
};
BorderColor (Type:int)
{
    { "repeat" ,  },
    { "wrap" ,  },
    { "clamp" ,  },
    { "clamptoedge" ,  },
    { "clamptoborder" ,  },
    { "border" ,  },
    { "mirroredrepeat" ,  },
    { "mirror" ,  },
    { "mirrorclamp" ,  },
    { "mirrorclamptoedge" ,  },
    { "mirrorclamptoborder" ,  },
    { "mirroronce" ,  },
};
MaxAnisotropy (Type:int)
{
    { "repeat" ,  },
    { "wrap" ,  },
    { "clamp" ,  },
    { "clamptoedge" ,  },
    { "clamptoborder" ,  },
    { "border" ,  },
    { "mirroredrepeat" ,  },
    { "mirror" ,  },
    { "mirrorclamp" ,  },
    { "mirrorclamptoedge" ,  },
    { "mirrorclamptoborder" ,  },
    { "mirroronce" ,  },
};
MaxMipLevel (Type:int)
{
    { "repeat" ,  },
    { "wrap" ,  },
    { "clamp" ,  },
    { "clamptoedge" ,  },
    { "clamptoborder" ,  },
    { "border" ,  },
    { "mirroredrepeat" ,  },
    { "mirror" ,  },
    { "mirrorclamp" ,  },
    { "mirrorclamptoedge" ,  },
    { "mirrorclamptoborder" ,  },
    { "mirroronce" ,  },
};
MinFilter (Type:int)
{
    { "repeat" ,  },
    { "wrap" ,  },
    { "clamp" ,  },
    { "clamptoedge" ,  },
    { "clamptoborder" ,  },
    { "border" ,  },
    { "mirroredrepeat" ,  },
    { "mirror" ,  },
    { "mirrorclamp" ,  },
    { "mirrorclamptoedge" ,  },
    { "mirrorclamptoborder" ,  },
    { "mirroronce" ,  },
};
MagFilter (Type:int)
{
    { "repeat" ,  },
    { "wrap" ,  },
    { "clamp" ,  },
    { "clamptoedge" ,  },
    { "clamptoborder" ,  },
    { "border" ,  },
    { "mirroredrepeat" ,  },
    { "mirror" ,  },
    { "mirrorclamp" ,  },
    { "mirrorclamptoedge" ,  },
    { "mirrorclamptoborder" ,  },
    { "mirroronce" ,  },
};
MipFilter (Type:int)
{
    { "repeat" ,  },
    { "wrap" ,  },
    { "clamp" ,  },
    { "clamptoedge" ,  },
    { "clamptoborder" ,  },
    { "border" ,  },
    { "mirroredrepeat" ,  },
    { "mirror" ,  },
    { "mirrorclamp" ,  },
    { "mirrorclamptoedge" ,  },
    { "mirrorclamptoborder" ,  },
    { "mirroronce" ,  },
    { "none" ,  },
    { "point" ,  },
    { "linear" ,  },
    { "anisotropic" ,  },
    { "pyramidalquad" ,  },
    { "gaussianquad" ,  },
};
MipMapLodBias (Type:int)
{
    { "repeat" ,  },
    { "wrap" ,  },
    { "clamp" ,  },
    { "clamptoedge" ,  },
    { "clamptoborder" ,  },
    { "border" ,  },
    { "mirroredrepeat" ,  },
    { "mirror" ,  },
    { "mirrorclamp" ,  },
    { "mirrorclamptoedge" ,  },
    { "mirrorclamptoborder" ,  },
    { "mirroronce" ,  },
};
ColorWriteEnable1 (Type:bool4)
ColorWriteEnable2 (Type:bool4)
ColorWriteEnable3 (Type:bool4)
TwoSidedStencilMode (Type:bool)
BlendOpAlpha (Type:int)
{
    { "add" ,  },
    { "subtract" ,  },
    { "revsubtract" ,  },
    { "min" ,  },
    { "max" ,  },
};
SrcBlendAlpha (Type:int)
{
    { "zero" ,  },
    { "one" ,  },
    { "destcolor" ,  },
    { "invdestcolor" ,  },
    { "srcalpha" ,  },
    { "invsrcalpha" ,  },
    { "dstalpha" ,  },
    { "invdestalpha" ,  },
    { "srcalphasat" ,  },
    { "srccolor" ,  },
    { "invsrccolor" ,  },
    { "blendfactor" ,  },
    { "invblendfactor" ,  },
};
DestBlendAlpha (Type:int)
{
    { "zero" ,  },
    { "one" ,  },
    { "destcolor" ,  },
    { "invdestcolor" ,  },
    { "srcalpha" ,  },
    { "invsrcalpha" ,  },
    { "dstalpha" ,  },
    { "invdestalpha" ,  },
    { "srcalphasat" ,  },
    { "srccolor" ,  },
    { "invsrccolor" ,  },
    { "blendfactor" ,  },
    { "invblendfactor" ,  },
};
SeparateAlphaBlendEnable (Type:bool)
AlphaOp (Type:int)
{
    { "disable" ,  },
    { "selectarg1" ,  },
    { "selectarg2" ,  },
    { "modulate" ,  },
    { "modulate2x" ,  },
    { "modulate4x" ,  },
    { "add" ,  },
    { "addsigned" ,  },
    { "addsigned2x" ,  },
    { "subtract" ,  },
    { "addsmooth" ,  },
    { "blenddiffusealpha" ,  },
    { "blendtexturealpha" ,  },
    { "blendfactoralpha" ,  },
    { "blendtexturealphapm" ,  },
    { "blendcurrentalpha" ,  },
    { "premodulate" ,  },
    { "modulatealpha_addcolor" ,  },
    { "modulatecolor_addalpha" ,  },
    { "modulateinvalpha_addcolor" ,  },
    { "modulateinvcolor_addalpha" ,  },
    { "bumpenvmap" ,  },
    { "bumpenvmapluminance" ,  },
    { "dotproduct3" ,  },
    { "multiplyadd" ,  },
    { "lerp" ,  },
};
ColorOp (Type:int)
{
    { "disable" ,  },
    { "selectarg1" ,  },
    { "selectarg2" ,  },
    { "modulate" ,  },
    { "modulate2x" ,  },
    { "modulate4x" ,  },
    { "add" ,  },
    { "addsigned" ,  },
    { "addsigned2x" ,  },
    { "subtract" ,  },
    { "addsmooth" ,  },
    { "blenddiffusealpha" ,  },
    { "blendtexturealpha" ,  },
    { "blendfactoralpha" ,  },
    { "blendtexturealphapm" ,  },
    { "blendcurrentalpha" ,  },
    { "premodulate" ,  },
    { "modulatealpha_addcolor" ,  },
    { "modulatecolor_addalpha" ,  },
    { "modulateinvalpha_addcolor" ,  },
    { "modulateinvcolor_addalpha" ,  },
    { "bumpenvmap" ,  },
    { "bumpenvmapluminance" ,  },
    { "dotproduct3" ,  },
    { "multiplyadd" ,  },
    { "lerp" ,  },
};
AlphaArg0 (Type:int)
{
    { "constant" ,  },
    { "current" ,  },
    { "diffuse" ,  },
    { "selectmask" ,  },
    { "specular" ,  },
    { "temp" ,  },
    { "texture" ,  },
    { "tfactor" ,  },
};
AlphaArg1 (Type:int)
{
    { "constant" ,  },
    { "current" ,  },
    { "diffuse" ,  },
    { "selectmask" ,  },
    { "specular" ,  },
    { "temp" ,  },
    { "texture" ,  },
    { "tfactor" ,  },
};
AlphaArg2 (Type:int)
{
    { "constant" ,  },
    { "current" ,  },
    { "diffuse" ,  },
    { "selectmask" ,  },
    { "specular" ,  },
    { "temp" ,  },
    { "texture" ,  },
    { "tfactor" ,  },
};
ColorArg0 (Type:int)
{
    { "constant" ,  },
    { "current" ,  },
    { "diffuse" ,  },
    { "selectmask" ,  },
    { "specular" ,  },
    { "temp" ,  },
    { "texture" ,  },
    { "tfactor" ,  },
};
ColorArg1 (Type:int)
{
    { "constant" ,  },
    { "current" ,  },
    { "diffuse" ,  },
    { "selectmask" ,  },
    { "specular" ,  },
    { "temp" ,  },
    { "texture" ,  },
    { "tfactor" ,  },
};
ColorArg2 (Type:int)
{
    { "constant" ,  },
    { "current" ,  },
    { "diffuse" ,  },
    { "selectmask" ,  },
    { "specular" ,  },
    { "temp" ,  },
    { "texture" ,  },
    { "tfactor" ,  },
};
BumpEnvLScale (Type:float)
BumpEnvLOffset (Type:float)
BumpEnvMat00 (Type:float)
BumpEnvMat01 (Type:float)
BumpEnvMat10 (Type:float)
BumpEnvMat11 (Type:float)
ResultArg (Type:int)
{
    { "constant" ,  },
    { "current" ,  },
    { "diffuse" ,  },
    { "selectmask" ,  },
    { "specular" ,  },
    { "temp" ,  },
    { "texture" ,  },
    { "tfactor" ,  },
};
TexCoordIndex (Type:int)
{
    { "passthru" ,  },
    { "cameraspacenormal" ,  },
    { "cameraspaceposition" ,  },
    { "cameraspacereflectionvector" ,  },
    { "spheremap" ,  },
};
TextureTransformFlags (Type:int)
{
    { "disable" ,  },
    { "count1" ,  },
    { "count2" ,  },
    { "count3" ,  },
    { "count4" ,  },
    { "projected" ,  },
};
GeometryProgram (Type:program)
GeometryShader (Type:program)
HullShader (Type:program)
DomainShader (Type:program)
ClipPlaneEnable (Type:bool)
CullFace (Type:int)
{
    { "front" ,  },
    { "back" ,  },
    { "frontandback" ,  },
};
FogCoordSrc (Type:int)
{
    { "fragmentdepth" ,  },
    { "fogcoord" ,  },
};
FrontFace (Type:int)
{
    { "cw" ,  },
    { "ccw" ,  },
};
LightSpotExponent (Type:float)
WorldTransform (Type:float4x4)
Texture1D (Type:sampler1D)
VertexProgramPointSizeEnable (Type:bool)
VertexProgramTwoSideEnable (Type:bool)
TessellationControlProgram (Type:program)
TessellationEvaluationProgram (Type:program)
TessellationControlShader (Type:program)
TessellationEvaluationShader (Type:program)

*/