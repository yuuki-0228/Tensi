#pragma once
#include "..\..\Global.h"

class CNormalFade;
class CMaskFade;
class CUFOFade;

// �t�F�[�h�^�C�v.
enum class enFadeType : unsigned char
{
	None,
	
	NormalFade,	// ���ʂ̃t�F�[�h.
	MaskFade,	// �}�X�N�摜���g�p�����t�F�[�h.
	UFOFade,	// UFO�^�ɐ؂蔲���ꂽ�t�F�[�h.
} typedef EFadeType;

/************************************************
*	�t�F�[�h�}�l�[�W���[�N���X.
**/
class FadeManager final
{
public:
	FadeManager();
	~FadeManager();

	// ������.
	static bool Init();
	// �`��.
	static void Render();

	// �t�F�[�h�̐ݒ�.
	static void FadeSetting( const EFadeType Type );

	// �t�F�[�h�C��.
	static bool FadeIn( const EFadeType Type, const float Speed );
	// �t�F�[�h�A�E�g.
	static bool FadeOut( const EFadeType Type, const float Speed );

	// �t�F�[�h�����擾.
	static bool GetIsFade() { return GetInstance()->m_NowFadeType != EFadeType::None; }

private:
	// �C���X�^���X�̎擾.
	static FadeManager* GetInstance();

private:
	EFadeType						m_NowFadeType;	// ���݂̃t�F�[�h�^�C�v.
	std::unique_ptr<CNormalFade>	m_pNormalFade;	// ���ʂ̃t�F�[�h.
	std::unique_ptr<CMaskFade>		m_pMaskFade;	// �}�X�N�摜���g�p�����t�F�[�h.
	std::unique_ptr<CUFOFade>		m_pUFOFade;		// UFO�^�ɐ؂蔲���ꂽ�t�F�[�h.
};