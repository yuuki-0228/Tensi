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
// 初期化.
//---------------------------.
bool CGameMainWidget::Init()
{
	m_SceneType = ESceneList::GameMain;
	return true;
}

//---------------------------.
// 更新.
//---------------------------.
void CGameMainWidget::Update( const float& DeltaTime )
{
	m_DeltaTime = DeltaTime;
}

//---------------------------.
// 描画.
//---------------------------.
void CGameMainWidget::Render()
{
}
