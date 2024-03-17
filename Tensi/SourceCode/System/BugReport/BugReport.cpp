#include "BugReport.h"
#include "..\..\Utility\FileManager\FileManager.h"
#include <fstream>
#include <sstream>
#include <chrono>
#include <iomanip>
#include <filesystem>

namespace {
	constexpr char SAVE_FILE_PATH[]			= "Data\\BugReport";	// �ۑ�����ꏊ�̃t�@�C���p�X.
	constexpr int  BUG_TEXT_NORMAL_COLUMN	= 10;					// �o�O�񍐃e�L�X�g�̒ʏ펞(���e�ɉ��s���܂܂Ȃ���)�̗�.
	constexpr int  BUG_TEXT_TITLE_POS		= 1;					// �o�O�񍐃e�L�X�g�̃^�C�g���̈ʒu.
	constexpr int  BUG_TEXT_LV_POS			= 4;					// �o�O�񍐃e�L�X�g�̃��x���̈ʒu.
	constexpr int  BUG_TEXT_CONTENTS_POS	= 6;					// �o�O�񍐃e�L�X�g�̓��e�̈ʒu.
	constexpr int  BUG_TEXT_DISCOVERER_POS	= 8;					// �o�O�񍐃e�L�X�g�̔����҂̈ʒu.
}

CBugReport::CBugReport()
	: m_BugTitle	( "" )
	, m_BugContents	( "" )
	, m_Discoverer	( "" )
	, m_BugLv		( EBugLv::S )
{
	Init();
}

CBugReport::~CBugReport()
{
}

//---------------------------.
// �������֐�.
//---------------------------.
void CBugReport::Init()
{
	// �o�O���X�g�̓ǂݍ���.
	BugListLoad();

	// �`�揈���̐ݒ�.
	m_RenderFunc = [&] () {
		ImGui::Text( u8"���������o�O���L�����Ǘ����邱�Ƃ��ł��܂��B" );
		ImGui::Text( u8"���������o�O�́u�^�C�g���v�A�u�o�O���x���v�A�u���e�v���L�����Ă��������B" );
		ImGui::Text( u8"�L�������o�O�́uData\\BugReport�v�̒��ɕۑ�����܂��B" );
		ImGui::Text( u8"�u�C�������v�{�^�����������ƂŁuData\\BugReport\\Fixed�v�̒��Ɉړ�����܂��B" );
		ImGui::Separator();

		// �o�O�̕�.
		ImGui::SetNextTreeNodeOpen( true, ImGuiCond_Once );
		if ( ImGui::TreeNode( "BugReport" ) ) {

			// �����҂̋L��.
			ImGui::Text( u8"< ������ >" );
			ImGui::InputText( "##Discoverer", &m_Discoverer );
			ImGui::Separator();

			// �o�O���x���̎w��.
			ImGui::SameLine();
			ImGuiManager::Combo( u8"< �o�O���x�� >", &m_BugLv );
			ImGui::SameLine();
			ImGuiManager::HelpMarker( 
				u8"---------------------------------------------\n"
				u8"�o�O�̃��x�����w�肵�Ă��������B\n"
				u8"���x���̐����͈ȉ��̒ʂ�ł��B\n"
				u8"---------------------------------------------\n"
				u8"[ S ] : �Q�[���i�s�ɖ�肪�o��B\n"
				u8"      : ���ʂɃQ�[�����Ă��đ�������B\n"
				u8"---------------------------------------------\n"
				u8"[ A ] : �v���C���[�ɒ������s���v�������炷�B\n"
				u8"      : ����̎菇�Ŕ�������B\n"
				u8"---------------------------------------------\n"
				u8"[ B ] : �v���C���[�ɂ�����x�s���v�������炷�B\n"
				u8"      : �Q�[���i�s�ɖ��͂Ȃ��B\n"
				u8"---------------------------------------------\n"
				u8"[ C ] : ���炩�ɕs������v���I�ł͂Ȃ��B\n"
				u8"---------------------------------------------\n"
				u8"[ D ] : �s��Ƃ܂ł͂����Ȃ����A�C�ɂȂ�B\n"
				u8"---------------------------------------------\n"
			);
			ImGui::Separator();

			// �o�O�^�C�g���̋L��.
			ImGui::Text( u8"< �o�O�̃^�C�g�� >" );
			ImGui::InputText( "##BugTitle", &m_BugTitle );
			// �o�O���e�̋L��.
			ImGui::Text( u8"< �o�O�̓��e >" );
			ImGui::InputTextMultiline( "##BugContent", &m_BugContents, ImVec2( 0.0f, 100.0f ) );
			ImGui::Separator();

			// �o�̓{�^��.
			if ( ImGuiManager::Button( u8"�ۑ�##BugSave", !m_BugTitle.empty() && !m_BugContents.empty() && !m_Discoverer.empty() ) ) {
				const EBugLv		Lv		= m_BugLv;
				const std::string	Title	= m_BugTitle;
				const std::string	Msg		= m_BugContents;
				const std::string	Name	= m_Discoverer;

				// ���݂̎��Ԃ��擾.
				auto now = std::chrono::system_clock::now();
				auto now_c = std::chrono::system_clock::to_time_t( now );
				std::stringstream Time;
				Time << std::put_time( localtime( &now_c ), "%Y%m%d_%H%M%S" );

				// �e�L�X�g�t�@�C���o��.
				const std::string FilePath = std::string( SAVE_FILE_PATH ) + "\\" + Time.str() + ".txt";
				FileManager::TextSave( FilePath, std::string( 
					u8"----------------------------------------\n" +
					Title + u8"\n" +
					u8"----------------------------------------\n" +
					u8"[ �o�O���x�� ]\n" +
					StringConversion::Enum( Lv ) + u8"\n" +
					u8"[ �o�O���e ]\n" +
					Msg + u8"\n\n" +
					u8"----------------------------------------\n" +
					u8"������ : " + Name + u8"\n" +
					u8"----------------------------------------"
				) );

				// ������.
				m_BugLv			= EBugLv::S;
				m_BugTitle		= "";
				m_BugContents	= "";

				// �o�O���X�g�̍ēǂݍ���.
				BugListLoad();
			}
			// �폜�{�^��.
			ImGui::SameLine();
			if ( ImGuiManager::Button( u8"�폜" ) ) {
				m_BugLv			= EBugLv::S;
				m_BugContents	= "";
			}
			ImGui::TreePop();
		}

		// �o�O���X�g.
		ImGui::Separator();
		if ( m_BugList.empty() ) return;
		if ( ImGui::TreeNode( "BugList" ) ) {
			ImGui::Separator();
			auto ListItr = m_BugList.begin();
			while ( ListItr != m_BugList.end() ) {
				// �o�O�^�C�g���̕\��.
				ImGui::Text( ListItr->second[BUG_TEXT_TITLE_POS].c_str() );
				ImGui::SameLine();

				// �C�������{�^��.
				if( ImGuiManager::Button( std::string( u8"�C������##" + ListItr->first ).c_str() ) ) {
					const std::string NowPath = std::string( SAVE_FILE_PATH ) + "\\" + ListItr->first + ".txt";
					const std::string NewPath = std::string( SAVE_FILE_PATH ) + "\\Fixed\\" + ListItr->first + ".txt";
					std::filesystem::create_directories( std::string( SAVE_FILE_PATH ) + "\\Fixed" );
					std::filesystem::rename( NowPath, NewPath );
					ListItr = m_BugList.erase( ListItr );
					continue;
				}
				ImGui::SameLine();

				// �폜�{�^��.
				if ( ImGuiManager::Button( std::string( u8"�폜##" + ListItr->first ).c_str() ) ) {
					std::filesystem::remove( std::string( SAVE_FILE_PATH ) + "\\" + ListItr->first + ".txt" );
					ListItr = m_BugList.erase( ListItr );
					continue;
				}
				ImGui::SameLine();

				// �o�O���e�̕\��.
				if ( ImGui::TreeNode( std::string( "##" + ListItr->first ).c_str() ) ) {
					ImGui::Separator();
					ImGui::Text( u8"[ �o�O���x�� ]" );
					ImGui::Text( ListItr->second[BUG_TEXT_LV_POS].c_str() );
					ImGui::Separator();
					ImGui::Text( u8"[ �o�O���e ]" );
					const int ColumnNum = static_cast<int>( ListItr->second.size() ) - BUG_TEXT_NORMAL_COLUMN + 1;
					for ( int i = 0; i < ColumnNum; ++i ) {
						const int& ContentsPos = BUG_TEXT_CONTENTS_POS + i;
						ImGui::Text( ListItr->second[ContentsPos].c_str() );
					}
					ImGui::Separator();
					ImGui::Text( u8"[ ������ ]" );
					ImGui::Text( ListItr->second[BUG_TEXT_DISCOVERER_POS].c_str() );
					ImGui::Separator();
					ImGui::TreePop();
				}
				ImGui::Separator();
				ListItr++;
			}
			ImGui::TreePop();
		}
		ImGui::Separator();
	};

	// �E�B���h�E���̐ݒ�.
	InitWndName( "BugReport" );
}

//---------------------------.
// �o�O���X�g��ǂݍ���.
//---------------------------.
HRESULT CBugReport::BugListLoad()
{
	bool IsFixedFile = false;
	auto ListLoad = [&]( const std::filesystem::directory_entry& Entry )
	{
		const std::string Extension = Entry.path().extension().string();	// �g���q.
		const std::string FilePath	= Entry.path().string();				// �t�@�C���p�X.
		const std::string FileName	= Entry.path().stem().string();			// �t�@�C����.

		// �g���q�� ".txt" �ł͂Ȃ��ꍇ�ǂݍ��܂Ȃ�.
		if ( Extension != ".txt"							) return;

		//	�C�������ς݂̃o�O�͓ǂݍ��܂Ȃ�.
		if ( FilePath.find( "Fixed" ) != std::string::npos ) {
			IsFixedFile = true;
			return;
		}

		// �o�O�񍐃t�@�C���̓ǂݍ���.
		m_BugList[FileName] = FileManager::TextLoad( FilePath );
	};

	try {
		m_BugList.clear();
		if( !std::filesystem::exists( SAVE_FILE_PATH ) ) return S_OK;
		std::filesystem::recursive_directory_iterator Dir_itr( SAVE_FILE_PATH ), End_itr;
		std::for_each( Dir_itr, End_itr, ListLoad );
	}
	catch ( const std::filesystem::filesystem_error& e ) {
		// �G���[���b�Z�[�W��\��.
		ErrorMessage( e.path1().string().c_str() );
		return E_FAIL;
	}

	// �t�@�C������̏ꍇ�폜����.
	if ( !IsFixedFile ) {
		if ( m_BugList.empty() ) {
			std::filesystem::remove_all( SAVE_FILE_PATH );
			return S_OK;;
		}
		if ( std::filesystem::exists( std::string( SAVE_FILE_PATH ) + "\\Fixed" ) ) {
			std::filesystem::remove_all( std::string( SAVE_FILE_PATH ) + "\\Fixed" );
			return S_OK;
		}
	}

	return S_OK;
}
