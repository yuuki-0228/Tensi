#pragma once
#include "..\..\ColliderBase.h"
#include "..\..\..\..\Common\Sprite\Sprite.h"

/**************************************
*	2Dの円クラス.
*		﨑田友輝.
**/
class CSphere2D final
	: public CColliderBase
{
public:
	CSphere2D();
	virtual ~CSphere2D();

	// 左上の座標を取得.
	D3DXPOSITION2 GetPosition();
	// 半径を取得.
	float GetRadius();

	// スプライトを設定.
	void SetSprite( CSprite* pSprite, SSpriteRenderState* pRenderState );

	// 中心の座標を設定.
	inline void SetPosition( const D3DXPOSITION2& Position ) {
		m_CPosition = Position;
	}
	// 半径を設定.
	inline void SetRadius( const float Radius ) {
		m_Radius = Radius;
	}

private:
	// 中心の座標を取得する.
	D3DXPOSITION2 GetSpriteCenterPos( SSpriteRenderState* pRenderState );

private:
	CSprite*			m_pSprite;		// スプライト.
	SSpriteRenderState* m_pRenderState;	// スプライトの情報.
	D3DXPOSITION2		m_CPosition;	// 中心座標.
	float 				m_Radius;		// 半径.
};