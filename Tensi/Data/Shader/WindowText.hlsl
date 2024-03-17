//ｸﾞﾛｰﾊﾞﾙ変数.
//ﾃｸｽﾁｬは ﾚｼﾞｽﾀ t(n).
Texture2D		g_Texture	: register(t0);
//ｻﾝﾌﾟﾗは ﾚｼﾞｽﾀ s(n).
SamplerState	g_samLinear	: register(s0);

//ｺﾝｽﾀﾝﾄﾊﾞｯﾌｧ.
//ｱﾌﾟﾘ側と同じﾊﾞｯﾌｧｻｲｽﾞになっている必要がある.
cbuffer per_mesh : register(b0)	//ﾚｼﾞｽﾀ番号.
{
	matrix	g_mWVP		: packoffset(c0);	//ﾜｰﾙﾄﾞ,ﾋﾞｭｰ,ﾌﾟﾛｼﾞｪｸｼｮﾝの変換合成行列.
	float4	g_Color		: packoffset(c4);	//色(RGBA:xyzw).
	float	g_ViewPortW : packoffset(c5);	//ﾋﾞｭｰﾎﾟｰﾄ幅.
	float	g_ViewPortH : packoffset(c6);	//ﾋﾞｭｰﾎﾟｰﾄ高さ.
};

//頂点ｼｪｰﾀﾞの出力ﾊﾟﾗﾒｰﾀ.
//頂点ｼｪｰﾀﾞで制御した複数の値をﾋﾟｸｾﾙｼｪｰﾀﾞ側に渡すために用意している.
struct VS_OUTPUT
{
	float4	Pos		: SV_Position;	//座標(SV_:System-Value Semantics).
	float2	UV		: TEXCOORD0;	//UV座標.
};

//頂点ｼｪｰﾀﾞ.
//主にﾓﾃﾞﾙの頂点の表示位置を決定する.
VS_OUTPUT VS_Main(
	float4 Pos	: POSITION,
	float2 UV	: TEXCOORD)
{
	VS_OUTPUT output = (VS_OUTPUT)0;
	output.Pos = mul( Pos, g_mWVP );

	//ｽｸﾘｰﾝ座標に合わせる計算.
	output.Pos.x = (output.Pos.x / g_ViewPortW) * 2.0f - 1.0f;
	output.Pos.y = 1.0f - (output.Pos.y / g_ViewPortH) * 2.0f;

	output.UV = UV;

	return output;
}

//ﾋﾟｸｾﾙｼｪｰﾀﾞ.
//画面上に評されるﾋﾟｸｾﾙ(ﾄﾞｯﾄ1つ分)の色を決定する.
float4 PS_Main( VS_OUTPUT input ) : SV_Target
{
	float4 color = g_Texture.Sample( g_samLinear, input.UV );//色を返す.

	//ﾌﾟﾛｸﾞﾗﾑ制御のα値をﾃｸｽﾁｬが持っているα値にかけ合わせる.
	color *= g_Color;

	return color;
}