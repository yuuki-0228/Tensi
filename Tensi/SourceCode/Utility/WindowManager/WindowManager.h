#pragma once
#include "..\..\Global.h"

// ウィンドウクラス定数.
namespace WindowClassType {
	const std::string NOTEPAD_CLASS		= "Notepad";			// メモ帳.
	const std::string IE_BROWSER_CLASS	= "IEFrame";			// IEブラウザ.
	const std::string BROWSER_CLASS		= "Chrome_WidgetWin_1";	// GoogleChrome / MicrosoftEdge.
	const std::string EXPLORER_CLASS	= "CabinetWClass";		// エクスプローラ.
}
namespace {
	// アイコンインデックス型.
	using ICOINDEX = int;
}

/************************************************
*	ウィンドウの情報マネージャークラス.
**/
class WindowManager
{
public:
	using IconList		= std::vector<std::pair<D3DXPOSITION2, std::string>>;
	using IconFindMap	= std::unordered_map<std::string, ICOINDEX>;
	using IconPosMap	= std::unordered_map<ICOINDEX, D3DXPOSITION2>;
	using IconNameMap	= std::unordered_map<ICOINDEX, std::string>;
	using WndList		= std::vector<std::pair<RECT, HWND>>;
	using WndFindMap	= std::unordered_map<std::string, HWND>;
	using WndRectMap	= std::unordered_map<HWND, RECT>;
	using WndNameMap	= std::unordered_map<HWND, std::string>;
	using WndZOrderMap	= std::unordered_map<HWND, int>;

public:
	WindowManager();
	~WindowManager();

	// 初期化.
	static HRESULT Init();
	// 更新.
	static void Update();

	// デスクトップのハンドルの取得.
	static HWND GetDesktop() { return GetInstance()->m_hDesktop; }
	// このゲームのウィンドウハンドルの取得.
	static HWND GetWnd() { return GetInstance()->m_hMyWindow; }
	static HWND GetSubWnd() { return GetInstance()->m_hMySubWindow; }

	// このゲームのウィンドウの情報を取得.
	static RECT GetMyWndSize() { return GetInstance()->m_MyWindowRect; }

	// タスクバーの情報を取得.
	static RECT GetTaskBarRect();
	// タスクバーのウィンドウハンドルの取得.
	static HWND GetTaskBarWnd();

	// ウィンドウの情報の補正値の取得.
	static RECT GetAddWindowRect() { return GetInstance()->m_AddWindowRect; }

	// マウスが他のウィンドウの上にある場合そのウィンドウのハンドルを取得(上にない場合:NULL).
	static HWND GetMouseOverTheWindow();
	// マウスが他のウィンドウの上にあるか取得.
	static bool GetIsMouseOverTheWindow();

	// 指定した座標が他のウィンドウの上にある場合そのウィンドウのハンドルを取得(上にない場合:NULL).
	static HWND GetPointOverTheWindow( const D3DXPOSITION3& Pos );
	static HWND GetPointOverTheWindow( const D3DXPOSITION2& Pos );
	// 指定した座標が他のウィンドウの上にあるか取得.
	static bool GetIsPointOverTheWindow( const D3DXPOSITION3& Pos );
	static bool GetIsPointOverTheWindow( const D3DXPOSITION2& Pos );

	// ウィンドウの名前からハンドルを取得.
	static HWND GetFindWindow( const std::string& Name );
	// デスクトップのアイコン名からアイコンインデックスを取得.
	static ICOINDEX GetFindDesktopIcon( const std::string& Name );

	// ウィンドウの情報リストを取得.
	//	<ドロップシャドウを除いたウィンドウの情報, ウィンドウハンドル>.
	static WndList GetWindowList();
	// デスクトップのアイコンの位置リストの取得.
	//	<アイコンの位置, アイコン名>.
	static IconList	GetDesktopIconList();

	// ウィンドウの情報の取得.
	static RECT GetWindowSize( const HWND& hWnd );
	// ドロップシャドウを除いたウィンドウの情報の取得.
	static RECT GetWindowNoShadowSize( const HWND& hWnd );
	// ウィンドウ名の取得.
	static std::string GetWindowName( const HWND& hWnd );
	// ウィンドウクラス名の取得.
	static std::string GetWindowClassName( const HWND& hWnd );
	// ウィンドウのZオーダーの取得( 数字が小さい程上側 ).
	static int GetWindowZOrder( const HWND& hWnd );

	// デスクトップのアイコンの位置を取得.
	static D3DXPOSITION2 GetDesktopIconPosition( const ICOINDEX Index );
	// デスクトップのアイコンの名前を取得.
	static std::string GetDesktopIconName( const ICOINDEX Index );
	// デスクトップのアイコンのファイルパスを取得.
	static std::string GetDesktopIconPath( const ICOINDEX Index );

	// マウスの下にあるウィンドウの取得.
	static HWND GetMouseDownWindow() { return GetInstance()->m_MouseDownWindow; }

	// デスクトップのアイコンの位置を設定.
	static void SetDesktopIconPosition( const ICOINDEX Index, const int x, const int y );
	static void SetDesktopIconPosition( const ICOINDEX Index, const D3DXPOSITION2& Pos );
	static void SetDesktopIconPosition( const ICOINDEX Index, const D3DXPOSITION3& Pos );
	// デスクトップのアイコンの位置を追加.
	static void AddDesktopIconPosition( const ICOINDEX Index, const int x, const int y );
	static void AddDesktopIconPosition( const ICOINDEX Index, const D3DXPOSITION2& Pos );
	static void AddDesktopIconPosition( const ICOINDEX Index, const D3DXPOSITION3& Pos );

	// ウィンドウの位置を設定.
	static void SetWindowPosition( const HWND& hWnd, const int x, const int y );
	static void SetWindowPosition( const HWND& hWnd, const D3DXPOSITION2& Pos );
	static void SetWindowPosition( const HWND& hWnd, const D3DXPOSITION3& Pos );
	// ウィンドウの位置を追加.
	static void AddWindowPosition( const HWND& hWnd, const int x, const int y );
	static void AddWindowPosition( const HWND& hWnd, const D3DXPOSITION2& Pos );
	static void AddWindowPosition( const HWND& hWnd, const D3DXPOSITION3& Pos );

	// ウィンドウのサイズを設定.
	static void SetWindowSize( const HWND& hWnd, const int x, const int y );
	static void SetWindowSize( const HWND& hWnd, const D3DXSCALE2& Size );
	static void SetWindowSize( const HWND& hWnd, const D3DXSCALE3& Size );
	// ウィンドウのサイズを追加.
	static void AddWindowSize( const HWND& hWnd, const int x, const int y );
	static void AddWindowSize( const HWND& hWnd, const D3DXSCALE2& Size );
	static void AddWindowSize( const HWND& hWnd, const D3DXSCALE3& Size );

	// ウィンドウを最前面に移動.
	static void SetWindowTop( const HWND& hWnd, const bool IsLock = false );
	// ウィンドウを最背面に移動.
	static void SetWindowBottom( const HWND& hWnd );
	// ウィンドウを指定したウィンドウの下に移動.
	static void SetWindowSelectDown( const HWND& hWnd, const HWND& hSelectWnd );
	// ウィンドウを指定したウィンドウの上に移動.
	static void SetWindowSelectUp( const HWND& hWnd, const HWND& hSelectWnd );

	// ウィンドウの破棄( "hWnd" が "NULL" の場合このウィンドウを破棄する ).
	static void WindowDelete( const HWND& hWnd = NULL );

	// ウィンドウを強調する.
	static void WindowFlash( const HWND& hWnd );

	// ウィンドウハンドルの設定.
	static void SetWnd( HWND hWnd ) { GetInstance()->m_hMyWindow = hWnd; }
	static void SetSubWnd( HWND hWnd ) { GetInstance()->m_hMySubWindow = hWnd; }

	// アプリのショートカットを作成する.
	static bool CreateShortcut(
		LPCTSTR pszLink,				LPCTSTR pszFile,
		LPCTSTR pszDescription = NULL,	LPCTSTR pszArgs		= NULL,
		LPCTSTR pszWorkingDir  = NULL,	LPCTSTR pszIconPath	= NULL,
		int nIcon = 0, int nShowCmd = SW_SHOWNORMAL );

private:
	// インスタンスの取得.
	static WindowManager* GetInstance();

	// ウィンドウの更新.
	static void WindowListUpdate();

	// デスクトップのアイコンの位置の更新.
	static void DesktopIconUpdate();
	
	// タスクバーの更新.
	static void TaskBarUpdate();

private:
	HWND			m_hDesktop;					// デスクトップのハンドル.
	HWND			m_hMyWindow;				// このゲームのウィンドウのハンドル.
	HWND			m_hMySubWindow;				// このゲームのサブウィンドウのハンドル.
	HWND			m_MouseDownWindow;			// マウスカーソルの下にあるウィンドウ.
	RECT			m_MyWindowRect;				// このゲームのウィンドウの情報.
	RECT			m_AddWindowRect;			// ウィンドウの情報に追加する補正値.
	APPBARDATA		m_TaskBar;					// タスクバーの情報.
	DWORD			m_DProcessID;				// デスクトップのプロセスID.
	HANDLE			m_DProcessHandle;			// デスクトップのプロセスのハンドル.
	LPVOID			m_DProcessMemory;			// デスクトップのプロセスのメモリ.
	WndList			m_WindowList;				// ウィンドウの情報リスト.
	WndFindMap		m_WindowFindMap;			// ウィンドウの名前でハンドル取得用リスト.
	WndRectMap		m_WindowRectMap;			// ウィンドウ情報取得用リスト.
	WndRectMap		m_WindowNoShadowRectMap;	// ドロップシャドウを除いたウィンドウ情報取得用リスト.
	WndNameMap		m_WindowNameMap;			// ウィンドウ名取得用リスト.
	WndNameMap		m_WindowClassMap;			// ウィンドウクラス名取得用リスト.
	WndZOrderMap	m_WindowZOrderMap;			// ウィンドウのZオーダー取得用リスト.
	IconList		m_IconList;					// アイコンの位置リスト.
	IconFindMap		m_IconFindMap;				// アイコンの名前でアイコンの位置取得用リスト.
	IconPosMap		m_IconPosMap;				// アイコンのインデックス取得用リスト.
	IconNameMap		m_IconNameMap;				// アイコンのインデックス取得用リスト.
	std::string		m_UserName;					// ユーザー名.
	bool			m_IsWindowUpdate;			// ウィンドウリストを更新したか.
	bool			m_IsDesktopIconUpdate;		// デスクトップのアイコンの位置を更新したか.
	bool			m_IsTaskBarUpdate;			// タスクバーの更新をしたか.
};