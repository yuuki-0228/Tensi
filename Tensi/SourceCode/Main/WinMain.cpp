#include "Main.h"
#include "..\Global.h"
#include "..\Common\DirectX\DirectX11.h"
#include "..\Utility\Input\MouseSpeedGuard\MouseSpeedGuard.h"
#include <filesystem>
#include <string>
#include <crtdbg.h>	//_ASSERT_EXPR()で必要.

//グローバル変数.
std::unique_ptr<CMain>	g_pCMain = nullptr;

namespace {
	//---------------------------.
	// 現在のディレクトリをexeファイルの保存場所に設定
	//---------------------------.
	void MoveCurrentDirectoryToExePath()
	{
		WCHAR Path[MAX_PATH] = {};
		if ( GetModuleFileNameW( nullptr, Path, MAX_PATH ) == 0 ) return;
		const std::filesystem::path ExeDir = std::filesystem::path( Path ).parent_path();
		if ( ExeDir.empty() ) return;

		std::error_code ec;
		if ( std::filesystem::exists( ExeDir / L"Data", ec ) == false ) return;
		SetCurrentDirectoryW( ExeDir.c_str() );
	}

	//---------------------------.
	// 実行時にDPI対応を有効にします。
	//	これを有効にしないと、拡大表示（125% / 150%）では座標が仮想化され、
	//	ウィンドウがぼやけて表示されるほか、DPIが異なるマルチモニター
	//	環境では、仮想画面の寸法が正しく報告されなくなります。
	//	モニターごとの設定 V2（Win10 1703以降）→ システムDPI対応のフォールバック。
	//---------------------------.
	void EnableDpiAwareness()
	{
		using SetCtxFunc = BOOL( WINAPI* )( HANDLE );
		SetCtxFunc pSetCtx = nullptr;
		const HMODULE hUser32 = GetModuleHandleW( L"user32.dll" );
		if ( hUser32 != nullptr ) {
			pSetCtx = reinterpret_cast<SetCtxFunc>(
				GetProcAddress( hUser32, "SetProcessDpiAwarenessContext" ) );
		}
		// DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2 = (HANDLE)-4.
		const HANDLE PerMonitorV2 = reinterpret_cast<HANDLE>( static_cast<INT_PTR>( -4 ) );
		if ( pSetCtx == nullptr || pSetCtx( PerMonitorV2 ) == FALSE ) {
			SetProcessDPIAware();
		}
	}

	//---------------------------.
	// D3DX（DirectX End-User Runtime）のDLLを確認します。
	//	これらは遅延ロードされるため（vcxprojのDelayLoadDLLsを参照）、ランタイムがインストールされていないPCでは、
	//	意味不明なローダーエラーを表示する代わりに、どのDLLが不足しているか、
	//	およびそれらを入手する方法を表示することができます。
	//---------------------------.
	bool CheckDirectXRuntime()
	{
		constexpr const wchar_t* DllNames[] = {
			L"d3dx9_43.dll", L"d3dx10_43.dll", L"d3dx11_43.dll", L"D3DCompiler_43.dll"
		};
		std::wstring Missing;
		for ( const auto* Name : DllNames ) {
			const HMODULE hDll = LoadLibraryW( Name );
			if ( hDll != nullptr ) { FreeLibrary( hDll ); continue; }
			Missing += L"  ";
			Missing += Name;
			Missing += L"\n";
		}
		if ( Missing.empty() ) return true;

		const std::wstring Msg =
			L"DirectX runtime files were not found:\n\n" + Missing +
			L"\nPlease install the DirectX End-User Runtime.\n"
			L"https://www.microsoft.com/download/details.aspx?id=35";
		MessageBoxW( nullptr, Msg.c_str(), L"DirectX Runtime", MB_OK | MB_ICONERROR );
		return false;
	}
}

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

	// Environment dependent setup and checks.
	MoveCurrentDirectoryToExePath();

	// マウス速度復元の監視プロセスとして起動された場合は、監視処理のみ行い終了する.
	if ( MouseSpeedGuard::RunWatchdogIfRequested( lpCmdLine ) ) return 0;
	EnableDpiAwareness();
	if ( CheckDirectXRuntime() == false ) return 0;

	try {

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

	}
	catch ( const std::exception& e ) {
		// Do not die silently : leave the reason in the log and tell the user.
		Log::PushLog( std::string( "Unhandled exception : " ) + e.what() );
		MessageBoxA( nullptr, e.what(), "Fatal Error", MB_OK | MB_ICONERROR );
	}
	catch ( ... ) {
		Log::PushLog( "Unhandled exception : unknown" );
		MessageBoxA( nullptr, "An unknown fatal error occurred.", "Fatal Error", MB_OK | MB_ICONERROR );
	}

	// マウスを表示する.
	ShowCursor( TRUE );
	return 0;
}
