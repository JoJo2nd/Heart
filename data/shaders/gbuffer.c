#include "hGlobalConstants.h"

Texture2D albedoTexture;
SamplerState albedoSampler;
Texture2D normalTexture;
SamplerState normalSampler;
Texture2D specTexture;
SamplerState specSampler;

#define WORLD_SPACE_NORMALS
//#ifndef NO_NORMAL_MAP
//#   define NO_NORMAL_MAP
//#endif

struct VSInput {
    float3 position : POSITION;
    float3 normal   : NORMAL;
#ifndef NO_NORMAL_MAP    
    float3 bitangent : BITANGENT;
    float3 tangent  : TANGENT;
#endif
    float2 tex0     : TEXCOORD0;
};

struct VSOutput {
    float4 position : SV_POSITION;
    float3 normal   : TEXCOORD0;
#ifndef NO_NORMAL_MAP    
    float3 bitangent : TEXCOORD1;
    float3 tangent  : TEXCOORD2;
#endif
    float2 tex0     : TEXCOORD3;
};

void vertexMain(in VSInput input, out VSOutput output) {
    //float4 world = mul(g_World, float4(input.position.xyz,1));
    float4x4 worldView=g_View;
    float4 world = float4(input.position.xyz,1);
    output.position = mul(g_ViewProjection, world);

#ifdef WORLD_SPACE_NORMALS
    output.normal = input.normal;
#   ifndef NO_NORMAL_MAP    
    output.bitangent = input.bitangent;
    output.tangent = input.tangent;
#   endif
#else
    output.normal = mul(worldView, input.normal);
#   ifndef NO_NORMAL_MAP    
    output.bitangent = mul(worldView, input.bitangent);
    output.tangent = mul(worldView, input.tangent);
#   endif
#endif
    output.tex0 = input.tex0;
}

void pixelMain(in VSOutput input, 
    out float4 albedo : SV_TARGET0, 
    out float4 viewnormal : SV_TARGET1,
    out float4 spec : SV_TARGET2) {
#ifndef NO_NORMAL_MAP
    float3x3 tangentMtx=float3x3(
        normalize(float3(input.tangent.x, input.tangent.y, input.tangent.z)),
        normalize(float3(input.bitangent.x, input.bitangent.y, input.bitangent.z)),
        normalize(float3(input.normal.x, input.normal.y, input.normal.z)));
#endif
    //float4 albedo;
    //float4 viewnormal;
    //float4 spec;
    
    albedo = albedoTexture.Sample(albedoSampler, input.tex0);
    /*
        In the normal map DXT compression from the nvtt the colour is encoded
        like so...R=1, G=Y, B=0, A=X, 
        Spliting the X into alpha avoids compression on the X component and frees
        the palette abit for the Y. Reconstruct the normal by
        X=red*alpha
        Y=green
        Z=sqrt(1 - X^2 - Y^2)
    */
#ifndef NO_NORMAL_MAP
    float4 normal = 2*normalTexture.Sample(normalSampler, input.tex0)-1;
    normal.x = normal.r*normal.a;
    normal.y = normal.g;
    normal.z = sqrt(1-normal.x*normal.x-normal.y*normal.y);
    /*
        do inverse! (i.e. swap matrix mul order) because normal is in tangent space
        tangentMtx vectors are in view space ??
    */
    viewnormal.rgb=normalize(mul(normal, tangentMtx));
#else
    viewnormal.rgb=normalize(input.normal.xyz);
#endif
    viewnormal.a=0; 
    
    spec = specTexture.Sample(specSampler, input.tex0);
    spec.a=length(spec)*2+0.1;
    spec.rgb=float3(0.04, 0.04, 0.04);
}
