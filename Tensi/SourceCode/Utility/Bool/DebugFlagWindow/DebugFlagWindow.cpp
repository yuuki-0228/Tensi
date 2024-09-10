#include "DebugFlagWindow.h"
#ifdef ENABLE_CLASS_BOOL
#include "..\BitFlagManager\BitFlagManager.h"
#include "..\..\ImGuiManager\ImGuiManager.h"
#include <fstream>
#include <sstream>
#include <chrono>
#include <iomanip>

DebugFlagWindow::DebugFlagWindow()
	: m_FindString			( "" )
	, m_IsDispNameNoneFlag	( false, u8"�����̃t���O��\��/��\��", "" )
{
}

DebugFlagWindow::~DebugFlagWindow()
{
}

//----------------------------.
// �C���X�^���X�̎擾.
//----------------------------.
DebugFlagWindow* DebugFlagWindow::GetInstance()
{
	static std::unique_ptr<DebugFlagWindow> pInstance = std::make_unique<DebugFlagWindow>();
	return pInstance.get();
}

//----------------------------.
// �`��.
//----------------------------.
void DebugFlagWindow::Render()
{
	DebugFlagWindow* pI = GetInstance();

	ImGui::Begin( "DebugFlagWindow" );

	// ���b�Z�[�W�����̕\��.
	FlagFind();

	// �e�t���O�̃O���[�v���Ƃ̏������X�g.
	std::unordered_map<std::string, std::vector<BitFlagManager::Handle>> GroupFuncList;

	// �t���O��\�����邩���ׂ�.
	ImGui::BeginChild( ImGui::GetID( ( void* )0 ), ImVec2( 0.0f, 0.0f ), ImGuiWindowFlags_NoTitleBar );
	const BitFlagManager::Handle Size = BitFlagManager::GetFlagNum();
	for ( BitFlagManager::Handle i = 0; i < Size; i++ ) {
		std::string Name	= BitFlagManager::GetName( i );
		std::string Group	= BitFlagManager::GetGroup( i );

		// �\������t���O�����ׂ�.
		if ( pI->m_IsDispNameNoneFlag == false && Name == "" ) continue;
		if ( Name == "" ) {
			// �����̃t���O�ɉ��̖��O������.
			Name	= "NameNoneFlag_" + std::to_string( static_cast<int>( i ) );
			BitFlagManager::SetGroup( i, "NameNone" );
		}
		if ( Name.find( pI->m_FindString ) == std::string::npos ) continue;

		// �����̒ǉ�.
		GroupFuncList[Group.c_str()].emplace_back( i );
	}

	// �t���O�̕\��.
	for ( auto& [Group, Handle] : GroupFuncList ) {
		// �O���[�v�Ɏw�肪�Ȃ��ꍇ���̂܂ܕ\������.
		if ( Group == "" ) {
			for ( auto& h : Handle ) {
				bool				Flag = BitFlagManager::IsBitFlag( h );
				const std::string	Name = BitFlagManager::GetName( h );
				BitFlagManager::SetBitFlag( h, ImGuiManager::CheckBox( Name.c_str(), &Flag ) );
			}
			continue;
		}

		// �O���[�v�Ɏw�肪����O���[�v���쐬���\������.
		if ( ImGui::TreeNodeEx( Group.c_str(), ImGuiTreeNodeFlags_Framed ) ) {
			for ( auto& h : Handle ) {
				bool		Flag = BitFlagManager::IsBitFlag( h );
				std::string	Name = BitFlagManager::GetName( h );
				if ( Group == "NameNone" ) Name = "NameNoneFlag_" + std::to_string( static_cast<int>( h ) );
				BitFlagManager::SetBitFlag( h, ImGuiManager::CheckBox( Name.c_str(), &Flag ) );
			}
			ImGui::TreePop();
		}
	}

	ImGui::EndChild();
	ImGui::End();
}

//---------------------.
//	�t���O��������.
//---------------------.
void DebugFlagWindow::FlagFind()
{
	ImGui::Text( u8"���� :" );
	ImGui::SameLine();
	ImGui::InputText( " ", &GetInstance()->m_FindString );
	ImGui::Separator();
}

#endif