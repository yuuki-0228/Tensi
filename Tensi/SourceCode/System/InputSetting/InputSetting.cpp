#include "InputSetting.h"
#include "..\..\Utility\Input\InputList.h"
#include "..\..\Utility\FileManager\FileManager.h"
#include "..\..\Utility\Input\Input.h"

namespace {
	constexpr char KEY_BIND_FILE_PATH[] = "Data\\Parameter\\Config\\KeyBind.json";	// キーバインドの保存場所.
}

InputSetting::InputSetting()
	: m_KeyBindList	()
	, m_ActionName	()
{
	Init();
}

InputSetting::~InputSetting()
{
}

//---------------------------.
// 初期化関数.
//---------------------------.
void InputSetting::Init()
{
	// 現在の設定を取得する.
	json KeyBindData = FileManager::JsonLoad( KEY_BIND_FILE_PATH );
	for ( auto& [ActionName, Obj] : KeyBindData.items() ) {
		for ( auto&[KeyType, Value] : Obj.items() ) {
			// キー割り当ての取得.
			if ( KeyType == "Key" ) {
				for ( auto& v : Value ) m_KeyBindList[ActionName].Key.emplace_back( InputList::StringToKeyData( v ) );
			}
			// ボタン割り当ての取得.
			else if ( KeyType == "But" ) {
				for ( auto& v : Value ) m_KeyBindList[ActionName].But.emplace_back( InputList::StringToButData( v ) );
			}
		}
	}

	// 描画処理の設定.
	m_RenderFunc = [&] () {
		ImGui::Text( u8"設定した動作は「Input」クラスで押した時、離した時など取得できます。" );
		ImGui::Text( u8"「追加する動作名」にこの動作が何なのか記入し「＋」で追加します。" );
		ImGui::Text( u8"その後、「キーボード」には対応したキーボード入力、「コントローラ」には対応したコントローラ入力を設定します。" );
		ImGui::Separator();

		// 新しい行動の名前の記入欄.
		ImGui::Text( u8"< KeyBind >" );
		ImGui::Text( u8"追加する動作名 : " );
		ImGui::SameLine();
		ImGui::InputText( "##ActionName", &m_ActionName );
		ImGui::SameLine();

		// 新しい行動の追加ボタン.
		if ( ImGuiManager::Button( "+", !m_ActionName.empty() ) ) {
			m_KeyBindList[m_ActionName] = SKeyBindPair();
			m_ActionName = "";
		}

		// 行動の表示.
		auto ActItr = m_KeyBindList.begin();
		while ( ActItr != m_KeyBindList.end() ) {
			const std::string	Name	= ActItr->first;
			SKeyBindPair*		KeyBind = &ActItr->second;

			// 行動の削除.
			ImGui::Text( u8"%16s", Name.c_str() );
			ImGui::SameLine();
			if ( ImGuiManager::Button( std::string( "x##DeleteButton_" + Name ).c_str() ) ) {
				ActItr = m_KeyBindList.erase( ActItr );
				continue;
			}
			ImGui::SameLine();

			// 入力設定.
			if ( ImGui::TreeNode( std::string( "##ActionTree_" + Name ).c_str() ) ) {
				int No = 0;

				ImGui::Separator();
				ImGui::Text( u8"< キーボード >" );
				ImGui::SameLine();

				// キー入力の追加ボタン.
				if ( ImGuiManager::Button( "+##Action_Key" ) ) {
					KeyBind->Key.emplace_back( 0 );
				}

				// キー入力のコンボボックスを表示.
				auto KeyItr = KeyBind->Key.begin();
				while ( KeyItr != KeyBind->Key.end() ) {
					std::string NewKey	= ImGuiManager::Combo( std::string( "KeyCombo##NoName" + std::to_string( No ) ).c_str(), InputList::KeyDataToString( *KeyItr ), InputList::GetKeyList() );
					*KeyItr				= InputList::StringToKeyData( NewKey );
					ImGui::SameLine();

					// キー入力の削除ボタン.
					if ( ImGuiManager::Button( std::string( "x##KeyDelete_" + std::to_string( No ) ).c_str() ) ) {
						KeyItr = KeyBind->Key.erase( KeyItr );
					}
					else KeyItr++;
					No++;
				}

				ImGui::Text( u8"< コントローラ >" );
				ImGui::SameLine();

				// コントローラ入力の追加ボタン.
				if ( ImGuiManager::Button( "+##Action_But" ) ) {
					KeyBind->But.emplace_back( 0 );
				}

				// コントローラ入力のコンボボックスを表示.
				auto ButItr = KeyBind->But.begin();
				while ( ButItr != KeyBind->But.end() ) {
					std::string NewBut	= ImGuiManager::Combo( std::string( "ButCombo##NoName" + std::to_string( No ) ).c_str(), InputList::ButDataToString( *ButItr ), InputList::GetButList() );
					*ButItr				= InputList::StringToButData( NewBut );
					ImGui::SameLine();

					// コントローラ入力の削除ボタン.
					if ( ImGuiManager::Button( std::string( "x##ButDelete_" + std::to_string( No ) ).c_str() ) ) {
						ButItr = KeyBind->But.erase( ButItr );
					}
					else ButItr++;
					No++;
				}
				ImGui::TreePop();
			}
			ActItr++;
			ImGui::Separator();
		}

		// キーバインドの保存ボタン.
		ImGui::Separator();
		if ( ImGuiManager::Button( u8"保存##KeyBindSave" ) ) {
			// 現在のキーバインドをjson形式で保存する.
			json j;
			j[".Comment"] = {
				u8"機能とその機能に対応したキーの設定が行えます。",
				u8"ここで宣言した機能は 'Input.cpp' で使用できます。",
				u8"----------------------------------------------",
				u8"[But] : コントローラ(XInput)の対応したボタン",
				u8"[Key] : キーボードの対応したキー",
				u8"----------------------------------------------"
			};
			for ( auto& [ActionName, List] : m_KeyBindList ) {
				for ( auto& k : List.Key ) j[ActionName]["Key"].emplace_back( InputList::KeyDataToString( k ) );
				for ( auto& b : List.But ) j[ActionName]["But"].emplace_back( InputList::ButDataToString( b ) );
			}
			FileManager::JsonSave( KEY_BIND_FILE_PATH, j );

			// キーバインドの更新.
			Input::SetKeyBind( m_KeyBindList );
		}
	};

	// ウィンドウ名の設定.
	InitWndName( "InputSetting" );
}