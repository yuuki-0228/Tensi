#pragma once
#include "..\..\Actor.h"

class CIconSmokeEffect;

/************************************************
*	探索中のアイコンの煙エフェクトのマネージャークラス.
**/
class CIconSmokeEffectManager final
	: public CActor
{
public:
	CIconSmokeEffectManager();
	~CIconSmokeEffectManager();

	// 初期化.
	virtual bool Init() override;

	// 更新.
	virtual void Update( const float& DeltaTime ) override;

	// 描画( メインウィンドウには描画しない ).
	virtual void Render() override {}
	// サブウィンドウ( アイコンの後ろ )に描画.
	virtual void SubRender() override;

	// 煙の発生位置の設定.
	void SetPosition( const D3DXPOSITION3& Pos ) { m_Position = Pos; }
	// 煙の大きさの倍率の設定.
	void SetSize( const float Size ) { m_Size = Size; }
	// 煙の発生位置のランダムずれ幅(px)の設定.
	void SetSpread( const float Spread ) { m_Spread = Spread; }

	// 煙の発生を開始する.
	void Play();
	// 煙の発生を停止する( 発生済みの煙は消えるまで残る ).
	void Stop();

private:
	// 煙を1つ発生させる.
	void SpawnSmoke();

private:
	std::vector<std::unique_ptr<CIconSmokeEffect>>	m_pSmokeList;		// 煙エフェクトのリスト.
	D3DXPOSITION3									m_Position;			// 煙の発生位置.
	float											m_SpawnCoolTime;	// 次の煙を発生させるまでの時間.
	float											m_Size;				// 煙の大きさの倍率.
	float											m_Spread;			// 煙の発生位置のランダムずれ幅(px).
	bool											m_IsPlay;			// 煙を発生させ続けるか.
};
