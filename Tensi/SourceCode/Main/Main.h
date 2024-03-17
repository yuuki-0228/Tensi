#pragma once
#include "..\Global.h"
#include "..\Common\Sprite\Sprite.h"

class CFrameRate;
class CLoadManager;
class CMaskFade;

/************************************************
*	メインクラス.
**/
class CMain final
{
public:
	CMain();
	~CMain();

	void	Update(		const float& DeltaTime );	// 更新処理.
	HRESULT Create();								// 構築処理.
	void	Loop();									// メインループ.

	// ウィンドウ初期化関数.
	HRESULT InitWindow( HINSTANCE hInstance );

private:
	// ウィンドウ関数(メッセージ毎の処理).
	static LRESULT CALLBACK MsgProc(
		HWND hWnd, UINT uMsg,
		WPARAM wParam, LPARAM lParam );
	
	// ウィンドウ起動時の初期化( 起動時一回のみ ).
	void WindowInit();
	// FPSの描画.
	void FPSRender();

	// ウィンドウのクリック判定の更新.
	void ClickUpdate();

private:
	HWND							m_hWnd;			// ウィンドウハンドル.
	HWND							m_hSubWnd;		// サブウィンドウハンドル.
	HDC								m_hDc;			// ウィンドウのDC.
	HDC								m_hSubDc;		// サブウィンドウのDC.
	std::unique_ptr<CFrameRate>		m_pFrameRate;	// フレームレート.
	std::unique_ptr<CLoadManager>	m_pLoadManager;	// ロードマネージャー.
	bool							m_IsGameLoad;	// 読み込みが終了したか.
	bool							m_IsFPSRender;	// FPSを表示させるか.
	bool							m_IsWindowInit;	// ウィンドウ起動時の初期化を行ったか.
	bool							m_IsWindowTop;	// ウィンドウを最前面で固定するか.
};