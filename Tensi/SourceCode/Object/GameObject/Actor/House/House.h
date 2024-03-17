#pragma once
#include "..\Actor.h"

/************************************************
*	家クラス.
**/
class CHouse final
	: public CActor
{
public:
	CHouse();
	~CHouse();

	// 初期化.
	virtual bool Init() override;

	// 更新.
	virtual void Update( const float& DeltaTime ) override;

	// 描画.
	virtual void Render() override {};
	virtual void SubRender() override;

	// 設置する.
	void Setting( const D3DXPOSITION3& Pos );

private:
	CSprite*			m_pTent;		// テントの画像.
	SSpriteRenderState	m_TentState;	// テントの画像の情報.

};