#include "WindowManager.h"
#include "..\Input\Input.h"
#include <dwmapi.h>
#include <psapi.h>
#include <shlobj.h>

WindowManager::WindowManager()
	: m_hDesktop				( NULL )
	, m_hMyWindow				( NULL )
	, m_hMySubWindow			( NULL )
	, m_MouseDownWindow			( NULL )
	, m_MyWindowRect			()
	, m_AddWindowRect			()
	, m_TaskBar					()
	, m_DProcessID				( NULL )
	, m_DProcessHandle			( NULL )
	, m_DProcessMemory			( NULL )
	, m_WindowList				()
	, m_WindowFindMap			()
	, m_WindowRectMap			()
	, m_WindowNoShadowRectMap	()
	, m_WindowNameMap			()
	, m_WindowClassMap			()
	, m_WindowZOrderMap			()
	, m_IconList				()
	, m_IconFindMap				()
	, m_IconPosMap				()
	, m_IconNameMap				()
	, m_UserName				( "" )
	, m_IsWindowUpdate			( false )
	, m_IsDesktopIconUpdate		( false )
	, m_IsTaskBarUpdate			( false )
{
}

WindowManager::~WindowManager()
{
	// 後処理.
	VirtualFreeEx( m_DProcessHandle, m_DProcessMemory, 0, MEM_RELEASE );
	CloseHandle( m_DProcessHandle );
}

//----------------------------.
// インスタンスの取得.
//----------------------------.
WindowManager* WindowManager::GetInstance()
{
	static std::unique_ptr<WindowManager> pInstance = std::make_unique<WindowManager>();
	return pInstance.get();
}

//---------------------------.
// 初期化.
//---------------------------.
HRESULT WindowManager::Init()
{
	WindowManager* pI = GetInstance();

	// ウィンドウの情報に追加する補正値の作成.
	GetWindowRect( pI->m_hMyWindow, &pI->m_MyWindowRect );
	pI->m_AddWindowRect.left	= -pI->m_MyWindowRect.left;
	pI->m_AddWindowRect.top		= -pI->m_MyWindowRect.top;
	pI->m_AddWindowRect.right	= -pI->m_MyWindowRect.left;
	pI->m_AddWindowRect.bottom	= -pI->m_MyWindowRect.top;

	// ユーザー名の取得.
	char Name[256];
	DWORD Size = sizeof( Name );
	if ( GetUserNameA( Name, &Size ) == 0 ) return E_FAIL;
	pI->m_UserName = Name;

	// デスクトップのハンドルの取得.
	pI->m_hDesktop = FindWindow( L"Progman", L"Program Manager" );
	pI->m_hDesktop = FindWindowEx( pI->m_hDesktop, NULL, L"SHELLDLL_DefView", NULL );
	pI->m_hDesktop = FindWindowEx( pI->m_hDesktop, NULL, L"SysListView32", NULL );

	if ( pI->m_hDesktop == NULL ) {
		// 壁紙を変更した場合等Program Manager→SHELLDLL_DefView→SysListView32ではなく.
		// WorkerW→SHELLDLL_DefView→SysListView32になることがあるので、両方試す.
		// WorkerWは複数あるのでループして目的の物が見つかるまで探す.
		HWND hWorkerW		= NULL;
		HWND hShellViewWin	= NULL;

		do {
			hWorkerW		= FindWindowEx( NULL, hWorkerW, L"WorkerW", NULL );
			hShellViewWin	= FindWindowEx( hWorkerW, NULL, L"SHELLDLL_DefView", NULL );
		} while ( hShellViewWin == NULL && hWorkerW != NULL );
		pI->m_hDesktop = FindWindowEx( hShellViewWin, NULL, L"SysListView32", NULL );
	}
	if ( pI->m_hDesktop == NULL ) return E_FAIL;

	// プロセスIDの取得.
	GetWindowThreadProcessId( pI->m_hDesktop, &pI->m_DProcessID );
	if ( pI->m_DProcessID == NULL ) return E_FAIL;

	// プロセスのハンドルの取得.
	pI->m_DProcessHandle = OpenProcess( PROCESS_VM_OPERATION | PROCESS_VM_READ |
		PROCESS_VM_WRITE, false, pI->m_DProcessID );
	if ( pI->m_DProcessHandle == NULL ) return E_FAIL;

	// そのプロセスでメモリを確保する.
	pI->m_DProcessMemory = VirtualAllocEx( pI->m_DProcessHandle, NULL, 4096,
		MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE );
	if ( pI->m_DProcessMemory == NULL ) return E_FAIL;
	return S_OK;
}

//---------------------------.
// 更新.
//---------------------------.
void WindowManager::Update()
{
	WindowManager* pI = GetInstance();

	// フラグの初期化.
	pI->m_IsWindowUpdate		= false;
	pI->m_IsDesktopIconUpdate	= false;
	pI->m_IsTaskBarUpdate		= false;
}

//---------------------------.
// タスクバーの情報を取得.
//---------------------------.
RECT WindowManager::GetTaskBarRect()
{
	WindowManager* pI = GetInstance();

	// タスクバーを更新していない場合更新する.
	if ( pI->m_IsTaskBarUpdate == false ) TaskBarUpdate();

	return GetInstance()->m_TaskBar.rc;
}

//---------------------------.
// タスクバーのウィンドウハンドルの取得.
//---------------------------.
HWND WindowManager::GetTaskBarWnd()
{
	WindowManager* pI = GetInstance();

	// タスクバーを更新していない場合更新する.
	if ( pI->m_IsTaskBarUpdate == false ) TaskBarUpdate();

	return GetInstance()->m_TaskBar.hWnd;
}

//---------------------------.
// マウスが他のウィンドウの上にある場合そのウィンドウのハンドルを取得(上にない場合:NULL).
//---------------------------.
HWND WindowManager::GetMouseOverTheWindow()
{
	const D3DXPOSITION3& MousePos = Input::GetMousePosition3();
	return GetPointOverTheWindow( MousePos );
}

//---------------------------.
// マウスが他のウィンドウの上にあるか取得.
//---------------------------.
bool WindowManager::GetIsMouseOverTheWindow()
{
	const D3DXPOSITION3& MousePos = Input::GetMousePosition3();
	return GetIsPointOverTheWindow( MousePos );
}

//---------------------------.
// 指定した座標が他のウィンドウの上にある場合そのウィンドウのハンドルを取得(上にない場合:NULL).
//---------------------------.
HWND WindowManager::GetPointOverTheWindow( const D3DXPOSITION3& Pos )
{
	WindowManager* pI = GetInstance();

	// ウィンドウリストを更新していない場合更新する.
	if ( pI->m_IsWindowUpdate == false ) WindowListUpdate();

	// ウィンドウに当たっているか調べる.
	for ( auto& [w, hWnd] : pI->m_WindowList ) {
		// マウスが当たっているか調べる.
		if ( ( Pos.x < w.right  ) && ( Pos.x > w.left ) &&
			 ( Pos.y < w.bottom ) && ( Pos.y > w.top  ) )
		{
			return hWnd;
		}
	}
	return NULL;
}
HWND WindowManager::GetPointOverTheWindow( const D3DXPOSITION2& Pos )
{
	return GetPointOverTheWindow( { Pos.x, Pos.y, INIT_FLOAT } );
}

//---------------------------.
// 指定した座標が他のウィンドウの上にあるか取得.
//---------------------------.
bool WindowManager::GetIsPointOverTheWindow( const D3DXPOSITION3& Pos )
{
	return GetPointOverTheWindow( Pos ) != NULL;
}
bool WindowManager::GetIsPointOverTheWindow( const D3DXPOSITION2& Pos )
{
	return GetIsPointOverTheWindow( { Pos.x, Pos.y, INIT_FLOAT } );
}

//---------------------------.
// ウィンドウの名前からハンドルを取得.
//---------------------------.
HWND WindowManager::GetFindWindow( const std::string& Name )
{
	WindowManager* pI = GetInstance();

	// ウィンドウリストを更新していない場合更新する.
	if ( pI->m_IsWindowUpdate == false ) WindowListUpdate();
	return pI->m_WindowFindMap[Name];
}

//---------------------------.
// デスクトップのアイコン名からアイコンインデックスを取得.
//---------------------------.
ICOINDEX WindowManager::GetFindDesktopIcon( const std::string& Name )
{
	WindowManager* pI = GetInstance();

	// アイコンリストを更新していない場合更新する.
	if ( pI->m_IsDesktopIconUpdate == false ) DesktopIconUpdate();
	return pI->m_IconFindMap[Name];
}

//---------------------------.
// ウィンドウの情報を取得.
//---------------------------.
WindowManager::WndList WindowManager::GetWindowList()
{
	WindowManager* pI = GetInstance();

	// ウィンドウクラスを更新していない場合更新する.
	if ( pI->m_IsWindowUpdate == false ) WindowListUpdate();
	return pI->m_WindowList;
}

//---------------------------.
// デスクトップのアイコンの位置リストの取得.
//---------------------------.
WindowManager::IconList WindowManager::GetDesktopIconList()
{
	WindowManager* pI = GetInstance();

	// ウィンドウクラスを更新していない場合更新する.
	if ( pI->m_IsDesktopIconUpdate == false ) DesktopIconUpdate();
	return pI->m_IconList;
}

//---------------------------.
// ウィンドウの情報の取得.
//---------------------------.
RECT WindowManager::GetWindowSize( const HWND& hWnd )
{
	WindowManager* pI = GetInstance();

	// ウィンドウクラスを更新していない場合更新する.
	if ( pI->m_IsWindowUpdate == false ) WindowListUpdate();
	return pI->m_WindowRectMap[hWnd];
}

//---------------------------.
// ドロップシャドウを除いたウィンドウの情報の取得.
//---------------------------.
RECT WindowManager::GetWindowNoShadowSize( const HWND& hWnd )
{
	WindowManager* pI = GetInstance();

	// ウィンドウクラスを更新していない場合更新する.
	if ( pI->m_IsWindowUpdate == false ) WindowListUpdate();
	return pI->m_WindowNoShadowRectMap[hWnd];
}

//---------------------------.
// ウィンドウ名の取得.
//---------------------------.
std::string WindowManager::GetWindowName( const HWND& hWnd )
{
	WindowManager* pI = GetInstance();

	// ウィンドウクラスを更新していない場合更新する.
	if ( pI->m_IsWindowUpdate == false ) WindowListUpdate();
	return pI->m_WindowNameMap[hWnd];
}

//---------------------------.
// ウィンドウクラス名の取得.
//---------------------------.
std::string WindowManager::GetWindowClassName( const HWND& hWnd )
{
	WindowManager* pI = GetInstance();

	// ウィンドウクラスを更新していない場合更新する.
	if ( pI->m_IsWindowUpdate == false ) WindowListUpdate();
	return pI->m_WindowClassMap[hWnd];
}

//---------------------------.
// Zオーダーの取得( 数字が小さい程上側 ).
//---------------------------.
int WindowManager::GetWindowZOrder( const HWND& hWnd )
{
	WindowManager* pI = GetInstance();

	// ウィンドウクラスを更新していない場合更新する.
	if ( pI->m_IsWindowUpdate == false ) WindowListUpdate();
	return pI->m_WindowZOrderMap[hWnd];
}

//---------------------------.
// アイコンの位置を取得.
//---------------------------.
D3DXPOSITION2 WindowManager::GetDesktopIconPosition( const ICOINDEX Index )
{
	WindowManager* pI = GetInstance();

	// アイコンリストを更新していない場合更新する.
	if ( pI->m_IsDesktopIconUpdate == false ) DesktopIconUpdate();
	return pI->m_IconPosMap[Index];
}

//---------------------------.
// アイコンの名前を取得.
//---------------------------.
std::string WindowManager::GetDesktopIconName( const ICOINDEX Index )
{
	WindowManager* pI = GetInstance();

	// アイコンリストを更新していない場合更新する.
	if ( pI->m_IsDesktopIconUpdate == false ) DesktopIconUpdate();
	return pI->m_IconNameMap[Index];
}

//---------------------------.
// デスクトップのアイコンのファイルパスを取得.
//---------------------------.
std::string WindowManager::GetDesktopIconPath( const ICOINDEX Index )
{
	WindowManager* pI = GetInstance();

	// アイコンリストを更新していない場合更新する.
	if ( pI->m_IsDesktopIconUpdate == false ) DesktopIconUpdate();
	return "C:\\Users\\" + pI->m_UserName + "\\Desktop\\" + pI->m_IconNameMap[Index];
}

//---------------------------.
// デスクトップのアイコンの位置を設定.
//---------------------------.
void WindowManager::SetDesktopIconPosition( const ICOINDEX Index, const int x, const int y )
{
	WindowManager* pI = GetInstance();

	// 設定する位置を調整する.
	const int PosX = x - pI->m_AddWindowRect.left;
	const int PosY = y - pI->m_AddWindowRect.top;

	SendMessage( 
		pI->m_hDesktop,
		LVM_SETITEMPOSITION,
		Index,
		MAKELPARAM( x, y )
	);
}
void WindowManager::SetDesktopIconPosition( const ICOINDEX Index, const D3DXPOSITION2& Pos )
{
	SetDesktopIconPosition( Index, static_cast<int>( Pos.x ), static_cast<int>( Pos.y ) );
}
void WindowManager::SetDesktopIconPosition( const ICOINDEX Index, const D3DXPOSITION3& Pos )
{
	SetDesktopIconPosition( Index, static_cast<int>( Pos.x ), static_cast<int>( Pos.y ) );
}

//---------------------------.
// デスクトップのアイコンの位置を追加.
//---------------------------.
void WindowManager::AddDesktopIconPosition( const ICOINDEX Index, const int x, const int y )
{
	WindowManager* pI = GetInstance();

	SetDesktopIconPosition( Index, static_cast<int>( pI->m_IconPosMap[Index].x + x ), static_cast<int>( pI->m_IconPosMap[Index].y + y ) );
}
void WindowManager::AddDesktopIconPosition( const ICOINDEX Index, const D3DXPOSITION2& Pos )
{
	AddDesktopIconPosition( Index, static_cast<int>( Pos.x ), static_cast<int>( Pos.y ) );
}
void WindowManager::AddDesktopIconPosition( const ICOINDEX Index, const D3DXPOSITION3& Pos )
{
	AddDesktopIconPosition( Index, static_cast<int>( Pos.x ), static_cast<int>( Pos.y ) );
}

//---------------------------.
// ウィンドウの位置を設定.
//---------------------------.
void WindowManager::SetWindowPosition( const HWND& hWnd, const int x, const int y )
{
	SetWindowPos( hWnd, NULL, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER );
}
void WindowManager::SetWindowPosition( const HWND& hWnd, const D3DXPOSITION2& Pos )
{
	SetWindowPosition( hWnd, static_cast<int>( Pos.x ), static_cast<int>( Pos.y ) );
}
void WindowManager::SetWindowPosition( const HWND& hWnd, const D3DXPOSITION3& Pos )
{
	SetWindowPosition( hWnd, static_cast<int>( Pos.x ), static_cast<int>( Pos.y ) );
}

//---------------------------.
// ウィンドウの位置を追加.
//---------------------------.
void WindowManager::AddWindowPosition( const HWND& hWnd, const int x, const int y )
{
	WindowManager* pI = GetInstance();

	SetWindowPosition( hWnd, pI->m_WindowRectMap[hWnd].left + x, pI->m_WindowRectMap[hWnd].top  + y );
}
void WindowManager::AddWindowPosition( const HWND& hWnd, const D3DXPOSITION2& Pos )
{
	AddWindowPosition( hWnd, static_cast<int>( Pos.x ), static_cast<int>( Pos.y ) );
}
void WindowManager::AddWindowPosition( const HWND& hWnd, const D3DXPOSITION3& Pos )
{
	AddWindowPosition( hWnd, static_cast<int>( Pos.x ), static_cast<int>( Pos.y ) );
}

//---------------------------.
// ウィンドウのサイズを設定.
//---------------------------.
void WindowManager::SetWindowSize( const HWND& hWnd, const int x, const int y )
{
	SetWindowPos( hWnd, NULL, 0, 0, x, y, SWP_NOMOVE | SWP_NOZORDER );
}
void WindowManager::SetWindowSize( const HWND& hWnd, const D3DXSCALE2& Size )
{
	SetWindowSize( hWnd, static_cast<int>( Size.x ), static_cast<int>( Size.y ) );
}
void WindowManager::SetWindowSize( const HWND& hWnd, const D3DXSCALE3& Size )
{
	SetWindowSize( hWnd, static_cast<int>( Size.x ), static_cast<int>( Size.y ) );
}

//---------------------------.
// ウィンドウのサイズを追加.
//---------------------------.
void WindowManager::AddWindowSize( const HWND& hWnd, const int x, const int y )
{
	WindowManager* pI = GetInstance();

	SetWindowSize( hWnd, pI->m_WindowRectMap[hWnd].right - pI->m_WindowRectMap[hWnd].left + x, pI->m_WindowRectMap[hWnd].bottom - pI->m_WindowRectMap[hWnd].top + y );
}
void WindowManager::AddWindowSize( const HWND& hWnd, const D3DXSCALE2& Size )
{
	AddWindowSize( hWnd, static_cast<int>( Size.x ), static_cast< int >( Size.y ) );
}
void WindowManager::AddWindowSize( const HWND& hWnd, const D3DXSCALE3& Size )
{
	AddWindowSize( hWnd, static_cast<int>( Size.x ), static_cast< int >( Size.y ) );
}

//---------------------------.
// ウィンドウを最前面に移動.
//---------------------------.
void WindowManager::SetWindowTop( const HWND& hWnd, const bool IsLock )
{
	SetWindowPos( hWnd, IsLock ? HWND_TOPMOST : HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW );
	if ( IsLock ) return;
	SetWindowPos( hWnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW );
}

//---------------------------.
// ウィンドウを最背面に移動.
//---------------------------.
void WindowManager::SetWindowBottom( const HWND& hWnd )
{
	SetWindowPos( hWnd, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW );
}

//---------------------------.
// ウィンドウを指定したウィンドウの下に移動.
//---------------------------.
void WindowManager::SetWindowSelectDown( const HWND& hWnd, const HWND& hSelectWnd )
{
	SetWindowPos( hWnd, hSelectWnd, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW );
}

//---------------------------.
// ウィンドウを指定したウィンドウの上に移動.
//---------------------------.
void WindowManager::SetWindowSelectUp( const HWND& hWnd, const HWND& hSelectWnd )
{
	SetWindowPos( hWnd, GetWindow( hSelectWnd, GW_HWNDPREV ), 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW );
}

//---------------------------.
// ウィンドウの破棄.
//---------------------------.
void WindowManager::WindowDelete( const HWND& hWnd )
{
	// ウィンドウの破棄.
	if ( hWnd == NULL ) PostMessage( GetInstance()->m_hMyWindow, WM_CLOSE, 0, 0 );
	else				PostMessage( hWnd, WM_CLOSE, 0, 0 );
}

//---------------------------.
// ウィンドウを強調する.
//---------------------------.
void WindowManager::WindowFlash( const HWND& hWnd )
{
	FLASHWINFO fInfo {
		sizeof( FLASHWINFO ),
		hWnd,
		FLASHW_TRAY | FLASHW_TIMERNOFG
	};
	FlashWindowEx( &fInfo );
}

//---------------------------.
// アプリのショートカットを作成する
//---------------------------.
bool WindowManager::CreateShortcut(
	LPCTSTR pszLink,		LPCTSTR pszFile,
	LPCTSTR pszDescription, LPCTSTR pszArgs,
	LPCTSTR pszWorkingDir,  LPCTSTR pszIconPath,
	int nIcon, int nShowCmd )
{
	HRESULT			hr;
	IShellLink*		pIShellLink;
	IPersistFile*	pIPersistFile;

	//IShellLinkの作成
	pIShellLink = NULL;
	hr = ::CoCreateInstance( CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, ( void** )&pIShellLink );
	if ( pIShellLink == NULL || FAILED( hr ) ) return	false;

	//IPersistFileの取得
	pIPersistFile = NULL;
	hr = pIShellLink->QueryInterface( IID_IPersistFile, ( void** )&pIPersistFile );
	if ( pIPersistFile == NULL || FAILED( hr ) )
	{
		pIShellLink->Release();
		return false;
	}

	//ショートカット詳細設定
	hr = pIShellLink->SetPath( pszFile );
	if ( SUCCEEDED( hr ) && pszDescription	) hr = pIShellLink->SetDescription( pszDescription );
	if ( SUCCEEDED( hr ) && pszArgs			) hr = pIShellLink->SetArguments( pszArgs );
	if ( SUCCEEDED( hr ) && pszWorkingDir	) hr = pIShellLink->SetWorkingDirectory( pszWorkingDir );
	if ( SUCCEEDED( hr ) && pszIconPath		) hr = pIShellLink->SetIconLocation( pszIconPath, nIcon );
	if ( SUCCEEDED( hr )					) hr = pIShellLink->SetShowCmd( nShowCmd );

#ifndef UNICODE
	WCHAR*	pwszUnicode;
	int		nLen;

	//Unicode変換
	nLen = ::MultiByteToWideChar( CP_ACP, 0, pszLink, -1, NULL, 0 );
	pwszUnicode = new WCHAR[nLen + 1];
	if ( pwszUnicode == NULL )
	{
		pIPersistFile->Release();
		pIShellLink->Release();
		return	false;
	}
	nLen = ::MultiByteToWideChar( CP_ACP, 0, pszLink, -1, pwszUnicode, nLen + 1 );
	if ( nLen == 0 ) hr = E_FAIL;

	//ショートカットの保存
	if ( SUCCEEDED( hr ) ) hr = pIPersistFile->Save( pwszUnicode, TRUE );

	delete	pwszUnicode;
#else
	//ショートカットの保存
	if ( SUCCEEDED( hr ) ) hr = pIPersistFile->Save( pszLink, TRUE );
#endif

	pIPersistFile->Release();
	pIShellLink->Release();

	return SUCCEEDED( hr ) ? true : false;
}

//---------------------------.
// ウィンドウの更新.
//---------------------------.
void WindowManager::WindowListUpdate()
{
	WindowManager* pI = GetInstance();

	// リストの初期化.
	pI->m_WindowList.clear();
	pI->m_WindowFindMap.clear();
	pI->m_WindowRectMap.clear();
	pI->m_WindowNameMap.clear();
	pI->m_WindowClassMap.clear();
	pI->m_WindowZOrderMap.clear();
	pI->m_MouseDownWindow = NULL;

	// マウスカーソルの位置の取得.
	const D3DXPOSITION2& MousePos = Input::GetMousePosition();

	// メインウィンドウ( 最前面で固定 )より下のウィンドウを調べていく.
	HWND hCheckWnd	= pI->m_hMyWindow;
	int	 ZOrder		= 0;
	while ( 1 ) {
		// 確認するウィンドウの下のウィンドウを取得.
		hCheckWnd = GetWindow( hCheckWnd, GW_HWNDNEXT );

		// 下にウィンドウが無い場合終了する.
		if ( hCheckWnd == NULL ) break;

		// このゲームの場合スキップする.
		if ( hCheckWnd == pI->m_hMyWindow		) continue;
		if ( hCheckWnd == pI->m_hMySubWindow	) continue;

		// ウィンドウ名を取得.
		WCHAR szName[256];
		GetWindowTextW( hCheckWnd, szName, 256 );

		// デスクトップウィンドウマネージャーの確認.
		int IsCloaked;
		DwmGetWindowAttribute( hCheckWnd, DWMWA_CLOAKED, &IsCloaked, sizeof( int ) );
		if ( IsCloaked != 0 ) continue;

		// ウィンドウを調べる.
		if ( ( wcslen( szName ) <= 0 ) ||
			!IsWindowVisible( hCheckWnd ) ||
			( GetParent( hCheckWnd ) != NULL ) ||
			( GetWindow( hCheckWnd, GW_OWNER ) != NULL ) ||
			( GetWindowLongPtrW( hCheckWnd, GWL_EXSTYLE ) & WS_EX_TOOLWINDOW ) )
		{
			continue;
		}

		// 縮小化中か調べる.
		if ( IsIconic( hCheckWnd ) ) continue;

		// ウィンドウの情報を追加する.
		RECT NoShadowRect;
		RECT Rect;
		DwmGetWindowAttribute( hCheckWnd, DWMWA_EXTENDED_FRAME_BOUNDS, &NoShadowRect, sizeof( RECT ) );
		GetWindowRect( hCheckWnd, &Rect );

		// ウィンドウの情報を調整する.
		NoShadowRect.left	+= pI->m_AddWindowRect.left;
		NoShadowRect.top	+= pI->m_AddWindowRect.top;
		NoShadowRect.right	+= pI->m_AddWindowRect.right;
		NoShadowRect.bottom	+= pI->m_AddWindowRect.bottom;
		Rect.left			+= pI->m_AddWindowRect.left;
		Rect.top			+= pI->m_AddWindowRect.top;
		Rect.right			+= pI->m_AddWindowRect.right;
		Rect.bottom			+= pI->m_AddWindowRect.bottom;

		// クラス名の取得.
		WCHAR szClass[256];
		GetClassNameW( hCheckWnd, szClass, 256 );

		// 名前を文字列に変換.
		const std::string WndName  = StringConversion::to_String( szName );
		const std::string WndClass = StringConversion::to_String( szClass );

		// マウスの下にあるか確認.
		if ( pI->m_MouseDownWindow == NULL ){
			if ( ( MousePos.x < NoShadowRect.right )  && ( MousePos.x > NoShadowRect.left ) &&
				 ( MousePos.y < NoShadowRect.bottom ) && ( MousePos.y > NoShadowRect.top  ) )
			{
				pI->m_MouseDownWindow = hCheckWnd;
			}
		}

		// ウィンドウを保存.
		pI->m_WindowList.emplace_back( std::make_pair( NoShadowRect, hCheckWnd ) );
		pI->m_WindowFindMap[WndName]			= hCheckWnd;
		pI->m_WindowRectMap[hCheckWnd]			= Rect;
		pI->m_WindowNoShadowRectMap[hCheckWnd]	= NoShadowRect;
		pI->m_WindowNameMap[hCheckWnd]			= WndName;
		pI->m_WindowClassMap[hCheckWnd]			= WndClass;
		pI->m_WindowZOrderMap[hCheckWnd]		= ZOrder;
		ZOrder++;
	}

	// ウィンドウの位置の更新を行った.
	pI->m_IsWindowUpdate = true;
}

//---------------------------.
// デスクトップのアイコンの位置の更新.
//---------------------------.
void WindowManager::DesktopIconUpdate()
{
	WindowManager* pI = GetInstance();

	// アイテム数取得.
	int nCount;
	nCount = ListView_GetItemCount( pI->m_hDesktop );
	pI->m_IconList.clear();
	pI->m_IconList.resize( nCount );

	// 対象アイテムの取得.
	for( int i = 0; i < nCount; i++ ) {
		// アイコン名の取得.
		TCHAR	buff[1024] = L"";	
		LVITEM	li;
		li.mask			= LVIF_TEXT;
		li.iItem		= i;
		li.iSubItem		= 0;
		li.cchTextMax	= 1024;
		li.pszText		= ( ( LPTSTR )pI->m_DProcessMemory ) + 1024;
		WriteProcessMemory( pI->m_DProcessHandle, pI->m_DProcessMemory, &li, sizeof( LVITEM ), NULL );
		SendMessage( pI->m_hDesktop, LVM_GETITEM, i, ( LPARAM )pI->m_DProcessMemory );
		ReadProcessMemory( pI->m_DProcessHandle, ( ( LPTSTR )pI->m_DProcessMemory ) + 1024, buff, 1024, NULL );

		// アイコンの位置の取得.
		POINT p;
		SendMessage( pI->m_hDesktop, LVM_GETITEMPOSITION, i, ( LPARAM )pI->m_DProcessMemory );
		ReadProcessMemory( pI->m_DProcessHandle, pI->m_DProcessMemory, &p, sizeof( POINT ), NULL );
		
		// アイコンの位置を調整する.
		p.x += pI->m_AddWindowRect.left;
		p.y += pI->m_AddWindowRect.top;

		// アイコン情報を保存.
		const D3DXPOSITION2 IconPos		= { static_cast<float>( p.x ), static_cast<float>( p.y ) };
		const std::string	IconName	= StringConversion::to_String( buff );
		pI->m_IconList[i]				= std::make_pair( IconPos, IconName );
		pI->m_IconFindMap[IconName]		= i;
		pI->m_IconPosMap[i]				= IconPos;
		pI->m_IconNameMap[i]			= IconName;
	}

	// デスクトップのアイコンの位置の更新を行った.
	pI->m_IsDesktopIconUpdate = true;
	return;
}

//---------------------------.
// タスクバーの更新.
//---------------------------.
void WindowManager::TaskBarUpdate()
{
	WindowManager* pI = GetInstance();

	if ( pI->m_IsTaskBarUpdate ) return;

	ZeroMemory( &pI->m_TaskBar, sizeof( pI->m_TaskBar ) );
	pI->m_TaskBar.cbSize = sizeof( pI->m_TaskBar );
	SHAppBarMessage( ABM_GETTASKBARPOS, &pI->m_TaskBar );
}
