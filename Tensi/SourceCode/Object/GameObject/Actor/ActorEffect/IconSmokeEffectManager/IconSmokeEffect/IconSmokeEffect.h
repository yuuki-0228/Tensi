#pragma once
#include "..\..\ActorEffect.h"

/************************************************
*	探索中のアイコンの煙エフェクトクラス( 1つ分 ).
**/
class CIconSmokeEffect final
	: public CActorEffect
{
public:
	CIconSmokeEffect();
	~CIconSmokeEffect();

	// 初期化.
	virtual bool Init() override;

	// 更新.
	virtual void Update( const float& DeltaTime ) override;

	// 描画( メインウィンドウには描画しない ).
	virtual void Render() override {}
	// サブウィンドウ( アイコンの後ろ )に描画.
	virtual void SubRender() override;

	// 再生.
	virtual void Play( const D3DXPOSITION3& Pos, const float Size = 1.0f ) override;

private:
	float m_Size;	// 煙の大きさの倍率.
};
