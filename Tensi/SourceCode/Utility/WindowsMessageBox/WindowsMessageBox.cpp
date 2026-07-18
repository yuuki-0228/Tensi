#include "WindowsMessageBox.h"
#include <Windows.h>
#include <thread>
#include "..\StringConversion\StringConversion.h"
#include "..\WindowManager\WindowManager.h"

namespace {
	// メッセージボックスのオーナーとするウィンドウのハンドルを取得する.
	HWND GetOwnerWnd()
	{
#ifdef ENABLE_WINDOWS_WINDOW
		return WindowManager::GetWnd();
#else
		return NULL;
#endif
	}
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
	// メッセージボックスの表示は別スレッドで行い、メインループを止めない.
	//	オーナーウィンドウを指定するとそのウィンドウが無効化され操作不能になるため、
	//	非ブロッキング表示ではオーナーを NULL にする.
	std::thread( [Text, Caption, OnResult]() {
		const std::wstring wText	= StringConversion::to_wString( Text );
		const std::wstring wCaption = StringConversion::to_wString( Caption );

		const int Result = MessageBoxW(
			NULL,
			wText.c_str(),
			wCaption.c_str(),
			MB_YESNO | MB_ICONQUESTION | MB_SETFOREGROUND | MB_TOPMOST );

		// 応答結果をコールバックに渡す( はい=true / いいえ=false ).
		//	ワーカースレッド上で実行されるため、結果を控える等の軽い処理のみを渡すこと.
		if ( OnResult ) OnResult( Result == IDYES );
	} ).detach();
}

//---------------------------.
// アプリを止めずに OK のみのメッセージボックスを表示する.
//---------------------------.
void WindowsMessageBox::InfoAsync( const std::string& Text, const std::string& Caption )
{
	std::thread( [Text, Caption]() {
		const std::wstring wText	= StringConversion::to_wString( Text );
		const std::wstring wCaption = StringConversion::to_wString( Caption );

		MessageBoxW(
			NULL,
			wText.c_str(),
			wCaption.c_str(),
			MB_OK | MB_ICONINFORMATION | MB_SETFOREGROUND | MB_TOPMOST );
	} ).detach();
}
