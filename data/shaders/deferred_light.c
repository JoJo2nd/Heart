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
    uint quadLightCount;
};

struct SkyLight {
    float4 direction;
    float4 colour;
};

struct QuadLight {
    float4 points[4];
    float4 centre;
    float4 halfv[2];
    float4 colour;
};

StructuredBuffer<SkyLight> direction_lighting;
StructuredBuffer<QuadLight> quad_lighting;

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

float3 intersectLinePlane(float3 linea, float3 lineb, float3 n, float d) {
    float3 l=lineb-linea;
    float t=(d-dot(n,linea))/dot(n,l);
    return linea+(t*l);
}

float3 closestPointOnOBB(in float4 halfv[2], in float3 centre, in float3 pt) {
    float3 d=pt-centre;
    float3 q=centre;
    [unroll] for (uint i=0; i<2; ++i) {
        float halfwidth=length(halfv[i]);
        float3 halfnrm=normalize(halfv[i]);
        float dst=dot(d, halfnrm);
        //
        if (dst > halfwidth) dst = halfwidth;
        if (dst < -halfwidth) dst = -halfwidth;
        
        q += dst*halfnrm;
    }
    
    return q;
}

void getQuadClippedPoints(in float3 worldpos, in float3 normal, in float4 points[4], out float3 clipped[5], out uint pointcount) {
    float d=dot(normal, worldpos);
    uint written=0;
    uint previ=3;
    float4 normald=float4(normal.xyz, -d);
    int pdist[4] = {
        sign(dot(normald,float4(points[0].xyz,1))),
        sign(dot(normald,float4(points[1].xyz,1))),
        sign(dot(normald,float4(points[2].xyz,1))),
        sign(dot(normald,float4(points[3].xyz,1))),
    };
    pointcount=0;
    previ=3;
    [unroll] for (uint i=0; i<4; previ=i, ++i) {
        if (pdist[previ] < 0 && pdist[i] > 0) {//prev b/h, next i/f
            clipped[pointcount++]=intersectLinePlane(points[previ].xyz, points[i].xyz, normal, d);
            clipped[pointcount++].xyz=points[i].xyz;
        } else if (pdist[previ] > 0 && pdist[i] > 0) {//both i/f
            clipped[pointcount++]=points[i];
        } else if (pdist[previ] > 0 && pdist[i] < 0) {//prev i/f, next b/h
            clipped[pointcount++].xyz=intersectLinePlane(points[previ].xyz, points[i].xyz, normal, d);
        }
    }
}

float3 getLightVector(in float3 worldpos, in float3 normal, in float3 vtx[5], in uint vtxcount) {
    uint previ=vtxcount-1;
    float3 lv=0;
    [loop]for(uint i=0; i<vtxcount; previ=i, ++i) {
        float omega=acos(dot(normalize(vtx[previ]-worldpos), normalize(vtx[i]-worldpos)));
        float3 gamma=normalize(cross(vtx[previ]-worldpos, vtx[i]-worldpos));
        lv+=omega*gamma; 
    }
    return normalize(lv);
}

float3 brdf(in float3 dir, in float3 eye, in float3 normal, in float3 albedo, in float spec) {
    float3 halfvec=normalize(dir+eye);
    eye=normalize(eye);
    float NdotL=saturate(dot(normal, dir));
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
    float specular = schlick*((spec+2)/PI8)*pow(NdotH,spec);
    
    return albedo;// + specular;
}

void pixelMain(in VSOutput input, out float4 outputcolour : SV_TARGET0) {
    outputcolour=float4(0,0,0,0);
    float3 albedo=gbuffer_albedo.Sample(tex_sampler, input.tex0).rgb;
    float3 normal=gbuffer_normal.Sample(tex_sampler, input.tex0).rgb;
    float4 spec=gbuffer_spec.Sample(tex_sampler, input.tex0);
    float  depth=gbuffer_depth.Sample(tex_sampler, input.tex0).r;
    float4 viewspacePos=float4(float2(input.tex0.x,1-input.tex0.y)*2-float2(1,1), depth, 1);
    viewspacePos=mul(inverseProjectMtx, viewspacePos);
    viewspacePos/=viewspacePos.w;//inverse perspective devide
    viewspacePos.w=1;
    float3 worldspacePos=mul(inverseViewMtx, viewspacePos).xyz; 
    float3 eye=eyePos.xyz-worldspacePos;
#ifndef WORLD_SPACE_NORMALS
    normal.z=-sqrt(1-normal.x*normal.x-normal.y*normal.y);
#endif
    normal=normalize(normal);
    uint i;
    
    [loop] for (i=0; i<directionLightCount; ++i) {
        float3 dir=-direction_lighting[i].direction.xyz;
#ifndef WORLD_SPACE_NORMALS
        dir=mul(viewMtx, dir);
#endif
#if 0
        float3 halfvec=normalize(dir+eye);
        float NdotL=saturate(dot(normal, dir));
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
        outputcolour.rgb += albedo + specular;
#endif
        outputcolour.rgb += brdf(dir, eye, normal, albedo, spec.a);
    }
    
    //float4 worldspacePlane=float4(normal, -dot(normal, worldspacePos));
    float d=dot(normal, worldspacePos);
    [loop] for (i=0; i<quadLightCount; ++i) {
        QuadLight quad=quad_lighting[i];
        //do obb test
        float r=abs(dot(normal, quad.halfv[0].xyz)) + abs(dot(normal, quad.halfv[1].xyz));
        float obbdist=dot(float4(normal.xyz,-d), float4(quad.centre.xyz,1));
        if (obbdist < -r) {
            continue; //quad behind plane
        }
        
        float3 clipped[5];
        
        uint clippointcount;
        if (abs(obbdist) <= r) {
            //intersect intersecting normal plane
            getQuadClippedPoints(worldspacePos, normal, quad.points, clipped, clippointcount);
        } else {
            //not intersecting normal plane
            clippointcount=4;
            clipped[0].xyz=quad.points[0].xyz;
            clipped[1].xyz=quad.points[1].xyz;
            clipped[2].xyz=quad.points[2].xyz;
            clipped[3].xyz=quad.points[3].xyz;
        }
        float3 lightVec=getLightVector(worldspacePos, normal, clipped, clippointcount);
        float centredist=saturate(1/pow(distance(worldspacePos,closestPointOnOBB(quad.halfv, quad.centre.xyz, worldspacePos))/100,2));
        outputcolour.rgb += centredist*brdf(lightVec, eye, normal, albedo, spec.a);
    }
    
    //outputcolour.rgb = frac(length(worldspacePos).xxx/100);
    //outputcolour.rgb = frac(length(viewspacePos).xxx/100);
    outputcolour.a=1;
}
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////