//================================================.
//	ガウスブラーシェーダ.
//	PS_BlurH → PS_BlurV( または PS_BlurFinish )の2パスで使用する.
//	Bloom / Frost の中間パスとしても使用される.
//================================================.

// テクスチャはレジスタ t(n).
Texture2D		g_SrcTexture	: register( t0 );	// 入力.
Texture2D		g_SubTexture	: register( t1 );	// 補助入力( PS_BlurFinish の元画像 ).
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
	float4	g_P0;		// x:ぼかし半径(px).
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

// 最大タップ数(片側).
static const int MAX_TAP = 32;

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

// 1方向のガウスブラー.
float4 GaussianBlur( float2 uv, float2 dir )
{
	float radius = clamp( g_P0.x, 0.0f, (float)MAX_TAP );
	if ( radius < 0.5f ) return g_SrcTexture.Sample( g_SamLinear, uv );

	// 標準偏差は半径の半分にする(端の重みがほぼ0になる).
	float sigma = radius * 0.5f;
	float denom = 2.0f * sigma * sigma;

	float4 sum = 0.0f;
	float weightSum = 0.0f;
	int tap = (int)ceil( radius );
	[loop]
	for ( int s = -tap; s <= tap; ++s ) {
		float w = exp( -(float)( s * s ) / denom );
		sum += g_SrcTexture.Sample( g_SamLinear, uv + dir * (float)s ) * w;
		weightSum += w;
	}
	return sum / weightSum;
}

//================================================.
//	横ブラー(中間パス. 適用率などは反映しない).
//================================================.
float4 PS_BlurH( VS_OUTPUT i ) : SV_Target
{
	return GaussianBlur( i.UV, float2( 1.0f / g_Common0.x, 0.0f ) );
}

//================================================.
//	縦ブラー(中間パス. 適用率などは反映しない).
//================================================.
float4 PS_BlurV( VS_OUTPUT i ) : SV_Target
{
	return GaussianBlur( i.UV, float2( 0.0f, 1.0f / g_Common0.y ) );
}

//================================================.
//	縦ブラー+仕上げ( t1:元画像 と適用率合成する ).
//================================================.
float4 PS_BlurFinish( VS_OUTPUT i ) : SV_Target
{
	float4 src = g_SubTexture.Sample( g_SamLinear, i.UV );
	if ( IsOutArea( i.Pos.xy ) ) return src;
	float4 col = GaussianBlur( i.UV, float2( 0.0f, 1.0f / g_Common0.y ) );
	return FinishColor( src, col );
}
