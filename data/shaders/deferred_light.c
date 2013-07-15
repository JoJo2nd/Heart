#include "hGlobalConstants.h"

SamplerState tex_sampler;
Texture2D gbuffer_albedo;
Texture2D gbuffer_normal;
Texture2D gbuffer_spec;
Texture2D gbuffer_depth;

#define WORLD_SPACE_NORMALS

#define SPEC_POWER (256) // How should this come from the material?
#define SPEC_INTENSITY (0.02)
#define PI (3.1415926535)
#define PI8 (PI*8)

//DEBUG
//#define OUTPUT_NORMALS_AS_COLOUR
//#define OUTPUT_LIGHT_ONLY
//#define OUTPUT_SPEC_ONLY

cbuffer lighting_setup {
    float4x4 viewMtx;
    float4x4 inverseViewMtx;
    float4x4 projectMtx;
    float4x4 inverseProjectMtx;
    float4 eyePos;
    uint directionLightCount;
}

struct SkyLight {
    float4 direction;
    float4 colour;
};

StructuredBuffer<SkyLight> direction_lighting;

struct VSInput {
    float3 position : POSITION;
    float2 tex0     : TEXCOORD0;
};

struct VSOutput {
    float4 position : SV_POSITION;
    float2 tex0     : TEXCOORD0;
};

void vertexMain(in VSInput input, out VSOutput output) {
    output.position = float4(input.position.xyz*2, 1)-float4(1, 1, 0, 0);
    output.tex0 = input.tex0;
}

void pixelMain(in VSOutput input, out float4 outputcolour : SV_TARGET0) {
    outputcolour=float4(0,0,0,0);
    float3 albedo=gbuffer_albedo.Sample(tex_sampler, input.tex0);
    float3 normal=gbuffer_normal.Sample(tex_sampler, input.tex0);
    float4 spec=gbuffer_spec.Sample(tex_sampler, input.tex0);
    float  depth=gbuffer_depth.Sample(tex_sampler, input.tex0);
    float4 viewspacePos=float4(input.tex0.xy, depth, 1);
    viewspacePos=mul(inverseProjectMtx, viewspacePos);
    viewspacePos/=viewspacePos.w;//inverse perspective devide
    viewspacePos.w=1;
    float3 worldspacePos=mul(inverseViewMtx, viewspacePos); 
    float3 eye=eyePos-worldspacePos;
#ifndef WORLD_SPACE_NORMALS
    normal.z=-sqrt(1-normal.x*normal.x-normal.y*normal.y);
#endif
    
    [loop] for (uint i=0; i<directionLightCount; ++i) {
        float3 dir=normalize(-direction_lighting[i].direction);
#ifndef WORLD_SPACE_NORMALS
        dir=mul(viewMtx, dir);
#endif
        float3 halfvec=normalize(dir+eye);
        float NdotL=saturate(dot(normalize(normal), dir));
        float NdotH=saturate(dot(normal, halfvec));
        float EdotH=saturate(dot(eye, halfvec));
        //energy conserving blinn-phong spec
        //float3 speccomp=spec*SPEC_INTENSITY*NdotL*(SPEC_POWER+8/PI8)*(pow(NdotH,SPEC_POWER));
        //outputcolour.rgb += ((albedo*NdotL*direction_lighting[i].colour)/PI)+speccomp;
        
        //energy conserving Blinn (based on http://www.fxguide.com/featured/game-environments-parta-remember-me-rendering/)
        albedo = (albedo*(NdotL/PI));
        //schlick specular R(theta)=R0 + (1-R0)(1-cos(theta))^5
        //where \theta is the angle between the viewing direction and the half-angle direction, 
        //which is halfway between the incident light direction and the viewing direction, 
        //hence cos\theta=(H\cdot V). And n_1,\,n_2 are the indices of refraction of the 
        //two medias at the interface and R_0 is the reflection coefficient for light incoming 
        //parallel to the normal (i.e. the value of the Fresnel term when \theta = 0 or minimal 
        //reflection). In computer graphics, one of the interfaces is usually air, meaning 
        //that n_1 very well can be approximated as 1.
        // ROI of asphalt = 1.635
        float r0 = pow((1-1.635)/(1+1.635), 2);
        float schlick = r0 + (1-r0)*pow(1-EdotH,5);
        float specular = schlick*((spec.a+2)/PI8)*pow(NdotH,spec.a);
        outputcolour.rgb = albedo + specular;
    }
    outputcolour.a=1;
}