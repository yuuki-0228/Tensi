//ｸﾞﾛｰﾊﾞﾙ変数.
//ﾃｸｽﾁｬは ﾚｼﾞｽﾀ t(n).
Texture2D		g_Texture	    : register(t0);
Texture2D       g_MaskTexture   : register(t1);
Texture2D		g_RuleTexture	: register(t2);
//ｻﾝﾌﾟﾗは ﾚｼﾞｽﾀ s(n).
SamplerState	g_samLinear	    : register(s0);

//ｺﾝｽﾀﾝﾄﾊﾞｯﾌｧ.
//ｱﾌﾟﾘ側と同じﾊﾞｯﾌｧｻｲｽﾞになっている必要がある.
cbuffer per_mesh : register(b0)	//ﾚｼﾞｽﾀ番号.
{
	matrix	g_mWVP				: packoffset(c0);	//ﾜｰﾙﾄﾞ,ﾋﾞｭｰ,ﾌﾟﾛｼﾞｪｸｼｮﾝの変換合成行列.
	float4	g_Color				: packoffset(c4);	//色(RGBA:xyzw).
	float4	g_UV				: packoffset(c5);	//UV座標(xyしか使わない).
	float4	g_RenderArea		: packoffset(c6);	//描画する範囲(左上x, 左上y, 幅, 高さ).
	float	g_ViewPortW			: packoffset(c7);	//ﾋﾞｭｰﾎﾟｰﾄ幅.
	float	g_ViewPortH			: packoffset(c8);	//ﾋﾞｭｰﾎﾟｰﾄ高さ.
	float4	g_vFlag				: packoffset(c9);	//ﾌﾗｸﾞ(x:ﾃﾞｲｻﾞ抜きを使用するか)(y:ｱﾙﾌｧﾌﾞﾛｯｸを使用するか)(z:ｶﾗｰにﾏｽｸを使用するか)(w:高品質ｻﾝﾌﾟﾘﾝｸﾞを使用するか).
};

//頂点ｼｪｰﾀﾞの出力ﾊﾟﾗﾒｰﾀ.
//頂点ｼｪｰﾀﾞで制御した複数の値をﾋﾟｸｾﾙｼｪｰﾀﾞ側に渡すために用意している.
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

//Catmull-Rom の重みを求める(t:ﾃｸｾﾙ中心からの距離).
float4 CatmullRomWeights( float t )
{
	//4ﾃｸｾﾙ分の重み(合計1になる).
	float t2 = t * t;
	float t3 = t2 * t;
	float4 w;
	w.x = -0.5f * t3 + 1.0f * t2 - 0.5f * t;
	w.y =  1.5f * t3 - 2.5f * t2 + 1.0f;
	w.z = -1.5f * t3 + 2.0f * t2 + 0.5f * t;
	w.w =  0.5f * t3 - 0.5f * t2;
	return w;
}

//高品質ｻﾝﾌﾟﾘﾝｸﾞ(Catmull-Rom ﾊﾞｲｷｭｰﾋﾞｯｸ 16ﾀｯﾌﾟ).
//拡大時にﾊﾞｲﾘﾆｱよりｼｬｰﾌﾟで滑らかな結果になる.
//各ﾀｯﾌﾟをｱﾙﾌｧで重み付け(ﾌﾟﾘﾏﾙﾁﾌﾟﾗｲﾄﾞ)して合成することで、
//透明ﾋﾟｸｾﾙの色が混ざって縁が黒ずむのも防ぐ.
float4 SampleTextureHQ( Texture2D tex, float2 uv )
{
	//ﾃｸｽﾁｬｻｲｽﾞの取得.
	float2 texSize;
	tex.GetDimensions( texSize.x, texSize.y );

	//基準ﾃｸｾﾙ(左上から2番目)と小数部を求める.
	float2 samplePos	= uv * texSize - 0.5f;
	float2 basePos		= floor( samplePos );
	float2 f			= samplePos - basePos;

	//縦横それぞれの重み.
	float4 wx = CatmullRomWeights( f.x );
	float4 wy = CatmullRomWeights( f.y );

	//4x4ﾃｸｾﾙを合成する.
	float4 result = (float4)0;
	[unroll]
	for ( int y = 0; y < 4; ++y ) {
		[unroll]
		for ( int x = 0; x < 4; ++x ) {
			float2 tapUV	= ( basePos + float2( x - 1, y - 1 ) + 0.5f ) / texSize;
			float4 tapColor	= tex.SampleLevel( g_samLinear, tapUV, 0 );

			//ﾌﾟﾘﾏﾙﾁﾌﾟﾗｲﾄﾞ化して重み付け.
			tapColor.rgb *= tapColor.a;
			result += tapColor * ( wx[x] * wy[y] );
		}
	}

	//ｽﾄﾚｰﾄｱﾙﾌｧに戻す(ｵｰﾊﾞｰｼｭｰﾄはｸﾗﾝﾌﾟする).
	result.rgb /= max( result.a, 0.0001f );
	return saturate( result );
}

//ｽﾌﾟﾗｲﾄの色の取得.
//高品質ｻﾝﾌﾟﾘﾝｸﾞﾌﾗｸﾞが有効な場合は Catmull-Rom で、無効な場合はﾊﾞｲﾘﾆｱでｻﾝﾌﾟﾘﾝｸﾞする.
float4 SampleSpriteColor( float2 uv )
{
	if ( g_vFlag.w >= 1.0f ) return SampleTextureHQ( g_Texture, uv );
	return g_Texture.Sample( g_samLinear, uv );
}

//頂点ｼｪｰﾀﾞ.
//主にﾓﾃﾞﾙの頂点の表示位置を決定する.
VS_OUTPUT VS_Main(
	float4 Pos	: POSITION,
	float2 UV	: TEXCOORD)
{
	VS_OUTPUT output = (VS_OUTPUT)0;
	output.Pos = mul( Pos, g_mWVP );
	
	//ｽｸﾘｰﾝ座標に合わせる計算.
	if ( g_ViewPortW > 0 && g_ViewPortH > 0 ) {
		output.Pos.x = ( output.Pos.x / g_ViewPortW ) * 2.0f - 1.0f;
		output.Pos.y = 1.0f - ( output.Pos.y / g_ViewPortH ) * 2.0f;
	}
	output.UV = UV;

	//UVｽｸﾛｰﾙ(UV座標を操作する).
	output.UV.x += g_UV.x;
	output.UV.y += g_UV.y;

	return output;
}

//ﾋﾟｸｾﾙｼｪｰﾀﾞ(マスクを使用しない).
//画面上に評されるﾋﾟｸｾﾙ(ﾄﾞｯﾄ1つ分)の色を決定する.
float4 PS_Main( VS_OUTPUT input ) : SV_Target
{
	float4 finalColor = SampleSpriteColor( input.UV ); //色を返す.

	//ﾌﾟﾛｸﾞﾗﾑ制御の色をかけ合わせる.
	if (g_vFlag.z >= 1.0f){
		float4 maskColor = g_MaskTexture.Sample(g_samLinear, input.UV);
		finalColor.rgb *= (1.0f - (1.0f - g_Color.rgb)*maskColor.r);
	}
	else{
		finalColor.rgba *= g_Color.rgba;
	}
	
	// 描画するエリア外の場合透明にする.
	if ( input.Pos.x < g_RenderArea.x || input.Pos.x > g_RenderArea.x + g_RenderArea.z ||
		 input.Pos.y < g_RenderArea.y || input.Pos.y > g_RenderArea.y + g_RenderArea.w )
	{
		finalColor.a = 0.0f;
	}
	
	// アルファブロック計算.
	if (g_vFlag.y >= 1.0f){
		clip(finalColor.a - 0.0001f);
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

//ﾋﾟｸｾﾙｼｪｰﾀﾞ(マスクテクスチャを使用する).
//画面上に評されるﾋﾟｸｾﾙ(ﾄﾞｯﾄ1つ分)の色を決定する.
float4 PS_Mask(VS_OUTPUT input) : SV_Target
{
	float4 finalColor = SampleSpriteColor( input.UV ); //色を返す.
	float4 maskColor  = g_MaskTexture.Sample(g_samLinear, input.UV - g_UV.xy);

	// 色を掛け合わせる.
	finalColor.a *= 1.0f - maskColor.r;
	finalColor.rgb *= g_Color.rgb;
	
	// 描画するエリア外の場合透明にする.
	if ( input.Pos.x < g_RenderArea.x || input.Pos.x > g_RenderArea.x + g_RenderArea.z ||
		 input.Pos.y < g_RenderArea.y || input.Pos.y > g_RenderArea.y + g_RenderArea.w )
	{
		finalColor.a = 0.0f;
	}
	
	// アルファブロック計算.
	if (g_vFlag.y >= 1.0f){
		clip(finalColor.a - 0.0001f);
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

//ﾋﾟｸｾﾙｼｪｰﾀﾞ(ルールテクスチャを使用する).
//画面上に評されるﾋﾟｸｾﾙ(ﾄﾞｯﾄ1つ分)の色を決定する.
float4 PS_Transition(VS_OUTPUT input) : SV_Target
{
	float4 maskColor = g_RuleTexture.Sample(g_samLinear, input.UV);
	float4 texColor  = SampleSpriteColor( input.UV );
	
	// マスクを使ったアルファ抜き.
	float  maskAlpha  = saturate(maskColor.r + (g_Color.a * 2.0f - 1.0f));
	float4 finalColor = texColor * maskAlpha + texColor * (1 - texColor.a);
	
	// 色をかけ合わせる.
	if (g_vFlag.z >= 1.0f){
		finalColor.rgb *= (1.0f - (1.0f - g_Color.rgb) * maskColor.r);
	}
	else{
		finalColor.rgba *= g_Color.rgba;
	}
	
	// 描画するエリア外の場合透明にする.
	if ( input.Pos.x < g_RenderArea.x || input.Pos.x > g_RenderArea.x + g_RenderArea.z ||
		 input.Pos.y < g_RenderArea.y || input.Pos.y > g_RenderArea.y + g_RenderArea.w )
	{
		finalColor.a = 0.0f;
	}
	
	// アルファブロック計算.
	if (g_vFlag.y >= 1.0f){
		clip(finalColor.a - 0.0001f);
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