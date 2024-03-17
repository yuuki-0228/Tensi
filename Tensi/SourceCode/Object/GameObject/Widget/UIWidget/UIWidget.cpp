#include "UIWidget.h"

CUIWidget::CUIWidget()
	: m_pSpriteList		()
	, m_pSpriteState	()
{
}

CUIWidget::~CUIWidget()
{
}

//---------------------------.
// �X�v���C�g���̎擾.
//---------------------------.
void CUIWidget::SetSpriteState( State_map& pState )
{
	for ( auto& [Key, List] : pState ) {
		m_pSpriteState[Key].reserve( List.size() );
		for ( auto& s : List ) m_pSpriteState[Key].emplace_back( &s );
	}
}
