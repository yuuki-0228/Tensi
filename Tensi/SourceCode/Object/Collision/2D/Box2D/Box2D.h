#pragma once
#include "..\..\ColliderBase.h"
#include "..\..\..\..\Common\Sprite\Sprite.h"

/**************************************
*	2Dの四角クラス.
*		﨑田友輝.
**/
class CBox2D final
	: public CColliderBase
{
public:
	CBox2D();
	virtual ~CBox2D();

	// 左上の座標を取得.
	D3DXPOSITION2 GetPosition();
	// 幅高さを取得.
	SSize GetSize();

	// スプライトを設定.
	void SetSprite( CSprite* pSprite, SSpriteRenderState* pRenderState );

	// 左上の座標を設定.
	inline void SetPosition( const D3DXPOSITION2& Position ) {
		m_LUPosition = Position;
	}
	// 幅、高さを設定.
	inline void SetSize( const SSize& Size ) {
		m_Size = Size;
	}

private:
	// 左上の座標を取得する.
	D3DXPOSITION2 GetSpriteUpperLeftPos( SSpriteRenderState* pRenderState );

private:
	CSprite*			m_pSprite;		// スプライト.
	SSpriteRenderState* m_pRenderState;	// スプライトの情報.
	D3DXPOSITION2		m_LUPosition;	// 左上座標.
	SSize				m_Size;			// 幅、高さ.
};