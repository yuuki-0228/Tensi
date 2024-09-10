#pragma once
#include "..\ActorBackGround.h"

/************************************************
*	�ƃN���X.
**/
class CHouse final
	: public CActorBackGround
{
public:
	CHouse();
	~CHouse();

	// ������.
	virtual bool Init() override;

	// �X�V.
	virtual void Update( const float& DeltaTime ) override;

	// �`��.
	virtual void SubRender() override;

	// �ݒu����.
	void Setting( const D3DXPOSITION3& Pos );

private:
	CSprite*			m_pTent;		// �e���g�̉摜.
	SSpriteRenderState	m_TentState;	// �e���g�̉摜�̏��.

};