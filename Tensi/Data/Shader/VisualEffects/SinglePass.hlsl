//================================================.
//	単パスの視覚効果シェーダまとめ.
//	全画面三角形( VS_Main )に対して各エフェクトのピクセルシェーダを適用する.
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
	float4	g_Area;		// 適用範囲(px). x:左上x, y:左上y, z:幅, w:高さ. 幅か高さが0以下で画面全体.
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

//================================================.
//	コピー(マネージャーの最終出力・合成用).
//================================================.
float4 PS_Copy( VS_OUTPUT i ) : SV_Target
{
	return g_SrcTexture.Sample( g_SamLinear, i.UV );
}

//================================================.
//	白黒.
//================================================.
float4 PS_Grayscale( VS_OUTPUT i ) : SV_Target
{
	float4 src = g_SrcTexture.Sample( g_SamLinear, i.UV );
	if ( IsOutArea( i.Pos.xy ) ) return src;
	float g = GetLuma( src.rgb );
	return FinishColor( src, float4( g, g, g, src.a ) );
}

//================================================.
//	セピア.
//	g_P0 : rgb:セピアの色味.
//================================================.
float4 PS_Sepia( VS_OUTPUT i ) : SV_Target
{
	float4 src = g_SrcTexture.Sample( g_SamLinear, i.UV );
	if ( IsOutArea( i.Pos.xy ) ) return src;
	float g = GetLuma( src.rgb );
	return FinishColor( src, float4( g * g_P0.rgb, src.a ) );
}

//================================================.
//	色反転.
//================================================.
float4 PS_Invert( VS_OUTPUT i ) : SV_Target
{
	float4 src = g_SrcTexture.Sample( g_SamLinear, i.UV );
	if ( IsOutArea( i.Pos.xy ) ) return src;
	return FinishColor( src, float4( 1.0f - src.rgb, src.a ) );
}

//================================================.
//	ポスタライズ(減色).
//	g_P0 : x:色の段階数.
//================================================.
float4 PS_Posterize( VS_OUTPUT i ) : SV_Target
{
	float4 src = g_SrcTexture.Sample( g_SamLinear, i.UV );
	if ( IsOutArea( i.Pos.xy ) ) return src;
	float levels = max( 2.0f, g_P0.x );
	float3 col = floor( src.rgb * levels ) / ( levels - 1.0f );
	return FinishColor( src, float4( saturate( col ), src.a ) );
}

//================================================.
//	カラーフィルター.
//	g_P0 : rgba:乗算する色.
//	g_P1 : rgba:加算する色.
//================================================.
float4 PS_ColorFilter( VS_OUTPUT i ) : SV_Target
{
	float4 src = g_SrcTexture.Sample( g_SamLinear, i.UV );
	if ( IsOutArea( i.Pos.xy ) ) return src;
	float3 col = saturate( src.rgb * g_P0.rgb + g_P1.rgb );
	return FinishColor( src, float4( col, src.a ) );
}

//================================================.
//	色調補正.
//	g_P0 : x:明度(加算), y:コントラスト, z:彩度, w:ガンマ.
//================================================.
float4 PS_ColorCorrection( VS_OUTPUT i ) : SV_Target
{
	float4 src = g_SrcTexture.Sample( g_SamLinear, i.UV );
	if ( IsOutArea( i.Pos.xy ) ) return src;

	float3 col = src.rgb;
	col = pow( abs( col ), 1.0f / max( g_P0.w, 0.01f ) );	// ガンマ.
	col = ( col - 0.5f ) * g_P0.y + 0.5f;					// コントラスト.
	col += g_P0.x;											// 明度.
	col = lerp( GetLuma( col ).xxx, col, g_P0.z );			// 彩度.
	return FinishColor( src, float4( saturate( col ), src.a ) );
}

//================================================.
//	ビネット(周辺減光).
//	g_P0 : x:減光が始まる半径, y:滑らかさ, z:縁の不透明度.
//	g_P1 : rgb:縁の色.
//================================================.
float4 PS_Vignette( VS_OUTPUT i ) : SV_Target
{
	float4 src = g_SrcTexture.Sample( g_SamLinear, i.UV );
	if ( IsOutArea( i.Pos.xy ) ) return src;

	// アスペクト比を補正した中心からの距離.
	float2 d = i.UV - 0.5f;
	d.x *= g_Common0.x / max( g_Common0.y, 1.0f );
	float dist = length( d );

	float v = smoothstep( g_P0.x, g_P0.x + max( g_P0.y, 0.001f ), dist );
	float3 col = lerp( src.rgb, g_P1.rgb, v );
	float a = max( src.a, v * g_P0.z );
	return FinishColor( src, float4( col, a ) );
}

//================================================.
//	ノイズ.
//	g_P0 : x:強さ, y:粒の大きさ(px), z:カラーノイズか, w:更新回数(回/秒).
//================================================.
float4 PS_Noise( VS_OUTPUT i ) : SV_Target
{
	float4 src = g_SrcTexture.Sample( g_SamLinear, i.UV );
	if ( IsOutArea( i.Pos.xy ) ) return src;

	// 時間を量子化してノイズを更新する.
	float t = floor( g_Common0.z * max( g_P0.w, 0.001f ) );
	float2 cell = floor( i.Pos.xy / max( g_P0.y, 1.0f ) );

	float3 n;
	if ( g_P0.z > 0.5f ) {
		n = float3(
			Rand( cell + float2( t, 0.0f ) ),
			Rand( cell + float2( t, 17.0f ) ),
			Rand( cell + float2( t, 31.0f ) ) ) - 0.5f;
	}
	else {
		n = ( Rand( cell + float2( t, 0.0f ) ) - 0.5f ).xxx;
	}
	float3 col = saturate( src.rgb + n * g_P0.x );
	return FinishColor( src, float4( col, src.a ) );
}

//================================================.
//	フィルムグレイン.
//	g_P0 : x:強さ, y:粒の大きさ(px), z:明滅の強さ, w:明部で弱める度合い.
//================================================.
float4 PS_FilmGrain( VS_OUTPUT i ) : SV_Target
{
	float4 src = g_SrcTexture.Sample( g_SamLinear, i.UV );
	if ( IsOutArea( i.Pos.xy ) ) return src;

	float t = floor( g_Common0.z * 24.0f );	// フィルム風に24fpsで更新する.
	float2 cell = floor( i.Pos.xy / max( g_P0.y, 1.0f ) );

	// 明るい部分ほど粒子を弱める.
	float luma = GetLuma( src.rgb );
	float grain = ( Rand( cell + float2( t, 5.0f ) ) - 0.5f ) * g_P0.x * ( 1.0f - luma * g_P0.w );

	// 全体の明滅.
	float flicker = 1.0f + ( Rand( float2( t, 3.0f ) ) - 0.5f ) * g_P0.z;

	float3 col = saturate( ( src.rgb + grain ) * flicker );
	return FinishColor( src, float4( col, src.a ) );
}

//================================================.
//	スキャンライン.
//	g_P0 : x:線の間隔(px), y:濃さ, z:スクロール速度(px/秒), w:縦線か.
//================================================.
float4 PS_Scanline( VS_OUTPUT i ) : SV_Target
{
	float4 src = g_SrcTexture.Sample( g_SamLinear, i.UV );
	if ( IsOutArea( i.Pos.xy ) ) return src;

	float coord = ( g_P0.w > 0.5f ) ? i.Pos.x : i.Pos.y;
	float s = 0.5f + 0.5f * sin( ( coord + g_Common0.z * g_P0.z ) * PI * 2.0f / max( g_P0.x, 1.0f ) );
	float3 col = src.rgb * ( 1.0f - g_P0.y * s );
	return FinishColor( src, float4( col, src.a ) );
}

//================================================.
//	CRT(ブラウン管風).
//	g_P0 : x:湾曲量, y:走査線の濃さ, z:RGBマスクの濃さ, w:色ずれ量(px).
//	g_P1 : x:四隅の減光量.
//================================================.
float4 PS_CRT( VS_OUTPUT i ) : SV_Target
{
	float4 src = g_SrcTexture.Sample( g_SamLinear, i.UV );
	if ( IsOutArea( i.Pos.xy ) ) return src;

	// 画面の湾曲.
	float2 uv = i.UV * 2.0f - 1.0f;
	float r2 = dot( uv, uv );
	uv *= 1.0f + g_P0.x * 0.25f * r2;
	uv /= 1.0f + g_P0.x * 0.25f;	// 四隅が収まるように正規化する.
	uv = uv * 0.5f + 0.5f;

	// 画面外は黒(透明)にする.
	if ( uv.x < 0.0f || uv.x > 1.0f || uv.y < 0.0f || uv.y > 1.0f ) {
		return FinishColor( src, float4( 0.0f, 0.0f, 0.0f, 0.0f ) );
	}

	// 色ずれ.
	float2 chroma = float2( g_P0.w / max( g_Common0.x, 1.0f ), 0.0f );
	float3 col;
	col.r = g_SrcTexture.Sample( g_SamLinear, uv + chroma ).r;
	col.g = g_SrcTexture.Sample( g_SamLinear, uv ).g;
	col.b = g_SrcTexture.Sample( g_SamLinear, uv - chroma ).b;
	float a = g_SrcTexture.Sample( g_SamLinear, uv ).a;

	// 走査線.
	float scan = 0.5f + 0.5f * sin( uv.y * g_Common0.y * PI );
	col *= 1.0f - g_P0.y * scan;

	// RGBマスク(アパーチャグリル).
	float m = fmod( uv.x * g_Common0.x, 3.0f );
	float3 mask = float3( step( m, 1.0f ), step( 1.0f, m ) * step( m, 2.0f ), step( 2.0f, m ) );
	col = lerp( col, col * ( mask * 2.0f + 0.5f ), g_P0.z );

	// 四隅の減光.
	float2 edge = uv * ( 1.0f - uv );
	float fade = saturate( edge.x * edge.y * 32.0f );
	col *= lerp( 1.0f, fade, g_P1.x );

	return FinishColor( src, float4( saturate( col ), a ) );
}

//================================================.
//	ピクセル化(モザイク).
//	g_P0 : x:1ドットの大きさ(px).
//================================================.
float4 PS_Pixelate( VS_OUTPUT i ) : SV_Target
{
	float4 src = g_SrcTexture.Sample( g_SamLinear, i.UV );
	if ( IsOutArea( i.Pos.xy ) ) return src;

	float size = max( g_P0.x, 1.0f );
	float2 pos = floor( i.Pos.xy / size ) * size + size * 0.5f;
	float4 col = g_SrcTexture.Sample( g_SamLinear, pos / g_Common0.xy );
	return FinishColor( src, col );
}

//================================================.
//	RGB分離(色収差).
//	g_P0 : x:ずらす量(px), y:ずらす方向(度).
//================================================.
float4 PS_RGBShift( VS_OUTPUT i ) : SV_Target
{
	float4 src = g_SrcTexture.Sample( g_SamLinear, i.UV );
	if ( IsOutArea( i.Pos.xy ) ) return src;

	float rad = radians( g_P0.y );
	float2 offset = g_P0.x * float2( cos( rad ), sin( rad ) ) / g_Common0.xy;

	float4 colR = g_SrcTexture.Sample( g_SamLinear, i.UV + offset );
	float4 colB = g_SrcTexture.Sample( g_SamLinear, i.UV - offset );
	float3 col = float3( colR.r, src.g, colB.b );
	float a = max( src.a, max( colR.a, colB.a ) );
	return FinishColor( src, float4( col, a ) );
}

//================================================.
//	グリッチ.
//	g_P0 : x:更新速度(回/秒), y:ブロックの高さ(px), z:ブロックの最大ずれ量(px), w:色ずれ量(px).
//	g_P1 : x:ノイズ線の発生量, y:乱れの発生率.
//================================================.
float4 PS_Glitch( VS_OUTPUT i ) : SV_Target
{
	float4 src = g_SrcTexture.Sample( g_SamLinear, i.UV );
	if ( IsOutArea( i.Pos.xy ) ) return src;

	float t = floor( g_Common0.z * max( g_P0.x, 0.001f ) );
	float row = floor( i.Pos.y / max( g_P0.y, 1.0f ) );

	// ブロック毎に乱れるかを決める.
	float active = step( 1.0f - g_P1.y, Rand( float2( row, t ) ) );

	// ブロックの横ずれ.
	float shift = ( Rand( float2( row, t + 7.0f ) ) - 0.5f ) * 2.0f * g_P0.z * active;
	float2 uv = i.UV + float2( shift / g_Common0.x, 0.0f );

	// 乱れ発生時の色ずれ.
	float2 chroma = float2( g_P0.w * active / g_Common0.x, 0.0f );
	float4 colC = g_SrcTexture.Sample( g_SamLinear, uv );
	float3 col;
	col.r = g_SrcTexture.Sample( g_SamLinear, uv + chroma ).r;
	col.g = colC.g;
	col.b = g_SrcTexture.Sample( g_SamLinear, uv - chroma ).b;
	float a = colC.a;

	// ノイズ線.
	float line1 = step( 1.0f - g_P1.x * 0.05f, Rand( float2( i.Pos.y, t * 3.0f ) ) );
	float lineNoise = Rand( float2( i.Pos.x * 0.1f, i.Pos.y + t ) );
	col = lerp( col, lineNoise.xxx, line1 * active );
	a = max( a, line1 * active );

	return FinishColor( src, float4( col, a ) );
}

//================================================.
//	波歪み.
//	g_P0 : x:横方向の振れ幅(px), y:縦方向の振れ幅(px), z:波の数, w:波の速さ.
//================================================.
float4 PS_Wave( VS_OUTPUT i ) : SV_Target
{
	float4 src = g_SrcTexture.Sample( g_SamLinear, i.UV );
	if ( IsOutArea( i.Pos.xy ) ) return src;

	float t = g_Common0.z * g_P0.w;
	float2 offset = float2(
		sin( i.UV.y * g_P0.z * PI * 2.0f + t ) * g_P0.x,
		sin( i.UV.x * g_P0.z * PI * 2.0f + t * 0.9f ) * g_P0.y ) / g_Common0.xy;
	float4 col = g_SrcTexture.Sample( g_SamLinear, i.UV + offset );
	return FinishColor( src, col );
}

//================================================.
//	シャープ.
//	g_P0 : x:強調の強さ.
//================================================.
float4 PS_Sharpen( VS_OUTPUT i ) : SV_Target
{
	float4 src = g_SrcTexture.Sample( g_SamLinear, i.UV );
	if ( IsOutArea( i.Pos.xy ) ) return src;

	float2 px = 1.0f / g_Common0.xy;
	float3 col = src.rgb * ( 1.0f + 4.0f * g_P0.x );
	col -= g_SrcTexture.Sample( g_SamLinear, i.UV + float2(  px.x, 0.0f ) ).rgb * g_P0.x;
	col -= g_SrcTexture.Sample( g_SamLinear, i.UV + float2( -px.x, 0.0f ) ).rgb * g_P0.x;
	col -= g_SrcTexture.Sample( g_SamLinear, i.UV + float2( 0.0f,  px.y ) ).rgb * g_P0.x;
	col -= g_SrcTexture.Sample( g_SamLinear, i.UV + float2( 0.0f, -px.y ) ).rgb * g_P0.x;
	return FinishColor( src, float4( saturate( col ), src.a ) );
}

//================================================.
//	放射ブラー.
//	g_P0 : x:中心U, y:中心V, z:強さ, w:サンプル数.
//================================================.
float4 PS_RadialBlur( VS_OUTPUT i ) : SV_Target
{
	float4 src = g_SrcTexture.Sample( g_SamLinear, i.UV );
	if ( IsOutArea( i.Pos.xy ) ) return src;

	int samples = clamp( (int)g_P0.w, 2, 24 );
	float4 sum = 0.0f;
	[loop]
	for ( int s = 0; s < samples; ++s ) {
		float rate = g_P0.z * ( (float)s / (float)samples );
		float2 uv = lerp( i.UV, g_P0.xy, rate );
		sum += g_SrcTexture.Sample( g_SamLinear, uv );
	}
	float4 col = sum / (float)samples;
	return FinishColor( src, col );
}
