#pragma once
#include "..\..\SystemSetting.h"
#ifdef ENABLE_WINDOWS_MENU
#include "..\..\Global.h"

/************************************************
*	メニュークラス.
**/
class WindowsMenuManager
{
public:
	WindowsMenuManager();
	~WindowsMenuManager();

	// 初期化.
	static HRESULT Init( const HWND& hWnd );

	// 表示.
	static void Disp();
	// メニューの選択.
	static void SelectMenu( const WORD No );

private:
	// インスタンスの取得.
	static WindowsMenuManager* GetInstance();

private:
	HWND	m_hWnd;				// ウィンドウハンドル.
	HMENU	m_hMenu;			// メニュー.
	bool	m_IsDispBall;		// ボールを表示しているか.
	bool	m_IsDispHeavyBall;	// 重いボールを表示しているか.
	bool	m_IsSuperBall;		// スーパーボールを表示しているか.
	bool	m_IsPlayer;			// プレイヤーを表示しているか.
};
#endif