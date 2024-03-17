#include "WindowSetting.h"

namespace {
	constexpr char WINDOW_SETTING_FILE_PATH[] = "Data\\Parameter\\Config\\WindowSetting.json";	// ウィンドウの設定のファイルパス.
}

CWindowSetting::CWindowSetting()
	: m_Color					( INIT_FLOAT3 )
	, m_Size					( INIT_FLOAT2 )
	, m_Pos						( INIT_FLOAT2 )
	, m_AppName					( "" )
	, m_WndName					( "" )
	, m_IsDispCenter			( false )
	, m_IsDispMouseCursor		( false )
	, m_IsFPSRender				( false )
	, m_IsFullScreenLock		( false )
	, m_IsSizeLock				( false )
	, m_IsStartFullScreen		( false )
	, m_IsMaxLock				( false )
	, m_IsMinLock				( false )
	, m_IsPopUpWnd				( false )
	, m_IsShowOnTop				( false )
	, m_IsLogStop				( false )
	, m_IsInputNotActiveStop	( false )
	, m_Version					( "1.0.0" )
{
	Init();
}

CWindowSetting::~CWindowSetting()
{
}

//---------------------------.
// 初期化関数.
//---------------------------.
void CWindowSetting::Init()
{
	// 現在の設定を取得する.
	json WndData = FileManager::JsonLoad( WINDOW_SETTING_FILE_PATH );
	m_Color.x				= WndData["BackColor"]["R"];
	m_Color.y				= WndData["BackColor"]["G"];
	m_Color.z				= WndData["BackColor"]["B"];
	m_Size.x				= WndData["Size"]["w"];
	m_Size.y				= WndData["Size"]["h"];
	m_Pos.x					= WndData["Pos"]["x"];
	m_Pos.y					= WndData["Pos"]["y"];
	m_AppName				= WndData["Name"]["App"];
	m_WndName				= WndData["Name"]["Wnd"];
	m_IsDispCenter			= WndData["IsDispCenter"];
	m_IsDispMouseCursor		= WndData["IsDispMouseCursor"];
	m_IsFPSRender			= WndData["IsFPSRender"];
	m_IsFullScreenLock		= WndData["IsFullScreenLock"];
	m_IsInputNotActiveStop	= WndData["IsInputNotActiveStop"];
	m_IsSizeLock			= WndData["IsSizeLock"];
	m_IsStartFullScreen		= WndData["IsStartFullScreen"];
	m_IsMaxLock				= WndData["IsMaxLock"];
	m_IsMinLock				= WndData["IsMinLock"];
	m_IsPopUpWnd			= WndData["IsPopUpWnd"];
	m_IsShowOnTop			= WndData["IsShowOnTop"];
	m_IsLogStop				= WndData["IsLogStop"];
	m_IsDispCloseMessage	= WndData["IsDispCloseMessage"];
	m_Version				= WndData["Version"];

	// 描画処理の設定.
	m_RenderFunc = [&] () {
		ImGui::Text( u8"ウィンドウの設定をすることができます。" );
		ImGui::Text( u8"保存後、再起動することで設定が反映されます。" );
		ImGui::Separator();

		// アプリバージョンの記入欄.
		ImGui::Text( u8"アプリケーションバージョン　　: " );
		ImGui::SameLine();
		ImGui::InputText( "##AppVersion", &m_Version );

		// アプリ名の記入欄.
		ImGui::Text( u8"アプリケーション名　　: " );
		ImGui::SameLine();
		ImGui::InputText( "##AppName", &m_AppName );

		// ウィンドウ名の記入欄.
		ImGui::Text( u8"ウィンドウ名　　　　　: " );
		ImGui::SameLine();
		ImGui::InputText( "##WndName", &m_WndName );

		// ウィンドウのサイズ指定.
		ImGui::Separator();
		ImGui::Text( u8"ウィンドウサイズ　　　: " );
		ImGui::SameLine();
		ImGui::InputFloat2( "##InputWndSize", m_Size );

		// ウィンドウの表示位置指定.
		ImGui::Text( u8"ウィンドウの表示位置　: " );
		ImGui::SameLine();
		ImGui::InputFloat2( "##InputWndPose", m_Pos );

		// 背景の色の指定.
		ImGui::Text( u8"背景の色　　　　　　　: " );
		ImGui::SameLine();
		ImGui::ColorEdit3( "##BackColor", m_Color );

		// 各フラグのチェックボックスの表示.
		ImGui::Separator();
		ImGuiManager::CheckBox( u8"中央に表示",				&m_IsDispCenter );
		ImGui::SameLine();
		ImGuiManager::HelpMarker( u8"起動時、画面の中央に配置します。" );
		ImGuiManager::CheckBox( u8"フルスクリーンの禁止",	&m_IsFullScreenLock );
		ImGui::SameLine();
		ImGuiManager::HelpMarker( u8"「ALT + Enter」でフルスクリーンにできないようにします。" );
		ImGuiManager::CheckBox( u8"サイズの固定化",			&m_IsSizeLock );
		ImGui::SameLine();
		ImGuiManager::HelpMarker( u8"サイズを変更できないようにします。" );
		ImGuiManager::CheckBox( u8"最大化の無効化",			&m_IsMaxLock );
		ImGui::SameLine();
		ImGuiManager::HelpMarker( u8"最大化ボタンを押せないようにします。" );
		ImGuiManager::CheckBox( u8"最小化の無効化",			&m_IsMinLock );
		ImGui::SameLine();
		ImGuiManager::HelpMarker( u8"最小化ボタンを押せないようにします。" );
		ImGuiManager::CheckBox( u8"ポップアップウィンド",	&m_IsPopUpWnd );
		ImGui::SameLine();
		ImGuiManager::HelpMarker( u8"ウィンドウをポップアップウィンドウ（ 枠無し ）にします。" );
		ImGuiManager::CheckBox( u8"最前面で固定",			&m_IsShowOnTop );
		ImGui::SameLine();
		ImGuiManager::HelpMarker( u8"ウィンドウを最前面で固定します。" );
		ImGuiManager::CheckBox( u8"フルスクリーンで起動",	&m_IsStartFullScreen );
		ImGui::SameLine();
		ImGuiManager::HelpMarker( u8"起動時にフルスクリーンで起動します。" );
		ImGuiManager::CheckBox( u8"ログファイルの停止",		&m_IsLogStop );
		ImGui::SameLine();
		ImGuiManager::HelpMarker( u8"ログファイルの作成を停止します。" );
		ImGuiManager::CheckBox( u8"アクティブウィンドウでは無い場合の操作の停止", &m_IsInputNotActiveStop );
		ImGui::SameLine();
		ImGuiManager::HelpMarker( u8"アクティブウィンドウでは無い場合操作を停止させます。" );
		ImGuiManager::CheckBox( u8"メッセージボックスの表示", &m_IsDispCloseMessage );
		ImGui::SameLine();
		ImGuiManager::HelpMarker( u8"「Escape」を押した時にウィンドウを閉じるかのメッセージボックスを表示させます。" );;
		ImGuiManager::CheckBox( u8"FPSの表示", &m_IsFPSRender );
		ImGui::SameLine();
		ImGuiManager::HelpMarker( u8"画面にFPSを表示させます。" );
		ImGuiManager::CheckBox( u8"マウスカーソルの表示", &m_IsDispMouseCursor );
		ImGui::SameLine();
		ImGuiManager::HelpMarker( u8"マウスカーソルを画面に表示させます。" );

		// 保存ボタン.
		ImGui::Separator();
		if ( ImGuiManager::Button( u8"保存##WindowSettingSave" ) ) {
			// 現在のウィンドウ設定をjson形式で保存する.
			json j;
			j[".Comment"] = {
				u8"ウィンドウの設定が行えます。",
				u8"----------------------------------------------",
				u8"[BackColor           ] : 背景の色",
				u8"[IsDispCenter        ] : 起動時に画面の中央に配置するか",
				u8"[IsDispCloseMessage  ] : 「Escape」を押した時にメッセージボックスを表示させるか",
				u8"[IsDispMouseCursor   ] : 画面にマウスカーソルを表示するか",
				u8"[IsFPSRender         ] : 画面にFPSを描画するか",
				u8"[IsFullScreenLock    ] : 「ALT + Enter」でフルスクリーンにできないようにするか",
				u8"[IsInputNotActiveStop] : アクティブウィンドウでは無い場合操作を停止させるか",
				u8"[IsLogStop           ] : ログファイルの作成を停止するか",
				u8"[IsMaxLock           ] : 最大化ボタンを押せないようにするか",
				u8"[IsMinLock           ] : 最小化ボタンを押せないようにするか",
				u8"[IsPopUpWnd          ] : ウィンドウをポップアップウィンドウ（ 枠無し ）にするか",
				u8"[IsShowOnTop         ] : ウィンドウを最前面で固定するか",
				u8"[IsStartFullScreen   ] : 起動時にフルスクリーンで起動するか",
				u8"[IsSizeLock          ] : サイズを変更できないようにするか",
				u8"[Name                ] : アプリ / ウィンドウ名",
				u8"[Pos                 ] : ウィンドウの表示位置",
				u8"[Size                ] : ウィンドウのサイズ",
				u8"[Version             ] : アプリバージョン",
				u8"----------------------------------------------"
			};
			j["BackColor"]["R"]			= m_Color.x;
			j["BackColor"]["G"]			= m_Color.y;
			j["BackColor"]["B"]			= m_Color.z;
			j["Size"]["w"]				= m_Size.x;
			j["Size"]["h"]				= m_Size.y;
			j["Pos"]["x"]				= m_Pos.x;
			j["Pos"]["y"]				= m_Pos.y;
			j["Name"]["App"]			= m_AppName;
			j["Name"]["Wnd"]			= m_WndName;
			j["IsDispCenter"]			= m_IsDispCenter;
			j["IsDispCloseMessage"]		= m_IsDispCloseMessage;
			j["IsDispMouseCursor"]		= m_IsDispMouseCursor;
			j["IsFPSRender"]			= m_IsFPSRender;
			j["IsFullScreenLock"]		= m_IsFullScreenLock;
			j["IsInputNotActiveStop"]	= m_IsInputNotActiveStop;
			j["IsSizeLock"]				= m_IsSizeLock;
			j["IsMaxLock"]				= m_IsMaxLock;
			j["IsMinLock"]				= m_IsMinLock;
			j["IsPopUpWnd"]				= m_IsPopUpWnd;
			j["IsShowOnTop"]			= m_IsShowOnTop;
			j["IsStartFullScreen"]		= m_IsStartFullScreen;
			j["IsLogStop"]				= m_IsLogStop;
			j["Version"]				= m_Version;
			FileManager::JsonSave( WINDOW_SETTING_FILE_PATH, j );
		}
	};

	// ウィンドウ名の設定.
	InitWndName( "WindowSetting" );
}
