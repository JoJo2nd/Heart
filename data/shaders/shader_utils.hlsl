#ifndef GLOBAL_UTILS_CGH_
#define GLOBAL_UTILS_CGH_

#define PI  (3.1415926535)
#define PI2 (PI*2)
#define PI8 (PI*8)
#define COS_45 (0.7071067811865)

//Lambert Azimuthal Equal-Area projection to encode/decode normals
float2 laeq_NormalEncode(float3 n)
{
    n=normalize(n);
    float f = sqrt(8*-n.z+8);
    return n.xy / f + 0.5;
}

float3 laeq_NormalDecode(float4 enc)
{
    float2 fenc = (enc*4-2).xy;
    float f = dot(fenc,fenc);
    float g = sqrt(1-f/4);
    float3 n;
    n.xy = fenc*g;
    n.z = -(1-f/2);
    return n;
}
 /*
    NOTE: roughness is modelled so 1 is very rough, 0 is super smooth 
    Blinn-Phong Normal Distribution Function
 */
float blinnPhongNDF(float r/*roughness*/, float NdotH/*dot(normal, halfvec)*/) {
    return ((r+2)/(PI2+1))*pow(NdotH,r);
}
float blinnPhongNDF(float r/*roughness*/, float3 n/*normal*/, float3 h/*halfvec*/) {
    float NdotH=dot(n, h);
    return blinnPhongNDF(r, NdotH);
}

/*
    NOTE: roughness is modelled so 1 is very rough, 0 is super smooth 
    GGX Normal Distribution Function aka Trowbridge and Reitz
*/
float ggxNDF(float r/*roughness*/, float NdotH) {
    float a=pow(r,4);
    float NdotH2=pow(NdotH, 2);
    float d=NdotH2*(a-1)+1;
    return a/(PI*pow(d, 2));
    //return a/(PI2*pow(d, 2));
}
float ggxNDF(float r/*roughness*/, float3 n/*normal*/, float3 h/*halfvec*/) {
    float NdotH=dot(n, h);
    return ggxNDF(r, NdotH);
}

/*
    Geometric Attenuation Term: Schlick
*/
float schlickG1(float NdotV, float k) {
    return NdotV/(NdotV*(1-k)+k);
}
float schlickG(float r/*roughness*/, float3 n/*normal*/, float3 l/*light*/, float3 v/*view/eye*/) {
    float k = pow(r+1,2)/8;// for runtime
    //float k=r/2;// for image based lighting generation?
    return schlickG1(saturate(dot(n,l)),k)*schlickG1(saturate(dot(n,v)),k);
}


/*
    Specular Fresnel: Schlick approx.
*/
float schlickF(float spec/*F0*/, float EdotH, float NdotH) {
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
    float f = schlick*((spec+2)/PI8)*pow(NdotH,spec);
    return f;
}

/*
    Unreal method - Spherical Gaussian Approximation - A little cheaper?
*/
float schlickF_SGA(float spec/*F0*/, float VdotH) {
    float r0 = pow(2, (-5.55473*VdotH-6.98316)*VdotH);
    float f = spec+(1-spec)*r0;
    return f;
}

/*
    light falloff
*/
float lightFalloff(float dist, float radius) {
    //float n=pow(saturate(1-pow(dist/radius, 4)),2);
    float d=(pow(dist,2)+1);
    return radius/d;
}

/*
    simple Blinn-Phong style lighting
*/
float simpleSpec(float NdotH, float NdotL) {
    return pow(NdotH, 32)*NdotL;
}

#endif // GLOBAL_UTILS_CGH_