#pragma once
#include "..\..\Actor.h"

class CNoteEffect;

/************************************************
*	音符エフェクトマネージャー.
**/
class CNoteEffectManager final
	: public CActor
{
public:
	CNoteEffectManager();
	~CNoteEffectManager();

	// 初期化.
	virtual bool Init() override;

	// 更新.
	virtual void Update( const float& DeltaTime ) override;

	// 描画.
	virtual void Render() override;

	// アニメーションの開始.
	void Play( const float Size );
	// 一時停止.
	void Pause( const bool Flag );
	// アニメーションの停止.
	void Stop();

private:
	// 音符エフェクトの再生.
	void NoteEffectPlay();

	// 音符エフェクトリストの削除.
	void NoteEffectListDelete();

private:
	std::vector<std::unique_ptr<CNoteEffect>>	m_pNoteEffectList;	// 音符エフェクトリスト.
	float										m_CoolTime;			// クールタイム.
	float										m_EffectSize;		// エフェクトのサイズ.
	bool										m_IsAnimation;		// アニメーション中か.
	bool										m_IsPause;			// ポーズ中か.
};