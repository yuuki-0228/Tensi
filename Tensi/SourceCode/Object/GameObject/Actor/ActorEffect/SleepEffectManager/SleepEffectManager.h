#pragma once
#include "..\..\Actor.h"

class CSleepEffect;

/************************************************
*	睡眠エフェクトマネージャークラス.
**/
class CSleepEffectManager final
	: public CActor
{
public:
	CSleepEffectManager();
	~CSleepEffectManager();

	// 初期化.
	virtual bool Init() override;

	// 更新.
	virtual void Update( const float& DeltaTime ) override;

	// 描画.
	virtual void Render() override;

	// アニメーションの開始.
	void Play();
	// 一時停止.
	void Pause( const bool Flag );
	// アニメーションの停止.
	void Stop();

private:
	// びっくりエフェクトの再生.
	void SurprisedEffect();
	// 眠りエフェクトの再生.
	void SleepEffectPlay();

	// 眠りエフェクトリストの削除.
	void SleepEffectListDelete();

private:
	CSprite*									m_pSurprisedEffect;		// びっくりエフェクト.
	SSpriteRenderState							m_SurprisedEffectState;	// びっくりエフェクトの情報.
	std::vector<std::unique_ptr<CSleepEffect>>	m_pSleepEffectList;		// 睡眠エフェクトリスト.
	float										m_CoolTime;				// クールタイム.
	float										m_EffectAnimAng;		// びっくりエフェクトアニメーション用の角度.
	bool										m_IsAnimation;			// アニメーション中か.
	bool										m_IsPause;				// ポーズ中か.
};