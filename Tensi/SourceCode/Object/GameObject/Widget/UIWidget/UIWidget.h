#pragma once
#include "..\Widget.h"

/************************************************
*	UI�x�[�X�N���X.
*		���c�F�P.
**/
class CUIWidget
	: public CWidget
{
public:
	using pState_map = std::unordered_map<std::string, std::vector<SSpriteRenderState*>>;

public:
	CUIWidget();
	virtual ~CUIWidget();

	// �X�v���C�g���X�g�̎擾.
	void SetSpriteList( const Sprite_map& pState ) { m_pSpriteList = pState; }
	// �X�v���C�g���̎擾.
	void SetSpriteState( State_map& pState );

protected:
	Sprite_map	m_pSpriteList;	// �X�v���C�g���X�g.
	pState_map	m_pSpriteState;	// �X�v���C�g���.
};
