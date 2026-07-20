#include "UIEdit.h"
#include "..\..\Object\GameObject\Widget\SceneWidget\UIEditWidget\UIEditWidget.h"

CUIEdit::CUIEdit()
	: m_pUIEditWidget	( nullptr )
{
}

CUIEdit::~CUIEdit()
{
}

//---------------------------.
// 初期化.
//---------------------------.
bool CUIEdit::Init()
{
	m_pUIEditWidget = std::make_unique<CUIEditWidget>();
	m_pUIEditWidget->Init();
	return true;
}

//---------------------------.
// 初回起動時の初期化.
//---------------------------.
bool CUIEdit::FirstPlayInit()
{
	return true;
}

//---------------------------.
// 初回ログイン時の初期化.
//---------------------------.
bool CUIEdit::LoginInit( std::tm lastDay )
{
	return false;
}

//---------------------------.
// 更新.
//---------------------------.
void CUIEdit::Update( const float& DeltaTime )
{
	// UIの更新.
	m_pUIEditWidget->Update( DeltaTime );
}

//---------------------------.
// モデルの描画.
//---------------------------.
void CUIEdit::ModelRender()
{
}

//---------------------------.
// スプライト(UI)の描画.
//---------------------------.
void CUIEdit::SpriteUIRender()
{
	m_pUIEditWidget->Render();
}

//---------------------------.
// スプライト(3D)/Effectの描画.
//	_A：上に表示される / _B：下に表示される.
//---------------------------.
void CUIEdit::Sprite3DRender_A()
{
}
void CUIEdit::Sprite3DRender_B()
{
}
