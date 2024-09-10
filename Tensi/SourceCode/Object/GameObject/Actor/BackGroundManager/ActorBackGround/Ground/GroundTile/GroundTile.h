#pragma once
#include "..\..\ActorBackGround.h"

/************************************************
*	地面タイルクラス.
**/
class CGroundTile
	: public CActorBackGround
{
public:
	CGroundTile();
	~CGroundTile();

	// 初期化.
	virtual bool Init() override;

	// 更新.
	virtual void Update( const float& DeltaTime ) override;

	// 描画.
	virtual void SubRender() override;

	// 地面の設置
	void Setting( const D3DXPOSITION3& Pos );

private:
	CSprite*			m_pGround;		// 地面の画像.
	SSpriteRenderState	m_GroundState;	// 地面の画像の情報.


};