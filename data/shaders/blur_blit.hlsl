
#include "hGlobalConstants.hlsl"

// R11G11B10 texture containing the filtered image
Texture2D<uint>		texColorInput;

//-----------------------------------------------------------------------------
// Structures
//-----------------------------------------------------------------------------
struct VSInput                                                
{                                                             
    float3 position : POSITION;                               
    float2 uv 		: TEXCOORD0;                              
};                                                            

struct QuadVS_Output
{
    float4 position : SV_POSITION;              
    float2 texcoord : TEXCOORD;
};

QuadVS_Output mainVP( VSInput input )                               
{                                                             
    QuadVS_Output output;                                           
    float4 pos = float4(input.position.xyz,1);                
    output.position = mul(mul(g_ViewProjection,g_World), pos);
    output.texcoord = input.uv*float2(1,0.5);                                     
    return output;                                            
}                                                                                                                   

//-----------------------------------------------------------------------------
// Name: BlitColor_PS
// Type: Pixel shader                                      
// Desc: 
//-----------------------------------------------------------------------------
float4 BlitColor_PS(QuadVS_Output In) : SV_TARGET
{
	uint2 tex_dim;
	texColorInput.GetDimensions(tex_dim.x, tex_dim.y);

	uint2 location = uint2((uint)(tex_dim.x * In.texcoord.x), (uint)(tex_dim.y * In.texcoord.y));
	uint int_color = texColorInput[location];
#ifdef USE_R11G11B10
	// Convert R11G11B10 to float3
	float4 color;
	color.r = (float)(int_color >> 21) / 2047.0f;
	color.g = (float)((int_color >> 10) & 0x7ff) / 2047.0f;
	color.b = (float)(int_color & 0x0003ff) / 1023.0f;
	color.a = 1;
#else
	float4 color;
	color.r = (float)((int_color >> 16) & 0xff) / 255.0f;
	color.g = (float)((int_color >>  8) & 0xff) / 255.0f;
	color.b = (float)(int_color & 0x0000ff)     / 255.0f;
	color.a = 1;
#endif

	return color;
}