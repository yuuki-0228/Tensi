#pragma once
#include "..\..\ActorBackGround.h"

/************************************************
*	�n�ʃ^�C���N���X.
**/
class CGroundTile
	: public CActorBackGround
{
public:
	CGroundTile();
	~CGroundTile();

	// ������.
	virtual bool Init() override;

	// �X�V.
	virtual void Update( const float& DeltaTime ) override;

	// �`��.
	virtual void SubRender() override;

	// �n�ʂ̐ݒu
	void Setting( const D3DXPOSITION3& Pos );

private:
	CSprite*			m_pGround;		// �n�ʂ̉摜.
	SSpriteRenderState	m_GroundState;	// �n�ʂ̉摜�̏��.


};