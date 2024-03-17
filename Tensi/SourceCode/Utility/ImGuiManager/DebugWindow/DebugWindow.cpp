#include "DebugWindow.h"
#include "..\..\..\Common\SoundManeger\SoundManeger.h"

DebugWindow::DebugWindow()
	: m_WatchQueue		()
	, m_MenuQueue		()
{
}

DebugWindow::~DebugWindow()
{
}

//----------------------------.
// �C���X�^���X�̎擾.
//----------------------------.
DebugWindow* DebugWindow::GetInstance()
{
	static std::unique_ptr<DebugWindow> pInstance = std::make_unique<DebugWindow>();
	return pInstance.get();
}

//----------------------------.
// �`��.
//----------------------------.
void DebugWindow::Render()
{
	ImGui::Begin( "DebugWindow" );
	
	// �f�o�b�N�̕\��.
	const int QueueSize = static_cast<int>( GetInstance()->m_WatchQueue.size() );
	if ( ImGui::BeginTabBar( "TabBar", ImGuiTabBarFlags_FittingPolicyScroll | ImGuiTabBarFlags_TabListPopupButton ) ) {
		for ( int i = 0; i < QueueSize; i++ ) {
			std::function<void()> proc = GetInstance()->m_WatchQueue.front();
			proc();
			GetInstance()->m_WatchQueue.pop();
		}
		ImGui::EndTabBar();
	}
	ImGui::End();
}

//----------------------------.
// �E�H�b�`�Ɋ֐���ǉ�����.
//----------------------------.
void DebugWindow::PushProc( const char* Name, const std::function<void()>& Proc )
{
	GetInstance()->m_WatchQueue.push(
	[=]() {
		if ( ImGui::BeginTabItem( Name ) ) {
			Proc();
			ImGui::EndTabItem();
		}
	} );
}
