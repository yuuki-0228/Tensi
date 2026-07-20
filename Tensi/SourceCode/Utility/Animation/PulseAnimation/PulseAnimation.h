#pragma once
#include "..\..\..\SystemSetting.h"
#ifdef ENABLE_ANIMATION

/************************************************
*	鼓動アニメーションクラス.
*	呼吸のように拡縮を繰り返すループ演出を生成する.
*	注目してほしいボタンやアイコンの強調に利用する.
*	Update 後に GetScale() を描画側の拡縮へ反映して使う.
**/
class CPulseAnimation
{
public:
	CPulseAnimation();

	// 鼓動を開始する.
	//	MinScale : 最も縮んだときの拡縮値.
	//	MaxScale : 最も膨らんだときの拡縮値.
	//	Period   : 1回の鼓動にかかる秒数.
	void Start( const float MinScale = 1.0f, const float MaxScale = 1.08f, const float Period = 1.2f );

	// 更新.
	void Update( const float DeltaTime );

	// 鼓動を止めて拡縮値を最小値へ戻す.
	void Stop();

	// 現在の拡縮値を取得( 等倍が 1.0 ).
	float GetScale() const { return m_Scale; }

	// 鼓動中かどうか.
	bool IsPulsing() const { return m_IsPulsing; }

private:
	float	m_Scale;		// 現在の拡縮値.
	float	m_MinScale;		// 最も縮んだときの拡縮値.
	float	m_MaxScale;		// 最も膨らんだときの拡縮値.
	float	m_Period;		// 1回の鼓動にかかる秒数.
	float	m_Elapsed;		// 経過秒数.
	bool	m_IsPulsing;	// 鼓動中か.
};

#endif
