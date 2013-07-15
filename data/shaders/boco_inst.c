/*

% Description of my shader.
% Second line of description for my shader.

keywords: material classic

date: YYMMDD

*/

#include "hGlobalConstants.h"

struct VSInputInst
{
	float3 position : POSITION;
	float4 colour 	: COLOR0;
	float3 normal	: NORMAL;
    float3 xform    : INSTANCE0;
};

struct VSInput
{
	float3 position : POSITION;
	float4 colour 	: COLOR0;
	float3 normal	: NORMAL;
};

struct PSInput
{
    float4 position : SV_POSITION;
    float4 colour   : COLOR0;
    float3 normal   : TEXCOORD0;
};

PSInput mainInstVP( VSInputInst input )
{
	PSInput output;
	output.position = mul( g_ViewProjection, float4(input.position+input.xform,1) );
	output.colour   = input.colour;	
	output.normal   = input.normal;
	return output;
}

PSInput mainVP( VSInput input )
{
	PSInput output;
	output.position = mul( g_ViewProjection, float4(input.position,1) );
	output.colour   = input.colour;	
	output.normal   = input.normal;
	return output;
}

float4 mainFP( PSInput input ) : SV_TARGET0 
{
    float3 sun = normalize(float3(0.3, -.7, -0.2));
    float NdotL = dot(normalize(input.normal),sun);
    //half lambert 
    NdotL = (NdotL * .5f) + 0.5f;
    return input.colour*NdotL;
}