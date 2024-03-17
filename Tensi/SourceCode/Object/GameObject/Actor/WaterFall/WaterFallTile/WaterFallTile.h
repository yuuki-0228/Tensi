#pragma once
#include "..\..\Actor.h"

/************************************************
*	滝のタイルクラス.
**/
class CWaterFallTile final
	: public CActor
{
public:
	CWaterFallTile();
	~CWaterFallTile();

	// 初期化.
	virtual bool Init() override;

	// 更新.
	virtual void Update( const float& DeltaTime ) override;

	// 描画.
	virtual void Render() override {};
	virtual void SubRender() override;

	// 当たり判定関数.
	virtual void Collision( CActor* pActor ) override;

	// タイルの設定.
	void Setting( const D3DXPOSITION3& Pos, const bool IsWater );

protected:
	// 当たり判定の初期化.
	//	Init関数の最後に呼ぶ.
	virtual void InitCollision() override;
	// 当たり判定の更新.
	//	Update関数の最後に呼ぶ.
	virtual void UpdateCollision() override;

private:
	CSprite*			m_pWaterFall;			// 滝の画像.
	SSpriteRenderState	m_WaterFallState;		// 滝の画像の情報.
	SSpriteRenderState	m_SplashState;			// 水しぶきの画像の情報.
	float				m_ImageSize;			// 画像サイズ.

};