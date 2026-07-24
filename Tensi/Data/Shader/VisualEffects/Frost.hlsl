//================================================.
//	曇り(すりガラス)シェーダ.
//	ぼかし( GaussianBlur.hlsl )した画( t1 )へ、ざらつきによる歪みと白みを加える.
//================================================.

// テクスチャはレジスタ t(n).
Texture2D		g_SrcTexture	: register( t0 );	// 入力(元画像).
Texture2D		g_SubTexture	: register( t1 );	// 補助入力(ぼかした画).
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
	float4	g_P0;		// x:白みの強さ(0～1), y:ざらつきによる歪み量(px), z:ざらつきの粒の大きさ(px).
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

// 乱数(0～1).
float Rand( float2 seed )
{
	return frac( sin( dot( seed, float2( 12.9898f, 78.233f ) ) ) * 43758.5453f );
}

//================================================.
//	曇りの仕上げ.
//================================================.
float4 PS_Frost( VS_OUTPUT i ) : SV_Target
{
	float4 src = g_SrcTexture.Sample( g_SamLinear, i.UV );
	if ( IsOutArea( i.Pos.xy ) ) return src;

	// ざらつき(粒単位のランダムな歪み).
	float2 cell = floor( i.Pos.xy / max( g_P0.z, 1.0f ) );
	float2 noise = float2( Rand( cell ), Rand( cell + 31.0f ) ) - 0.5f;
	float2 uv = i.UV + noise * g_P0.y / g_Common0.xy;

	// ぼかした画を歪めて取得して白みを加える.
	float4 col = g_SubTexture.Sample( g_SamLinear, uv );
	col.rgb = lerp( col.rgb, float3( 1.0f, 1.0f, 1.0f ), g_P0.x * 0.35f );
	col.a = max( col.a, src.a );
	return FinishColor( src, col );
}
