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
	SetSceneName( ESceneList::GameMain );
	GetSpriteList();
	return true;
}

//---------------------------.
// XV.
//---------------------------.
void CGameMainWidget::Update( const float& DeltaTime )
{
	m_DeltaTime = DeltaTime;
}
