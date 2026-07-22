#pragma once
#include "..\..\..\SystemSettings.h"
#ifdef ENABLE_ANIMATION
#include "..\..\Easing\Easing.h"

/************************************************
*	フェードアニメーションクラス.
*	アルファ値のフェードイン/フェードアウトを行う.
*	画面遷移や UI の表示切替に利用する.
*	Update 後に GetAlpha() を描画側へ反映して使う.
**/
class CFadeAnimation
{
public:
	CFadeAnimation();

	// フェードインを開始する( 現在のアルファ値から 1.0 へ ).
	//	Duration : 到達までの秒数.
	//	Func     : 補間カーブ( 例 : Easing::OutQuad ).
	void FadeIn( const float Duration = 0.3f, const EasingFunc Func = Easing::OutQuad );

	// フェードアウトを開始する( 現在のアルファ値から 0.0 へ ).
	//	Duration : 到達までの秒数.
	//	Func     : 補間カーブ( 例 : Easing::OutQuad ).
	void FadeOut( const float Duration = 0.3f, const EasingFunc Func = Easing::OutQuad );

	// アルファ値を即座に設定する.
	void SetAlpha( const float Alpha );

	// 更新.
	void Update( const float DeltaTime );

	// 現在のアルファ値を取得( 0.0~1.0 ).
	float GetAlpha() const { return m_Alpha; }

	// 再生中かどうか.
	bool IsPlaying() const { return m_IsPlaying; }

	// 再生が完了したか.
	bool IsFinished() const { return m_IsFinished; }

private:
	// 指定のアルファ値へのフェードを開始する.
	void Play( const float End, const float Duration, const EasingFunc Func );

private:
	float		m_Start;		// 開始アルファ値.
	float		m_End;			// 終了アルファ値.
	float		m_Alpha;		// 現在のアルファ値.
	float		m_Duration;		// 到達までの秒数.
	float		m_Elapsed;		// 経過秒数.
	EasingFunc	m_Func;			// 補間カーブ.
	bool		m_IsPlaying;	// 再生中か.
	bool		m_IsFinished;	// 完了したか.
};

#endif
