#include "FadeManager.h"
#include "FadeBase\NormalFade\NormalFade.h"
#include "FadeBase\MaskFade\MaskFade.h"
#include "FadeBase\UFOFade\UFOFade.h"

FadeManager::FadeManager()
	: m_NowFadeType	( EFadeType::None )
	, m_pNormalFade	( nullptr )
	, m_pMaskFade	( nullptr )
	, m_pUFOFade	( nullptr )
{
	m_pNormalFade	= std::make_unique<CNormalFade>();
	m_pMaskFade		= std::make_unique<CMaskFade>();
	m_pUFOFade		= std::make_unique<CUFOFade>();
}

FadeManager::~FadeManager()
{
}

//---------------------------.
// �C���X�^���X�̎擾.
//---------------------------.
FadeManager* FadeManager::GetInstance()
{
	static std::unique_ptr<FadeManager> pInstance = std::make_unique<FadeManager>();
	return pInstance.get();
}

//---------------------------.
// ������.
//---------------------------.
bool FadeManager::Init()
{
	FadeManager* pI = GetInstance();

	if ( pI->m_pNormalFade->Init()	== false ) return false;
	if ( pI->m_pMaskFade->Init()	== false ) return false;
	if ( pI->m_pUFOFade->Init()		== false ) return false;
	return true;
}

//---------------------------.
// �`��.
//---------------------------.
void FadeManager::Render()
{
	FadeManager* pI = GetInstance();

	if ( pI->m_NowFadeType == EFadeType::None ) return;

	// �^�C�v�ɉ������t�F�[�h�̕`��.
	switch ( pI->m_NowFadeType ) {
	case EFadeType::NormalFade:
		pI->m_pNormalFade->Render();
		break;
	case EFadeType::MaskFade:
		pI->m_pMaskFade->Render();
		break;
	case EFadeType::UFOFade:
		pI->m_pUFOFade->Render();
		break;
	default:
		break;
	}
}

//---------------------------.
// �t�F�[�h�̐ݒ�.
//---------------------------.
void FadeManager::FadeSetting( const EFadeType Type )
{
	FadeManager* pI = GetInstance();

	// �^�C�v�ɉ������t�F�[�h�̐ݒ���s��.
	switch ( Type ) {
	case EFadeType::NormalFade:
		pI->m_pNormalFade->FadeSetting();
		return;
	case EFadeType::MaskFade:
		pI->m_pMaskFade->FadeSetting();
		return;
	case EFadeType::UFOFade:
		pI->m_pUFOFade->FadeSetting();
		return;
	default:
		return;
	}
}

//---------------------------.
// �t�F�[�h�C��.
//---------------------------.
bool FadeManager::FadeIn( const EFadeType Type, const float Speed )
{
	FadeManager* pI = GetInstance();

	// �t�F�[�h�^�C�v��ۑ�.
	pI->m_NowFadeType = Type;

	// �^�C�v�ɉ������t�F�[�h�C�����s��.
	switch ( Type ) {
	case EFadeType::NormalFade:
		return pI->m_pNormalFade->FadeIn( Speed );
	case EFadeType::MaskFade:
		return pI->m_pMaskFade->FadeIn( Speed );
	case EFadeType::UFOFade:
		return pI->m_pUFOFade->FadeIn( Speed );
	default:
		return true;
	}
}

//---------------------------.
// �t�F�[�h�A�E�g.
//---------------------------.
bool FadeManager::FadeOut( const EFadeType Type, const float Speed )
{
	FadeManager* pI = GetInstance();

	// �t�F�[�h�^�C�v��ۑ�.
	pI->m_NowFadeType = Type;

	// �^�C�v�ɉ������t�F�[�h�A�E�g���s��.
	bool IsEnd = false;
	switch ( Type ) {
	case EFadeType::NormalFade:
		IsEnd = pI->m_pNormalFade->FadeOut( Speed );
		break;
	case EFadeType::MaskFade:
		IsEnd = pI->m_pMaskFade->FadeOut( Speed );
		break;
	case EFadeType::UFOFade:
		IsEnd = pI->m_pUFOFade->FadeOut( Speed );
		break;
	default:
		IsEnd = true;
		break;
	}

	// �t�F�[�h���I��������.
	if ( IsEnd == false ) return false;
	pI->m_NowFadeType = EFadeType::None;
	return true;
}