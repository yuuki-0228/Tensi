#pragma once
#include "..\ActorBackGround.h"

/************************************************
*	�؃N���X.
**/
class CTree final
	: public CActorBackGround
{
public:
	CTree();
	~CTree();

	// ������.
	virtual bool Init() override;

	// �X�V.
	virtual void Update( const float& DeltaTime ) override;

	// �`��.
	virtual void SubRender() override;

	// �c�؂𖄂߂�.
	void Fill( const D3DXPOSITION3& Pos );

private:
	CSprite*			m_pSapling;				// �c�؂̉摜.
	CSprite*			m_pSmallTree;			// �������؂̉摜.
	CSprite*			m_pSmallTreeLeaf;		// �������؂̗t���ς̉摜.
	CSprite*			m_pTree;				// �؂̉摜.
	CSprite*			m_pTreeLeaf;			// �؂̗t���ς̉摜.
	SSpriteRenderState	m_SaplingState;			// �c�؂̉摜�̏��.
	SSpriteRenderState	m_SmallTreeState;		// �������؂̉摜�̏��.
	SSpriteRenderState	m_SmallTreeLeafState;	// �������؂̗t���ς̉摜�̏��.
	SSpriteRenderState	m_TreeState;			// �؂̉摜�̏��.
	SSpriteRenderState	m_TreeLeafState;		// �؂̗t���ς̉摜�̏��.
	D3DXCOLOR4			m_LeafColor;			// �t���ς̐F.
	int					m_TreeNo;				// ���݂̖؂̏��.
};