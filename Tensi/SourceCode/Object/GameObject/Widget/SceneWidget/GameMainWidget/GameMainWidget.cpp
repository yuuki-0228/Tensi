#include "GameMainWidget.h"
#include "..\..\..\..\..\Common\SoundManeger\SoundManeger.h"
#include "..\..\..\..\..\Utility\Input\Input.h"

CGameMainWidget::CGameMainWidget()
{
}

CGameMainWidget::~CGameMainWidget()
{
}

//---------------------------.
// ‰Šú‰».
//---------------------------.
bool CGameMainWidget::Init()
{
	m_SceneType = ESceneList::GameMain;
	return true;
}

//---------------------------.
// XV.
//---------------------------.
void CGameMainWidget::Update( const float& DeltaTime )
{
	m_DeltaTime = DeltaTime;
}

//---------------------------.
// •`‰æ.
//---------------------------.
void CGameMainWidget::Render()
{
}
