#include "hGlobalConstants.h"
#include "shader_utils.h"

SamplerState tex_sampler;
Texture2D gbuffer_albedo;
Texture2D gbuffer_normal;
Texture2D gbuffer_spec;
Texture2D gbuffer_depth;

//#define WORLD_SPACE_NORMALS

#define SPEC_POWER (256) // How should this come from the material?
#define SPEC_INTENSITY (0.02)

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
    uint sphereLightCount;
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

struct SphereLight {
    float4 centreRadius;
    float4 colour;
};

StructuredBuffer<SkyLight> direction_lighting;
StructuredBuffer<QuadLight> quad_lighting;
StructuredBuffer<SphereLight> sphere_lighting;

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

float3 intersectLinePlane(float3 a, float3 b, float3 n, float d, float3 delta) {
    float3 l=b-a;
    float t=(d-dot(n,a))/dot(n,l);
    return a+(t*l);
}
/*
float3 intersectLinePlane(float3 linea, float3 lineb, float3 n, float d, float3 delta) {
    float3 l=lineb-linea;
    float t=(dot(float4(n,-d),linea))/dot(float4(n,-d),l);
    return lineb-(t*delta);
}
*/
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

void getQuadClippedPoints(in float3 worldpos, in float3 normal, in float4 points[4], in float3 quadcentre, out float3 clipped[5], out uint pointcount) {
    float d=dot(normal, worldpos);
    float4 normald=float4(normal.xyz, -d);

    uint written=0;
    uint previ=3;
    int pdist[4] = {
        sign(dot(normald,float4(points[0].xyz,1))),
        sign(dot(normald,float4(points[1].xyz,1))),
        sign(dot(normald,float4(points[2].xyz,1))),
        sign(dot(normald,float4(points[3].xyz,1))),
    };
    pointcount=0;
    previ=3;
    float3 t,p;
    float3 delta=quadcentre-worldpos;
    [loop] for (uint i=0; i<4; previ=i, ++i) {
        if (pdist[previ] < 0 && pdist[i] > 0) {//prev b/h, next i/f
            t=intersectLinePlane(points[previ].xyz, points[i].xyz, normal, d, delta);
            p=points[i].xyz;
            clipped[pointcount++]=t;
            clipped[pointcount++].xyz=p;
        } else if (pdist[previ] > 0 && pdist[i] > 0) {//both i/f
            p=points[i];
            clipped[pointcount++]=p;
        } else if (pdist[previ] > 0 && pdist[i] < 0) {//prev i/f, next b/h
            t=intersectLinePlane(points[previ].xyz, points[i].xyz, normal, d, delta);
            clipped[pointcount++].xyz=t;
        }
    }
}

float3 getLightVector(in float3 worldpos, in float3 normal, in float3 vtx[5], in uint vtxcount) {
    uint previ=vtxcount-1;
    float3 lv=0;
    [loop]for(uint i=0; i<vtxcount; previ=i, ++i) {
        float d=dot(normalize(vtx[previ]-worldpos), normalize(vtx[i]-worldpos));
        float omega=acos(d);
        float3 gamma=normalize(cross(vtx[previ]-worldpos, vtx[i]-worldpos));
        lv+=omega*gamma; 
    }
    return normalize(-lv);
}

float3 brdf(in float3 dir, in float3 eye, in float3 normal, in float3 albedo, in float spec) {
    eye=normalize(eye);
    float3 halfvec=normalize(eye+dir);
    float NdotL=saturate(dot(normal, dir));
    float NdotH=saturate(dot(normal, halfvec));
    float NdotE=saturate(dot(normal, eye));
    float EdotH=saturate(dot(eye, halfvec));
    
    //energy conserving Blinn (based on http://www.fxguide.com/featured/game-environments-parta-remember-me-rendering/)
    albedo = (albedo*(NdotL/PI));
    
    float d=ggxNDF(spec, NdotH);
    float g=schlickG(spec, normal, dir, eye);
    //float f=schlickF(0.2, EdotH, NdotH);
    float f=schlickF_SGA(0.2, EdotH);
    float specular=(d*f*g)/(4*NdotL*NdotE +1);
    //specular=simpleSpec(NdotH, NdotL);
    return albedo + specular;
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
    //float3 eye=eyePos.xyz-worldspacePos;
    float3 eye=-viewspacePos;
#ifndef WORLD_SPACE_NORMALS
    normal=laeq_NormalDecode(float4(normal.rgb,1));
#endif
#ifdef OUTPUT_NORMALS_AS_COLOUR
    outputcolour = float4(normal.rgb, 1);
    return;
#endif
    uint i;
    
    [loop] for (i=0; i<directionLightCount; ++i) {
        float3 dir=-direction_lighting[i].direction.xyz;
#ifndef WORLD_SPACE_NORMALS
        dir=mul(dir.xyz, (float3x3)inverseViewMtx).xyz; //so mul by inverseViewTranspose
#endif
        outputcolour.rgb += brdf(dir, eye, normal, albedo, spec.a);
    }

//#define SPHERE_AS_POINT
    [loop] for (i=0; i<sphereLightCount; ++i) {    
#ifdef SPHERE_AS_POINT
        float3 viewspaceCentre=mul(viewMtx, float4(sphere_lighting[i].centreRadius.xyz,1)).xyz;
        float3 L=viewspaceCentre-viewspacePos;//pixel to light
        float dist=length(viewspaceCentre-viewspacePos);
        float falloff = lightFalloff(dist, 50*400);
        outputcolour.rgb += falloff*brdf(normalize(L), eye, normal, albedo, spec.a);
#else
        float radius=sphere_lighting[i].centreRadius.w;
        float lumens=sphere_lighting[i].colour.w;
        float3 viewspaceCentre=mul(viewMtx, float4(sphere_lighting[i].centreRadius.xyz,1)).xyz;
        float3 L=viewspaceCentre-viewspacePos;//pixel to light
        float3 r=normalize(reflect(L, normal));
        float3 centreToRay=L-dot(L,r)*r;
        float3 closestPoint=L+centreToRay*saturate(radius/length(centreToRay));
        float3 lightVec=normalize(closestPoint);
        float dist=length(viewspaceCentre-viewspacePos);
        float falloff = lightFalloff(dist, radius*400);
        //float specalpha=saturate(pow(spec.a,2)+radius/3*dist);
        //specalpha=pow(specalpha/specalpha,2);
        float specalpha=spec.a;
        outputcolour.rgb += falloff*brdf(lightVec, eye, normal, albedo, specalpha);
#endif
    }
    
#ifdef ALLOW_QUAD_LIGHTS
    //float4 worldspacePlane=float4(normal, -dot(normal, worldspacePos));
    float d=dot(normal, worldspacePos);
    [loop] for (i=0; i<quadLightCount; ++i) {
        QuadLight quad=quad_lighting[i];
        //do obb test
        float3 obbnormal=cross(normalize(quad.halfv[0].xyz), normalize(quad.halfv[1].xyz));
        float obbd=dot(obbnormal, quad.centre.xyz);
        float r=abs(dot(normal, quad.halfv[0].xyz)) + abs(dot(normal, quad.halfv[1].xyz));
        float obbdist=dot(float4(normal.xyz,-d), float4(quad.centre.xyz,1));
        float planedist=dot(float4(obbnormal.xyz,-obbd), float4(worldspacePos.xyz,1));
        if (obbdist <= -r) {
            continue; //quad behind plane
        }
        
        float3 clipped[5];
        uint clippointcount;
        float3 obbpos=closestPointOnOBB(quad.halfv, quad.centre.xyz, worldspacePos);
        float factor=1;
        if (abs(obbdist) <= r) {
            //intersect intersecting normal plane
            getQuadClippedPoints(worldspacePos, normal, quad.points, quad.centre, clipped, clippointcount);
            factor = 0;
        } else {
            //not intersecting normal plane
            clippointcount=4;
            clipped[0].xyz=quad.points[0].xyz;
            clipped[1].xyz=quad.points[1].xyz;
            clipped[2].xyz=quad.points[2].xyz;
            clipped[3].xyz=quad.points[3].xyz;
        }
        float3 lightVec=getLightVector(worldspacePos, normal, clipped, clippointcount);
        float centredist=1/pow(distance(worldspacePos, obbpos),2);
        float fudgefactor=dot(normalize(obbpos-worldspacePos), normal);
        outputcolour.rgb += /*fudgefactor*/500*centredist*brdf(lightVec, eye, normal, albedo, spec.a);
        //outputcolour.gb *= factor;
        /*float vvv=brdf(lightVec, eye, normal, albedo, spec.a)*factor;
        if (clippointcount == 5)
            outputcolour.r += vvv;
        else if (clippointcount == 4)
            outputcolour.g += vvv;
        else if (clippointcount == 3)
            outputcolour.b += vvv;
        else 
            outputcolour.rb += vvv;*/
    }
#endif

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