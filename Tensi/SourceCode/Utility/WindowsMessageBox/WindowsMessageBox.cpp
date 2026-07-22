#include "WindowsMessageBox.h"
#include <Windows.h>
#include <algorithm>
#include <chrono>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>
#include "..\StringConversion\StringConversion.h"
#include "..\WindowsWindowManager\WindowsWindowManager.h"
#include "..\ThreadManager\ThreadManager.h"

namespace {
	// メッセージボックスのウィンドウクラス名( ダイアログの共通クラス ).
	constexpr wchar_t DIALOG_CLASS_NAME[] = L"#32770";

	// 表示中のメッセージボックスの情報.
	struct SDialogInfo
	{
		DWORD	ThreadId;	// 表示しているスレッドのID.
		int		CancelId;	// 強制的に閉じるときに押すボタンのID.
	};

	std::mutex					g_DialogMutex;			// 表示中のメッセージボックス用ミューテックス.
	std::vector<SDialogInfo>	g_DialogList;			// 表示中のメッセージボックスの一覧.
	bool						g_IsCloseAll = false;	// 全て閉じる要求が来たか( 以降は表示しない ).

	// メッセージボックスのオーナーとするウィンドウのハンドルを取得する.
	HWND GetOwnerWnd()
	{
#ifdef ENABLE_WINDOWS_WINDOW
		return WindowsWindowManager::GetWnd();
#else
		return NULL;
#endif
	}

	// 表示するメッセージボックスを一覧へ登録する( 終了要求済みの場合は false ).
	bool AddDialog( const int CancelId )
	{
		std::lock_guard<std::mutex> Lock( g_DialogMutex );
		if ( g_IsCloseAll ) return false;

		g_DialogList.emplace_back( SDialogInfo{ GetCurrentThreadId(), CancelId } );
		return true;
	}

	// 表示を終えたメッセージボックスを一覧から取り除く.
	void RemoveDialog()
	{
		const DWORD ThreadId = GetCurrentThreadId();

		std::lock_guard<std::mutex> Lock( g_DialogMutex );
		g_DialogList.erase(
			std::remove_if( g_DialogList.begin(), g_DialogList.end(),
				[ThreadId]( const SDialogInfo& Info ) { return Info.ThreadId == ThreadId; } ),
			g_DialogList.end() );
	}

	// スレッド上のメッセージボックスへ閉じる指示を送る.
	BOOL CALLBACK CloseDialogProc( HWND hWnd, LPARAM lParam )
	{
		// メッセージボックスのウィンドウのみを対象にする.
		wchar_t ClassName[MAX_PATH] = L"";
		GetClassNameW( hWnd, ClassName, MAX_PATH );
		if ( wcscmp( ClassName, DIALOG_CLASS_NAME ) != 0 ) return TRUE;

		// ボタンを押したことにして閉じる.
		//	( MB_YESNO は閉じるボタンが無効なため WM_CLOSE では閉じられない ).
		const int CancelId = static_cast<int>( lParam );
		PostMessageW( hWnd, WM_COMMAND, MAKEWPARAM( CancelId, BN_CLICKED ), 0 );
		return TRUE;
	}

	// メッセージボックスを表示する( 終了要求時に外部から閉じられるよう一覧へ登録する ).
	//	終了要求済みで表示しなかった場合は CancelId を返す.
	//	オーナーウィンドウを指定するとそのウィンドウが無効化され操作不能になるため、
	//	非ブロッキング表示ではオーナーを NULL にする.
	int ShowTrackedMessageBox( const std::string& Text, const std::string& Caption, const UINT Type, const int CancelId )
	{
		if ( AddDialog( CancelId ) == false ) return CancelId;

		const std::wstring wText	= StringConversion::to_wString( Text );
		const std::wstring wCaption = StringConversion::to_wString( Caption );
		const int Result = MessageBoxW( NULL, wText.c_str(), wCaption.c_str(), Type );

		RemoveDialog();
		return Result;
	}

	// 非ブロッキング表示のメッセージボックスの種類.
	constexpr UINT	YESNO_TYPE		= MB_YESNO | MB_ICONQUESTION | MB_SETFOREGROUND | MB_TOPMOST;
	constexpr UINT	INFO_TYPE		= MB_OK | MB_ICONINFORMATION | MB_SETFOREGROUND | MB_TOPMOST;
	// 強制的に閉じるときに押すボタンのID( はい・いいえは「いいえ」扱いにする ).
	constexpr int	YESNO_CANCEL_ID	= IDNO;
	constexpr int	INFO_CANCEL_ID	= IDOK;
}

//---------------------------.
// はい・いいえのメッセージボックスを表示する.
//---------------------------.
bool WindowsMessageBox::YesNo( const std::string& Text, const std::string& Caption )
{
	const std::wstring wText	= StringConversion::to_wString( Text );
	const std::wstring wCaption = StringConversion::to_wString( Caption );

	const int Result = MessageBoxW(
		GetOwnerWnd(),
		wText.c_str(),
		wCaption.c_str(),
		MB_YESNO | MB_ICONQUESTION | MB_SETFOREGROUND | MB_TOPMOST );

	return Result == IDYES;
}

//---------------------------.
// OKのみのメッセージボックスを表示する.
//---------------------------.
void WindowsMessageBox::Info( const std::string& Text, const std::string& Caption )
{
	const std::wstring wText	= StringConversion::to_wString( Text );
	const std::wstring wCaption = StringConversion::to_wString( Caption );

	MessageBoxW(
		GetOwnerWnd(),
		wText.c_str(),
		wCaption.c_str(),
		MB_OK | MB_ICONINFORMATION | MB_SETFOREGROUND | MB_TOPMOST );
}

//---------------------------.
// アプリを止めずに「はい・いいえ」のメッセージボックスを表示する.
//---------------------------.
void WindowsMessageBox::YesNoAsync( const std::string& Text, const std::string& Caption, std::function<void(bool)> OnResult )
{
#ifdef ENABLE_THREAD
	// メッセージボックスは閉じられるまで戻らないため、専用スレッドで表示する.
	//	( ワーカースレッドで表示すると閉じられるまで1本占有してしまうため ).
	if ( ThreadManager::GetIsAvailable() ) {
		auto pIsYes = std::make_shared<bool>( false );
		ThreadManager::StartDedicated(
			[Text, Caption, pIsYes]( const std::atomic<bool>& ) {
				*pIsYes = ( ShowTrackedMessageBox( Text, Caption, YESNO_TYPE, YESNO_CANCEL_ID ) == IDYES );
			},
			// 応答結果はメインスレッドで受け取る( はい=true / いいえ=false ).
			[pIsYes, OnResult]() { if ( OnResult ) OnResult( *pIsYes ); } );
		return;
	}
	// スレッドマネージャーが使えない場合( アプリ終了時など )は表示せずいいえ扱いにする.
	if ( OnResult ) OnResult( false );
#else
	// スレッド機能が無効な場合は別スレッドで表示する.
	//	( この場合のコールバックは別スレッド上で実行される ).
	std::thread( [Text, Caption, OnResult]() {
		const int Result = ShowTrackedMessageBox( Text, Caption, YESNO_TYPE, YESNO_CANCEL_ID );
		if ( OnResult ) OnResult( Result == IDYES );
	} ).detach();
#endif // ENABLE_THREAD
}

//---------------------------.
// アプリを止めずに OK のみのメッセージボックスを表示する.
//---------------------------.
void WindowsMessageBox::InfoAsync( const std::string& Text, const std::string& Caption )
{
#ifdef ENABLE_THREAD
	// メッセージボックスは閉じられるまで戻らないため、専用スレッドで表示する.
	if ( ThreadManager::GetIsAvailable() == false ) return;

	ThreadManager::StartDedicated(
		[Text, Caption]( const std::atomic<bool>& ) {
			ShowTrackedMessageBox( Text, Caption, INFO_TYPE, INFO_CANCEL_ID );
		} );
#else
	// スレッド機能が無効な場合は別スレッドで表示する.
	std::thread( [Text, Caption]() {
		ShowTrackedMessageBox( Text, Caption, INFO_TYPE, INFO_CANCEL_ID );
	} ).detach();
#endif // ENABLE_THREAD
}

//---------------------------.
// 表示中のメッセージボックスを全て閉じる.
//---------------------------.
void WindowsMessageBox::Release()
{
	// 閉じるまでの再試行( 表示要求の直後でウィンドウがまだ作られていない場合があるため ).
	constexpr int CLOSE_RETRY_NUM		= 200;	// 再試行の回数.
	constexpr int CLOSE_RETRY_WAIT_MS	= 10;	// 再試行の間隔( ミリ秒 ).

	// 以降の表示を止める.
	{
		std::lock_guard<std::mutex> Lock( g_DialogMutex );
		g_IsCloseAll = true;
	}

	// 表示中のメッセージボックスが無くなるまで閉じる指示を送り続ける.
	for ( int i = 0; i < CLOSE_RETRY_NUM; ++i ) {
		std::vector<SDialogInfo> List;
		{
			std::lock_guard<std::mutex> Lock( g_DialogMutex );
			List = g_DialogList;
		}
		if ( List.empty() ) return;

		for ( const auto& Info : List ) {
			EnumThreadWindows( Info.ThreadId, CloseDialogProc, static_cast<LPARAM>( Info.CancelId ) );
		}
		std::this_thread::sleep_for( std::chrono::milliseconds( CLOSE_RETRY_WAIT_MS ) );
	}
}
