#pragma once
#include "..\..\Actor.h"

/************************************************
*	�����G�t�F�N�g�N���X.
**/
class CSleepEffect final
	: public CActor
{
public:
	CSleepEffect();
	~CSleepEffect();

	// ������.
	virtual bool Init() override;

	// �X�V.
	virtual void Update( const float& DeltaTime ) override;

	// �`��.
	virtual void Render() override;

	// �A�j���[�V�����̊J�n.
	void Play( const D3DXPOSITION3& Pos );

private:
	CSprite*			m_pSprite;		// �摜.
	SSpriteRenderState	m_SpriteState;	// �摜�̏��.
	float				m_NowPoint;		// ���݂̈ʒu.
};
