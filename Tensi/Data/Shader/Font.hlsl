//グローバル変数.
//テクスチャは レジスタ t(n).
Texture2D		g_Texture	    : register(t0);	//SDFテクスチャ(rに符号付き距離、0.5が輪郭).
//サンプラは レジスタ s(n).
SamplerState	g_samLinear	    : register(s0);

//コンスタントバッファ.
//アプリ側と同じバッファサイズになっている必要がある.
cbuffer per_mesh : register(b0)	//レジスタ番号.
{
	matrix	g_mWVP				: packoffset(c0);	//ワールド,ビュー,プロジェクションの変換合成行列.
	float4	g_Color				: packoffset(c4);	//文字の色(RGBA:xyzw).
	float4	g_OutLineColor		: packoffset(c5);	//アウトラインの色(RGBA:xyzw).
	float4	g_GlowColor			: packoffset(c6);	//グローの色(RGBA:xyzw).
	float4	g_UV				: packoffset(c7);	//UV座標(xyしか使わない).
	float4	g_RenderArea		: packoffset(c8);	//描画する範囲(左上x, 左上y, 幅, 高さ).
	float4	g_ViewPort			: packoffset(c9);	//ビューポート(x:幅, y:高さ).
	float4	g_Flag				: packoffset(c10);	//フラグ(x:ディザ抜き)(y:アルファブロック)(z:矩形描画).
	float4	g_SDFParam			: packoffset(c11);	//SDF情報(x:アウトライン幅)(y:太字の膨張量)(z:ぼかし量)(w:アウトライン使用).
	float4	g_GlowParam			: packoffset(c12);	//グロー情報(x:グロー幅)(y:グローの減衰)(z:グロー使用).
};

//頂点シェーダの出力パラメータ.
//頂点シェーダで制御した複数の値をピクセルシェーダ側に渡すために用意している.
struct VS_OUTPUT
{
	float4	Pos		: SV_Position;	//座標(SV_:System-Value Semantics).
	float2	UV		: TEXCOORD0;	//UV座標.
};

static const float DITHER_PATTERN[4][4] =
{
	{ 0.00f, 0.32f, 0.08f, 0.40f, },
	{ 0.48f, 0.16f, 0.56f, 0.24f, },
	{ 0.12f, 0.44f, 0.04f, 0.36f, },
	{ 0.60f, 0.28f, 0.52f, 0.20f, },
};

//頂点シェーダ.
//主にモデルの頂点の表示位置を決定する.
VS_OUTPUT VS_Main(
	float4 Pos	: POSITION,
	float2 UV	: TEXCOORD)
{
	VS_OUTPUT output = (VS_OUTPUT)0;
	output.Pos = mul( Pos, g_mWVP );

	//スクリーン座標に合わせる計算.
	if ( g_ViewPort.x > 0 && g_ViewPort.y > 0 ) {
		output.Pos.x = ( output.Pos.x / g_ViewPort.x ) * 2.0f - 1.0f;
		output.Pos.y = 1.0f - ( output.Pos.y / g_ViewPort.y ) * 2.0f;
	}
	output.UV = UV;

	//UVスクロール(UV座標を操作する).
	output.UV.x += g_UV.x;
	output.UV.y += g_UV.y;

	return output;
}

//ピクセルシェーダ.
//SDF(符号付き距離場)から文字・アウトライン・グローを1ドローで描画する.
float4 PS_Main( VS_OUTPUT input ) : SV_Target
{
	float4 finalColor = g_Color;

	//矩形描画(下線・取り消し線など)以外はSDFから色を求める.
	if ( g_Flag.z < 1.0f ) {
		//SDF値(0.5が文字の輪郭).
		float dist = g_Texture.Sample( g_samLinear, input.UV ).r;

		//縁のぼかし量(0の場合は画面上での1ピクセル相当から自動計算).
		float soft = max( g_SDFParam.z, fwidth( dist ) * 0.75f );

		//太字は輪郭のしきい値を外側へ広げる.
		float faceEdge	= 0.5f - g_SDFParam.y;
		float faceAlpha	= smoothstep( faceEdge - soft, faceEdge + soft, dist );
		finalColor = float4( g_Color.rgb, g_Color.a * faceAlpha );

		//アウトライン.
		float edge = faceEdge;
		if ( g_SDFParam.w >= 1.0f ) {
			edge = faceEdge - g_SDFParam.x;
			float outLineAlpha = smoothstep( edge - soft, edge + soft, dist );
			finalColor.rgb	= lerp( g_OutLineColor.rgb, g_Color.rgb, faceAlpha );
			finalColor.a	= max( g_Color.a * faceAlpha, g_OutLineColor.a * outLineAlpha );
		}

		//グロー(輪郭の外側に向かって減衰する光).
		if ( g_GlowParam.z >= 1.0f ) {
			float glow = smoothstep( edge - g_GlowParam.x, edge, dist );
			glow = pow( saturate( glow ), max( g_GlowParam.y, 0.001f ) );
			finalColor.rgb	= lerp( g_GlowColor.rgb, finalColor.rgb, finalColor.a );
			finalColor.a	= max( finalColor.a, g_GlowColor.a * glow );
		}
	}

	// 描画するエリア外の場合透明にする.
	if ( input.Pos.x < g_RenderArea.x || input.Pos.x > g_RenderArea.x + g_RenderArea.z ||
		 input.Pos.y < g_RenderArea.y || input.Pos.y > g_RenderArea.y + g_RenderArea.w )
	{
		finalColor.a = 0.0f;
	}

	// アルファブロック計算.
	if ( g_Flag.y >= 1.0f ){
		clip(finalColor.a - 0.0001f);
	}

	// ディザ抜き計算.
	if ( g_Flag.x >= 1.0f ) {
		const int pt_x = (int) fmod( input.Pos.x, 4.0f );
		const int pt_y = (int) fmod( input.Pos.y, 4.0f );
		const float dither = DITHER_PATTERN[pt_y][pt_x];
		clip( finalColor.a - dither );
	}

	return finalColor;
}
