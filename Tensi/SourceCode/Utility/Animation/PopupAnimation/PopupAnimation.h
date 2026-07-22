#pragma once
#include "..\..\..\SystemSettings.h"
#ifdef ENABLE_ANIMATION
#include "..\..\Easing\Easing.h"

/************************************************
*	ポップアップ開閉アニメーションクラス.
*	開くときはぽよんと弾んで拡大しながらフェードインし,
*	閉じるときは縮小しながらフェードアウトする.
*	Update 後に GetScale() / GetAlpha() を描画側へ反映して使う.
**/

// ポップアップの状態.
enum class EPopupState
{
	Closed,		// 閉じている.
	Opening,	// 開いている最中.
	Opened,		// 開いている.
	Closing,	// 閉じている最中.
};

class CPopupAnimation
{
public:
	CPopupAnimation();

	// 初期化.
	//	StartScale : 開閉時の縮小側の拡縮値.
	//	Back       : 開くときの弾み量( 大きいほど大きく弾む ).
	void Init( const float StartScale = 0.8f, const float Back = 1.7f );

	// 開く.
	//	Duration : 開き切るまでの秒数.
	void Open( const float Duration = 0.25f );

	// 閉じる.
	//	Duration : 閉じ切るまでの秒数.
	void Close( const float Duration = 0.18f );

	// 更新.
	void Update( const float DeltaTime );

	// 現在の拡縮値を取得( 等倍が 1.0 ).
	float GetScale() const { return m_Scale; }

	// 現在のアルファ値を取得( 0.0~1.0 ).
	float GetAlpha() const { return m_Alpha; }

	// 現在の状態を取得.
	EPopupState GetState() const { return m_State; }

	// 完全に閉じているか( 非表示にする判定に使う ).
	bool IsClosed() const { return m_State == EPopupState::Closed; }

	// 開閉の最中かどうか.
	bool IsPlaying() const { return m_State == EPopupState::Opening || m_State == EPopupState::Closing; }

private:
	EPopupState	m_State;		// 現在の状態.
	float		m_Elapsed;		// 経過秒数.
	float		m_Duration;		// 開閉にかける秒数.
	float		m_StartScale;	// 縮小側の拡縮値.
	float		m_Back;			// 開くときの弾み量.
	float		m_Scale;		// 現在の拡縮値.
	float		m_Alpha;		// 現在のアルファ値.
};

#endif
