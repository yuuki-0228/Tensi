#include "Main.h"
#include "..\Common\DirectX\DirectX9.h"
#include "..\Common\DirectX\DirectX11.h"
#include "..\Common\WindowTextRenderer\WindowTextRenderer.h"
#include "..\Common\SoundManeger\SoundManeger.h"
#include "..\Object\Camera\CameraManager\CameraManager.h"
#include "..\Object\Light\Light.h"
#include "..\Object\Collision\CollisionRenderer\CollisionRenderer.h"
#include "..\Resource\LoadManager\LoadManager.h"
#include "..\Scene\SceneManager\SceneManager.h"
#include "..\Utility\FrameRate\FrameRate.h"
#include "..\Utility\ImGuiManager\ImGuiManager.h"
#include "..\Utility\Random\Random.h"
#include "..\Utility\Input\Input.h"
#include "..\Utility\FileManager\FileManager.h"
#include "..\Utility\FileManager\DragAndDrop\DragAndDrop.h"
#include "..\Utility\FileManager\DragAndDrop\DragAndDrop.h"
#include "..\Utility\Message\Message.h"
#include "..\Utility\WindowManager\WindowManager.h"
#include "..\Utility\MenuManager\MenuManager.h"
#include "..\System\SystemWindowManager\SystemWindowManager.h"
#include <dwmapi.h>

// ImGUiで使用.
extern LRESULT ImGui_ImplWin32_WndProcHandler( HWND, UINT, WPARAM, LPARAM );

static const decltype( WM_USER ) WM_NOTIFYICON = WM_USER + 100;

// サブウィンドウを常時クリック判定を無くすか
#define SUB_WND_ALWAY_CLICK_LOSE

namespace{
	constexpr char		PARAMETER_FILE_PATH[]		= "Data\\Parameter\\";								// パラメーターファイルパス.
	constexpr char		WINDOW_SETTING_FILE_PATH[]	= "Data\\Parameter\\Config\\WindowSetting.json";	// ウィンドウの設定のファイルパス.
	constexpr float		FPS_RENDER_SIZE				= 0.5f;												// FPS描画サイズ.
	constexpr float		FPS_RENDER_POS[2]			= { 0.0f, 1.0f };									// FPS描画位置.
	constexpr float		FPS_RENDER_COLOR[4]			= { 0.9f, 0.2f, 0.2f, 1.0f };						// FPS描画色.

	// フェードの状態.
	enum enFadeState : unsigned char {
		None,
		FadeIn,		// フェードイン中.
		FadeOut		// フェードアウト中.
	} typedef EFadeState;
}

CMain::CMain()
	: m_hWnd			( nullptr )
	, m_hSubWnd			( nullptr )
	, m_hDc				( nullptr )
	, m_hSubDc			( nullptr )
	, m_pFrameRate		( nullptr )
	, m_pLoadManager	( nullptr )
	, m_IsGameLoad		( false )
	, m_IsFPSRender		( false )
#ifdef _DEBUG
	, m_IsWindowTop		( false )
#else
	, m_IsWindowTop		( true )
#endif
{
	m_pFrameRate	= std::make_unique<CFrameRate>( FPS );
	m_pLoadManager	= std::make_unique<CLoadManager>();
}

CMain::~CMain()
{
	DeleteDC(		m_hSubDc	);
	DeleteDC(		m_hDc		);
	DeleteObject(	m_hSubWnd	);
	DeleteObject(	m_hWnd		);
}

//---------------------------.
// 更新処理.
//---------------------------.
void CMain::Update( const float& DeltaTime )
{
	// 起動時のウィンドウの初期化.
	if ( m_IsWindowInit == false ) WindowInit();

	// 最前面で固定し続ける.
	if ( m_IsWindowTop ) SetWindowPos( m_hWnd, HWND_TOPMOST, 0, 0, 0, 0, ( SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW ) );
#ifdef _DEBUG
	// 最前面で固定を解除.
	if ( KeyInput::IsANDKeyDown( 'W', 'T' ) ) {
		m_IsWindowTop = !m_IsWindowTop;
		if ( m_IsWindowTop == false ) {
			SetWindowPos( m_hWnd, HWND_NOTOPMOST,	0, 0, 0, 0, ( SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW ) );
			SetWindowPos( m_hWnd, HWND_TOP,			0, 0, 0, 0, ( SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW ) );
		}
	}
#endif

	// ウィンドウのクリック判定の更新.
	ClickUpdate();

	// 更新処理.
	DirectX11::CheckActiveWindow();
	ImGuiManager::SetingNewFrame();
	WindowManager::Update();
	Input::Update();
	SceneManager::Update( DeltaTime );
	CameraManager::Update( DeltaTime );
	Light::Update();
	Message::Update( DeltaTime );
	SystemWindowManager::Update( DeltaTime );

	// バックバッファをクリアにする.
	DirectX11::ClearBackBuffer( 0 );

	// 描画処理.
	SceneManager::Render();
	FPSRender();

	DirectX11::Present( 0 );

	// サブウィンドウのバックバッファをクリアにする.
	DirectX11::ClearBackBuffer( 1 );

	// サブウィンドウの描画処理.
	SceneManager::SubRender();

	CollisionRenderer::Render();
	ImGuiManager::Render();

	// 画面に表示.
	DirectX11::Present( 1 );

	// TODO: もう一度呼ぶとちらつきが起きない?
	DirectX11::Present( 0 );

	// 操作のログを出力.
	Input::KeyLogOutput();

	// ホイール値を初期化.
	Input::SetMouseWheelDelta( 0 );
}

//---------------------------.
// 構築処理.
//---------------------------.
HRESULT CMain::Create()
{
	// DirectX9の構築.
	if ( FAILED( DirectX9::Create( m_hWnd	)	) ) return E_FAIL;
	// DirectX11の構築.
	if ( FAILED( DirectX11::Create( { m_hWnd, m_hSubWnd } ) ) ) return E_FAIL;
	// オーディオインターフェースの構築.
	if ( FAILED( SoundManager::Create()		) ) return E_FAIL;
	// 乱数の初期化.
	if ( FAILED( Random::Init()				) ) return E_FAIL;
	// 入力クラスの初期化.
	if ( FAILED( Input::Init()					) ) return E_FAIL;
	// デバックテキストの初期化.
	if ( FAILED( WindowTextRenderer::Init()			) ) return E_FAIL;
	// ImGuiの初期化.
	if ( FAILED( ImGuiManager::Init( m_hWnd )	) ) return E_FAIL;
	// ウィンドウマネージャーの初期化.
	if ( FAILED( WindowManager::Init()			) ) return E_FAIL;
	// メニューの初期化.
	if ( FAILED( MenuManager::Init( m_hWnd )	) ) return E_FAIL;

	// 深度を無くす.
	DirectX11::SetDepth( false );

	// フルスクリーンで起動するか.
	json WndSetting = FileManager::JsonLoad( WINDOW_SETTING_FILE_PATH );
	if ( WndSetting["IsStartFullScreen"] ) {
		// フルスクリーンに設定.
		DirectX11::SetFullScreen( true );
	}

	// リソースの読み込み.
	m_pLoadManager->LoadResource( m_hWnd );

	// ユーザー名の取得.
#ifndef _DEBUG
	char UserName[256];
	DWORD Size = sizeof( UserName );
	if ( GetUserNameA( UserName, &Size ) == 0 ) {
		//取得に失敗した
		puts( "ユーザー名の取得に失敗しました。" );
	}

	// ショートカットの作成.
	const std::string WorkFile		= std::filesystem::current_path().string();
	const std::string ExePath		= WorkFile + "\\Slime.exe";
	const std::string ShortcutPath	= "C:\\Users\\" + std::string( UserName ) + "\\AppData\\Roaming\\Microsoft\\Windows\\Start Menu\\Programs\\Startup\\Slime.lnk";
	WindowManager::CreateShortcut(
		StringConversion::to_wString( ShortcutPath ).c_str(),
		StringConversion::to_wString( ExePath ).c_str(),
		NULL,
		NULL,
		StringConversion::to_wString( WorkFile ).c_str() );
#endif // #ifndef _DEBUG.
	
	const std::string appv = WndSetting["Version"];
	const std::string fp   = PARAMETER_FILE_PATH + appv + std::string( ".bin" );
#ifdef _DEBUG
	// バージョンファイルの作成
	FileManager::BinarySave( fp.c_str(), appv );
#else
	if ( FileManager::FileCheck( fp ) == false ) {
		InfoMessage( "古いバージョンのデータです.." );
	}
#endif

	// ウィンドウハンドルの設定.
	Input::SethWnd( m_hWnd );
	DragAndDrop::SethWnd( m_hWnd );
	return S_OK;
}

//---------------------------.
// メッセージループ.
//---------------------------.
void CMain::Loop()
{
	// メッセージループ.
	MSG msg = { 0 };
	ZeroMemory( &msg, sizeof( msg ) );

	while ( msg.message != WM_QUIT ) {
		if ( PeekMessage( &msg, nullptr, 0, 0, PM_REMOVE ) ) {
			TranslateMessage( &msg );
			DispatchMessage( &msg );
		}
		else {
			// フレームレートの待機処理.
			if ( m_pFrameRate->Wait() ) continue;

			// ロード中の更新処理.
			m_IsGameLoad = m_pLoadManager->ThreadRelease();
			//更新処理.
			if ( m_IsGameLoad ) {
				Update( static_cast<float>( m_pFrameRate->GetDeltaTime() ) );
			}
		}
	}
	Log::PushLog( "------ メインループ終了 ------" );
}

//---------------------------.
// ウィンドウ初期化関数.
//---------------------------.
HRESULT CMain::InitWindow( HINSTANCE hInstance )
{
	// ウィンドウの設定の取得.
	json WndSetting = FileManager::JsonLoad( WINDOW_SETTING_FILE_PATH );
	
	// アプリ名/ウィンドウ名を取得.
	const std::wstring wAppName = StringConversion::to_wString( WndSetting["Name"]["App"], ECodePage::UTF8 );
	const std::wstring wWndName = StringConversion::to_wString( WndSetting["Name"]["Wnd"], ECodePage::UTF8 );

	// FPSを描画するか.
	m_IsFPSRender = WndSetting["IsFPSRender"];

	// ウィンドウの定義.
	WNDCLASSEX wc;
	ZeroMemory( &wc, sizeof( wc ) );// 初期化(0を設定).

	wc.cbSize			= sizeof( wc );
	wc.style			= CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	wc.lpfnWndProc		= MsgProc;// WndProc;
	wc.hInstance		= hInstance;
	wc.hIcon			= LoadIcon( hInstance, _T( "ICON" ) );
	wc.hCursor			= LoadCursor( nullptr, IDC_ARROW );
	wc.hbrBackground	= (HBRUSH)GetStockObject( LTGRAY_BRUSH );
	wc.lpszClassName	= wAppName.c_str();
	wc.hIconSm			= wc.hIcon;
	
	// ウィンドウクラスをWindowsに登録.
	if ( !RegisterClassEx( &wc ) ) {
		ErrorMessage( "ウィンドウクラスの登録に失敗" );
		return E_FAIL;
	}

	// ウィンドウ領域の調整.
	RECT	rect;			// 矩形構造体.
	DWORD	dwStyle;		// ウィンドウスタイル.
	DWORD	dwExStyle;		// ウィンドウの拡張スタイル.

	int sizex = GetSystemMetrics( SM_CXVIRTUALSCREEN );
	int sizey = GetSystemMetrics( SM_CYVIRTUALSCREEN );
	int dispx = GetSystemMetrics( SM_XVIRTUALSCREEN );
	int dispy = GetSystemMetrics( SM_YVIRTUALSCREEN );

	rect.left	= dispx;											// 左.
	rect.top	= dispy;											// 上.
	rect.right	= sizex + dispx;									// 右.
	rect.bottom = sizey + dispy;									// 下.
	dwStyle		= WS_OVERLAPPEDWINDOW;								// ウィンドウ種別.
	dwExStyle	= WS_EX_LAYERED | WS_EX_TOOLWINDOW;					// ウィンドウ拡張機能.
	if ( WndSetting["IsSizeLock"]	) dwStyle ^= WS_THICKFRAME;		// サイズの変更を禁止するか.
	if ( WndSetting["IsMaxLock"]	) dwStyle ^= WS_MAXIMIZEBOX;	// 拡大化を禁止するか.
	if ( WndSetting["IsMinLock"]	) dwStyle ^= WS_MINIMIZEBOX;	// 拡大化を禁止するか.
	if ( WndSetting["IsPopUpWnd"]	) dwStyle  = WS_POPUP;			// 枠無しウィンドウ.

	if ( AdjustWindowRect(
		&rect,	// (in)画面サイズが入った矩形構造体.(out)計算結果.
		dwStyle,// ウィンドウ種別.
		FALSE ) == 0 )// メニューを持つかどうか指定.
	{
		ErrorMessage( "ウィンドウ領域の調整に失敗" );
		return E_FAIL;
	}

	// ウィンドウの作成.
	m_hWnd = CreateWindowEx(
		dwExStyle,					// 拡張機能( 透過ウィンドウ:WS_EX_LAYERED ).
		wAppName.c_str(),			// アプリ名.
		wWndName.c_str(),			// ウィンドウタイトル.
		dwStyle,					// ウィンドウ種別(普通).
		rect.left, rect.top,		// 表示位置x,y座標.
		sizex,						// ウィンドウ幅.
		sizey,						// ウィンドウ高さ.
		nullptr,					// 親ウィンドウハンドル.
		nullptr,					// メニュー設定.
		hInstance,					// インスタンス番号.
		nullptr );					// ウィンドウ作成時に発生するイベントに渡すデータ.
	if ( !m_hWnd ) {
		ErrorMessage( "ウィンドウ作成失敗" );
		return E_FAIL;
	}
	WindowManager::SetWnd( m_hWnd );

	// サブウィンドウの作成.
	m_hSubWnd = CreateWindowEx(
		dwExStyle,					// 拡張機能( 透過ウィンドウ:WS_EX_LAYERED ).
		wAppName.c_str(),			// アプリ名.
		wWndName.c_str(),			// ウィンドウタイトル.
		dwStyle,					// ウィンドウ種別(普通).
		rect.left, rect.top,		// 表示位置x,y座標.
		sizex,						// ウィンドウ幅.
		sizey,						// ウィンドウ高さ.
		nullptr,					// 親ウィンドウハンドル.
		nullptr,					// メニュー設定.
		hInstance,					// インスタンス番号.
		nullptr );					// ウィンドウ作成時に発生するイベントに渡すデータ.

	if ( !m_hSubWnd ) {
		ErrorMessage( "サブウィンドウ作成失敗" );
		return E_FAIL;
	}
	WindowManager::SetSubWnd( m_hSubWnd );

	// DCの取得.
	m_hDc		= GetDC( m_hWnd );
	m_hSubDc	= GetDC( m_hSubWnd );

	// タスクレイの作成.
	NOTIFYICONDATA nid;
	nid.cbSize				= sizeof( NOTIFYICONDATA );				// この構造体のバイト数.
	nid.hWnd				= m_hWnd;								// ウィンドウハンドル.
	nid.uID					= 0;									// 複数のアイコンを表示したときの識別ID.
	nid.uFlags				= NIF_MESSAGE | NIF_ICON | NIF_TIP;		// この構造体のどのメンバが有効かを示すフラグ.
	nid.uCallbackMessage	= WM_NOTIFYICON;						// WM_USER以降の定数.
	nid.hIcon				= LoadIcon( hInstance, _T( "ICON" ) );	// タスクレイのアイコン.
	_tcscpy( nid.szTip, wAppName.c_str() );							// タスクレイに表示する文字.
	int ret = ( int ) Shell_NotifyIcon( NIM_ADD, &nid );			// テクスレイの開始.

	// ウィンドウの表示.
	ShowWindow( m_hWnd, SW_SHOW );
	ShowWindow( m_hSubWnd, SW_SHOW );
	UpdateWindow( m_hWnd );
	UpdateWindow( m_hSubWnd );

	// 最背面に移動させる.
	SetWindowPos( m_hWnd,	 HWND_BOTTOM, 0, 0, 0, 0, ( SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW ) );
	SetWindowPos( m_hSubWnd, HWND_BOTTOM, 0, 0, 0, 0, ( SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW ) );

	// マウスカーソルを表示するか.
	if ( WndSetting["IsDispMouseCursor"] == false ) {
		DirectX11::SetIsDispMouseCursor( false );
		ShowCursor( FALSE );
	}
	return S_OK;
}

//---------------------------.
// ウィンドウ関数(メッセージ毎の処理).
//---------------------------.
LRESULT CALLBACK CMain::MsgProc(
	HWND hWnd, UINT uMsg,
	WPARAM wParam, LPARAM lParam )
{
	// ImGuiのウィンドウを動かしたりサイズ変更させたりできるようにする.
	if ( ImGui_ImplWin32_WndProcHandler( hWnd, uMsg, wParam, lParam ) ) return true;
	IMGUI_API

	switch ( uMsg ) {
	case WM_CREATE:
	{
		// 指定した色の背景を透明にする.
//		SetLayeredWindowAttributes( hWnd, RGB( 0, 0, 0 ), 0, LWA_COLORKEY );
		break;
	}
	case WM_KEYDOWN:// キーボードが押されたとき.
	{
		// ウィンドウの設定の取得.
		json WndSetting = FileManager::JsonLoad( WINDOW_SETTING_FILE_PATH );
		if ( WndSetting["IsDispCloseMessage"] == false ) break;
		
		// キー別の処理.
		switch ( static_cast<char>( wParam ) ) {
		case VK_ESCAPE:	// ESCキー.
			if ( MessageBox( nullptr, _T( "ゲームを終了しますか？" ), _T( "警告" ), MB_YESNO ) == IDYES ) {
				PostQuitMessage( 0 );
			}
		}
		break;
	}
	case WM_MOUSEWHEEL:
	{
		// マウスホイールの入力.
		const int Delta = GET_WHEEL_DELTA_WPARAM(wParam);
		Input::SetMouseWheelDelta( Delta );
		break;
	}
	case  WM_LBUTTONDBLCLK:
		Input::SetIsLeftDoubleClick( true );
		break;
	case  WM_MBUTTONDBLCLK:
		Input::SetIsCenterDoubleClick( true );
		break;
	case  WM_RBUTTONDBLCLK:
		Input::SetIsRightDoubleClick( true );
		break;
	case WM_DROPFILES:
	{
		// ドロップしたファイルパスを追加していく.
		HDROP	hDrop = (HDROP) wParam;
		int		Num = DragQueryFile( hDrop, -1, NULL, 0 );
		for ( int i = 0; i < Num; ++i ) {
			// 移動後のファイルパスを作成.
			TCHAR DropPath[100];
			DragQueryFile( hDrop, i, DropPath, sizeof( DropPath ) / sizeof( TCHAR ) );
			std::string sDropPath = StringConversion::to_String( DropPath );
			size_t		Num = sDropPath.find( "Desktop\\" );
			std::filesystem::create_directories( "Data\\DropFile" );
			std::string FilePath = "Data\\DropFile\\" + sDropPath.erase( 0, Num + sizeof( "Desktop\\" ) - 1 );

			// ファイルを一時的に移動させファイルパスを保存する.
			MoveFile( DropPath, StringConversion::to_wString( FilePath ).c_str() );
			DragAndDrop::AddFilePath( FilePath );
		}
		DragFinish( hDrop );
		break;
	}
	case WM_CLOSE:
		// ウィンドウの破棄.
		DestroyWindow( hWnd );
		break;
	case WM_DESTROY:
		// タスクレイを非表示にする.
		NOTIFYICONDATA nid;
		nid.cbSize	= sizeof( NOTIFYICONDATA );
		nid.hWnd	= hWnd;
		nid.uID		= 0;
		nid.uFlags	= 0;
		Shell_NotifyIcon( NIM_DELETE, &nid );

		// 終了.
		PostQuitMessage( 0 );
		break;
	case WM_NOTIFYICON:
		// メニューの表示.
		if ( wParam == 0 && lParam == WM_RBUTTONDOWN ) MenuManager::Disp();
		break;
	case WM_COMMAND:
		// メニューの選択.
		MenuManager::SelectMenu( LOWORD( wParam ) );
		break;
	case WM_SIZE:
		break;
	case WM_WINDOWPOSCHANGING:
		// サブウィンドウの位置が更新された時に最背面に変更する
		if ( hWnd != WindowManager::GetSubWnd() ) break;
		( ( LPWINDOWPOS )lParam )->hwndInsertAfter = HWND_BOTTOM;
		break;
	}

	// メインに返す情報.
	return DefWindowProc( hWnd, uMsg, wParam, lParam );
}

//---------------------------.
// ウィンドウ起動時の初期化( 起動時一回のみ ).
//---------------------------.
void CMain::WindowInit()
{
	// ウィンドウにガラス効果を付与する.
	MARGINS Margins = { -1 };
	DwmExtendFrameIntoClientArea( m_hWnd,	 &Margins );
	DwmExtendFrameIntoClientArea( m_hSubWnd, &Margins );
	
	// サブウィンドウの透明効果を無効にする.
	LONG SublStyle = GetWindowLong( m_hSubWnd, GWL_EXSTYLE );
	SublStyle ^= WS_EX_LAYERED;
	SetWindowLong( m_hSubWnd, GWL_EXSTYLE, SublStyle );
#ifdef SUB_WND_ALWAY_CLICK_LOSE
	// サブは常時クリックの判定を無くす
	SublStyle |= WS_EX_TRANSPARENT | WS_EX_LAYERED | WS_EX_TOOLWINDOW;
	SetWindowLong( m_hSubWnd, GWL_EXSTYLE, SublStyle );
#endif

	// ウィンドウの透明効果を無効にする.
	LONG MainlStyle = GetWindowLong( m_hWnd, GWL_EXSTYLE );
	MainlStyle ^= WS_EX_LAYERED;
	SetWindowLong( m_hWnd, GWL_EXSTYLE, MainlStyle );

	// ウィンドウの初期化を行った.
	m_IsWindowInit = true;
}

//---------------------------.
// FPSの描画.
//---------------------------.
void CMain::FPSRender()
{
#ifdef _DEBUG
	if ( DebugKeyInput::IsANDKeyDown( VK_CONTROL, VK_F12 )	) m_IsFPSRender = !m_IsFPSRender;
	if ( m_IsFPSRender == false								) return;

	// 表示する文字の作成.
	std::string FPSText =
		"    FPS    : " + std::to_string( static_cast<int>( m_pFrameRate->GetFPS() ) ) + '\n' +
		" DeltaTime : " + std::to_string( m_pFrameRate->GetDeltaTime() );

	// FPSの描画.
	WindowTextRenderer::DebugRender( FPSText, D3DXPOSITION3( FPS_RENDER_POS ), FPS_RENDER_SIZE, D3DXCOLOR4( FPS_RENDER_COLOR ) );

//	const D3DXVECTOR2 Pos = Input::GetMousePosition();
//
//	COLORREF MainColor = GetPixel( m_hDc, static_cast< int >( Pos.x ), static_cast< int >( Pos.y ) );
//	std::string Text = "<Main> R:" + std::to_string( GetRValue( MainColor ) ) + ", G:" + std::to_string( GetGValue( MainColor ) ) + ", B:" + std::to_string( GetBValue( MainColor ) );
//	WindowTextRenderer::DebugRender( Text, D3DXPOSITION3( 0.0f, 50.0f, 0.0f ), FPS_RENDER_SIZE, D3DXCOLOR4( FPS_RENDER_COLOR ) );
//
//	COLORREF SubColor = GetPixel( m_hSubDc, static_cast< int >( Pos.x ), static_cast< int >( Pos.y ) );
//	Text = "<Sub>  R:" + std::to_string( GetRValue( SubColor ) ) + ", G:" + std::to_string( GetGValue( SubColor ) ) + ", B:" + std::to_string( GetBValue( SubColor ) );
//	WindowTextRenderer::DebugRender( Text, D3DXPOSITION3( 0.0f, 80.0f, 0.0f ), FPS_RENDER_SIZE, D3DXCOLOR4( FPS_RENDER_COLOR ) );

#endif	// #ifdef _DEBUG.
}

//---------------------------.
// ウィンドウのクリック判定の更新.
//---------------------------.
void CMain::ClickUpdate()
{
	// マウスの座標の取得.
	const D3DXVECTOR2 Pos = Input::GetMousePosition();

	// メインウィンドウのマウスの下のカラーの取得.
	COLORREF MainColor = GetPixel( m_hDc, static_cast<int>( Pos.x ), static_cast<int>( Pos.y ) );
	
	// 真っ黒の場合クリック判定を無くす.
	if ( GetRValue( MainColor ) == 0 && GetGValue( MainColor ) == 0 && GetBValue( MainColor ) == 0 ) {
		LONG lStyle  = GetWindowLong( m_hWnd, GWL_EXSTYLE );
		lStyle		|= WS_EX_TRANSPARENT | WS_EX_LAYERED | WS_EX_TOOLWINDOW;
		SetWindowLong( m_hWnd, GWL_EXSTYLE, lStyle );
	}
	else {
		LONG lStyle  = GetWindowLong( m_hWnd, GWL_EXSTYLE );
		lStyle		&= WS_EX_TRANSPARENT & WS_EX_LAYERED | WS_EX_TOOLWINDOW;
		SetWindowLong( m_hWnd, GWL_EXSTYLE, lStyle );
	}

#ifndef SUB_WND_ALWAY_CLICK_LOSE
	// サブウィンドウのマウスの下のカラーの取得.
	COLORREF SubColor = GetPixel( m_hSubDc, static_cast<int>( Pos.x ), static_cast<int>( Pos.y ) );
	
	// 真っ黒の場合クリック判定を無くす.
	if ( GetRValue( SubColor ) == 0 && GetGValue( SubColor ) == 0 && GetBValue( SubColor ) == 0 ) {
		LONG lStyle  = GetWindowLong( m_hSubWnd, GWL_EXSTYLE );
		lStyle		|= WS_EX_TRANSPARENT | WS_EX_LAYERED | WS_EX_TOOLWINDOW;
		SetWindowLong( m_hSubWnd, GWL_EXSTYLE, lStyle );
	}
	else {
		LONG lStyle  = GetWindowLong( m_hSubWnd, GWL_EXSTYLE );
		lStyle		&= WS_EX_TRANSPARENT & WS_EX_LAYERED | WS_EX_TOOLWINDOW;
		SetWindowLong( m_hSubWnd, GWL_EXSTYLE, lStyle );
	}
#endif
}
