#pragma once
#include "..\..\..\Global.h"
#include "..\..\..\Common\Sprite\Sprite.h"

/************************************************
*	�t�F�[�h�N���X.
*		���c�F�P.
**/
class CFadeBase
{
protected:
	// �t�F�[�h�̏��.
	enum class enFadeState : unsigned char
	{
		None,
		FadeIn,		// �t�F�[�h�C����.
		FadeOut		// �t�F�[�h�A�E�g��.
	} typedef EFadeState;

public:
	CFadeBase();
	virtual ~CFadeBase();

	// ������.
	virtual bool Init() = 0;
	// �`��.
	virtual void Render() = 0;

	// �t�F�[�h�̐ݒ�.
	virtual void FadeSetting() = 0;

	// �t�F�[�h�C��.
	virtual bool FadeIn( const float Speed ) = 0;
	// �t�F�[�h�A�E�g.
	virtual bool FadeOut( const float Speed ) = 0;


protected:
	CSprite*					m_pFadeSprite;	// �t�F�[�h�p�̉摜.
	ID3D11ShaderResourceView*	m_pMaskTexture;	// �}�X�N�e�N�X�`��.
	SSpriteRenderState			m_FadeState;	// �t�F�[�h�p�̉摜�̏��.
	EFadeState					m_NowFade;		// ���݂̃t�F�[�h�̏��.
	float						m_FadeSpeed;	// �t�F�[�h���x.
};