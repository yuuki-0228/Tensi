#pragma once
#include "..\..\..\SystemSetting.h"
#ifdef ENABLE_ANIMATION
#include "..\..\..\Global.h"

/************************************************
*	ふわふわ浮遊アニメーションクラス.
*	サイン波で上下に揺れ続けるオフセットを生成する.
*	アイコンやキャラクターの待機演出に利用する.
*	Update 後に GetOffset() を描画側の座標へ加算して使う.
**/
class CFloatingAnimation
{
public:
	CFloatingAnimation();

	// 浮遊を開始する.
	//	Amplitude : 揺れ幅( ピクセルなどの単位 ).
	//	Period    : 1往復にかかる秒数.
	//	Phase     : 開始位相のずらし秒数( 複数並べたときの同期ずらし用 ).
	void Start( const float Amplitude = 5.0f, const float Period = 2.0f, const float Phase = 0.0f );

	// 更新.
	void Update( const float DeltaTime );

	// 浮遊を止めてオフセットを0へ戻す.
	void Stop();

	// 現在の浮遊オフセットを取得.
	const D3DXVECTOR2& GetOffset() const { return m_Offset; }

	// 浮遊中かどうか.
	bool IsFloating() const { return m_IsFloating; }

private:
	D3DXVECTOR2	m_Offset;		// 現在の浮遊オフセット.
	float		m_Amplitude;	// 揺れ幅.
	float		m_Period;		// 1往復にかかる秒数.
	float		m_Elapsed;		// 経過秒数.
	bool		m_IsFloating;	// 浮遊中か.
};

#endif
