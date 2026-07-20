#include "UIEditWidget.h"

CUIEditWidget::CUIEditWidget()
{
}

CUIEditWidget::~CUIEditWidget()
{
}

//---------------------------.
// èâä˙âª.
//---------------------------.
bool CUIEditWidget::Init()
{
	m_SceneType = ESceneList::UIEdit;
	return true;
}

//---------------------------.
// çXêV.
//---------------------------.
void CUIEditWidget::Update( const float& DeltaTime )
{
	m_DeltaTime = DeltaTime;
}

//---------------------------.
// ï`âÊ.
//---------------------------.
void CUIEditWidget::Render()
{
}
