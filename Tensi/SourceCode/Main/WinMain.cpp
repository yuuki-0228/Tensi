#include "Main.h"
#include "..\Global.h"
#include "..\Common\DirectX\DirectX11.h"
#include <crtdbg.h>	//_ASSERT_EXPR()で必要.

//グローバル変数.
std::unique_ptr<CMain>	g_pCMain = nullptr;

//================================================
//	メイン関数.
//================================================
INT WINAPI WinMain(
	HINSTANCE hInstance,	//インスタンス番号(ウィンドウの番号).
	HINSTANCE hPrevInstance,
	PSTR lpCmdLine,
	INT nCmdShow )
{
#ifdef _DEBUG
	// メモリリーク検出
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif	// #ifdef _DEBUG.

	Log::OpenLogText();

	// 初期化＆クラス宣言.
	g_pCMain = std::make_unique<CMain>();

	if ( g_pCMain != nullptr ) {

		// ウィンドウ作成成功したら.
		if ( SUCCEEDED( g_pCMain->InitWindow( hInstance ) ) )
		{
			// Dx11用の初期化.
			if ( SUCCEEDED( g_pCMain->Create() ) )
			{
				// メッセージループ.
				g_pCMain->Loop();
			}
		}
	}
	Log::CloseLogText();

	// マウスを表示する.
	ShowCursor( TRUE );
	return 0;
}
