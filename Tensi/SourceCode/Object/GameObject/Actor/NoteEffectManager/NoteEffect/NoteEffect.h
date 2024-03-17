#pragma once
#include "..\..\Actor.h"

/************************************************
*	�����G�t�F�N�g.
**/
class CNoteEffect final
	: public CActor
{
public:
	CNoteEffect();
	~CNoteEffect();

	// ������.
	virtual bool Init() override;

	// �X�V.
	virtual void Update( const float& DeltaTime ) override;

	// �`��.
	virtual void Render() override;

	// �A�j���[�V�����̊J�n.
	void Play( const D3DXPOSITION3& Pos, const float Size );

private:
	CSprite*			m_pSprite;		// �摜.
	SSpriteRenderState	m_SpriteState;	// �摜�̏��.
	float				m_NowPoint;		// ���݂̈ʒu.

};