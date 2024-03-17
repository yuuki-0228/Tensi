#include "MessageWindow.h"
#include "..\..\FileManager\FileManager.h"
#include <fstream>
#include <sstream>
#include <chrono>
#include <iomanip>

namespace{
	// メッセージウィンドウの保存上限.
	constexpr int	MESSAGE_MAX				= 20;
	// 新しいメッセージが来た時の色の変更時間.
	constexpr int	NEW_MESSAGE_COLOR_TIME	= 120;
	// テキストのパス.
	constexpr char	TEXT_PATH[]				= "Data\\Parameter\\Config\\MessageWindow.json";

	// 使用色の列挙体.
	enum enColor{
		Red,		// 重要なことなどに使用.
		Green,		// 通知やおしらせなどに使用.
		Blue,		// 未使用.
		Yellow,		// 画面の切替に使用.
		Cyan,		// 音源関係で使用.
		Magenta,	// エディタで使用.
		white,		// デフォルト.
		Gray,		// 全てのメッセージの追加を停止.

		Max
	} typedef EColor;
}

MessageWindow::MessageWindow()
	: m_MessageList			()
	, m_ColorBlockFlags		()
	, m_ColorTable			()
	, m_FindString			( "" )
	, m_IsFindTime			( false )
	, m_IsNewMsg			( false )
{
	m_MessageList.reserve( MESSAGE_MAX );
	SetColorTable();
}

MessageWindow::~MessageWindow()
{
}

//----------------------------.
// インスタンスの取得.
//----------------------------.
MessageWindow* MessageWindow::GetInstance()
{
	static std::unique_ptr<MessageWindow> pInstance = std::make_unique<MessageWindow>();
	return pInstance.get();
}

//-----------------------------.
// 描画.
//-----------------------------.
void MessageWindow::Render()
{
	MessageWindow* pI = GetInstance();

	if ( pI->m_IsNewMsg ) {
		// 色を赤色に変更する.
		ImGui::PushStyleColor( ImGuiCol_TitleBgCollapsed,	ImVec4( 0.3f, 0.0f, 0.0f, 1.0f ) );
		ImGui::PushStyleColor( ImGuiCol_TitleBg,			ImVec4( 0.4f, 0.0f, 0.0f, 1.0f ) );
		ImGui::PushStyleColor( ImGuiCol_TitleBgActive,		ImVec4( 0.6f, 0.0f, 0.0f, 1.0f ) );
		ImGui::PushStyleColor( ImGuiCol_Tab,				ImVec4( 0.3f, 0.0f, 0.0f, 1.0f ) );
		ImGui::PushStyleColor( ImGuiCol_TabActive,			ImVec4( 0.4f, 0.0f, 0.0f, 1.0f ) );
		ImGui::PushStyleColor( ImGuiCol_TabHovered,			ImVec4( 0.6f, 0.0f, 0.0f, 1.0f ) );
		ImGui::PushStyleColor( ImGuiCol_TabUnfocused,		ImVec4( 0.4f, 0.0f, 0.0f, 1.0f ) );
		ImGui::PushStyleColor( ImGuiCol_TabUnfocusedActive,	ImVec4( 0.6f, 0.0f, 0.0f, 1.0f ) );
	}
	ImGui::Begin( "MessageWindow" );

	// メッセージ検索の表示.
	MessageFind();

	// カラーブロックの表示.
	ColorBlock();

	// 文字の表示.
	const int QueueSize = static_cast<int>( pI->m_MessageList.size() );
	ImGuiManager::Group( [&]() {
		for ( int i = QueueSize - 1; i >= 0; i-- ) {
			std::function<void()> proc = pI->m_MessageList[i];
			proc();
		}
	} );
	if ( pI->m_IsNewMsg ) {
		// 変更した色の個数分処理を行う.
		for( int i = 0; i < 8; ++i ) 
			ImGui::PopStyleColor();

		// カウントを減らす.
		pI->m_ColorResetCnt--;
		if ( pI->m_ColorResetCnt <= 0 ) {
			pI->m_IsNewMsg = false;
		}
	}
	ImGui::End();
}


//----------------------.
// メッセージウィンドウに追加.
//	全てのメッセージをブロック中は処理は行わない.
//----------------------.
void MessageWindow::PushMessage( const std::string& Message, const D3DXCOLOR3& Color )
{
	PushMessage( Message, Color4::RGBA( Color ) );
}
void MessageWindow::PushMessage( const std::string& Message, const D3DXCOLOR4& Color )
{
	MessageWindow* pI = GetInstance();

	if ( pI->m_ColorBlockFlags[Gray] ) return;

	// 現在の色にブロックフラグが立っているなら抜ける.
	for ( int i = 0; i < EColor::Max; i++ ){
		if ( Color == pI->m_ColorTable[i] &&
			!pI->m_ColorBlockFlags[i] ) return;
	}

	// 初期化.
	pI->m_IsNewMsg		= true;
	pI->m_ColorResetCnt = NEW_MESSAGE_COLOR_TIME;

	// 時間の取得.
	auto now = std::chrono::system_clock::now();
	auto now_c = std::chrono::system_clock::to_time_t( now );
	std::stringstream Time;
	Time << std::put_time( localtime( &now_c ), "[%H:%M:%S]" );

	// テキストの作成.
	std::string Text;
	Text = Time.str() + " : " + Message;

	// メッセージの数が上限以上なら古いメッセージを削除する.
	if ( pI->m_MessageList.size() >= MESSAGE_MAX )
		pI->m_MessageList.erase( pI->m_MessageList.begin() );

	// 関数を作成.
	pI->m_MessageList.emplace_back(
	[=]() {
		// 検索する文字の指定.
		std::string FindString;
		if ( pI->m_IsFindTime )	FindString = Text;		// 時間を含む.
		else					FindString = Message;	// 時間を含まない.

		// メッセージに検索する文字が含まれている場合.
		if ( FindString.find( pI->m_FindString ) != std::string::npos ){
			// 文字の表示.
			ImGui::TextColored( ImGuiManager::ConvertVector4ToIm( Color ), Text.c_str() );
		}
	} );
}

//----------------------.
// メッセージウィンドウ.
//----------------------.
void MessageWindow::MessageFind()
{
	MessageWindow* pI = GetInstance();

	ImGui::Text( u8"検索 :" );
	ImGui::SameLine();
	ImGui::InputText( "##MsgWnd_InputText", &pI->m_FindString );
	ImGui::SameLine();
	ImGuiManager::CheckBox( u8"##MsgWnd_CheckBox", &pI->m_IsFindTime );
	ImGui::SameLine();
	if ( ImGuiManager::Button( u8"削除" ) ) pI->m_MessageList.clear();
	ImGui::Separator();
}

//----------------------.
// メッセージウィンドウのカラーブロック処理.
//----------------------.
void MessageWindow::ColorBlock()
{
	MessageWindow* pI = GetInstance();

	const	std::vector<D3DXCOLOR4> Color = pI->m_ColorTable;
	bool	Flags[EColor::Max];

	// チェックボックスの表示.
	ImGui::Text( u8"設定 : " );
	ImGui::SameLine();
	for ( int i = 0; i < EColor::Max; i++ ){
		Flags[i] = pI->m_ColorBlockFlags[i];
		const std::string& No = std::to_string( i );
		ImGuiManager::CheckBox( No.c_str(), &Flags[i], Color[i] );
		pI->m_ColorBlockFlags[i] = Flags[i];
		ImGui::SameLine();
	}
	ImGuiManager::HelpMarker(
		u8"---------------------------------------------------------------------\n"
		u8"1:赤色 : 重要なことなどに使用     / [ エラー/警告など ]\n"
		u8"2:緑色 : 通知やおしらせなどに使用 / [ BGMの停止/ボスの行動変化など ]\n"
		u8"3:青色 : 未使用\n"
		u8"4:黄色 : 画面の切替に使用         / [ シーン切替/フェードなど ]\n"
		u8"5:水色 : 音源関係で使用           / [ BGM/SEの再生 ]\n"
		u8"6:紫色 : エディタで使用           / [ UIエディタなど ]\n"
		u8"7:白色 : デフォルト\n"
		u8"8:灰色 : 全てのメッセージの追加を停止\n"
		u8"---------------------------------------------------------------------\n"
	);
	ImGui::Separator();
}

//----------------------.
// カラーテーブルの設定.
//----------------------.
void MessageWindow::SetColorTable()
{
	const json& Config = FileManager::JsonLoad( TEXT_PATH );
	m_ColorBlockFlags.resize( EColor::Max, true );
	m_ColorBlockFlags[Red]		= Config["Red"];
	m_ColorBlockFlags[Green]	= Config["Green"];
	m_ColorBlockFlags[Blue]		= Config["Blue"];
	m_ColorBlockFlags[Yellow]	= Config["Yellow"];
	m_ColorBlockFlags[Cyan]		= Config["Cyan"];
	m_ColorBlockFlags[Magenta]	= Config["Magenta"];
	m_ColorBlockFlags[white]	= Config["white"];
	m_ColorBlockFlags[Gray]		= Config["Gray"];

	m_ColorTable = {
		Color4::Red,
		Color4::Green,
		Color4::Blue,
		Color4::Yellow,
		Color4::Cyan,
		Color4::Magenta,
		Color4::White,
		Color4::Gray
	};
}
