#pragma once
#include "..\..\..\SystemSettings.h"
#ifdef ENABLE_FONT
#include "..\FontStruct.h"

// 1文字分の描画スタイル.
struct stFontCharStyle
{
	D3DXCOLOR4	Color;			// 文字色.
	D3DXCOLOR4	OutLineColor;	// アウトラインの色.
	D3DXCOLOR4	GlowColor;		// グローの色.
	D3DXCOLOR4	MarkColor;		// マーカー(背景)の色.
	ETextAlign	Align;			// 行揃え.
	float		SizeScale;		// 文字サイズ倍率.
	float		OutLineSize;	// アウトラインの太さ(px).
	float		GlowSize;		// グローの幅(px).
	float		GlowPower;		// グローの減衰.
	float		CharSpace;		// 追加の字間(px).
	float		VOffset;		// 縦方向のオフセット(px, 正で上方向).
	bool		IsBold;			// 太文字にするか.
	bool		IsItalic;		// イタリック体にするか.
	bool		IsUnderline;	// 下線を引くか.
	bool		IsStrikeout;	// 取り消し線を引くか.
	bool		IsOutLine;		// アウトラインを使用するか.
	bool		IsGlow;			// グローを使用するか.
	bool		IsMark;			// マーカーを使用するか.
} typedef SFontCharStyle;

// タグ解析後の描画アイテム.
struct stFontDrawItem
{
	std::string		Char;		// 描画する文字(空の場合は送りのみ).
	SFontCharStyle	Style;		// 描画スタイル.
	float			Space;		// <space>タグによる追加送り(px).
	bool			IsNewLine;	// 改行アイテムか.
} typedef SFontDrawItem;
using SFontDrawItemList = std::vector<SFontDrawItem>;

/********************************************
*	リッチテキストタグ解析クラス.
*	  TextMeshPro相当のタグを解析して描画アイテムリストを作成する.
*	  対応タグ:
*	    <b> <i> <u> <s> <color=...> <alpha=...> <size=...>
*	    <align=...> <space=...> <cspace=...> <outline=...>
*	    <glow=...> <mark=...> <voffset=...> <sup> <sub>
*	    <br> <noparse>
**/
class CFontTagParser final
{
public:
	// テキストを解析して描画アイテムリストを作成する.
	//	IsRichText が false の場合はタグを解析せずそのまま文字を返す.
	//	BaseCharH は <size=数値> をサイズ倍率へ変換する際の基準の高さ.
	static SFontDrawItemList Parse( const std::string& Text, const SFontRenderState& State, const float BaseCharH );

private:
	// 色文字列(#RRGGBB/#RRGGBBAA/色名)の解析.
	static bool ParseColor( const std::string& Value, D3DXCOLOR4* pOut );
	// 数値文字列の解析.
	static bool ParseFloat( const std::string& Value, float* pOut );
};
#endif // ENABLE_FONT
