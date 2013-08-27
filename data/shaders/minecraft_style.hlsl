/*

% Description of my shader.
% Second line of description for my shader.

keywords: material classic

date: YYMMDD

*/

#include "hGlobalConstants.hlsl"

#define SUN_DIR (normalize(float3(0.1, -.9, -0.1)))

Texture2D    diffuseTexture;
SamplerState diffuseSampler;
Texture2D    lightTexture;
SamplerState lightSampler;

struct VSInput
{
	float3 position : POSITION;
	float3 normal	: NORMAL;
    float2 tex0     : TEXCOORD0;
};

struct PSInput
{
    float4 position : SV_POSITION;
    float3 normal   : TEXCOORD0;
    float2 tex0     : TEXCOORD1;
};

PSInput mainVP( VSInput input )
{
	PSInput output;
	output.position = mul(g_ViewProjection, float4(input.position,1));
	output.normal   = input.normal;
    output.tex0     = float2(input.tex0.x, input.tex0.y);
	return output;
}

float3 getLight() {
    return -normalize(g_View[2].xyz);
    //return -SUN_DIR;
}

float4 mainAlphaCutoutFP(PSInput input) : SV_TARGET0 {
    float4 diffusecol=diffuseTexture.Sample(diffuseSampler, input.tex0);
    clip(diffusecol.a-0.00001);
    float3 sun = getLight();
    float NdotL = dot(normalize(input.normal),sun);
    //half lambert 
    NdotL = (NdotL * .5f) + 0.5f;
    return float4(diffusecol.rgb*NdotL,diffusecol.a);
    //diffusecol.rgb=normalize(mul(g_View, input.normal)).xyz;
    //return float4(-diffusecol.rgb,diffusecol.a);
}

float4 mainFP( PSInput input ) : SV_TARGET0 
{
    float4 diffusecol=diffuseTexture.Sample(diffuseSampler, input.tex0);
    float3 sun = getLight();
    float NdotL = dot(normalize(input.normal),sun);
    //half lambert 
    NdotL = (NdotL * .5f) + 0.5f;
    return float4(diffusecol.rgb*NdotL,diffusecol.a);
    //diffusecol.rgb=normalize(mul(g_View, input.normal)).xyz;
    //return float4(-diffusecol.rgb,diffusecol.a);
}