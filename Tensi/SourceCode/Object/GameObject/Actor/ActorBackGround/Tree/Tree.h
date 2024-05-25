#pragma once
#include "..\ActorBackGround.h"

/************************************************
*	木クラス.
**/
class CTree final
	: public CActorBackGround
{
public:
	CTree();
	~CTree();

	// 初期化.
	virtual bool Init() override;

	// 更新.
	virtual void Update( const float& DeltaTime ) override;

	// 描画.
	virtual void SubRender() override;

	// 苗木を埋める.
	void Fill( const D3DXPOSITION3& Pos );

private:
	CSprite*			m_pSapling;				// 苗木の画像.
	CSprite*			m_pSmallTree;			// 小さい木の画像.
	CSprite*			m_pSmallTreeLeaf;		// 小さい木の葉っぱの画像.
	CSprite*			m_pTree;				// 木の画像.
	CSprite*			m_pTreeLeaf;			// 木の葉っぱの画像.
	SSpriteRenderState	m_SaplingState;			// 苗木の画像の情報.
	SSpriteRenderState	m_SmallTreeState;		// 小さい木の画像の情報.
	SSpriteRenderState	m_SmallTreeLeafState;	// 小さい木の葉っぱの画像の情報.
	SSpriteRenderState	m_TreeState;			// 木の画像の情報.
	SSpriteRenderState	m_TreeLeafState;		// 木の葉っぱの画像の情報.
	D3DXCOLOR4			m_LeafColor;			// 葉っぱの色.
	int					m_TreeNo;				// 現在の木の状態.
};