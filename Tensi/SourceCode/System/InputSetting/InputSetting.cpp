#include "InputSetting.h"
#include "..\..\Utility\Input\InputList.h"
#include "..\..\Utility\FileManager\FileManager.h"
#include "..\..\Utility\Input\Input.h"

namespace {
	constexpr char KEY_BIND_FILE_PATH[] = "Data\\Parameter\\Config\\KeyBind.json";	// �L�[�o�C���h�̕ۑ��ꏊ.
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
// �������֐�.
//---------------------------.
void InputSetting::Init()
{
	// ���݂̐ݒ���擾����.
	json KeyBindData = FileManager::JsonLoad( KEY_BIND_FILE_PATH );
	for ( auto& [ActionName, Obj] : KeyBindData.items() ) {
		for ( auto&[KeyType, Value] : Obj.items() ) {
			// �L�[���蓖�Ă̎擾.
			if ( KeyType == "Key" ) {
				for ( auto& v : Value ) m_KeyBindList[ActionName].Key.emplace_back( InputList::StringToKeyData( v ) );
			}
			// �{�^�����蓖�Ă̎擾.
			else if ( KeyType == "But" ) {
				for ( auto& v : Value ) m_KeyBindList[ActionName].But.emplace_back( InputList::StringToButData( v ) );
			}
		}
	}

	// �`�揈���̐ݒ�.
	m_RenderFunc = [&] () {
		ImGui::Text( u8"�ݒ肵������́uInput�v�N���X�ŉ��������A���������Ȃǎ擾�ł��܂��B" );
		ImGui::Text( u8"�u�ǉ����铮�얼�v�ɂ��̓��삪���Ȃ̂��L�����u�{�v�Œǉ����܂��B" );
		ImGui::Text( u8"���̌�A�u�L�[�{�[�h�v�ɂ͑Ή������L�[�{�[�h���́A�u�R���g���[���v�ɂ͑Ή������R���g���[�����͂�ݒ肵�܂��B" );
		ImGui::Separator();

		// �V�����s���̖��O�̋L����.
		ImGui::Text( u8"< KeyBind >" );
		ImGui::Text( u8"�ǉ����铮�얼 : " );
		ImGui::SameLine();
		ImGui::InputText( "##ActionName", &m_ActionName );
		ImGui::SameLine();

		// �V�����s���̒ǉ��{�^��.
		if ( ImGuiManager::Button( "+", !m_ActionName.empty() ) ) {
			m_KeyBindList[m_ActionName] = SKeyBindPair();
			m_ActionName = "";
		}

		// �s���̕\��.
		auto ActItr = m_KeyBindList.begin();
		while ( ActItr != m_KeyBindList.end() ) {
			const std::string	Name	= ActItr->first;
			SKeyBindPair*		KeyBind = &ActItr->second;

			// �s���̍폜.
			ImGui::Text( u8"%16s", Name.c_str() );
			ImGui::SameLine();
			if ( ImGuiManager::Button( std::string( "x##DeleteButton_" + Name ).c_str() ) ) {
				ActItr = m_KeyBindList.erase( ActItr );
				continue;
			}
			ImGui::SameLine();

			// ���͐ݒ�.
			if ( ImGui::TreeNode( std::string( "##ActionTree_" + Name ).c_str() ) ) {
				int No = 0;

				ImGui::Separator();
				ImGui::Text( u8"< �L�[�{�[�h >" );
				ImGui::SameLine();

				// �L�[���͂̒ǉ��{�^��.
				if ( ImGuiManager::Button( "+##Action_Key" ) ) {
					KeyBind->Key.emplace_back( 0 );
				}

				// �L�[���͂̃R���{�{�b�N�X��\��.
				auto KeyItr = KeyBind->Key.begin();
				while ( KeyItr != KeyBind->Key.end() ) {
					std::string NewKey	= ImGuiManager::Combo( std::string( "KeyCombo##NoName" + std::to_string( No ) ).c_str(), InputList::KeyDataToString( *KeyItr ), InputList::GetKeyList() );
					*KeyItr				= InputList::StringToKeyData( NewKey );
					ImGui::SameLine();

					// �L�[���͂̍폜�{�^��.
					if ( ImGuiManager::Button( std::string( "x##KeyDelete_" + std::to_string( No ) ).c_str() ) ) {
						KeyItr = KeyBind->Key.erase( KeyItr );
					}
					else KeyItr++;
					No++;
				}

				ImGui::Text( u8"< �R���g���[�� >" );
				ImGui::SameLine();

				// �R���g���[�����͂̒ǉ��{�^��.
				if ( ImGuiManager::Button( "+##Action_But" ) ) {
					KeyBind->But.emplace_back( 0 );
				}

				// �R���g���[�����͂̃R���{�{�b�N�X��\��.
				auto ButItr = KeyBind->But.begin();
				while ( ButItr != KeyBind->But.end() ) {
					std::string NewBut	= ImGuiManager::Combo( std::string( "ButCombo##NoName" + std::to_string( No ) ).c_str(), InputList::ButDataToString( *ButItr ), InputList::GetButList() );
					*ButItr				= InputList::StringToButData( NewBut );
					ImGui::SameLine();

					// �R���g���[�����͂̍폜�{�^��.
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

		// �L�[�o�C���h�̕ۑ��{�^��.
		ImGui::Separator();
		if ( ImGuiManager::Button( u8"�ۑ�##KeyBindSave" ) ) {
			// ���݂̃L�[�o�C���h��json�`���ŕۑ�����.
			json j;
			j[".Comment"] = {
				u8"�@�\�Ƃ��̋@�\�ɑΉ������L�[�̐ݒ肪�s���܂��B",
				u8"�����Ő錾�����@�\�� 'Input.cpp' �Ŏg�p�ł��܂��B",
				u8"----------------------------------------------",
				u8"[But] : �R���g���[��(XInput)�̑Ή������{�^��",
				u8"[Key] : �L�[�{�[�h�̑Ή������L�[",
				u8"----------------------------------------------"
			};
			for ( auto& [ActionName, List] : m_KeyBindList ) {
				for ( auto& k : List.Key ) j[ActionName]["Key"].emplace_back( InputList::KeyDataToString( k ) );
				for ( auto& b : List.But ) j[ActionName]["But"].emplace_back( InputList::ButDataToString( b ) );
			}
			FileManager::JsonSave( KEY_BIND_FILE_PATH, j );

			// �L�[�o�C���h�̍X�V.
			Input::SetKeyBind( m_KeyBindList );
		}
	};

	// �E�B���h�E���̐ݒ�.
	InitWndName( "InputSetting" );
}