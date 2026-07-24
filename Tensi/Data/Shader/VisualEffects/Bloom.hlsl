//================================================.
//	ブルームシェーダ.
//	PS_Bright(高輝度抽出) → ぼかし( GaussianBlur.hlsl ) → PS_Combine(加算合成).
//================================================.

// テクスチャはレジスタ t(n).
Texture2D		g_SrcTexture	: register( t0 );	// 入力(元画像).
Texture2D		g_SubTexture	: register( t1 );	// 補助入力(ぼかした高輝度).
// サンプラはレジスタ s(n).
SamplerState	g_SamLinear		: register( s0 );

// 共通定数バッファ.
cbuffer Common : register( b0 )
{
	float4	g_Common0;	// x:画面幅, y:画面高さ, z:経過時間(秒), w:適用率(0～1).
	float4	g_Common1;	// x:透明部分の持ち上げアルファ, yzw:未使用.
	float4	g_Area;		// 適用範囲(px). 幅か高さが0以下で画面全体.
};
// エフェクト毎のパラメータ定数バッファ.
cbuffer EffectParam : register( b1 )
{
	float4	g_P0;		// x:輝度のしきい値, y:光の強さ.
	float4	g_P1;
	float4	g_P2;
	float4	g_P3;
};

// 頂点シェーダの出力パラメータ.
struct VS_OUTPUT
{
	float4	Pos	: SV_Position;	// 座標.
	float2	UV	: TEXCOORD0;	// UV座標.
};

// 頂点シェーダ(全画面三角形. 頂点バッファ不要).
VS_OUTPUT VS_Main( uint id : SV_VertexID )
{
	VS_OUTPUT o;
	float2 uv = float2( ( id << 1 ) & 2, id & 2 );
	o.Pos	= float4( uv.x * 2.0f - 1.0f, 1.0f - uv.y * 2.0f, 0.0f, 1.0f );
	o.UV	= uv;
	return o;
}

// 適用範囲外か(ピクセル座標).
bool IsOutArea( float2 pos )
{
	if ( g_Area.z <= 0.0f || g_Area.w <= 0.0f ) return false;
	return ( pos.x <  g_Area.x || pos.y <  g_Area.y ||
			 pos.x >= g_Area.x + g_Area.z || pos.y >= g_Area.y + g_Area.w );
}

// 共通の仕上げ(元色との適用率合成と透明部分の持ち上げ).
float4 FinishColor( float4 src, float4 effect )
{
	float4 col = lerp( src, effect, g_Common0.w );
	col.a = max( col.a, g_Common1.x * g_Common0.w );
	return col;
}

// 輝度の取得.
float GetLuma( float3 col )
{
	return dot( col, float3( 0.299f, 0.587f, 0.114f ) );
}

//================================================.
//	高輝度抽出(中間パス).
//================================================.
float4 PS_Bright( VS_OUTPUT i ) : SV_Target
{
	float4 src = g_SrcTexture.Sample( g_SamLinear, i.UV );
	float luma = GetLuma( src.rgb );
	// しきい値を超えた分だけ滑らかに取り出す.
	float w = saturate( ( luma - g_P0.x ) / max( 1.0f - g_P0.x, 0.001f ) );
	return float4( src.rgb * w * w, src.a * w );
}

//================================================.
//	加算合成( t0:元画像 + t1:ぼかした高輝度 ).
//================================================.
float4 PS_Combine( VS_OUTPUT i ) : SV_Target
{
	float4 src = g_SrcTexture.Sample( g_SamLinear, i.UV );
	if ( IsOutArea( i.Pos.xy ) ) return src;

	float4 bloom = g_SubTexture.Sample( g_SamLinear, i.UV );
	float3 col = src.rgb + bloom.rgb * g_P0.y;
	// 光が乗った部分はアルファも持ち上げる(透明ウィンドウで光を見せるため).
	float a = max( src.a, saturate( GetLuma( bloom.rgb ) * g_P0.y ) );
	return FinishColor( src, float4( saturate( col ), a ) );
}
