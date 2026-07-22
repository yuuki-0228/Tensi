#pragma once
#include "..\..\..\SystemSettings.h"
#ifdef ENABLE_ANIMATION
#include "..\..\..\Global.h"
#include "..\..\Easing\Easing.h"

/************************************************
*	スライドアニメーションクラス.
*	画面外などの退避位置と目標位置の間をイージングで移動させる.
*	メニューや通知のスライドイン/アウトに利用する.
*	Update 後に GetPosition() を描画側の座標へ反映して使う.
**/
class CSlideAnimation
{
public:
	CSlideAnimation();

	// スライドインを開始する( 退避位置から目標位置へ ).
	//	Target   : 目標位置.
	//	Offset   : 退避位置への相対オフセット( 例 : 画面外方向 ).
	//	Duration : 到達までの秒数.
	//	Func     : 補間カーブ( 例 : Easing::OutCubic ).
	void SlideIn( const D3DXVECTOR2& Target, const D3DXVECTOR2& Offset,
				  const float Duration = 0.3f, const EasingFunc Func = Easing::OutCubic );

	// スライドアウトを開始する( 目標位置から退避位置へ ).
	//	引数は SlideIn と同じ.
	void SlideOut( const D3DXVECTOR2& Target, const D3DXVECTOR2& Offset,
				   const float Duration = 0.3f, const EasingFunc Func = Easing::InCubic );

	// 更新.
	void Update( const float DeltaTime );

	// 現在の位置を取得.
	const D3DXVECTOR2& GetPosition() const { return m_Position; }

	// 再生中かどうか.
	bool IsPlaying() const { return m_IsPlaying; }

	// 再生が完了したか.
	bool IsFinished() const { return m_IsFinished; }

private:
	// 開始位置から終了位置への移動を開始する.
	void Play( const D3DXVECTOR2& Start, const D3DXVECTOR2& End,
			   const float Duration, const EasingFunc Func );

private:
	D3DXVECTOR2	m_Start;		// 開始位置.
	D3DXVECTOR2	m_End;			// 終了位置.
	D3DXVECTOR2	m_Position;		// 現在の位置.
	float		m_Duration;		// 到達までの秒数.
	float		m_Elapsed;		// 経過秒数.
	EasingFunc	m_Func;			// 補間カーブ.
	bool		m_IsPlaying;	// 再生中か.
	bool		m_IsFinished;	// 完了したか.
};

#endif
