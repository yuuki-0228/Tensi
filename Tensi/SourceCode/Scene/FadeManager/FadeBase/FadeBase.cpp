#include "FadeBase.h"

CFadeBase::CFadeBase()
	: m_pFadeSprite		( nullptr )
	, m_pMaskTexture	( nullptr )
	, m_FadeState		()
	, m_NowFade			( EFadeState::None )
	, m_FadeSpeed		( INIT_FLOAT )
{
}

CFadeBase::~CFadeBase()
{
}
