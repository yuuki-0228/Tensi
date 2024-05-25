#pragma once
#include "..\..\WindowObject.h"

/************************************************
*	スーパーボールクラス.
**/
class CSuperBall final
	: public CWindowObject
{
public:
	CSuperBall();
	~CSuperBall();

	// 初期化.
	virtual bool Init() override;

	// 更新.
	virtual void Update( const float& DeltaTime ) override;
	// 当たり判定終了後呼び出される更新.
	virtual void LateUpdate( const float& DeltaTime ) override;

	// 描画.
	virtual void Render() override;

	// 当たり判定関数.
	virtual void Collision( CActor* pActor ) override;

protected:
	// 当たり判定の初期化.
	//	Init関数の最後に呼ぶ.
	virtual void InitCollision() override;
	// 当たり判定の更新.
	//	Update関数の最後に呼ぶ.
	virtual void UpdateCollision() override;

	// 上に当たった時の更新.
	virtual void HitUpUpdate() override;
	// 下に当たった時の更新.
	virtual void HitDownUpdate() override;
	// 左に当たった時の更新.
	virtual void HitLeftUpdate() override;
	// 右に当たった時の更新.
	virtual void HitRightUpdate() override;

private:

};