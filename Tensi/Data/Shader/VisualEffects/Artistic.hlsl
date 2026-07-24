//================================================.
//	アート系の視覚効果シェーダまとめ.
//	トゥーン / 水彩 / 油絵 / 色鉛筆 / ハーフトーン / エンボス / 2値化 / オールドフィルム.
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

// 円周率.
static const float PI = 3.14159265f;

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

// 輝度の取得.
float GetLuma( float3 col )
{
	return dot( col, float3( 0.299f, 0.587f, 0.114f ) );
}

// エッジ検出用の値(輝度×アルファ. 透明との境界も輪郭として拾える).
float GetEdgeValue( float2 uv )
{
	float4 c = g_SrcTexture.Sample( g_SamLinear, uv );
	return GetLuma( c.rgb ) * c.a + ( 1.0f - c.a ) * 0.0f;
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
//	トゥーン(セル塗り風).
//	g_P0 : x:色の段階数, y:輪郭のしきい値, z:輪郭の太さ(px).
//	g_P1 : rgba:輪郭線の色.
//================================================.
float4 PS_Toon( VS_OUTPUT i ) : SV_Target
{
	float4 src = g_SrcTexture.Sample( g_SamLinear, i.UV );
	if ( IsOutArea( i.Pos.xy ) ) return src;

	// 輝度を段階化して色に反映する(色相は保つ).
	float levels = max( 2.0f, g_P0.x );
	float luma = GetLuma( src.rgb );
	float quantized = floor( luma * levels ) / ( levels - 1.0f );
	float3 col = src.rgb * ( quantized / max( luma, 0.0001f ) );

	// 輪郭線.
	float edge = SobelEdge( i.UV, g_P0.z );
	float edgeMask = smoothstep( g_P0.y, g_P0.y + 0.15f, edge );
	col = lerp( col, g_P1.rgb, edgeMask );

	float a = max( src.a, edgeMask * g_P1.a );
	return FinishColor( src, float4( saturate( col ), a ) );
}

//================================================.
//	水彩.
//	g_P0 : x:にじみ(px), y:紙の質感, z:輪郭の溜まり, w:揺らぎ(px).
//================================================.
float4 PS_Watercolor( VS_OUTPUT i ) : SV_Target
{
	float4 src = g_SrcTexture.Sample( g_SamLinear, i.UV );
	if ( IsOutArea( i.Pos.xy ) ) return src;

	// 手描き風の揺らぎ(粒単位のランダムな歪み).
	float2 cell = floor( i.Pos.xy / 6.0f );
	float2 wobble = ( float2( Rand( cell ), Rand( cell + 31.0f ) ) - 0.5f ) * g_P0.w / g_Common0.xy;
	float2 uv = i.UV + wobble;

	// にじみ(軽いぼかし).
	float2 px = g_P0.x / g_Common0.xy;
	float4 col = g_SrcTexture.Sample( g_SamLinear, uv );
	col += g_SrcTexture.Sample( g_SamLinear, uv + float2(  px.x,  0.0f ) );
	col += g_SrcTexture.Sample( g_SamLinear, uv + float2( -px.x,  0.0f ) );
	col += g_SrcTexture.Sample( g_SamLinear, uv + float2(  0.0f,  px.y ) );
	col += g_SrcTexture.Sample( g_SamLinear, uv + float2(  0.0f, -px.y ) );
	col /= 5.0f;

	// 輪郭に絵の具が溜まる表現.
	float edge = SobelEdge( uv, 1.5f );
	col.rgb *= 1.0f - edge * g_P0.z;

	// 紙の質感(低周波+高周波のむら).
	float paper = Rand( floor( i.Pos.xy / 2.0f ) ) * 0.6f + Rand( floor( i.Pos.xy / 9.0f ) ) * 0.4f;
	col.rgb *= 1.0f - ( paper - 0.5f ) * g_P0.y * 0.35f;
	// 全体を少し明るく水っぽくする.
	col.rgb = saturate( col.rgb * 1.05f + 0.02f );

	return FinishColor( src, float4( col.rgb, col.a ) );
}

//================================================.
//	油絵(Kuwaharaフィルタ).
//	g_P0 : x:筆のタッチの大きさ(2～6).
//================================================.
float4 PS_OilPainting( VS_OUTPUT i ) : SV_Target
{
	float4 src = g_SrcTexture.Sample( g_SamLinear, i.UV );
	if ( IsOutArea( i.Pos.xy ) ) return src;

	int radius = clamp( (int)g_P0.x, 2, 6 );
	float2 px = 1.0f / g_Common0.xy;

	// 4象限のうち輝度の分散が最小の領域の平均色を採用する.
	float minVariance = 1e10f;
	float4 result = src;

	[unroll]
	for ( int q = 0; q < 4; ++q ) {
		// 象限の向き( (-1,-1), (1,-1), (-1,1), (1,1) ).
		float2 dir = float2( ( q & 1 ) * 2 - 1, ( q >> 1 ) * 2 - 1 );

		float4 sum = 0.0f;
		float lumaSum = 0.0f;
		float lumaSqSum = 0.0f;
		float count = 0.0f;
		[loop]
		for ( int y = 0; y <= radius; ++y ) {
			[loop]
			for ( int x = 0; x <= radius; ++x ) {
				float2 offset = float2( x, y ) * dir * px;
				float4 c = g_SrcTexture.Sample( g_SamLinear, i.UV + offset );
				float lum = GetLuma( c.rgb );
				sum += c;
				lumaSum += lum;
				lumaSqSum += lum * lum;
				count += 1.0f;
			}
		}
		float mean = lumaSum / count;
		float variance = lumaSqSum / count - mean * mean;
		if ( variance < minVariance ) {
			minVariance = variance;
			result = sum / count;
		}
	}
	return FinishColor( src, result );
}

//================================================.
//	色鉛筆.
//	g_P0 : x:ハッチングの間隔(px), y:輪郭の濃さ, z:彩度, w:紙の白さ.
//================================================.
float4 PS_ColoredPencil( VS_OUTPUT i ) : SV_Target
{
	float4 src = g_SrcTexture.Sample( g_SamLinear, i.UV );
	if ( IsOutArea( i.Pos.xy ) ) return src;

	// 彩度を落として紙の白さで持ち上げる.
	float luma = GetLuma( src.rgb );
	float3 col = lerp( luma.xxx, src.rgb, g_P0.z );
	col = lerp( col, float3( 1.0f, 1.0f, 1.0f ), g_P0.w );

	// 斜線のハッチング(暗い部分ほど濃く乗る).
	float shade = 1.0f - luma;
	float jitter = Rand( floor( i.Pos.xy / 24.0f ) ) * PI;
	float wave = sin( ( i.Pos.x + i.Pos.y ) * PI * 2.0f / max( g_P0.x, 1.0f ) + jitter ) * 0.5f + 0.5f;
	float hatch = smoothstep( 0.55f, 0.95f, wave ) * smoothstep( 0.1f, 0.7f, shade );
	// 濃い部分は逆方向の線も重ねる(クロスハッチング).
	float wave2 = sin( ( i.Pos.x - i.Pos.y ) * PI * 2.0f / max( g_P0.x, 1.0f ) + jitter * 1.7f ) * 0.5f + 0.5f;
	hatch = max( hatch, smoothstep( 0.55f, 0.95f, wave2 ) * smoothstep( 0.5f, 0.95f, shade ) );
	col *= 1.0f - hatch * 0.45f;

	// 輪郭のスケッチ線.
	float edge = SobelEdge( i.UV, 1.0f );
	float edgeMask = smoothstep( 0.1f, 0.5f, edge ) * g_P0.y;
	// 線に鉛筆らしいかすれを混ぜる.
	edgeMask *= 0.7f + 0.3f * Rand( floor( i.Pos.xy / 2.0f ) );
	col = lerp( col, float3( 0.25f, 0.22f, 0.22f ), edgeMask );

	float a = max( src.a, edgeMask );
	return FinishColor( src, float4( saturate( col ), a ) );
}

//================================================.
//	ハーフトーン(漫画のトーン風ドット).
//	g_P0 : x:ドットの間隔(px), y:網の角度(度), z:元の色を使うか.
//	g_P1 : rgba:インクの色.
//	g_P2 : rgba:紙の色.
//================================================.
float4 PS_Halftone( VS_OUTPUT i ) : SV_Target
{
	float4 src = g_SrcTexture.Sample( g_SamLinear, i.UV );
	if ( IsOutArea( i.Pos.xy ) ) return src;

	float cellSize = max( g_P0.x, 2.0f );
	float rad = radians( g_P0.y );
	float2x2 rot	= float2x2(  cos( rad ), sin( rad ), -sin( rad ), cos( rad ) );
	float2x2 rotInv	= float2x2(  cos( rad ), -sin( rad ), sin( rad ), cos( rad ) );

	// 回転した格子でセルの中心を求め、その位置の輝度でドットの大きさを決める.
	float2 rp = mul( rot, i.Pos.xy );
	float2 cellCenter = ( floor( rp / cellSize ) + 0.5f ) * cellSize;
	float2 centerPos = mul( rotInv, cellCenter );
	float4 centerCol = g_SrcTexture.Sample( g_SamLinear, centerPos / g_Common0.xy );
	float luma = GetLuma( centerCol.rgb ) * centerCol.a + ( 1.0f - centerCol.a );

	float radius = cellSize * 0.7f * ( 1.0f - luma );
	float dist = length( rp - cellCenter );
	float ink = smoothstep( radius + 0.7f, radius - 0.7f, dist );

	float3 inkCol = ( g_P0.z > 0.5f ) ? centerCol.rgb : g_P1.rgb;
	float3 col = lerp( g_P2.rgb, inkCol, ink );
	return FinishColor( src, float4( col, src.a ) );
}

//================================================.
//	エンボス(凹凸彫刻風).
//	g_P0 : x:凹凸の強さ, y:光の方向(度), z:検出幅(px).
//================================================.
float4 PS_Emboss( VS_OUTPUT i ) : SV_Target
{
	float4 src = g_SrcTexture.Sample( g_SamLinear, i.UV );
	if ( IsOutArea( i.Pos.xy ) ) return src;

	float rad = radians( g_P0.y );
	float2 offset = g_P0.z * float2( cos( rad ), sin( rad ) ) / g_Common0.xy;
	float g = GetEdgeValue( i.UV - offset ) - GetEdgeValue( i.UV + offset );
	float v = saturate( 0.5f + g * g_P0.x );
	return FinishColor( src, float4( v, v, v, src.a ) );
}

//================================================.
//	2値化.
//	g_P0 : x:しきい値, y:境界の滑らかさ.
//	g_P1 : rgba:明部の色.
//	g_P2 : rgba:暗部の色.
//================================================.
float4 PS_Threshold( VS_OUTPUT i ) : SV_Target
{
	float4 src = g_SrcTexture.Sample( g_SamLinear, i.UV );
	if ( IsOutArea( i.Pos.xy ) ) return src;

	float luma = GetLuma( src.rgb );
	float t = smoothstep( g_P0.x - g_P0.y, g_P0.x + g_P0.y, luma );
	float3 col = lerp( g_P2.rgb, g_P1.rgb, t );
	return FinishColor( src, float4( col, src.a ) );
}

//================================================.
//	オールドフィルム(古い映像風).
//	g_P0 : x:セピア調, y:粒子ノイズ, z:縦傷の発生量, w:上下ジッター.
//	g_P1 : x:明滅, y:周辺減光.
//================================================.
float4 PS_OldFilm( VS_OUTPUT i ) : SV_Target
{
	float4 src = g_SrcTexture.Sample( g_SamLinear, i.UV );
	if ( IsOutArea( i.Pos.xy ) ) return src;

	// コマ送り(12fps)で乱数を更新する.
	float t = floor( g_Common0.z * 12.0f );

	// 画面の上下ジッター(たまにガタつく).
	float jitterOn = step( 0.75f, Rand( float2( t, 1.0f ) ) );
	float2 uv = i.UV;
	uv.y += ( Rand( float2( t, 2.0f ) ) - 0.5f ) * 0.02f * g_P0.w * jitterOn;
	float4 col = g_SrcTexture.Sample( g_SamLinear, uv );

	// セピア調.
	float luma = GetLuma( col.rgb );
	float3 sepia = luma * float3( 1.07f, 0.74f, 0.43f );
	col.rgb = lerp( col.rgb, sepia, g_P0.x );

	// 粒子ノイズ.
	float grain = ( Rand( floor( i.Pos.xy / 1.5f ) + t ) - 0.5f ) * g_P0.y;
	col.rgb = saturate( col.rgb + grain );

	// 縦傷(位置が毎コマ変わる細い縦線).
	float scratchX = floor( i.Pos.x / 2.0f );
	float scratch = step( 1.0f - g_P0.z * 0.015f, Rand( float2( scratchX, t * 3.0f ) ) );
	// 傷の濃さを縦方向にも揺らす.
	scratch *= step( 0.3f, Rand( float2( floor( i.Pos.y / 24.0f ), scratchX + t ) ) );
	col.rgb = lerp( col.rgb, float3( 0.9f, 0.88f, 0.8f ), scratch * 0.6f );

	// 明滅.
	col.rgb *= 1.0f + ( Rand( float2( t, 5.0f ) ) - 0.5f ) * g_P1.x;

	// 周辺減光.
	float2 d = i.UV - 0.5f;
	float vig = smoothstep( 0.4f, 0.9f, length( d ) * 1.4f );
	col.rgb *= 1.0f - vig * g_P1.y;

	float a = max( col.a, scratch * 0.6f );
	return FinishColor( src, float4( saturate( col.rgb ), a ) );
}
