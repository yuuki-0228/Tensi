#pragma once
#include "..\..\ActorEffect.h"

/************************************************
*	�����G�t�F�N�g.
**/
class CNoteEffect final
	: public CActorEffect
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
	virtual void Play( const D3DXPOSITION3& Pos, const float Size = 1.0f ) override;

private:

};