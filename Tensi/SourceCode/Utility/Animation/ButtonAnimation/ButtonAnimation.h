#pragma once
#include "..\..\..\SystemSettings.h"
#ifdef ENABLE_ANIMATION
#include "..\..\..\Global.h"
#include "..\SpringAnimation\SpringAnimation.h"

/************************************************
*	ボタン演出アニメーションクラス.
*	押した瞬間に縮み, 離すとバネで弾みながら戻る.
*	ホバー中は少し拡大して押せることを伝える.
*	Update 後に GetScale() を描画側の拡縮へ反映して使う.
**/
class CButtonAnimation
{
public:
	CButtonAnimation();

	// 初期化.
	//	PressScale : 押下中の縮小率.
	//	HoverScale : ホバー中の拡大率.
	void Init( const float PressScale = 0.9f, const float HoverScale = 1.05f );

	// 更新.
	void Update( const float DeltaTime );

	// 押した瞬間に呼ぶ.
	void Press();

	// 離した瞬間に呼ぶ.
	void Release();

	// ホバー状態を設定する.
	void SetHover( const bool IsHover );

	// 即座に等倍へ戻す.
	void Reset();

	// 現在の拡縮値を取得.
	const D3DXVECTOR2& GetScale() const { return m_Spring.GetScale(); }

	// 押下中かどうか.
	bool IsPressed() const { return m_IsPressed; }

private:
	// 現在の状態に応じた追従先を設定する.
	void UpdateTarget();

private:
	CSpringAnimation	m_Spring;		// バネ挙動の拡縮.
	float				m_PressScale;	// 押下中の縮小率.
	float				m_HoverScale;	// ホバー中の拡大率.
	bool				m_IsPressed;	// 押下中か.
	bool				m_IsHover;		// ホバー中か.
};

#endif
