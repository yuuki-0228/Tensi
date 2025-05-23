#pragma once
#include "..\..\ActorEffect.h"

/************************************************
*	睡眠エフェクトクラス.
**/
class CSleepEffect final
	: public CActorEffect
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
	virtual void Play( const D3DXPOSITION3& Pos, const float Size = 1.0f ) override;

private:

};
