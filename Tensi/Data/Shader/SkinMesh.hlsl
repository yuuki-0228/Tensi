/***************************************************************************************************
*	SkinMeshCode Version 2.00
*	LastUpdate	: 2019/10/09.
**/
//定義.
#define MAX_BONE_MATRICES (255)

//グローバル.
Texture2D		g_Texture		: register( t0 );	//テクスチャーは レジスターt(n).
Texture2D		g_LightTexture	: register( t1 );	//テクスチャーは レジスターt(n).
SamplerState	g_Sampler		: register( s0 );	//サンプラーはレジスターs(n).


//コンスタントバッファ(メッシュごと).
cbuffer per_mesh		: register( b0 )
{
	matrix g_mW;		//ワールド行列.
	matrix g_mWVP;		//ワールドから射影までの変換行列.
	float4 g_Color;		//色(RGBA:xyzw).
};
//コンスタントバッファ(マテリアルごと).
cbuffer per_material	: register( b1 )
{
	float4 g_Ambient	= float4( 0, 0, 0, 0 );	//アンビエント光.
	float4 g_Diffuse	= float4( 1, 0, 0, 0 );	//拡散反射(色）
	float4 g_Specular	= float4( 1, 1, 1, 1 );	//鏡面反射.
	float4 g_UV			= float4( 0, 0, 0, 0 );	//鏡面反射.
};
//コンスタントバッファ(フレームごと).
cbuffer per_frame		: register( b2 )
{
    float4 g_CameraPos;				//カメラ位置(始点位置).
    float4 g_LightDir;				//ライトの方向ベクトル.
	float4 g_LightIntensity;		//ﾗｲﾄの強弱.
	float4 g_PointLightIntensity;	//メッシュに対してのポイントライトの強弱.
	float4 g_vFlag;					//x：ディザ抜きを使用するか、	y：アルファブロックを使用するか、	z：距離で透明にするか(以下)、w：距離で透明にするか(以上).
	float4 g_AlphaDistance;			//x：開始する距離(以下)、		y：開始する距離(以上)、			z：透明にしていく間隔.
};
//ボーンのポーズ行列が入る.
cbuffer per_bones		: register( b3 )
{
	matrix g_mConstBoneWorld[MAX_BONE_MATRICES];
};

//スキニング後の頂点・法線が入る.
struct Skin
{
	float4 Pos;
	float3 Norm;
};
//バーテックスバッファーの入力.
struct VSSkinIn
{
	float3 Pos		: POSITION;		//位置.  
	float3 Norm		: NORMAL;		//頂点法線.
	float2 Tex		: TEXCOORD;		//テクスチャー座標.
	uint4  Bones	: BONE_INDEX;	//ボーンのインデックス.
	float4 Weights	: BONE_WEIGHT;	//ボーンの重み.
};

//ピクセルシェーダーの入力（バーテックスバッファーの出力）　
struct PSSkinIn
{
	float4 Pos	: SV_Position;	//位置.
	float3 Norm : NORMAL;		//頂点法線.
	float2 Tex	: TEXCOORD;		//テクスチャー座標.
	float4 Color: COLOR0;		//最終カラー（頂点シェーダーにおいての）.
};

static const float DITHER_PATTERN[4][4] =
{
	{ 0.00f, 0.32f, 0.08f, 0.40f, },
	{ 0.48f, 0.16f, 0.56f, 0.24f, },
	{ 0.12f, 0.44f, 0.04f, 0.36f, },
	{ 0.60f, 0.28f, 0.52f, 0.20f, },
};


//指定した番号のボーンのポーズ行列を返す.
//サブ関数（バーテックスシェーダーで使用）.
matrix FetchBoneMatrix( uint iBone )
{
	return g_mConstBoneWorld[iBone];
}


//頂点をスキニング（ボーンにより移動）する.
//サブ関数（バーテックスシェーダーで使用）.
Skin SkinVert( VSSkinIn Input )
{
	Skin Output = (Skin)0;

	float4 Pos = float4(Input.Pos,1);
	float3 Norm = Input.Norm;
	//ボーン0.
	uint iBone	=Input.Bones.x;
	float fWeight= Input.Weights.x;
	matrix m	=  FetchBoneMatrix( iBone );
	Output.Pos	+= fWeight * mul( Pos, m );
	Output.Norm	+= fWeight * mul( Norm, (float3x3)m );
	//ボーン1.
	iBone	= Input.Bones.y;
	fWeight	= Input.Weights.y;
	m		= FetchBoneMatrix( iBone );
	Output.Pos	+= fWeight * mul( Pos, m );
	Output.Norm	+= fWeight * mul( Norm, (float3x3)m );
	//ボーン2.
	iBone	= Input.Bones.z;
	fWeight	= Input.Weights.z;
	m		= FetchBoneMatrix( iBone );
	Output.Pos	+= fWeight * mul( Pos, m );
	Output.Norm	+= fWeight * mul( Norm, (float3x3)m );
	//ボーン3.
	iBone	= Input.Bones.w;
	fWeight	= Input.Weights.w;
	m		= FetchBoneMatrix( iBone );
	Output.Pos	+= fWeight * mul( Pos, m );
	Output.Norm	+= fWeight * mul( Norm, (float3x3)m );

	return Output;
}

// バーテックスシェーダ.
PSSkinIn VS_Main( VSSkinIn input )
{
	PSSkinIn output;
	Skin vSkinned = SkinVert( input);

	output.Pos	= mul( vSkinned.Pos, g_mWVP );
	output.Norm	= normalize( mul( vSkinned.Norm, (float3x3)g_mW ) );
	output.Tex	= input.Tex + g_UV.xy;
    float3 LightDir = (float3)normalize( g_LightDir );
	float3 PosWorld	= (float3)mul( vSkinned.Pos, g_mW );
	float3 ViewDir	= normalize( (float3)g_CameraPos - PosWorld );
	float3 Normal	= normalize( output.Norm );
	float4 NL		= saturate( dot( Normal, LightDir ) );

	float3 Reflect = normalize( 2 * (float3)NL * Normal - LightDir );
	float4 specular = pow( saturate( dot( Reflect, ViewDir ) ), 4.0f ); 

	output.Color	= g_Diffuse * NL + specular * g_Specular;

	return output;
}


// ピクセルシェーダ.
float4 PS_Main( PSSkinIn input ) : SV_Target
{
	// テクスチャからピクセル色を取り出す.
	float4 TexDiffuse		= g_Texture.Sample( g_Sampler, input.Tex );
	float4 LightTexDiffuse	= g_LightTexture.Sample(g_Sampler, input.Tex); // 法線色.
	// 出力用を作成.
	float4 finalColor = ((input.Color / 2.0f) * (1.0f - LightTexDiffuse.r * 1.0f) + TexDiffuse / 2.0f) * (g_LightIntensity.x + (g_PointLightIntensity.x - g_LightIntensity.x) * LightTexDiffuse.r);

	// テクスチャのα値をそのまま使用。
	finalColor.a = TexDiffuse.a;
	finalColor *= g_Color;
	
	// カメラの距離によって透明にしていく.
	if ( g_vFlag.z >= 1.0f ) {
		const float Distance = length( mul( input.Pos, g_mWVP ) - g_CameraPos );
		if ( Distance <= g_AlphaDistance.x * 1000.0f ) {
			const float OverDistance = g_AlphaDistance.x * 1000.0f - Distance;
			finalColor.a -= OverDistance / ( g_AlphaDistance.z * 10.0f ) / 10000.0f;
		}
	}
	if ( g_vFlag.w >= 1.0f ) {
		const float Distance = length( mul( input.Pos, g_mWVP ) - g_CameraPos );
		if ( Distance >= g_AlphaDistance.y * 1000.0f ) {
			const float OverDistance = Distance - g_AlphaDistance.y * 1000.0f;
			finalColor.a -= OverDistance / ( g_AlphaDistance.z * 10.0f ) / 10000.0f;
		}
	}

	// アルファブロック計算.
	if ( g_vFlag.y >= 1.0f ){
		clip(finalColor.a - 0.0001f);
	}
	
	// ディザ抜き計算.
	if ( g_vFlag.x >= 1.0f ) {
		const int pt_x = (int) fmod( input.Pos.x, 4.0f );
		const int pt_y = (int) fmod( input.Pos.y, 4.0f );
		const float dither = DITHER_PATTERN[pt_y][pt_x];
		clip(finalColor.a - dither);
	}
	
	return finalColor;
}
