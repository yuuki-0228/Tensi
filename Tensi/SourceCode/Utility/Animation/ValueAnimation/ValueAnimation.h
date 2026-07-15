#pragma once
#include "..\..\..\SystemSetting.h"
#ifdef ENABLE_ANIMATION
#include "..\..\Easing\Easing.h"

/************************************************
*	汎用の値アニメーションクラス( float ).
*	開始値から終了値へ, 指定時間とイージングで補間する.
*	アルファ・回転角・単一の拡縮値など, あらゆる1次元の演出に利用できる.
*	補間カーブには既存の Easing 名前空間の関数をそのまま渡す.
**/

// イージング関数の型( Easing 名前空間の各関数を指す ).
typedef float ( *EasingFunc )( float Time, float TotalTime, float Max, float Min );

// 繰り返しの種類.
enum class ELoop
{
	Once,		// 1回のみ.
	Loop,		// 終端に達したら先頭へ戻して繰り返す.
	PingPong,	// 終端に達したら逆再生し, 往復を繰り返す.
};

class CValueAnimation
{
public:
	CValueAnimation();

	// 再生を開始する.
	//	Start    : 開始値.
	//	End      : 終了値.
	//	Duration : 到達までの秒数.
	//	Func     : 補間カーブ( 例 : Easing::OutQuad ).
	//	Loop     : 繰り返し方.
	void Play( const float Start, const float End, const float Duration,
			   const EasingFunc Func = Easing::OutQuad, const ELoop Loop = ELoop::Once );

	// 更新.
	void Update( const float DeltaTime );

	// 再生を止め, 現在値を保持する.
	void Stop() { m_IsPlaying = false; }

	// 現在の補間値を取得.
	float GetValue() const { return m_Value; }

	// 再生中かどうか.
	bool IsPlaying() const { return m_IsPlaying; }

	// ( Once のとき )再生が完了したか.
	bool IsFinished() const { return m_IsFinished; }

private:
	float		m_Start;		// 開始値.
	float		m_End;			// 終了値.
	float		m_Value;		// 現在値.
	float		m_Duration;		// 到達までの秒数.
	float		m_Elapsed;		// 経過秒数.
	EasingFunc	m_Func;			// 補間カーブ.
	ELoop		m_Loop;			// 繰り返し方.
	bool		m_IsPlaying;	// 再生中か.
	bool		m_IsFinished;	// 完了したか.
};

#endif
