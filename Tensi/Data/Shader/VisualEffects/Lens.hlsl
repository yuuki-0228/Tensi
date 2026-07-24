//================================================.
//	レンズ・光学系の視覚効果シェーダまとめ.
//	魚眼 / 色相シフト / 熱気揺らぎ / ソフトフォーカス.
//================================================.

// テクスチャはレジスタ t(n).
Texture2D		g_SrcTexture	: register( t0 );	// 入力(直前の描画結果).
Texture2D		g_SubTexture	: register( t1 );	// 補助入力(ぼかし結果等).
// サンプラはレジスタ s(n).
SamplerState	g_SamLinear		: register( s0 );

// 共通定数バッファ.
cbuffer Common : register( b0 )
{
	float4	g_Common0;	// x:画面幅, y:画面高さ, z:経過時間(秒), w:適用率(0～1).
	float4	g_Common1;	// x:透明部分の持ち上げアルファ, yzw:未使用.
	float4	g_Area;		// 適用範囲(px). 幅か高さが0以下で画面全体.
};
// エフェクト毎のパラメータ定数バッファ(内容は各エントリのコメントを参照).
cbuffer EffectParam : register( b1 )
{
	float4	g_P0;
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

//================================================.
//	共通関数.
//================================================.

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

// 乱数(0～1).
float Rand( float2 seed )
{
	return frac( sin( dot( seed, float2( 12.9898f, 78.233f ) ) ) * 43758.5453f );
}

// 値ノイズ(0～1. 滑らかに変化する).
float ValueNoise( float2 p )
{
	float2 i = floor( p );
	float2 f = frac( p );
	float2 u = f * f * ( 3.0f - 2.0f * f );
	float a = Rand( i );
	float b = Rand( i + float2( 1.0f, 0.0f ) );
	float c = Rand( i + float2( 0.0f, 1.0f ) );
	float d = Rand( i + float2( 1.0f, 1.0f ) );
	return lerp( lerp( a, b, u.x ), lerp( c, d, u.x ), u.y );
}

//================================================.
//	魚眼(樽型歪み).
//	g_P0 : x:歪みの強さ(-1～1. 負で糸巻き型).
//================================================.
float4 PS_Fisheye( VS_OUTPUT i ) : SV_Target
{
	float4 src = g_SrcTexture.Sample( g_SamLinear, i.UV );
	if ( IsOutArea( i.Pos.xy ) ) return src;

	float2 c = i.UV * 2.0f - 1.0f;
	float r2 = dot( c, c );
	// 中心からの距離に応じて拡大率を変える(端が収まるように正規化する).
	c *= ( 1.0f + g_P0.x * r2 ) / ( 1.0f + g_P0.x );
	float2 uv = c * 0.5f + 0.5f;
	float4 col = g_SrcTexture.Sample( g_SamLinear, uv );
	return FinishColor( src, col );
}

//================================================.
//	色相シフト.
//	g_P0 : x:色相の回転量(度), y:回転アニメーション速度(度/秒).
//================================================.
float4 PS_HueShift( VS_OUTPUT i ) : SV_Target
{
	float4 src = g_SrcTexture.Sample( g_SamLinear, i.UV );
	if ( IsOutArea( i.Pos.xy ) ) return src;

	float rad = radians( g_P0.x + g_Common0.z * g_P0.y );
	float c = cos( rad );
	float s = sin( rad );

	// 輝度を保ったまま色相を回転する行列.
	float3x3 hueMatrix = float3x3(
		0.299f + 0.701f * c + 0.168f * s,	0.587f - 0.587f * c + 0.330f * s,	0.114f - 0.114f * c - 0.497f * s,
		0.299f - 0.299f * c - 0.328f * s,	0.587f + 0.413f * c + 0.035f * s,	0.114f - 0.114f * c + 0.292f * s,
		0.299f - 0.300f * c + 1.250f * s,	0.587f - 0.588f * c - 1.050f * s,	0.114f + 0.886f * c - 0.203f * s );

	float3 col = saturate( mul( hueMatrix, src.rgb ) );
	return FinishColor( src, float4( col, src.a ) );
}

//================================================.
//	熱気揺らぎ(陽炎).
//	g_P0 : x:振れ幅(px), y:細かさ, z:揺らぎの速さ, w:立ち上る速さ.
//================================================.
float4 PS_HeatHaze( VS_OUTPUT i ) : SV_Target
{
	float4 src = g_SrcTexture.Sample( g_SamLinear, i.UV );
	if ( IsOutArea( i.Pos.xy ) ) return src;

	// 上へ流れるノイズで細かく歪ませる.
	float2 p = i.UV * g_P0.y + float2( 0.0f, g_Common0.z * g_P0.w );
	float n1 = ValueNoise( p + float2( g_Common0.z * g_P0.z, 0.0f ) );
	float n2 = ValueNoise( p * 1.7f + float2( 31.0f, g_Common0.z * g_P0.z * 0.8f ) );
	float2 offset = ( float2( n1, n2 ) - 0.5f ) * 2.0f * g_P0.x / g_Common0.xy;

	float4 col = g_SrcTexture.Sample( g_SamLinear, i.UV + offset );
	return FinishColor( src, col );
}

//================================================.
//	ソフトフォーカス( t0:元画像 + t1:ぼかし をスクリーン合成 ).
//	g_P0 : x:グローの強さ(0～1).
//================================================.
float4 PS_SoftFocus( VS_OUTPUT i ) : SV_Target
{
	float4 src = g_SrcTexture.Sample( g_SamLinear, i.UV );
	if ( IsOutArea( i.Pos.xy ) ) return src;

	float4 blur = g_SubTexture.Sample( g_SamLinear, i.UV );
	// スクリーン合成で明部をふんわり光らせる.
	float3 screen = 1.0f - ( 1.0f - src.rgb ) * ( 1.0f - blur.rgb );
	float3 col = lerp( src.rgb, screen, g_P0.x );
	float a = max( src.a, blur.a * g_P0.x );
	return FinishColor( src, float4( col, a ) );
}
