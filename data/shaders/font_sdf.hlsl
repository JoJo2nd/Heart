/*

% Description of my shader.
% Second line of description for my shader.

keywords: material classic

date: YYMMDD

*/

#include "hGlobalConstants.hlsl"

cbuffer FontParams
{
    float4 fontColour;
    float4 dropOffset;
}

Texture2D   SignedDistanceField;

SamplerState sdfSampler = sampler_state 
{
//    MagFilter = Point;
//    WrapS = Repeat;
//    WrapT = Repeat;
}; 

struct VSInput
{
	float3 position : POSITION;
	float4 colour 	: COLOR0;
	float2 uv 		: TEXCOORD0;
};

struct PSInput
{
	float4 position : SV_POSITION;
	float4 colour 	: COLOR0;
	float2 uv 		: TEXCOORD0;
};

PSInput mainVP( VSInput input )
{
	PSInput output;
    float4 pos = float4(input.position.xyz,1);
    pos.xyz += dropOffset.xyz;
	output.position = mul(mul(g_ViewProjection,g_World), pos);
	output.colour = input.colour;	
	output.uv = input.uv;
	return output;
}

float4 mainFP( PSInput input ) : SV_TARGET0 
{
float smoothwidth = 0.5;
    float sdf = SignedDistanceField.Sample(sdfSampler, input.uv).a;
    //float pw = fwidth(sdf)*smoothwidth;
    float pw = max(fwidth(sdf)*smoothwidth,0.025);
    //float pw = abs(max(ddx(sdf),ddy(sdf)));
    
    float d = smoothstep(.5-pw,.5+pw,sdf);
    //float d = step(.5-pw, sdf)-step(.5+pw, sdf);
    //float d = smoothstep(0, 1, 1-saturate(abs(sdf-.5)/pw));

    return float4(fontColour.rgb,d);
}