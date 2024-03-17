#include "BugReport.h"
#include "..\..\Utility\FileManager\FileManager.h"
#include <fstream>
#include <sstream>
#include <chrono>
#include <iomanip>
#include <filesystem>

namespace {
	constexpr char SAVE_FILE_PATH[]			= "Data\\BugReport";	// 保存する場所のファイルパス.
	constexpr int  BUG_TEXT_NORMAL_COLUMN	= 10;					// バグ報告テキストの通常時(内容に改行を含まない時)の列数.
	constexpr int  BUG_TEXT_TITLE_POS		= 1;					// バグ報告テキストのタイトルの位置.
	constexpr int  BUG_TEXT_LV_POS			= 4;					// バグ報告テキストのレベルの位置.
	constexpr int  BUG_TEXT_CONTENTS_POS	= 6;					// バグ報告テキストの内容の位置.
	constexpr int  BUG_TEXT_DISCOVERER_POS	= 8;					// バグ報告テキストの発見者の位置.
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
// 初期化関数.
//---------------------------.
void CBugReport::Init()
{
	// バグリストの読み込み.
	BugListLoad();

	// 描画処理の設定.
	m_RenderFunc = [&] () {
		ImGui::Text( u8"発生したバグを記入し管理することができます。" );
		ImGui::Text( u8"発生したバグの「タイトル」、「バグレベル」、「内容」を記入してください。" );
		ImGui::Text( u8"記入したバグは「Data\\BugReport」の中に保存されます。" );
		ImGui::Text( u8"「修正完了」ボタンを押すことで「Data\\BugReport\\Fixed」の中に移動されます。" );
		ImGui::Separator();

		// バグの報告.
		ImGui::SetNextTreeNodeOpen( true, ImGuiCond_Once );
		if ( ImGui::TreeNode( "BugReport" ) ) {

			// 発見者の記入.
			ImGui::Text( u8"< 発見者 >" );
			ImGui::InputText( "##Discoverer", &m_Discoverer );
			ImGui::Separator();

			// バグレベルの指定.
			ImGui::SameLine();
			ImGuiManager::Combo( u8"< バグレベル >", &m_BugLv );
			ImGui::SameLine();
			ImGuiManager::HelpMarker( 
				u8"---------------------------------------------\n"
				u8"バグのレベルを指定してください。\n"
				u8"レベルの説明は以下の通りです。\n"
				u8"---------------------------------------------\n"
				u8"[ S ] : ゲーム進行に問題が出る。\n"
				u8"      : 普通にゲームしていて遭遇する。\n"
				u8"---------------------------------------------\n"
				u8"[ A ] : プレイヤーに著しい不利益をもたらす。\n"
				u8"      : 特定の手順で発生する。\n"
				u8"---------------------------------------------\n"
				u8"[ B ] : プレイヤーにある程度不利益をもたらす。\n"
				u8"      : ゲーム進行に問題はない。\n"
				u8"---------------------------------------------\n"
				u8"[ C ] : 明らかに不具合だが致命的ではない。\n"
				u8"---------------------------------------------\n"
				u8"[ D ] : 不具合とまではいかないが、気になる。\n"
				u8"---------------------------------------------\n"
			);
			ImGui::Separator();

			// バグタイトルの記入.
			ImGui::Text( u8"< バグのタイトル >" );
			ImGui::InputText( "##BugTitle", &m_BugTitle );
			// バグ内容の記入.
			ImGui::Text( u8"< バグの内容 >" );
			ImGui::InputTextMultiline( "##BugContent", &m_BugContents, ImVec2( 0.0f, 100.0f ) );
			ImGui::Separator();

			// 出力ボタン.
			if ( ImGuiManager::Button( u8"保存##BugSave", !m_BugTitle.empty() && !m_BugContents.empty() && !m_Discoverer.empty() ) ) {
				const EBugLv		Lv		= m_BugLv;
				const std::string	Title	= m_BugTitle;
				const std::string	Msg		= m_BugContents;
				const std::string	Name	= m_Discoverer;

				// 現在の時間を取得.
				auto now = std::chrono::system_clock::now();
				auto now_c = std::chrono::system_clock::to_time_t( now );
				std::stringstream Time;
				Time << std::put_time( localtime( &now_c ), "%Y%m%d_%H%M%S" );

				// テキストファイル出力.
				const std::string FilePath = std::string( SAVE_FILE_PATH ) + "\\" + Time.str() + ".txt";
				FileManager::TextSave( FilePath, std::string( 
					u8"----------------------------------------\n" +
					Title + u8"\n" +
					u8"----------------------------------------\n" +
					u8"[ バグレベル ]\n" +
					StringConversion::Enum( Lv ) + u8"\n" +
					u8"[ バグ内容 ]\n" +
					Msg + u8"\n\n" +
					u8"----------------------------------------\n" +
					u8"発見者 : " + Name + u8"\n" +
					u8"----------------------------------------"
				) );

				// 初期化.
				m_BugLv			= EBugLv::S;
				m_BugTitle		= "";
				m_BugContents	= "";

				// バグリストの再読み込み.
				BugListLoad();
			}
			// 削除ボタン.
			ImGui::SameLine();
			if ( ImGuiManager::Button( u8"削除" ) ) {
				m_BugLv			= EBugLv::S;
				m_BugContents	= "";
			}
			ImGui::TreePop();
		}

		// バグリスト.
		ImGui::Separator();
		if ( m_BugList.empty() ) return;
		if ( ImGui::TreeNode( "BugList" ) ) {
			ImGui::Separator();
			auto ListItr = m_BugList.begin();
			while ( ListItr != m_BugList.end() ) {
				// バグタイトルの表示.
				ImGui::Text( ListItr->second[BUG_TEXT_TITLE_POS].c_str() );
				ImGui::SameLine();

				// 修正完了ボタン.
				if( ImGuiManager::Button( std::string( u8"修正完了##" + ListItr->first ).c_str() ) ) {
					const std::string NowPath = std::string( SAVE_FILE_PATH ) + "\\" + ListItr->first + ".txt";
					const std::string NewPath = std::string( SAVE_FILE_PATH ) + "\\Fixed\\" + ListItr->first + ".txt";
					std::filesystem::create_directories( std::string( SAVE_FILE_PATH ) + "\\Fixed" );
					std::filesystem::rename( NowPath, NewPath );
					ListItr = m_BugList.erase( ListItr );
					continue;
				}
				ImGui::SameLine();

				// 削除ボタン.
				if ( ImGuiManager::Button( std::string( u8"削除##" + ListItr->first ).c_str() ) ) {
					std::filesystem::remove( std::string( SAVE_FILE_PATH ) + "\\" + ListItr->first + ".txt" );
					ListItr = m_BugList.erase( ListItr );
					continue;
				}
				ImGui::SameLine();

				// バグ内容の表示.
				if ( ImGui::TreeNode( std::string( "##" + ListItr->first ).c_str() ) ) {
					ImGui::Separator();
					ImGui::Text( u8"[ バグレベル ]" );
					ImGui::Text( ListItr->second[BUG_TEXT_LV_POS].c_str() );
					ImGui::Separator();
					ImGui::Text( u8"[ バグ内容 ]" );
					const int ColumnNum = static_cast<int>( ListItr->second.size() ) - BUG_TEXT_NORMAL_COLUMN + 1;
					for ( int i = 0; i < ColumnNum; ++i ) {
						const int& ContentsPos = BUG_TEXT_CONTENTS_POS + i;
						ImGui::Text( ListItr->second[ContentsPos].c_str() );
					}
					ImGui::Separator();
					ImGui::Text( u8"[ 発見者 ]" );
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

	// ウィンドウ名の設定.
	InitWndName( "BugReport" );
}

//---------------------------.
// バグリストを読み込む.
//---------------------------.
HRESULT CBugReport::BugListLoad()
{
	bool IsFixedFile = false;
	auto ListLoad = [&]( const std::filesystem::directory_entry& Entry )
	{
		const std::string Extension = Entry.path().extension().string();	// 拡張子.
		const std::string FilePath	= Entry.path().string();				// ファイルパス.
		const std::string FileName	= Entry.path().stem().string();			// ファイル名.

		// 拡張子が ".txt" ではない場合読み込まない.
		if ( Extension != ".txt"							) return;

		//	修正完了済みのバグは読み込まない.
		if ( FilePath.find( "Fixed" ) != std::string::npos ) {
			IsFixedFile = true;
			return;
		}

		// バグ報告ファイルの読み込み.
		m_BugList[FileName] = FileManager::TextLoad( FilePath );
	};

	try {
		m_BugList.clear();
		if( !std::filesystem::exists( SAVE_FILE_PATH ) ) return S_OK;
		std::filesystem::recursive_directory_iterator Dir_itr( SAVE_FILE_PATH ), End_itr;
		std::for_each( Dir_itr, End_itr, ListLoad );
	}
	catch ( const std::filesystem::filesystem_error& e ) {
		// エラーメッセージを表示.
		ErrorMessage( e.path1().string().c_str() );
		return E_FAIL;
	}

	// ファイルが空の場合削除する.
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
