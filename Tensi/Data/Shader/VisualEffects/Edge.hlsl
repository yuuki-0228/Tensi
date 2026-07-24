//================================================.
//	エッジ系の視覚効果シェーダまとめ.
//	アウトライン / エッジ検出 / エッジ強調.
//================================================.

// テクスチャはレジスタ t(n).
Texture2D		g_SrcTexture	: register( t0 );	// 入力(直前の描画結果).
Texture2D		g_SubTexture	: register( t1 );	// 補助入力(未使用).
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

// 輝度の取得.
float GetLuma( float3 col )
{
	return dot( col, float3( 0.299f, 0.587f, 0.114f ) );
}

// エッジ検出用の値(輝度×アルファ. 透明との境界も輪郭として拾える).
float GetEdgeValue( float2 uv )
{
	float4 c = g_SrcTexture.Sample( g_SamLinear, uv );
	return GetLuma( c.rgb ) * c.a;
}

// Sobelフィルタによるエッジの強さ(0～1程度).
float SobelEdge( float2 uv, float thickness )
{
	float2 px = thickness / g_Common0.xy;
	float tl = GetEdgeValue( uv + float2( -px.x, -px.y ) );
	float  t = GetEdgeValue( uv + float2(  0.0f, -px.y ) );
	float tr = GetEdgeValue( uv + float2(  px.x, -px.y ) );
	float  l = GetEdgeValue( uv + float2( -px.x,  0.0f ) );
	float  r = GetEdgeValue( uv + float2(  px.x,  0.0f ) );
	float bl = GetEdgeValue( uv + float2( -px.x,  px.y ) );
	float  b = GetEdgeValue( uv + float2(  0.0f,  px.y ) );
	float br = GetEdgeValue( uv + float2(  px.x,  px.y ) );
	float gx = ( tr + 2.0f * r + br ) - ( tl + 2.0f * l + bl );
	float gy = ( bl + 2.0f * b + br ) - ( tl + 2.0f * t + tr );
	return saturate( length( float2( gx, gy ) ) );
}

//================================================.
//	アウトライン(輪郭線を元画像へ重ねる).
//	g_P0 : x:しきい値, y:太さ(px).
//	g_P1 : rgba:輪郭線の色.
//================================================.
float4 PS_Outline( VS_OUTPUT i ) : SV_Target
{
	float4 src = g_SrcTexture.Sample( g_SamLinear, i.UV );
	if ( IsOutArea( i.Pos.xy ) ) return src;

	float edge = SobelEdge( i.UV, g_P0.y );
	float mask = smoothstep( g_P0.x, g_P0.x + 0.15f, edge );
	float3 col = lerp( src.rgb, g_P1.rgb, mask * g_P1.a );
	float a = max( src.a, mask * g_P1.a );
	return FinishColor( src, float4( col, a ) );
}

//================================================.
//	エッジ検出(輪郭のみ表示する).
//	g_P0 : x:しきい値.
//	g_P1 : rgba:輪郭の色.
//	g_P2 : rgba:背景の色.
//================================================.
float4 PS_EdgeDetect( VS_OUTPUT i ) : SV_Target
{
	float4 src = g_SrcTexture.Sample( g_SamLinear, i.UV );
	if ( IsOutArea( i.Pos.xy ) ) return src;

	float edge = SobelEdge( i.UV, 1.0f );
	float mask = smoothstep( g_P0.x, g_P0.x + 0.1f, edge );
	float3 col = lerp( g_P2.rgb, g_P1.rgb, mask );
	float a = max( src.a, mask );
	return FinishColor( src, float4( col, a ) );
}

//================================================.
//	エッジ強調(輪郭部分だけ先鋭化する).
//	g_P0 : x:強調の強さ, y:検出幅(px).
//================================================.
float4 PS_EdgeEnhance( VS_OUTPUT i ) : SV_Target
{
	float4 src = g_SrcTexture.Sample( g_SamLinear, i.UV );
	if ( IsOutArea( i.Pos.xy ) ) return src;

	// 近傍平均との差分を輪郭部分だけ足し込む(エッジ選択のアンシャープマスク).
	float2 px = g_P0.y / g_Common0.xy;
	float3 avg = g_SrcTexture.Sample( g_SamLinear, i.UV + float2(  px.x, 0.0f ) ).rgb;
	avg += g_SrcTexture.Sample( g_SamLinear, i.UV + float2( -px.x, 0.0f ) ).rgb;
	avg += g_SrcTexture.Sample( g_SamLinear, i.UV + float2( 0.0f,  px.y ) ).rgb;
	avg += g_SrcTexture.Sample( g_SamLinear, i.UV + float2( 0.0f, -px.y ) ).rgb;
	avg *= 0.25f;

	float edge = SobelEdge( i.UV, g_P0.y );
	float mask = smoothstep( 0.05f, 0.3f, edge );
	float3 col = saturate( src.rgb + ( src.rgb - avg ) * g_P0.x * mask );
	return FinishColor( src, float4( col, src.a ) );
}
