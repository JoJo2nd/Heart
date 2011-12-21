
//// UN-TWEAKABLES - AUTOMATICALLY-TRACKED TRANSFORMS ////////////////

float4x4 WorldITXf : WORLDINVERSETRANSPOSE < string UIWidget="None"; >;
float4x4 WvpXf : WORLDVIEWPROJECTION < string UIWidget="None"; >;
float4x4 WorldXf : WORLD < string UIWidget="None"; >;
float4x4 ViewIXf : VIEWINVERSE < string UIWidget="None"; >;


//////// COLOR & TEXTURE /////////////////////

texture ColorTexture : DIFFUSE0 <
    string ResourceName = "default_color.dds";
    string UIName =  "Diffuse Texture";
    string ResourceType = "2D";
>;

sampler2D ColorSampler = sampler_state {
    Texture = <ColorTexture>;
    FILTER = MIN_MAG_MIP_LINEAR;
    AddressU = Wrap;
    AddressV = Wrap;
};  

// shared shadow mapping supported in Cg version

//////// CONNECTOR DATA STRUCTURES ///////////

/* data from application vertex buffer */
struct VSPosNormalUV 
{
    float4 Position	: POSITION;
    float3 Normal	: NORMAL;
    float2 UV		: TEXCOORD0;
};

struct VSPosNormal
{
	float4 Position : POSITION;
	float3 Normal	: NORMAL;
};

struct VSPos
{
	float4 Position : POSITION;
};

struct VSLine
{
	float4 Position : POSITION;
	float4 Colour	: COLOR0;
};

/* data passed from vertex shader to pixel shader */
struct PSPosNormalUV
{
    float4 Position		: POSITION;
    float3 Normal		: TEXCOORD0;
    float2 UV			: TEXCOORD1;
};

struct PSPosNormal
{
	float4 Position		: POSITION;
	float3 Normal		: TEXCOORD0;
};

struct PSPos
{
	float4 Position		: POSITION;
};
 
///////// VERTEX SHADING /////////////////////

/*********** Generic Vertex Shader ******/

PSPosNormalUV PosNormalUV_VS(VSPosNormalUV IN) 
{    
	float3 Pw = mul(IN.Position,WorldXf).xyz;
	
    PSPosNormalUV OUT = (PSPosNormalUV)0;
    OUT.UV = float2( IN.UV.x, 1.0 - IN.UV.y );
    OUT.Normal = mul( IN.Normal, WorldXf );
    OUT.Position = mul(IN.Position,WvpXf);
	
    return OUT;
}

PSPosNormal PosNormal_VS(VSPosNormal IN) 
{    
	float3 Pw = mul(IN.Position,WorldXf).xyz;
	
    PSPosNormal OUT = (PSPosNormal)0;
    OUT.Normal = mul( IN.Normal, WorldXf );
    OUT.Position = mul(IN.Position,WvpXf);
	
    return OUT;
}

PSPos Pos_VS(VSPos IN) 
{    
	float3 Pw = mul(IN.Position,WorldXf).xyz;
	
    PSPos OUT = (PSPos)0;
    OUT.Position = mul(IN.Position,WvpXf);
	
    return OUT;
}

VSLine Line_VS(VSLine IN) 
{    	
    VSLine OUT = (VSLine)0;
    OUT.Position = mul(IN.Position,WvpXf);
    OUT.Colour = IN.Colour;
	
    return OUT;
}


///////// PIXEL SHADING //////////////////////

float4 PosNormalUV_PS( PSPosNormalUV IN ) : COLOR 
{
    //float3 Ln = normalize(IN.LightVec);
    //float3 Nn = normalize(IN.WorldNormal);
    //float ldn = dot(Ln,Nn);
    //ldn = max(ldn,0.0);
    float3 diffuseColor = tex2D(ColorSampler,IN.UV).rgb;
    float3 result = diffuseColor;// * (ldn * Lamp0Color + AmbiColor);
    // return as float4
    return float4(result,1);
}

float4 PosNormal_PS( PSPosNormal IN ) : COLOR 
{

    float3 diffuseColor = float3( 0.6, 0.6, 0.6 );
    float3 result = diffuseColor;// * (ldn * Lamp0Color + AmbiColor);
    // return as float4
    return float4(result,1);
}

float4 Pos_PS( PSPos IN ) : COLOR 
{
    float3 diffuseColor = float3( 0.6, 0.6, 0.6 );
    float3 result = diffuseColor;// * (ldn * Lamp0Color + AmbiColor);
    return float4(result,1);
}

float4 Line_PS( VSLine IN ) : COLOR
{
	return IN.Colour;
}

///// TECHNIQUES /////////////////////////////

technique MainPosNormalUV
{
    pass p0
    {
        VertexShader = compile vs_2_0 PosNormalUV_VS();
		ZEnable = true;
		ZWriteEnable = true;
		ZFunc = LessEqual;
		AlphaBlendEnable = false;
		CullMode = None;
        PixelShader = compile ps_2_a PosNormalUV_PS();
    }
}

technique MainPosNormal
{
    pass p0
    {
        VertexShader = compile vs_2_0 PosNormal_VS();
		ZEnable = true;
		ZWriteEnable = true;
		ZFunc = LessEqual;
		AlphaBlendEnable = false;
		CullMode = None;
        PixelShader = compile ps_2_a PosNormal_PS();
    }
}

technique MainPos
{
    pass p0
    {
        VertexShader = compile vs_2_0 Pos_VS();
		ZEnable = true;
		ZWriteEnable = true;
		ZFunc = LessEqual;
		AlphaBlendEnable = false;
		CullMode = None;
        PixelShader = compile ps_2_a Pos_PS();
    }
}

technique MainLine
{
    pass p0
    {
        VertexShader = compile vs_2_0 Line_VS();
		ZEnable = true;
		ZWriteEnable = true;
		ZFunc = LessEqual;
		AlphaBlendEnable = false;
		CullMode = None;
        PixelShader = compile ps_2_a Line_PS();
    }
}

/////////////////////////////////////// eof //
