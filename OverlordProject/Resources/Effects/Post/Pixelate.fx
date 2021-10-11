//=============================================================================
//// Shader uses position and texture
//=============================================================================
SamplerState samPoint
{
	Filter = MIN_MAG_MIP_POINT;
	AddressU = Mirror;
	AddressV = Mirror;
};

Texture2D gTexture;
float2 gPixels = float2(100, 100);

/// Create Depth Stencil State (ENABLE DEPTH WRITING)
DepthStencilState NoDepth
{
	DepthEnable = TRUE;
};
/// Create Rasterizer State (Backface culling)
RasterizerState BackCulling
{
	CullMode = BACK;
};

//IN/OUT STRUCTS
//--------------
struct VS_INPUT
{
	float3 Position : POSITION;
	float2 TexCoord : TEXCOORD0;
};

struct PS_INPUT
{
	float4 Position : SV_POSITION;
	float2 TexCoord : TEXCOORD1;
};

//VERTEX SHADER
//-------------
PS_INPUT VS(VS_INPUT input)
{
	PS_INPUT output = (PS_INPUT)0;
	// Set the Position
	output.Position = float4(input.Position, 1.0f);
	// Set the TexCoord
	output.TexCoord = input.TexCoord;
	return output;
}

float2 pixelate(float2 texCoord)
{
	float newu = floor(texCoord.r * gPixels.r) / gPixels.r;
	float newv = floor(texCoord.g * gPixels.g) / gPixels.g;
	return float2(newu, newv);
}

//PIXEL SHADER
//------------
float4 PS(PS_INPUT input) : SV_Target
{
	// Step 1: sample the texture
	float4 col = gTexture.Sample(samPoint, pixelate(input.TexCoord));

	return col;
}

//TECHNIQUE
//---------
technique11 Grayscale
{
	pass P0
	{
		// Set states...
		SetVertexShader(CompileShader(vs_4_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_4_0, PS()));
	}
}