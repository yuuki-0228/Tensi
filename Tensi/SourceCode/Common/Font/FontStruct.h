#pragma once
#include "..\..\Global.h"
#include "..\Sprite\SpriteStruct.h"
#include "../../Utility/Const/Const.h"

// コンスタントバッファのアプリ側の定義.
//	※シェーダ内のコンスタントバッファと一致している必要あり.
struct stFontShaderConstantBuffer
{
	ALIGN16 D3DXMATRIX	mWVP;				// ワールド,ビュー,プロジェクションの合成変換行列.
	ALIGN16 D3DXCOLOR4	vColor;				// 文字の色(RGBA).
	ALIGN16 D3DXCOLOR4	vOutLineColor;		// アウトラインの色(RGBA).
	ALIGN16 D3DXCOLOR4	vGlowColor;			// グローの色(RGBA).
	ALIGN16 D3DXVECTOR4	vUV;				// UV座標(x,yのみ使用).
	ALIGN16 D3DXVECTOR4	vRenderArea;		// 描画するエリア.
	ALIGN16 D3DXVECTOR4	vViewPort;			// ビューポート(x:幅, y:高さ).
	ALIGN16 D3DXVECTOR4 vFlag;				// フラグ(1以上なら使用する)(x:ディザ抜き)(y:アルファブロック)(z:矩形描画).
	ALIGN16 D3DXVECTOR4 vSDFParam;			// SDF情報(x:アウトライン幅)(y:太字の膨張量)(z:ぼかし量)(w:アウトライン使用).
	ALIGN16 D3DXVECTOR4 vGlowParam;			// グロー情報(x:グロー幅)(y:グローの減衰)(z:グロー使用).
} typedef SFontShaderConstantBuffer;

//スプライト構造体.
struct stFontState
{
	SSize			Disp;				// 表示幅高さ.
	ELocalPosition	LocalPosNo;			// ローカル座標の番号.
} typedef SFontState;

// テキストの表示形式.
enum class enTextAlign : unsigned char
{
	Left,		// 左揃え.
	Center,		// 中央揃え.
	Right		// 右揃え.
} typedef ETextAlign;

// フォントグリフ(1文字分のSDFテクスチャ)構造体.
struct stFontGlyph
{
	ID3D11ShaderResourceView*	pTexture;	// SDFテクスチャ.
	float						CellW;		// 文字セルの幅(px).
	float						CellH;		// 文字セルの高さ(px).
	float						Pad;		// セル周囲の余白(px).
	float						Spread;		// SDFの距離レンジ(px).

	stFontGlyph()
		: pTexture	( nullptr )
		, CellW		( 0.0f )
		, CellH		( 0.0f )
		, Pad		( 0.0f )
		, Spread	( 1.0f )
	{}
} typedef SFontGlyph;

// 描画用画像情報構造体.
struct stFontRenderState
{
	STransform			Transform;			// トランスフォーム.
	std::string			Text;				// 描画するテキスト.
	ESamplerState		SmaplerNo;			// サンプラ番号.
	ELocalPosition		LocalPosNo;			// ローカル座標の番号.
	ETextAlign			TextAlign;			// テキストの表示形式.
	D3DXCOLOR4			Color;				// 色.
	D3DXCOLOR4			OutLineColor;		// アウトラインの色.
	D3DXVECTOR4			RenderArea;			// 描画するエリア( 左上x, 左上y, 幅, 高さ ).
	SSize				TextInterval;		// 文字同士の間隔.
	float				OutLineSize;		// アウトラインの太さ(px).
	bool				IsDisp;				// 表示するか.
	bool				IsOutLine;			// アウトラインを使用するか.
	bool				IsBold;				// 太文字にするか.
	bool				IsItalic;			// イタリック体にするか.
	bool				IsUnderline;		// 下線を引くか.
	bool				IsStrikeout;		// 取り消し線を引くか.
	bool				IsGlow;				// グローを使用するか.
	bool				IsRichText;			// リッチテキストタグを解析するか.
	float				ItalicSlant;		// イタリックの傾き(度).
	float				Softness;			// 縁のぼかし量(px, 0で自動).
	float				BoldWeight;			// 太文字の膨張量(px).
	D3DXCOLOR4			GlowColor;			// グローの色.
	float				GlowSize;			// グローの幅(px).
	float				GlowPower;			// グローの減衰(1で線形).

	stFontRenderState()
		: Transform		()
		, Text			( "" )
		, SmaplerNo		( ESamplerState::Wrap )
		, LocalPosNo	( ELocalPosition::Default )
		, TextAlign		( ETextAlign::Left )
		, Color			( Color4::White )
		, OutLineColor	( Color4::Black )
		, RenderArea	( 0, 0, Const::GameWindow().SIZE.x, Const::GameWindow().SIZE.y )
		, TextInterval	( 0.0f, 0.0f )
		, OutLineSize	( 4.0f )
		, IsDisp		( true )
		, IsOutLine		( false )
		, IsBold		( false )
		, IsItalic		( false )
		, IsUnderline	( false )
		, IsStrikeout	( false )
		, IsGlow		( false )
		, IsRichText	( false )
		, ItalicSlant	( 15.0f )
		, Softness		( 0.0f )
		, BoldWeight	( 3.0f )
		, GlowColor		( Color4::White )
		, GlowSize		( 8.0f )
		, GlowPower		( 1.0f )
	{}

} typedef SFontRenderState;
using SFontRenderStateList = std::vector<SFontRenderState>;
