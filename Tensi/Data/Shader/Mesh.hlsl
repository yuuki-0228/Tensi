//ｸﾞﾛｰﾊﾞﾙ変数.
//ﾃｸｽﾁｬは ﾚｼﾞｽﾀ t(n).
Texture2D		g_Texture	: register(t0);
//ｻﾝﾌﾟﾗは ﾚｼﾞｽﾀ s(n).
SamplerState	g_SamLinear	: register(s0);

//ｺﾝｽﾀﾝﾄﾊﾞｯﾌｧ.
//ﾒｯｼｭ単位.
cbuffer per_mesh	: register( b0 )
{
	matrix	g_mW;		//ﾜｰﾙﾄﾞ行列.
	matrix	g_mWVP;		//ﾜｰﾙﾄﾞ,ﾋﾞｭｰ,ﾌﾟﾛｼﾞｪｸｼｮﾝの合成行列.
    float4  g_Color;    //色(RGBA:xyzw).
};
//ﾏﾃﾘｱﾙ単位.
cbuffer per_material: register( b1 )
{
	float4	g_Ambient;		// アンビエント色(環境色).
	float4	g_Diffuse;		// ディフューズ色(拡散反射色).
	float4	g_Specular;		// スペキュラ色(鏡面反射色).
	float4	g_UV;			// UV(x,yのみ使用する).
};
//ﾌﾚｰﾑ単位.
cbuffer per_frame	: register( b2 )
{
	float4 g_CameraPos;				//ｶﾒﾗ位置(視点位置).
	float4 g_LightDir;				//ﾗｲﾄの方向ﾍﾞｸﾄﾙ.
	float4 g_LightIntensity;		//ﾗｲﾄの強弱.
	float4 g_vFlag;					//x：ﾃﾞｲｻﾞ抜きを使用するか、	y：ｱﾙﾌｧﾌﾞﾛｯｸを使用するか、z：距離で透明にするか(以下)、w：距離で透明にするか(以上).
	float4 g_AlphaDistance;			//x：開始する距離(以下)、		y：開始する距離(以上)、	z：透明にしていく間隔.
}

//頂点ｼｪｰﾀﾞの出力ﾊﾟﾗﾒｰﾀ.
struct VS_OUTPUT
{
	float4	Pos			: SV_Position;
	float3	Normal		: TEXCOORD0;
	float2	UV			: TEXCOORD1;
	float3	Light		: TEXCOORD2;
	float3	EyeVector	: TEXCOORD3;
	float4	PosWorld	: TEXCOORD4;
	float4	Color		: COLOR0;
};

static const float DITHER_PATTERN[4][4] = 
{
	{ 0.00f, 0.32f, 0.08f, 0.40f, },
	{ 0.48f, 0.16f, 0.56f, 0.24f, },
	{ 0.12f, 0.44f, 0.04f, 0.36f, },
	{ 0.60f, 0.28f, 0.52f, 0.20f, },
};

//-------------------------------------------------
//	頂点(ﾊﾞｰﾃｯｸｽ)ｼｪｰﾀﾞ.
//-------------------------------------------------
VS_OUTPUT VS_Main(
	float4 Pos	: POSITION,
	float4 Norm	: NORMAL,
	float2 UV	: TEXCOORD)
{
	VS_OUTPUT output = (VS_OUTPUT)0;

	//ﾌﾟﾛｼﾞｪｸｼｮﾝ変換(ﾜｰﾙﾄﾞ→ﾋﾞｭｰ→ﾌﾟﾛｼﾞｪｸｼｮﾝ).
	output.Pos = mul( Pos, g_mWVP );

	//法線をﾓﾃﾞﾙの姿勢に合わせる.
	// (ﾓﾃﾞﾙが回転すれば法線も回転させる必要があるため).
    output.Normal = mul( ( float1x3 )Norm, ( float3x3 )g_mW );
    output.Normal = normalize( output.Normal );
    
	//ﾗｲﾄ方向:
	// ﾃﾞｨﾚｸｼｮﾅﾙﾗｲﾄは、どこでも同じ方向.位置は無関係.
	output.Light = (float3)normalize( g_LightDir );

	output.PosWorld = mul( Pos, g_mW );

	//視線ﾍﾞｸﾄﾙ:
	// ﾜｰﾙﾄﾞ空間上での頂点から頂点へ向かうﾍﾞｸﾄﾙ.
	output.EyeVector = (float3)normalize( g_CameraPos - output.PosWorld );

	//ﾃｸｽﾁｬ座標.
	output.UV = UV + g_UV.xy;

	return output;
}

//*************************************************
//	ﾗｽﾀﾗｲｻﾞ(ｱﾌﾟﾘ側で設定する項目。ここでは触れない).
//*************************************************

//-------------------------------------------------
//	ﾋﾟｸｾﾙｼｪｰﾀﾞ.
//-------------------------------------------------
float4 PS_Main( VS_OUTPUT input ) : SV_Target
{
	//ﾃｸｽﾁｬｶﾗｰ.
	float4 texColor = g_Texture.Sample( g_SamLinear, input.UV );

	//環境光　①.
	float4 ambient = texColor * g_Ambient;

	//拡散反射光 ②.
    float NL = saturate( dot( input.Normal, input.Light ) );
	NL = NL * 0.5f + 0.5f;
	NL = NL * NL;
    float4 diffuse = ( g_Diffuse / 2 + texColor / 2 ) * NL;

	//鏡面反射光 ③.
	float3 reflect = normalize( 2 * NL * input.Normal - input.Light );
	float4 specular =
		pow( saturate( dot( reflect, input.EyeVector ) ), 4 )*g_Specular;
	
	//最終色　①②③の合計.
	float4 finalColor = ( ambient + diffuse + specular ) * g_LightIntensity.x;
	finalColor *= g_Color;
	
	// カメラの距離によって透明にしていく.
	if ( g_vFlag.z >= 1.0f ) {
		const float Distance = length( input.Pos - g_CameraPos );
		if ( Distance <= g_AlphaDistance.x ) {
			const float OverDistance = Distance - g_AlphaDistance.x;
			finalColor.a -= OverDistance / g_AlphaDistance.z / 10.0f;
		}
	}
	if ( g_vFlag.w >= 1.0f ) {
		const float Distance = length( input.Pos - g_CameraPos );
		if ( Distance >= g_AlphaDistance.y ) {
			const float OverDistance = Distance - g_AlphaDistance.y;
			finalColor.a -= OverDistance / g_AlphaDistance.z / 10.0f;
		}
	}
	
	// アルファブロック計算.
	if ( g_vFlag.y >= 1.0f ) {
		clip( finalColor.a - 0.0001f );
	}
	
	// ディザ抜き計算.
	if ( g_vFlag.x >= 1.0f ) {
		const int pt_x = (int) fmod( input.Pos.x, 4.0f );
		const int pt_y = (int) fmod( input.Pos.y, 4.0f );
		const float dither = DITHER_PATTERN[pt_y][pt_x];
		clip( finalColor.a - dither );
	}
	
	return finalColor;
}

//========= ﾃｸｽﾁｬ無し用 ========================================
//-------------------------------------------------
//	頂点(ﾊﾞｰﾃｯｸｽ)ｼｪｰﾀﾞ.
//-------------------------------------------------
VS_OUTPUT VS_NoTex(
	float4 Pos : POSITION,
	float4 Norm: NORMAL)
{
	VS_OUTPUT output = (VS_OUTPUT)0;

	//ﾌﾟﾛｼﾞｪｸｼｮﾝ変換(ﾜｰﾙﾄﾞ→ﾋﾞｭｰ→ﾌﾟﾛｼﾞｪｸｼｮﾝ).
	output.Pos = mul( Pos, g_mWVP );

	//法線をﾓﾃﾞﾙの姿勢に合わせる.
	// (ﾓﾃﾞﾙが回転すれば法線も回転させる必要があるため).
    output.Normal = mul( ( float1x3 )Norm, ( float3x3 )g_mW );
    
	//ﾗｲﾄ方向:
	// ﾃﾞｨﾚｸｼｮﾅﾙﾗｲﾄは、どこでも同じ方向.位置は無関係.
	output.Light = (float3)normalize( g_LightDir );

	output.PosWorld = mul( Pos, g_mW );

	//視線ﾍﾞｸﾄﾙ:
	// ﾜｰﾙﾄﾞ空間上での頂点から頂点へ向かうﾍﾞｸﾄﾙ.
	output.EyeVector = (float3)normalize( g_CameraPos - output.PosWorld );

	return output;
}

//-------------------------------------------------
//	ﾋﾟｸｾﾙｼｪｰﾀﾞ.
//-------------------------------------------------
float4 PS_NoTex( VS_OUTPUT input ) : SV_Target
{
	float4 finalColor = g_Color;
	
	// カメラの距離によって透明にしていく.
	if ( g_vFlag.z >= 1.0f ) {
		const float Distance = length( input.Pos - g_CameraPos );
		if ( Distance <= g_AlphaDistance.x ) {
			const float OverDistance = Distance - g_AlphaDistance.x;
			finalColor.a -= OverDistance / g_AlphaDistance.z / 10.0f;
		}
	}
	if ( g_vFlag.w >= 1.0f ) {
		const float Distance = length( input.Pos - g_CameraPos );
		if ( Distance >= g_AlphaDistance.y ) {
			const float OverDistance = Distance - g_AlphaDistance.y;
			finalColor.a -= OverDistance / g_AlphaDistance.z / 10.0f;
		}
	}
	
	// アルファブロック計算.
	if ( g_vFlag.y >= 1.0f){
		clip(finalColor.a - 0.0001f);
	}
	
	// ディザ抜き計算.
	if ( g_vFlag.x >= 1.0f ){
		const int pt_x = (int) fmod(input.Pos.x, 4.0f);
		const int pt_y = (int) fmod(input.Pos.y, 4.0f);
		const float dither = DITHER_PATTERN[pt_y][pt_x];
		clip(finalColor.a - dither);
	}
	return finalColor;
}
