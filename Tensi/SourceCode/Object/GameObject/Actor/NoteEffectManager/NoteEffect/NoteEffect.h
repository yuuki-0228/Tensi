#pragma once
#include "..\..\Actor.h"

/************************************************
*	音符エフェクト.
**/
class CNoteEffect final
	: public CActor
{
public:
	CNoteEffect();
	~CNoteEffect();

	// 初期化.
	virtual bool Init() override;

	// 更新.
	virtual void Update( const float& DeltaTime ) override;

	// 描画.
	virtual void Render() override;

	// アニメーションの開始.
	void Play( const D3DXPOSITION3& Pos, const float Size );

private:
	CSprite*			m_pSprite;		// 画像.
	SSpriteRenderState	m_SpriteState;	// 画像の情報.
	float				m_NowPoint;		// 現在の位置.

};