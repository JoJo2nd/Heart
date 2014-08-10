
#include "hGlobalConstants.hlsl"


struct VSInput {
    float3 position var_semantic(POSITION);
    float4 colour   var_semantic(COLOR0);
};

struct PSInput {
    float4 out_position var_semantic(SV_POSITION);
    float4 out_colour  var_semantic(COLOR0);
};

PSInput mainVP(VSInput input_vec)
{
	PSInput output_vec;
	output_vec.out_colour = input_vec.colour;
    
    output_vec.out_position = float4(input_vec.position.xyz,1);
    output_vec.out_position.xy *= g_viewportSize.xy;
    output_vec.out_position = mul( mul(g_ViewProjection,g_World), output_vec.out_position );
    
	return output_vec;
}

float4 mainFP( PSInput input_pix ) var_semantic(SV_Target0) 
{
	return input_pix.out_colour;   
}

#if HEART_IS_GLSL
#   if HEART_COMPILE_VERTEX_PROG
    layout(location=0) in float3 in_position;
    layout(location=1) in float4 in_colour;
    out float4 inout_colour;
    void main() {
        VSInput p;
        p.position = in_position;
        p.colour = in_colour;
        PSInput o = mainVP(p);
        inout_colour = o.out_colour;
        gl_Position = o.out_position;
    }
#   elif HEART_COMPILE_FRAGMENT_PROG
    in float4 inout_colour;
    out float4 fragColour;
    void main() {
        PSInput i;
        i.out_colour = inout_colour;
        fragColour = mainFP(i);
    }
#   endif
#endif
