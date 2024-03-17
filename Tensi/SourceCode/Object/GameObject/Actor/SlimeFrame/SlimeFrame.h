#pragma once
#include "..\Actor.h"

/************************************************
*	スライムフレーム.
**/
class CSlimeFrame final
	: public CActor
{
public:
	CSlimeFrame();
	~CSlimeFrame();

	// 初期化.
	virtual bool Init() override;

	// 更新.
	virtual void Update( const float& DeltaTime ) override;

	// 描画.
	virtual void Render() override;
	virtual void SubRender() override;

private:
	// ウィンドウの選択.
	void SelectWindow();

	// ウィンドウを元に戻す.
	void Reset();

private:
	CSprite*			m_pBack;		// 背景.
	CSprite*			m_pIcon;		// アイコン.
	SSpriteRenderState	m_BackState;	// 背景の情報.
	SSpriteRenderState	m_IconState;	// アイコンの情報.
	HWND				m_hWnd;			// フレームを付けるウィンドウ.
};