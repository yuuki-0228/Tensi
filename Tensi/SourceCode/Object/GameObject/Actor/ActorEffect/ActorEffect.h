#pragma once
#include "../Actor.h"

/************************************************
*	エフェクトクラス.
**/
class CActorEffect
	: public CActor
{
public:
	CActorEffect();
	~CActorEffect();

	// アニメーションの開始
	virtual void Play( const D3DXPOSITION3& Pos, const float Size = 1.0f ) {}
	// アニメーションの停止
	virtual void Stop() { m_IsAnimStop = true; }
	// アニメーションの再開
	virtual void ReStart() { m_IsAnimStop = false; }

	// アニメーションが再生中か
	bool GetIsAnimPlay() { return m_IsAnimEnd == false; }
	// アニメーションの再生が終了したか
	bool GetIsAnimEnd() { return m_IsAnimEnd == true; }

protected:
	// アニメーションの初期化
	void AnimInit();

protected:
	CSprite*			m_pSprite;		// 画像.
	SSpriteRenderState	m_SpriteState;	// 画像の情報.
	bool				m_IsAnimStop;	// 停止中か
	bool				m_IsAnimEnd;	// アニメーションが終了したか
	float				m_NowAnimPoint;	// 現在のアニメーションの位置.

};