#pragma once
#include "..\..\..\SystemSettings.h"
#ifdef ENABLE_ANIMATION

/************************************************
*	点滅アニメーションクラス.
*	サイン波でアルファ値をなめらかに明滅させる.
*	選択中の強調や警告などの注意を引く演出に利用する.
*	Update 後に GetAlpha() を描画側へ反映して使う.
**/
class CBlinkAnimation
{
public:
	CBlinkAnimation();

	// 点滅を開始する.
	//	Period   : 1回の明滅にかかる秒数.
	//	MinAlpha : 最も暗いときのアルファ値.
	//	MaxAlpha : 最も明るいときのアルファ値.
	void Start( const float Period = 0.8f, const float MinAlpha = 0.2f, const float MaxAlpha = 1.0f );

	// 更新.
	void Update( const float DeltaTime );

	// 点滅を止めてアルファ値を最大値へ戻す.
	void Stop();

	// 現在のアルファ値を取得.
	float GetAlpha() const { return m_Alpha; }

	// 点滅中かどうか.
	bool IsBlinking() const { return m_IsBlinking; }

private:
	float	m_Alpha;		// 現在のアルファ値.
	float	m_MinAlpha;		// 最も暗いときのアルファ値.
	float	m_MaxAlpha;		// 最も明るいときのアルファ値.
	float	m_Period;		// 1回の明滅にかかる秒数.
	float	m_Elapsed;		// 経過秒数.
	bool	m_IsBlinking;	// 点滅中か.
};

#endif
