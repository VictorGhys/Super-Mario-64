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


//PIXEL SHADER
//------------
float4 PS(PS_INPUT input): SV_Target
{
	// Step 1: find the dimensions of the texture (the texture has a method for that)
	float sceneWidth;
	float sceneHeight;
	gTexture.GetDimensions(sceneWidth, sceneHeight);
	// Step 2: calculate dx and dy (UV space for 1 pixel)
	float dx = 1 / sceneWidth;//du
	float dy = 1 / sceneHeight;//dv
	// Step 3: Create a double for loop (5 iterations each)
	float2 offset;
	float4 finalColor;
	for(int x = -2; x <= 2; x++)
	{
		for(int y = -2; y <= 2; y++)
		{
	//		Inside the loop, calculate the offset in each direction. Make sure not to take every pixel but move by 2 pixels each time
			offset.x = x*2*dx;
			offset.y = y*2*dy;
	//		Do a texture lookup using your previously calculated uv coordinates + the offset, and add to the final color
			finalColor += gTexture.Sample(samPoint, input.TexCoord + offset);
		}
	}
	// Step 4: Divide the final color by the number of passes (in this case 5*5)	
	finalColor.r /= 5*5;
	finalColor.g /= 5*5;
	finalColor.b /= 5*5;
	finalColor.a /= 5*5;
	// Step 5: return the final color

	return finalColor;
}


//TECHNIQUE
//---------
technique11 Blur
{
    pass P0
    {
		// Set states...
        SetVertexShader( CompileShader( vs_4_0, VS() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PS() ) );
    }
}