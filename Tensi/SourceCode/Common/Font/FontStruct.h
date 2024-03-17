#pragma once
#include "..\..\Global.h"
#include "..\Sprite\SpriteStruct.h"

// コンスタントバッファのアプリ側の定義.
//	※シェーダ内のコンスタントバッファと一致している必要あり.
struct stFontShaderConstantBuffer
{
	ALIGN16 D3DXMATRIX	mWVP;				// ワールド,ビュー,プロジェクションの合成変換行列.	
	ALIGN16 D3DXCOLOR4	vColor;				// カラー(RGBAの型に合わせる).
	ALIGN16 D3DXVECTOR4	vUV;				// UV座標(x,yのみ使用).
	ALIGN16 D3DXVECTOR4	vRenderArea;		// 描画するエリア.
	ALIGN16 float		fViewPortWidth;		// ビューポート幅.
	ALIGN16 float		fViewPortHeight;	// ビューポート高さ.
	ALIGN16 D3DXVECTOR4 vDitherFlag;		// ディザ抜きを使用するか(xが1以上なら使用する).
	ALIGN16 D3DXVECTOR4 vAlphaBlockFlag;	// アルファブロックを使用するか(xが1以上なら使用する).
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

// 描画用画像情報構造体.
struct stFontRenderState
{
	STransform			Transform;			// トランスフォーム.
	std::string			Text;				// 描画するテキスト.
	ESamplerState		SmaplerNo;			// サンプラ番号.
	ETextAlign			TextAlign;			// テキストの表示形式.
	D3DXCOLOR4			Color;				// 色.
	D3DXCOLOR4			OutLineColor;		// アウトラインの色.
	D3DXVECTOR4			RenderArea;			// 描画するエリア( 左上x, 左上y, 幅, 高さ ).
	SSize				TextInterval;		// 文字通しの間隔.
	float				OutLineSize;		// アウトラインの太さ.
	bool				IsDisp;				// 表示するか.
	bool				IsOutLine;			// アウトラインを使用するか.
	bool				IsBold;				// 太文字にするか.

	stFontRenderState()
		: Transform		()
		, Text			( "" )
		, SmaplerNo		( ESamplerState::Wrap )
		, TextAlign		( ETextAlign::Left )
		, Color			( Color4::White )
		, OutLineColor	( Color4::Black )
		, RenderArea	( 0, 0, WND_W, WND_H )
		, TextInterval	( 0.0f, 0.0f )
		, OutLineSize	( 4.0 )
		, IsDisp		( true )
		, IsOutLine		( false )
		, IsBold		( false )
	{}

} typedef SFontRenderState;
using SFontRenderStateList = std::vector<SFontRenderState>;