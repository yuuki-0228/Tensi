#include "SystemBase.h"
#include "..\Scene\SceneManager\SceneManager.h"

CSystemBase::CSystemBase()
	: m_RenderFunc	()
	, m_IsOpen		( false, "", "System" )
	, m_WndName		( "" )
	, m_IsOldOpen	( false )
{
}

CSystemBase::~CSystemBase()
{
}

//---------------------------.
// 描画.
//	描画しない場合は処理は行わない.
//---------------------------.
void CSystemBase::Render()
{
	// システムウィンドウを開いている間シーンを停止させる.
	bool Flag	= m_IsOpen.get();
	if ( m_IsOldOpen != Flag ) SceneManager::SetIsSceneStop( Flag );
	m_IsOldOpen = Flag;

	// ウィンドウを開いていない場合処理は行わない.
	if ( m_IsOpen == false ) return;

	// フラグが立っている場合、ウィンドウを開く.
	ImGui::Begin( m_WndName.c_str(), &Flag );

	// 描画処理.
	m_RenderFunc();

	// ウィンドウの処理終了.
	ImGui::End();
	m_IsOpen = Flag;
}

//---------------------------.
// ウィンドウ名の初期化.
//---------------------------.
void CSystemBase::InitWndName( const std::string& Name )
{
	m_WndName = Name;
	m_IsOpen.SetName( Name + u8"の表示/非表示" );
}