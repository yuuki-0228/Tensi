#pragma once
#include "..\..\ActorEffect.h"

/************************************************
*	�Ԃ̐����G�t�F�N�g�N���X.
**/
class CWateringEffect final
	: public CActorEffect
{
public:
	CWateringEffect();
	~CWateringEffect();

	// ������.
	virtual bool Init() override;

	// �X�V.
	virtual void Update( const float& DeltaTime ) override;

	// �`��.
	virtual void Render() override;

	// �Đ�.
	virtual void Play( const D3DXPOSITION3& Pos, const float Size = 1.0f ) override;

private:
	bool m_IsDispAnim;	// �\�����Ă�����.
};
