#pragma once
#include "..\..\..\..\Actor.h"

/************************************************
*	�Ԃ̐����G�t�F�N�g�N���X.
**/
class CWateringAnim final
	: public CActor
{
public:
	CWateringAnim();
	~CWateringAnim();

	// ������.
	virtual bool Init() override;

	// �X�V.
	virtual void Update( const float& DeltaTime ) override;

	// �`��.
	virtual void Render() override;

	// �Đ�.
	void Play( const D3DXPOSITION3& Pos );

private:
	CSprite*			m_pSprite;		// �摜.
	SSpriteRenderState	m_SpriteState;	// �摜�̏��.
	bool				m_IsDispAnim;	// �\�����Ă�����.
};
