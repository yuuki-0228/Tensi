#pragma once
#include "..\FadeBase.h"

/************************************************
*	�}�X�N�摜���g�p�����t�F�[�h�N���X.
*		���c�F�P.
**/
class CMaskFade final
	: public CFadeBase
{
public:
	CMaskFade();
	virtual ~CMaskFade();

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