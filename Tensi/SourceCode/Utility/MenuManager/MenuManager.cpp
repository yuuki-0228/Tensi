#include "MenuManager.h"
#ifdef ENABLE_MENU
#include "..\..\Object\GameObject\Actor\WindowObjectManager\WindowObject\WindowObject.h"
#include "..\..\Object\GameObject\Actor\WindowObjectManager\WindowObjectManager.h"

MenuManager::MenuManager()
	: m_hWnd			( NULL )
	, m_hMenu			( NULL )
	, m_IsDispBall		( false )
	, m_IsDispHeavyBall	( false )
	, m_IsSuperBall		( false )
	, m_IsPlayer		( true )
{
}

MenuManager::~MenuManager()
{
	// メニューの破棄.
	DestroyMenu( m_hMenu );
}

//---------------------------.
// インスタンスの取得.
//---------------------------.
MenuManager* MenuManager::GetInstance()
{
	static std::unique_ptr<MenuManager> pInstance = std::make_unique<MenuManager>();
	return pInstance.get();
}

//---------------------------.
// 初期化.
//---------------------------.
HRESULT MenuManager::Init( const HWND& hWnd )
{
	MenuManager* pI = GetInstance();

	// ウィンドウハンドルの保存.
	pI->m_hWnd	= hWnd;

	// メニューの作成.
	pI->m_hMenu = CreatePopupMenu();

	// サブメニューのテキスト.
	const std::vector<WCHAR*> Text = {
		( WCHAR* ) L"ボールの表示/非表示",
		( WCHAR* ) L"重いボールの表示/非表示",
		( WCHAR* ) L"スーパーボールの表示/非表示",
		( WCHAR* ) L"スライム君の表示/非表示",
		( WCHAR* ) L"ばいばい！"
	};

	// サブメニューの作成.
	const int Size = static_cast<int>( Text.size() );
	std::vector<MENUITEMINFO> SubMenu;
	SubMenu.resize( Size );
	for ( int i = 0; i < Size; ++i ) {
		SubMenu[i].cbSize		= sizeof( MENUITEMINFO );
		SubMenu[i].fMask		= MIIM_STRING | MIIM_ID;
		SubMenu[i].wID			= i;
		SubMenu[i].dwTypeData	= Text[i];
		SubMenu[i].cch			= 4;
		InsertMenuItem( pI->m_hMenu, i, TRUE, &SubMenu[i] );
	}
	return S_OK;
}

//---------------------------.
// メニューの表示.
//---------------------------.
void MenuManager::Disp()
{
	MenuManager* pI = GetInstance();

	// マウスカーソル座標取得.
	POINT Pos;
	GetCursorPos( &Pos );

	// メニューの表示.
	SetForegroundWindow( pI->m_hWnd );
	TrackPopupMenuEx( pI->m_hMenu, TPM_LEFTALIGN, Pos.x, Pos.y, pI->m_hWnd, NULL );
}

//---------------------------.
// メニューの選択.
//---------------------------.
void MenuManager::SelectMenu( const WORD No )
{
	MenuManager* pI = GetInstance();

	switch ( No ) {
	case 0:
		// ボールの表示/非表示の設定.
		pI->m_IsDispBall = !pI->m_IsDispBall;
		WindowObjectManager::GetObjectPtr( EObjectTag::NormalBall, 0 )->SetIsDisp( pI->m_IsDispBall );
		break;
	case 1:
		// 重いボールの表示/非表示の設定.
		pI->m_IsDispHeavyBall = !pI->m_IsDispHeavyBall;
		WindowObjectManager::GetObjectPtr( EObjectTag::HeavyBall, 0 )->SetIsDisp( pI->m_IsDispHeavyBall );
		break;
	case 2:
		// スーパーボールの表示/非表示の設定.
		pI->m_IsSuperBall = !pI->m_IsSuperBall;
		WindowObjectManager::GetObjectPtr( EObjectTag::SuperBall, 0 )->SetIsDisp( pI->m_IsSuperBall );
		break;
	case 3:
		// プレイヤーの表示/非表示の設定.
		pI->m_IsPlayer = !pI->m_IsPlayer;
		WindowObjectManager::GetObjectPtr( EObjectTag::Player, 0 )->SetIsDisp( pI->m_IsPlayer );
		break;
	case 4:
		// ウィンドウの破棄.
		DestroyWindow( pI->m_hWnd );
		break;
	}
}

#endif