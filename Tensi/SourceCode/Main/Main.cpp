#include "Main.h"
#include "..\Common\DirectX\DirectX9.h"
#include "..\Common\DirectX\DirectX11.h"
#include "..\Common\WindowTextRenderer\WindowTextRenderer.h"
#include "..\Common\XAudio2\SoundManager.h"
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
#include "..\Utility\WindowsShortCutManager\WindowsShortCutManager.h"
#include "..\Utility\WindowsMenuManager\WindowsMenuManager.h"
#include "..\Utility\WindowsMessageBox\WindowsMessageBox.h"
#include "..\Utility\ThreadManager\ThreadManager.h"
#include "..\System\SystemWindowManager\SystemWindowManager.h"
#include <dwmapi.h>
#include "../Utility/Const/Const.h"

// ImGUiで使用.
extern LRESULT ImGui_ImplWin32_WndProcHandler( HWND, UINT, WPARAM, LPARAM );

static const decltype( WM_USER ) WM_NOTIFYICON = WM_USER + 100;

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
	m_pFrameRate	= std::make_unique<CFrameRate>( Const::GameWindow().FPS );
	m_pLoadManager	= std::make_unique<CLoadManager>();
}

CMain::~CMain()
{
#ifdef ENABLE_SUB_WINDOW
	ReleaseDC( m_hSubWnd, m_hSubDc );
	DestroyWindow( m_hSubWnd );
#endif // ENABLE_SUB_WINDOW
	ReleaseDC( m_hWnd, m_hDc );
	DestroyWindow( m_hWnd );

	// デスクトップを再描画
	SystemParametersInfo( SPI_SETDESKWALLPAPER, 0, NULL, SPIF_SENDCHANGE );
}

//---------------------------.
// 更新処理.
//---------------------------.
void CMain::Update( const float& DeltaTime )
{
	// 起動時のウィンドウの初期化.
	if ( m_IsWindowInit == false ) WindowInit();

	// 最前面で固定し続ける.
	//	毎フレーム SetWindowPos を呼ぶと Zオーダー変更通知が大量に発生して負荷になるため、
	//	最前面フラグが外れた時と一定間隔の再アサートのみ行う.
	//	( SWP_NOACTIVATE を付けてフォーカスを奪わないようにする ).
	if ( m_IsWindowTop ) {
		constexpr float TOPMOST_REASSERT_INTERVAL = 1.0f;	// 再アサート間隔(秒).
		static float TopmostWait = 0.0f;
		TopmostWait -= DeltaTime;
		const bool IsTopmost = ( GetWindowLong( m_hWnd, GWL_EXSTYLE ) & WS_EX_TOPMOST ) != 0;
		if ( IsTopmost == false || TopmostWait <= 0.0f ) {
			SetWindowPos( m_hWnd, HWND_TOPMOST, 0, 0, 0, 0, ( SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_SHOWWINDOW ) );
			TopmostWait = TOPMOST_REASSERT_INTERVAL;
		}
	}
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

#ifdef ENABLE_TRANSPARENT_WINDOW
	// ウィンドウの透明部分のクリック判定の更新.
	ClickUpdate();
#endif // ENABLE_TRANSPARENT_WINDOW

	// 更新処理.
	DirectX11::CheckActiveWindow();
	ImGuiManager::SetingNewFrame();
	Input::Update();
	WindowManager::Update();
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

	// クリック透過判定用にカーソル位置のバックバッファの色をコピーしておく.
	//	( 次フレームの ClickUpdate で読み取る ).
	const D3DXVECTOR2 MousePos = Input::GetMousePosition();
	DirectX11::CopyCursorPixel( static_cast<int>( MousePos.x ), static_cast<int>( MousePos.y ) );
	DirectX11::Present( 0 );

#ifdef ENABLE_SUB_WINDOW
	// サブウィンドウのバックバッファをクリアにする.
	DirectX11::ClearBackBuffer( 1 );

	// サブウィンドウの描画処理.
	SceneManager::SubRender();
#endif // ENABLE_SUB_WINDOW

	CollisionRenderer::Render();
	ImGuiManager::Render();

#ifdef ENABLE_SUB_WINDOW
	// 画面に表示.
	DirectX11::Present( 1 );
#endif // ENABLE_SUB_WINDOW

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
	if ( FAILED( DirectX9::Create( m_hWnd ) ) ) return E_FAIL;
	// DirectX11の構築.
#ifdef ENABLE_SUB_WINDOW
	if ( FAILED( DirectX11::Create( { m_hWnd, m_hSubWnd } ) ) ) return E_FAIL;
#else
	if ( FAILED( DirectX11::Create( { m_hWnd } ) ) ) return E_FAIL;
#endif // ENABLE_SUB_WINDOW
	// 乱数の初期化.
	if ( FAILED( Random::Init() ) ) return E_FAIL;
	// 入力クラスの初期化.
	if ( FAILED( Input::Init() ) ) return E_FAIL;
	// デバックテキストの初期化.
	if ( FAILED( WindowTextRenderer::Init() ) ) return E_FAIL;
	// ImGuiの初期化.
	if ( FAILED( ImGuiManager::Init( m_hWnd ) ) ) return E_FAIL;
#ifdef ENABLE_WINDOWS_WINDOW
	// ウィンドウマネージャーの初期化.
	if ( FAILED( WindowManager::Init() ) ) return E_FAIL;
#endif
#ifdef ENABLE_WINDOWS_MENU
	// メニューの初期化.
	if ( FAILED( WindowsMenuManager::Init( m_hWnd ) ) ) return E_FAIL;
#endif

	// 深度を無くす.
	DirectX11::SetDepth( false );

	// フルスクリーンで起動するか.
	Json WndSetting = FileManager::JsonLoad( WINDOW_SETTING_FILE_PATH );
	if ( WndSetting["IsStartFullScreen"].Get( false ) ) {
		// フルスクリーンに設定.
		DirectX11::SetFullScreen( true );
	}

	// リソースの読み込み.
	m_pLoadManager->LoadResource( m_hWnd );

#ifndef _DEBUG
	// ユーザー名の取得.
	// 既知のフォルダ API と exe ファイルのパスを用いて、起動ショートカットを登録
	//	（「C:\Users\<name>」のようなハードコードされたパスは、ユーザープロファイルが移動または
	//	名前変更された場合や、標準以外のシステムドライブでは正常に動作しません）。
		WCHAR ModulePath[MAX_PATH] = {};
		if ( GetModuleFileNameW( nullptr, ModulePath, MAX_PATH ) != 0 ) {
			const std::wstring WorkDir = std::filesystem::path( ModulePath ).parent_path().wstring();
			// スタートアップフォルダにショートカットを作成し、有効化する.
			WindowsShortCutManager::CreateStartupShortcut(
				L"Slime.lnk",
				ModulePath,
				true,
				NULL,
				NULL,
				WorkDir.c_str() );
		}
#endif // #ifndef _DEBUG.
	
	// バージョンファイル( ファイル名は固定で、バージョンは中身に文字データとして保存する ).
	//	※ std::string をそのまま BinarySave すると文字列ではなくオブジェクトの生メモリ
	//	   ( 内部ポインタや未初期化領域 )が書き込まれ、毎回ファイル差分が出てしまうため、
	//	   決定的な形式( [サイズ][文字列データ] )になる std::vector<char> で読み書きする.
	const std::string appv = WndSetting["Version"].Get( std::string() );
	const std::string fp   = PARAMETER_FILE_PATH + std::string( "v.bin" );

	// 保存されているバージョンの読み込み( ファイルが無い場合は空のまま ).
	std::string SavedVersion;
	std::vector<char> VersionData;
	if ( SUCCEEDED( FileManager::BinaryLoad( fp.c_str(), VersionData ) ) ) {
		SavedVersion.assign( VersionData.begin(), VersionData.end() );
	}
#ifdef _DEBUG
	// バージョンファイルの作成.
	//	内部のバージョンが同じ場合は書き込まない( 不要なファイル差分を出さないため ).
	if ( SavedVersion != appv ) {
		const std::vector<char> NewVersionData( appv.begin(), appv.end() );
		FileManager::BinarySave( fp.c_str(), NewVersionData );
	}
#else
	// ファイルが無い、またはバージョンが一致しない場合は警告.
	if ( SavedVersion != appv ) {
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

#ifdef ENABLE_THREAD
			// スレッドマネージャーの更新( 完了コールバックの実行など ).
			ThreadManager::Update();
#endif // ENABLE_THREAD

			// ロード中の更新処理.
			m_IsGameLoad = m_pLoadManager->ThreadRelease();
			//更新処理.
			if ( m_IsGameLoad ) {
				Update( static_cast<float>( m_pFrameRate->GetDeltaTime() ) );
			}
		}
	}
	Log::PushLogInfo( "------ メインループ終了 ------" );

	// 表示中のメッセージボックスを閉じる.
	//	( 閉じられるまで戻らないため、スレッドの終了を待つ前に閉じる ).
	WindowsMessageBox::Release();

#ifdef ENABLE_THREAD
	// 全スレッドの終了を待つ.
	ThreadManager::Release();
#endif // ENABLE_THREAD
}

//---------------------------.
// ウィンドウ初期化関数.
//---------------------------.
HRESULT CMain::InitWindow( HINSTANCE hInstance )
{
	// ウィンドウの設定の取得.
	Json WndSetting = FileManager::JsonLoad( WINDOW_SETTING_FILE_PATH );
	
	// アプリ名/ウィンドウ名を取得.
	const std::wstring wAppName = StringConversion::to_wString( WndSetting["Name"]["App"].Get( std::string( "Tensi" ) ), ECodePage::UTF8 );
	const std::wstring wWndName = StringConversion::to_wString( WndSetting["Name"]["Wnd"].Get( std::string( "Tensi" ) ), ECodePage::UTF8 );

	// FPSを描画するか.
	m_IsFPSRender = WndSetting["IsFPSRender"].Get( false );

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
#ifdef ENABLE_TRANSPARENT_WINDOW
	dwExStyle	= WS_EX_LAYERED | WS_EX_TOOLWINDOW;					// ウィンドウ拡張機能( 透明ウィンドウ用にレイヤード化 ).
#else
	dwExStyle	= WS_EX_TOOLWINDOW;									// ウィンドウ拡張機能.
#endif // ENABLE_TRANSPARENT_WINDOW
	if ( WndSetting["IsSizeLock"].Get( false )	) dwStyle ^= WS_THICKFRAME;		// サイズの変更を禁止するか.
	if ( WndSetting["IsMaxLock"].Get( false )	) dwStyle ^= WS_MAXIMIZEBOX;	// 拡大化を禁止するか.
	if ( WndSetting["IsMinLock"].Get( false )	) dwStyle ^= WS_MINIMIZEBOX;	// 拡大化を禁止するか.
	if ( WndSetting["IsPopUpWnd"].Get( false )	) dwStyle  = WS_POPUP;			// 枠無しウィンドウ.

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

#ifdef ENABLE_SUB_WINDOW
	// デスクトップ壁紙の上・デスクトップアイコンの下のレイヤーに描画するための
	// 親ウィンドウ( 壁紙レイヤーの WorkerW もしくは Progman )を取得する.
	// 自プロセスのウィンドウを WS_CHILD でその配下に配置することで
	// 壁紙の上・アイコンの下に正しく描画できる.
	HWND hWorkerW = FindWorkerW();

	// 親ウィンドウのクライアント座標系での仮想スクリーン原点を求める.
	// ( マルチモニター環境では仮想スクリーン原点が (0,0) とは限らないため ).
	POINT SubWndPos = { dispx, dispy };
	if ( hWorkerW != NULL ) ScreenToClient( hWorkerW, &SubWndPos );

	m_hSubWnd = CreateWindowEx(
		WS_EX_NOREDIRECTIONBITMAP,									// 拡張機能( GDIリダイレクトサーフェス無し・DirectComposition 直接合成 ).
		wAppName.c_str(),											// アプリ名.
		NULL,														// ウィンドウタイトル.
		WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,	// ウィンドウ種類(既定).
		SubWndPos.x, SubWndPos.y,									// 表示位置x,y座標.
		sizex,														// ウィンドウ幅.
		sizey,														// ウィンドウ高さ.
		hWorkerW,													// 親ウィンドウハンドル.
		nullptr,													// メニュー設定.
		hInstance,													// インスタンス番号.
		nullptr);													// ウィンドウ作成時に発生するイベントに渡すデータ.
	if ( !m_hSubWnd ) {
		ErrorMessage( "サブウィンドウ作成失敗" );
		m_hSubWnd = hWorkerW;	// フォールバック: WorkerWを直接使用.
	} else {
		// Z-order の設定.
		HWND hDefView = FindWindowEx( hWorkerW, NULL, L"SHELLDLL_DefView", NULL );
		if ( hDefView != NULL ) {
			// 親が Progman ( アイコン層と同居 )の場合:
			// SHELLDLL_DefView の直下 = 壁紙の上・アイコンの下 に差し込む.
			SetWindowPos( m_hSubWnd, hDefView, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE );
		}
		else {
			// 親が壁紙レイヤーの WorkerW の場合: 兄弟内では最前面に置く.
			// WorkerW 全体がアイコン層(SHELLDLL_DefView)より下にあるため、
			// 最前面にしても「アイコンより下」は維持される.
			// ( Wallpaper Engine 等の壁紙アプリも同じ WorkerW 配下に入るため、
			//   HWND_BOTTOM だと壁紙アプリの裏に隠れて見えなくなる ).
			SetWindowPos( m_hSubWnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE );
		}
	}
	WindowManager::SetSubWnd( m_hSubWnd );
#endif // ENABLE_SUB_WINDOW

	// DCの取得.
	m_hDc		= GetDC( m_hWnd );
#ifdef ENABLE_SUB_WINDOW
	m_hSubDc	= GetDC( m_hSubWnd );
#endif // ENABLE_SUB_WINDOW

#ifdef ENABLE_WINDOWS_TASK_TRAY
	// タスクレイの作成.
	NOTIFYICONDATA nid;
	ZeroMemory( &nid, sizeof( nid ) );
	nid.cbSize				= sizeof( NOTIFYICONDATA );				// この構造体のバイト数.
	nid.hWnd				= m_hWnd;								// ウィンドウハンドル.
	nid.uID					= 0;									// 複数のアイコンを表示したときの識別ID.
	nid.uFlags				= NIF_MESSAGE | NIF_ICON | NIF_TIP;		// この構造体のどのメンバが有効かを示すフラグ.
	nid.uCallbackMessage	= WM_NOTIFYICON;						// WM_USER以降の定数.
	nid.hIcon				= LoadIcon( hInstance, _T( "ICON" ) );	// タスクレイのアイコン.
	_tcsncpy_s( nid.szTip, wAppName.c_str(), _TRUNCATE );							// タスクレイに表示する文字.
	int ret = ( int ) Shell_NotifyIcon( NIM_ADD, &nid );			// テクスレイの開始.
#endif

	// ウィンドウの表示.
	ShowWindow( m_hWnd, SW_SHOW );
	UpdateWindow( m_hWnd );
#ifdef ENABLE_SUB_WINDOW
	ShowWindow( m_hSubWnd, SW_SHOW );
	UpdateWindow( m_hSubWnd );
#endif // ENABLE_SUB_WINDOW

	// 最背面に移動させる.
	SetWindowPos( m_hWnd, HWND_BOTTOM, 0, 0, 0, 0, ( SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW ) );

	// マウスカーソルを表示するか.
	if ( WndSetting["IsDispMouseCursor"].Get( true ) == false ) {
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
		Json WndSetting = FileManager::JsonLoad( WINDOW_SETTING_FILE_PATH );
		if ( WndSetting["IsDispCloseMessage"].Get( true ) == false ) break;
		
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
			TCHAR DropPath[MAX_PATH];	// 長いファイルパスが切り捨てられないように MAX_PATH にする.
			DragQueryFile( hDrop, i, DropPath, sizeof( DropPath ) / sizeof( TCHAR ) );
			// ファイル名部分だけを使って移動先を作る.
			//	( 以前の「パスから "Desktop\" 以降を切り出す」方式は、
			//	  デスクトップ以外からのドロップで壊れたパスになるため ).
			const std::wstring FileName = std::filesystem::path( DropPath ).filename().wstring();
			std::string FilePath = "Data\\DropFile\\" + StringConversion::to_String( FileName );

#ifdef ENABLE_THREAD
			// ファイルの移動は重いことがある( 別ドライブからのドロップ等 )ためワーカースレッドで行い、
			//	移動が終わってからパスを登録する( 同タグの直列実行で複数ファイルの順番も保つ ).
			const std::wstring wSrcPath = DropPath;
			const std::wstring wDstPath = StringConversion::to_wString( FilePath );
			ThreadManager::StartSequential( "DropFile",
				[wSrcPath, wDstPath]() {
					std::filesystem::create_directories( "Data\\DropFile" );
					MoveFile( wSrcPath.c_str(), wDstPath.c_str() );
				},
				[FilePath]() { DragAndDrop::AddFilePath( FilePath ); } );
#else
			// ファイルを一時的に移動させファイルパスを保存する.
			std::filesystem::create_directories( "Data\\DropFile" );
			MoveFile( DropPath, StringConversion::to_wString( FilePath ).c_str() );
			DragAndDrop::AddFilePath( FilePath );
#endif // ENABLE_THREAD
		}
		DragFinish( hDrop );
		break;
	}
	case WM_CLOSE:
#ifdef ENABLE_SOUND
		SoundManager::Release();
#endif // ENABLE_SOUND
		DestroyWindow( hWnd );
		break;
	case WM_DESTROY:
		// タスクレイを非表示にする.
		NOTIFYICONDATA nid;
		ZeroMemory( &nid, sizeof( nid ) );
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
		if ( wParam == 0 && lParam == WM_RBUTTONDOWN ) WindowsMenuManager::Disp();
		break;
	case WM_COMMAND:
		// メニューの選択.
		WindowsMenuManager::SelectMenu( LOWORD( wParam ) );
		break;
	case WM_SIZE:
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
#ifdef ENABLE_TRANSPARENT_WINDOW
	// メインウィンドウにガラス効果を付与する.
	//	バックバッファのアルファ値が DWM に反映されるようになり、
	//	アルファ0でクリアした部分が透明になる.
	MARGINS Margins = { -1 };
	DwmExtendFrameIntoClientArea( m_hWnd, &Margins );

	// メインウィンドウの WS_EX_LAYERED を外す.
	//	※ XOR( ^= )だと状態によって逆に付与してしまうため、必ずクリアする.
	LONG MainlStyle = GetWindowLong( m_hWnd, GWL_EXSTYLE );
	MainlStyle &= ~WS_EX_LAYERED;
	SetWindowLong( m_hWnd, GWL_EXSTYLE, MainlStyle );

	// サブウィンドウは DirectComposition( プリマルチプライドアルファ )で
	// 透過合成されるため、ガラス効果や WS_EX_LAYERED の操作は不要.
#endif // ENABLE_TRANSPARENT_WINDOW

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
#endif	// #ifdef _DEBUG.
}

#ifdef ENABLE_TRANSPARENT_WINDOW
//---------------------------.
// ウィンドウのクリック判定の更新.
//---------------------------.
void CMain::ClickUpdate()
{
	// メインウィンドウのマウスの下のカラーの取得.
	//	前フレームの Present 前にコピーしておいたバックバッファの色を読む.
	//	( GDI の GetPixel は DWM との同期待ちが発生して重いため使用しない ).
	COLORREF MainColor = DirectX11::GetCursorPixel();

	// 真っ黒( 透明部分 )の場合クリック判定を無くす.
	const bool IsClickThrough =
		( GetRValue( MainColor ) == 0 && GetGValue( MainColor ) == 0 && GetBValue( MainColor ) == 0 );

	// 変更後の拡張スタイルを計算する.
	//	WS_EX_TOPMOST や WS_EX_ACCEPTFILES( D&D受付 )等の関係無いビットは変更しない.
	const LONG lStyle	= GetWindowLong( m_hWnd, GWL_EXSTYLE );
	LONG NewStyle		= lStyle;
	if ( IsClickThrough )	NewStyle |=  ( WS_EX_TRANSPARENT | WS_EX_LAYERED );
	else					NewStyle &= ~( WS_EX_TRANSPARENT | WS_EX_LAYERED );
	NewStyle |= WS_EX_TOOLWINDOW;

	// 変更がある場合のみ適用する( 毎フレームの SetWindowLong による通知を避ける ).
	if ( NewStyle != lStyle ) SetWindowLong( m_hWnd, GWL_EXSTYLE, NewStyle );
}
#endif // ENABLE_TRANSPARENT_WINDOW

#ifdef ENABLE_SUB_WINDOW
//---------------------------.
// WorkerWの取得
// デスクトップ壁紙の上、デスクトップアイコンの下のレイヤーに描画するための
// 親ウィンドウ( 壁紙レイヤーの WorkerW もしくは Progman )を取得する.
// Win10 / Win11( 24H2以降のウィンドウ階層変更を含む )対応.
//---------------------------.
HWND CMain::FindWorkerW()
{
	// 1) Progman を取得( デスクトップ管理ウィンドウ ).
	HWND hProgman = FindWindow( L"Progman", NULL );
	if ( hProgman == NULL ) return NULL;

	// 2) Progman に壁紙レイヤーの WorkerW を生成させるメッセージを送信.
	//    Windows 10          : (0x052C, 0,   0  ) で生成される.
	//    Windows 11 22H2以降 : (0x052C, 0xD, 0x1) が追加で必要.
	DWORD_PTR dwResult = 0;
	SendMessageTimeout( hProgman, 0x052C, 0,   0,   SMTO_NORMAL, 1000, &dwResult );
	SendMessageTimeout( hProgman, 0x052C, 0xD, 0x1, SMTO_NORMAL, 1000, &dwResult );

	// 3) Win11 24H2以降: SHELLDLL_DefView( アイコン層 )は Progman の直下に残り、
	//    壁紙レイヤーの WorkerW は「Progman の子」として DefView の下に作られる.
	//    この場合はその子 WorkerW が目的の親ウィンドウ.
	if ( FindWindowEx( hProgman, NULL, L"SHELLDLL_DefView", NULL ) != NULL ) {
		HWND hChildWorker = FindWindowEx( hProgman, NULL, L"WorkerW", NULL );
		if ( hChildWorker != NULL ) return hChildWorker;

		// 子 WorkerW が生成されないビルドでは Progman を親にする.
		// ( InitWindow 側で SHELLDLL_DefView の直下に Z-order を差し込む ).
		return hProgman;
	}

	// 4) Win10: 0x052C 送信後、SHELLDLL_DefView はトップレベルの WorkerW 内に移動する.
	//    その WorkerW の Z-order 直後( 背面側 )にある WorkerW が壁紙レイヤー.
	HWND hIconContainer = NULL;
	EnumWindows( []( HWND hWnd, LPARAM lParam ) -> BOOL {
		if ( FindWindowEx( hWnd, NULL, L"SHELLDLL_DefView", NULL ) != NULL ) {
			*(HWND*)lParam = hWnd;
			return FALSE;
		}
		return TRUE;
	}, ( LPARAM )&hIconContainer );

	if ( hIconContainer != NULL ) {
		HWND hBgWorker = FindWindowEx( NULL, hIconContainer, L"WorkerW", NULL );
		if ( hBgWorker != NULL ) return hBgWorker;
	}

	// 5) 最終手段: Progman を直接使用( 常に存在する ).
	return hProgman;
}
#endif // ENABLE_SUB_WINDOW
