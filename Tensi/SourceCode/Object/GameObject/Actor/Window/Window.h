#pragma once
#include "..\Actor.h"
#include "..\..\..\..\Common\Font\Font.h"

/************************************************
*	ウィンドウクラス.
**/
class CWindow final
	: public CActor
{
public:
	CWindow();
	~CWindow();

	// 初期化
	virtual bool Init() override;

	// 更新.
	virtual void Update( const float& DeltaTime ) override;

	// 描画.
	virtual void Render() override {};
	virtual void SubRender() override;

	// 作成
	void Create( const D3DXPOSITION3& pos, const D3DXSCALE2& size, const std::string& title = "", const D3DXCOLOR4& wndColor = Color4::Royalblue, const bool dispCancel = true);
	// 描画関数の設定
	void SetRender( const std::function<void()>& f ) { m_RenderFunction = f; }

private:
	CSprite*			m_pSprite;			// 画像.
	CSprite*			m_pCancelSprite;	// 閉じる画像.
	CFont*				m_pFont;			// フォント
	SSpriteRenderState	m_SpriteState;		// 画像の情報.
	SSpriteRenderState	m_CancelState;		// 閉じる画像の情報.
	SFontRenderState	m_FontState;		// フォントの情報
	stSpriteState		m_SpriteData;		// 画像に関する情報
	stSpriteState		m_CancelData;		// 閉じる画像に関する情報
	D3DXPOSITION3		m_LeftUpPos;		// 左上座標
	D3DXSCALE2			m_Size;				// ウィンドウのサイズ
	D3DXVECTOR4			m_DispAreaPos;		// ウィンドウの描画エリアの座標(x,y:左上, z,w:右下)
	D3DXVECTOR2			m_DispAreaSize;		// ウィンドウの描画エリアサイズ(x:幅, y:高さ)
	bool				m_IsDispCancel;		// 閉じるボタンを表示する
	std::function<void()> m_RenderFunction;	// 描画
};
