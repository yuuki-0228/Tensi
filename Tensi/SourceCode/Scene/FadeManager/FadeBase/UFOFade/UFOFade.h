#pragma once
#include "..\FadeBase.h"

/************************************************
*	UFO�^�ɐ؂蔲���ꂽ�t�F�[�h�N���X.
*		���c�F�P.
**/
class CUFOFade final
	: public CFadeBase
{
public:
	CUFOFade();
	virtual ~CUFOFade();

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

private:
	CSprite*			m_pSpriteUFO;	// UFO�摜.
	SSpriteRenderState	m_UFOState;		// UFO�̏��.
};