#pragma once
#include "..\FadeBase.h"

/************************************************
*	���ʂ̃t�F�[�h�N���X.
*		���c�F�P.
**/
class CNormalFade final
	: public CFadeBase
{
public:
	CNormalFade();
	virtual ~CNormalFade();

	// ������.
	virtual bool Init() override;
	// �`��.
	virtual void Render() override;

	// �t�F�[�h�̐ݒ�.
	virtual void FadeSetting() override;

	// �t�F�[�h�C��.
	virtual bool FadeIn( const float Speed ) override;
	// �t�F�[�h�A�E�g.
	virtual bool FadeOut( const float Speed ) override;

};