#pragma once
#include "..\..\..\SystemSetting.h"
#ifdef ENABLE_ANIMATION
#include "..\..\..\Global.h"

/************************************************
*	弾性的な拡縮アニメーションクラス.
*	目標の拡縮値へバネのように追従し, ぷにぷにとした柔らかい動きを演出する.
**/
class CScaleAnimation
{
public:
	CScaleAnimation();

	// 初期化.
	//	Stiffness : バネの硬さ( 大きいほど素早く戻る ).
	//	Damping   : 減衰率  ( 小さいほどよく揺れる, 0.0~1.0 ).
	void Init( const float Stiffness = 240.0f, const float Damping = 0.32f );

	// 更新.
	void Update( const float DeltaTime );

	// 瞬間的に拡縮させ, 弾性で等倍へ戻す( 衝突時などの一瞬の演出用 ).
	void Punch( const float ScaleX, const float ScaleY );

	// 追従先の拡縮値を設定する( 引っ張り時などの連続した演出用 ).
	void SetTarget( const float ScaleX, const float ScaleY );

	// 即座に等倍へ戻す.
	void Reset();

	// 現在の拡縮値を取得.
	const D3DXVECTOR2& GetScale() const { return m_Scale; }

private:
	D3DXVECTOR2	m_Scale;		// 現在の拡縮値.
	D3DXVECTOR2	m_Velocity;		// 拡縮の速度.
	D3DXVECTOR2	m_Target;		// 追従先の拡縮値.
	float		m_Stiffness;	// バネの硬さ.
	float		m_Damping;		// 減衰率.
};

#endif