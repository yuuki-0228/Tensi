#pragma once
#include "..\..\..\SystemSetting.h"
#ifdef ENABLE_ANIMATION
#include "..\..\..\Global.h"
#include "..\ValueAnimation\ValueAnimation.h"

/************************************************
*	汎用の2D値アニメーションクラス( D3DXVECTOR2 ).
*	開始ベクトルから終了ベクトルへ, 指定時間とイージングで補間する.
*	位置移動や2軸の拡縮など, 2次元の演出に利用できる.
*	内部でX/Yそれぞれに CValueAnimation を用いる.
**/
class CVector2Animation
{
public:
	CVector2Animation();

	// 再生を開始する.
	//	Start    : 開始ベクトル.
	//	End      : 終了ベクトル.
	//	Duration : 到達までの秒数.
	//	Func     : 補間カーブ( 例 : Easing::OutQuad ).
	//	Loop     : 繰り返し方.
	void Play( const D3DXVECTOR2& Start, const D3DXVECTOR2& End, const float Duration,
			   const EasingFunc Func = Easing::OutQuad, const ELoop Loop = ELoop::Once );

	// 更新.
	void Update( const float DeltaTime );

	// 再生を止め, 現在値を保持する.
	void Stop();

	// 現在の補間値を取得.
	const D3DXVECTOR2& GetValue() const { return m_Value; }

	// 再生中かどうか.
	bool IsPlaying() const { return m_X.IsPlaying() || m_Y.IsPlaying(); }

	// ( Once のとき )再生が完了したか.
	bool IsFinished() const { return m_X.IsFinished() && m_Y.IsFinished(); }

private:
	CValueAnimation	m_X;		// X軸の補間.
	CValueAnimation	m_Y;		// Y軸の補間.
	D3DXVECTOR2		m_Value;	// 現在値.
};

#endif
