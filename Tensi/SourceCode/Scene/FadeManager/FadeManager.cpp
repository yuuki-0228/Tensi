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
// インスタンスの取得.
//---------------------------.
FadeManager* FadeManager::GetInstance()
{
	static std::unique_ptr<FadeManager> pInstance = std::make_unique<FadeManager>();
	return pInstance.get();
}

//---------------------------.
// 初期化.
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
// 描画.
//---------------------------.
void FadeManager::Render()
{
	FadeManager* pI = GetInstance();

	if ( pI->m_NowFadeType == EFadeType::None ) return;

	// タイプに応じたフェードの描画.
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
// フェードの設定.
//---------------------------.
void FadeManager::FadeSetting( const EFadeType Type )
{
	FadeManager* pI = GetInstance();

	// タイプに応じたフェードの設定を行う.
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
// フェードイン.
//---------------------------.
bool FadeManager::FadeIn( const EFadeType Type, const float Speed )
{
	FadeManager* pI = GetInstance();

	// フェードタイプを保存.
	pI->m_NowFadeType = Type;

	// タイプに応じたフェードインを行う.
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
// フェードアウト.
//---------------------------.
bool FadeManager::FadeOut( const EFadeType Type, const float Speed )
{
	FadeManager* pI = GetInstance();

	// フェードタイプを保存.
	pI->m_NowFadeType = Type;

	// タイプに応じたフェードアウトを行う.
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

	// フェードが終了したか.
	if ( IsEnd == false ) return false;
	pI->m_NowFadeType = EFadeType::None;
	return true;
}