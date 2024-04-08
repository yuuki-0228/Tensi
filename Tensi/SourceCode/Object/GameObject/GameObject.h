#pragma once
#include "..\Object.h"
#include "GameObjectList.h"

/************************************************
*	ゲームオブジェクトクラス.
**/
class CGameObject
	: public CObject
{
public:
	CGameObject();
	virtual ~CGameObject();

	// 初期化.
	virtual bool Init() = 0;

	// 更新.
	virtual void Update( const float& DeltaTime ) = 0;
	// 当たり判定終了後呼び出される更新.
	virtual void LateUpdate( const float& DeltaTime ) {}
	// デバックの更新.
	virtual void DebugUpdate( const float& DeltaTime ) {}

	// 描画.
	virtual void Render() = 0;
	virtual void SubRender() {};

	// スプライト(3D)の描画.
	//	_A：後に表示される / _B：先に表示される.
	virtual void Sprite3DRender_A() {}
	virtual void Sprite3DRender_B() {}
	// スプライト(2D)の描画.
	virtual void Sprite2DRender() {}

	// オブジェクトタグの取得.
	EObjectTag GetObjectTag() const { return m_ObjectTag; }

protected:
	float		m_DeltaTime;	// デルタタイム(必要な場合Uptateの最初に代入する).
	EObjectTag	m_ObjectTag;	// オブジェクトタグ.
};