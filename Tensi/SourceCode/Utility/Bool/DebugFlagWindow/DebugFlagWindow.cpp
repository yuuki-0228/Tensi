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
	, m_IsDispNameNoneFlag	( false, u8"無名のフラグを表示/非表示", "" )
{
}

DebugFlagWindow::~DebugFlagWindow()
{
}

//----------------------------.
// インスタンスの取得.
//----------------------------.
DebugFlagWindow* DebugFlagWindow::GetInstance()
{
	static std::unique_ptr<DebugFlagWindow> pInstance = std::make_unique<DebugFlagWindow>();
	return pInstance.get();
}

//----------------------------.
// 描画.
//----------------------------.
void DebugFlagWindow::Render()
{
	DebugFlagWindow* pI = GetInstance();

	ImGui::Begin( "DebugFlagWindow" );

	// メッセージ検索の表示.
	FlagFind();

	// 各フラグのグループごとの処理リスト.
	std::unordered_map<std::string, std::vector<BitFlagManager::Handle>> GroupFuncList;

	// フラグを表示するか調べる.
	ImGui::BeginChild( ImGui::GetID( ( void* )0 ), ImVec2( 0.0f, 0.0f ), ImGuiWindowFlags_NoTitleBar );
	const BitFlagManager::Handle Size = BitFlagManager::GetFlagNum();
	for ( BitFlagManager::Handle i = 0; i < Size; i++ ) {
		std::string Name	= BitFlagManager::GetName( i );
		std::string Group	= BitFlagManager::GetGroup( i );

		// 表示するフラグか調べる.
		if ( pI->m_IsDispNameNoneFlag == false && Name == "" ) continue;
		if ( Name == "" ) {
			// 無名のフラグに仮の名前をつける.
			Name	= "NameNoneFlag_" + std::to_string( static_cast<int>( i ) );
			BitFlagManager::SetGroup( i, "NameNone" );
		}
		if ( Name.find( pI->m_FindString ) == std::string::npos ) continue;

		// 処理の追加.
		GroupFuncList[Group.c_str()].emplace_back( i );
	}

	// フラグの表示.
	for ( auto& [Group, Handle] : GroupFuncList ) {
		// グループに指定がない場合そのまま表示する.
		if ( Group == "" ) {
			for ( auto& h : Handle ) {
				bool				Flag = BitFlagManager::IsBitFlag( h );
				const std::string	Name = BitFlagManager::GetName( h );
				BitFlagManager::SetBitFlag( h, ImGuiManager::CheckBox( Name.c_str(), &Flag ) );
			}
			continue;
		}

		// グループに指定があるグループを作成し表示する.
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
//	フラグ検索処理.
//---------------------.
void DebugFlagWindow::FlagFind()
{
	ImGui::Text( u8"検索 :" );
	ImGui::SameLine();
	ImGui::InputText( " ", &GetInstance()->m_FindString );
	ImGui::Separator();
}

#endif