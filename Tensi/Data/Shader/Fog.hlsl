
//ｸﾞﾛｰﾊﾞﾙ変数.
Texture2D shaderTexture;
SamplerState SampleType;

struct PS_Input
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    unorm float fogFactor : FOG;
};

struct VS_Input
{
	float4 position : POSITION;
	float2 tex : TEXCOORD0;
};

/////////////
// GLOBALS //
/////////////
cbuffer per_frame
{
	matrix g_mW;
	matrix g_mView;
	matrix g_mProj;
};

cbuffer FogBuffer
{
	float fogStart;
	float fogEnd;
	float fog_e;
	float fog_density;
};



////////////////////////////////////////////////////////////////////////////////
// Pixel Shader
////////////////////////////////////////////////////////////////////////////////
float4 PS_Main(PS_Input input) : SV_TARGET
{
    float4 textureColor;
    float4 fogColor;
    float4 finalColor;

	
    // Sample the texture pixel at this location.
    textureColor = shaderTexture.Sample( SampleType, input.tex );
    
    // Set the color of the fog to grey.
    fogColor = float4(0.0f, 0.0f, 0.0f, 1.0f);

    // Calculate the final color using the fog effect equation.
    finalColor = input.fogFactor * textureColor + ( 1.0 - input.fogFactor ) * fogColor;

    return finalColor;
}




//-------------------------------------------------
//	頂点(ﾊﾞｰﾃｯｸｽ)ｼｪｰﾀﾞ.
//-------------------------------------------------
PS_Input VS_Main(VS_Input input)
{
    PS_Input output;
    float4 cameraPosition;
    

	// Change the position vector to be 4 units for proper matrix calculations.
    input.position.w = 1.0f;

	// Calculate the position of the vertex against the world, view, and projection matrices.
    output.position = mul(input.position, g_mW);
    output.position = mul(output.position, g_mView);
    output.position = mul(output.position, g_mProj);
    
	// Store the texture coordinates for the pixel shader.
    output.tex = input.tex;
    
    // Calculate the camera position.
    cameraPosition = mul(input.position, g_mW);
    cameraPosition = mul(cameraPosition, g_mView);

    //フォグファクターを作成.
    //output.fogFactor = saturate(1.0 / pow(fog_e, (cameraPosition.z * fog_density))); //指数フォグ.
	
    output.fogFactor = saturate( ( fogEnd - cameraPosition.z ) / ( fogEnd - fogStart ) ); //線形フォグ.
	
    return output;
}