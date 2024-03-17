#pragma once
#include "..\..\Actor.h"

/************************************************
*	睡眠エフェクトクラス.
**/
class CSleepEffect final
	: public CActor
{
public:
	CSleepEffect();
	~CSleepEffect();

	// 初期化.
	virtual bool Init() override;

	// 更新.
	virtual void Update( const float& DeltaTime ) override;

	// 描画.
	virtual void Render() override;

	// アニメーションの開始.
	void Play( const D3DXPOSITION3& Pos );

private:
	CSprite*			m_pSprite;		// 画像.
	SSpriteRenderState	m_SpriteState;	// 画像の情報.
	float				m_NowPoint;		// 現在の位置.
};
