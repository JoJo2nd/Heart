/*

% Description of my shader.
% Second line of description for my shader.

keywords: material classic

date: YYMMDD

*/

#include "hGlobalConstants.hlsl"

struct VSInput
{
    float3 position : POSITION;
	float4 colour   : COLOR0;
};

struct PSInput
{
	float4 colour   : COLOR0;
};

PSInput mainVP( VSInput input, out float4 position : SV_POSITION )
{
	PSInput output = (PSInput)0;
	output.colour = input.colour;	
    
    position = mul( mul(g_ViewProjection,g_World), float4(input.position.xyz,1) );
    
	return output;
}

float4 mainFP( PSInput input ) : SV_Target0 
{
	return input.colour;
}