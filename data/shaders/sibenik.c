#include "hGlobalConstants.h"

#define SUN_DIR (normalize(float3(0.4, -.6, -0.1)))

Texture2D    diffuseTexture;
SamplerState diffuseSampler;

cbuffer MaterialConstants {
    float4 matColour;
    float4 matSpecColour;
    float  matSpecPower;
};

struct VSInput {
	float3 position : POSITION;
	float3 normal	: NORMAL;
    float2 tex0     : TEXCOORD0;
};

struct VSColInput {
	float3 position : POSITION;
	float3 normal	: NORMAL;
};

struct PSInput {
    float4 position : SV_POSITION;
    float3 normal   : TEXCOORD0;
    float2 tex0     : TEXCOORD1;
    float3 worldpos : TEXCOORD2;
};

struct PSColInput {
    float4 position : SV_POSITION;
    float3 normal   : TEXCOORD0;
    float3 worldpos : TEXCOORD1;
};

float3 blinnPhongDiffuse(float3 N, float3 L, float3 C, float3 ambient) {
    float NdotL=saturate(dot(N,L));
    return (C*NdotL)+ambient;
}

float3 blinnPhongSpec(float3 N, float3 L, float3 V, float3 C, float specIntens) {
    float3 H=normalize(L+V);
    float NdotH=saturate(dot(N,H));
    return pow(C*NdotH, specIntens);
}

float3 getCamPos(float4x4 view) {
    return view[3].xyz;
}

float3 getCamDir(float4x4 view) {
    return normalize(view[2].xyz);
}

PSInput mainVP(VSInput input) {
	PSInput output;
	output.position = mul(g_ViewProjection, float4(input.position,1));
    output.worldpos = input.position;
	output.normal   = input.normal;
    output.tex0     = input.tex0;
	return output;
}

float4 mainFP(PSInput input) : SV_TARGET0 {
    float4 diffusecol=diffuseTexture.Sample(diffuseSampler, input.tex0);
    float3 sun = -getCamDir(g_View);
    float3 diff=blinnPhongDiffuse(input.normal, sun, matColour, .2);
    float3 spec=blinnPhongSpec(input.normal, sun, normalize(getCamPos(g_View)-input.worldpos), matSpecColour, matSpecPower);
    return float4(diffusecol.rgb*diff/* + spec*/,diffusecol.a);
}

PSColInput mainColVP(VSColInput input) {
	PSColInput output;
	output.position = mul(g_ViewProjection, float4(input.position,1));
    output.worldpos = input.position;
	output.normal   = input.normal;
	return output;
}

float4 mainColFP(PSColInput input) : SV_TARGET0 {
    float3 sun = -getCamDir(g_View);
    float3 diff=blinnPhongDiffuse(input.normal, sun, matColour, .2);
    float3 spec=blinnPhongSpec(input.normal, sun, normalize(getCamPos(g_View)-input.worldpos), matSpecColour, matSpecPower);
    return float4(diff/* + spec*/,1);
}