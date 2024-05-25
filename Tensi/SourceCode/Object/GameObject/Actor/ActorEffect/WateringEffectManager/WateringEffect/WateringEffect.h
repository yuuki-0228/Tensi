#pragma once
#include "..\..\ActorEffect.h"

/************************************************
*	花の水やりエフェクトクラス.
**/
class CWateringEffect final
	: public CActorEffect
{
public:
	CWateringEffect();
	~CWateringEffect();

	// 初期化.
	virtual bool Init() override;

	// 更新.
	virtual void Update( const float& DeltaTime ) override;

	// 描画.
	virtual void Render() override;

	// 再生.
	virtual void Play( const D3DXPOSITION3& Pos, const float Size = 1.0f ) override;

private:
	bool m_IsDispAnim;	// 表示していくか.
};
