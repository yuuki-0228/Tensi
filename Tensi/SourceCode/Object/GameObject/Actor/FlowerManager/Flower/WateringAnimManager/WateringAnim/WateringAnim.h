#pragma once
#include "..\..\..\..\Actor.h"

/************************************************
*	花の水やりエフェクトクラス.
**/
class CWateringAnim final
	: public CActor
{
public:
	CWateringAnim();
	~CWateringAnim();

	// 初期化.
	virtual bool Init() override;

	// 更新.
	virtual void Update( const float& DeltaTime ) override;

	// 描画.
	virtual void Render() override;

	// 再生.
	void Play( const D3DXPOSITION3& Pos );

private:
	CSprite*			m_pSprite;		// 画像.
	SSpriteRenderState	m_SpriteState;	// 画像の情報.
	bool				m_IsDispAnim;	// 表示していくか.
};
