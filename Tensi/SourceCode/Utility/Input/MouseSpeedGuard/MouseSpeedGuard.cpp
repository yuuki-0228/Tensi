#include "MouseSpeedGuard.h"
#include "..\..\FileManager\FileManager.h"
#include <csignal>
#include <cstdio>
#include <cstring>
#include <exception>
#include <string>

namespace {
	constexpr char	RECOVERY_FILE_PATH[]	= "Data\\Parameter\\Data\\ms.bin";				// 復帰用ファイルの保存場所.
	constexpr char	WATCHDOG_OPTION[]		= "--mouse-speed-watchdog";								// 監視プロセス起動用のコマンドライン引数.
	constexpr int	MOUSE_SPEED_MIN			= 1;	// マウス速度の最小値.
	constexpr int	MOUSE_SPEED_MAX			= 20;	// マウス速度の最大値.

	int								s_OriginalSpeed			= 0;		// 復元先のマウス速度.
	std::terminate_handler			s_PrevTerminateHandler	= nullptr;	// 元の terminate ハンドラ.
	LPTOP_LEVEL_EXCEPTION_FILTER	s_PrevExceptionFilter	= nullptr;	// 元の例外フィルタ.

	//---------------------------.
	// マウス速度をOSに設定する.
	//---------------------------.
	void ApplyMouseSpeed( const int Speed )
	{
		// 不正な値では設定しない.
		if ( Speed < MOUSE_SPEED_MIN || MOUSE_SPEED_MAX < Speed ) return;
		SystemParametersInfoA( SPI_SETMOUSESPEED, 0, reinterpret_cast<PVOID>( static_cast<INT_PTR>( Speed ) ), 0 );
	}

	//---------------------------.
	// 未処理例外の発生時に復元する.
	//---------------------------.
	LONG WINAPI CrashExceptionFilter( EXCEPTION_POINTERS* pExceptionInfo )
	{
		ApplyMouseSpeed( s_OriginalSpeed );

		// 元の例外フィルタに処理を引き継ぐ.
		if ( s_PrevExceptionFilter != nullptr ) return s_PrevExceptionFilter( pExceptionInfo );
		return EXCEPTION_CONTINUE_SEARCH;
	}

	//---------------------------.
	// terminate の発生時に復元する.
	//---------------------------.
	void TerminateHandler()
	{
		ApplyMouseSpeed( s_OriginalSpeed );

		// 元の terminate ハンドラに処理を引き継ぐ.
		if ( s_PrevTerminateHandler != nullptr ) s_PrevTerminateHandler();
		std::abort();
	}

	//---------------------------.
	// abort の発生時に復元する.
	//---------------------------.
	void AbortSignalHandler( int )
	{
		ApplyMouseSpeed( s_OriginalSpeed );
	}

	//---------------------------.
	// 監視プロセスを起動する.
	//	自分自身のexeを監視モードで起動し、本体がどのような形で
	//	終了してもマウス速度を復元できるようにする.
	//---------------------------.
	void LaunchWatchdog( const int Speed )
	{
		// 自分自身のexeのパスを取得.
		char ExePath[MAX_PATH] = {};
		if ( GetModuleFileNameA( nullptr, ExePath, MAX_PATH ) == 0 ) return;

		// コマンドラインの作成.
		std::string CmdLine =
			"\"" + std::string( ExePath ) + "\" " + WATCHDOG_OPTION + " " +
			std::to_string( GetCurrentProcessId() ) + " " + std::to_string( Speed );

		// 監視プロセスの起動.
		STARTUPINFOA		si = { sizeof( STARTUPINFOA ) };
		PROCESS_INFORMATION	pi = {};
		if ( CreateProcessA( nullptr, CmdLine.data(), nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi ) == FALSE ) return;
		CloseHandle( pi.hThread );
		CloseHandle( pi.hProcess );
	}
}

//---------------------------.
// 監視プロセスとして起動された場合、本体の終了を監視して復元する.
//---------------------------.
bool MouseSpeedGuard::RunWatchdogIfRequested( const char* lpCmdLine )
{
	if ( lpCmdLine == nullptr ) return false;

	// 監視プロセス用の引数が無ければ本体として起動する.
	const char* pOption = std::strstr( lpCmdLine, WATCHDOG_OPTION );
	if ( pOption == nullptr ) return false;

	// 引数から本体のプロセスIDと復元する速度を取得.
	unsigned long	ParentPid	= 0;
	int				Speed		= 0;
	if ( sscanf_s( pOption + std::strlen( WATCHDOG_OPTION ), "%lu %d", &ParentPid, &Speed ) != 2 ) return true;

	// 本体プロセスの終了を待機する.
	//	既に終了している場合はハンドルが開けないため、そのまま復元処理へ進む.
	HANDLE hParent = OpenProcess( SYNCHRONIZE, FALSE, ParentPid );
	if ( hParent != nullptr ) {
		WaitForSingleObject( hParent, INFINITE );
		CloseHandle( hParent );
	}

	// 復元用ファイルが残っている場合は異常終了のため復元する.
	//	※正常終了時は本体側でファイルが削除されているため何もしない.
	if ( FileManager::FileCheck( RECOVERY_FILE_PATH ) == false ) return true;
	int SavedSpeed = Speed;
	FileManager::BinaryLoad( RECOVERY_FILE_PATH, SavedSpeed );
	ApplyMouseSpeed( SavedSpeed );
	return true;
}

//---------------------------.
// 復元保証を開始する.
//---------------------------.
__int64 MouseSpeedGuard::Start()
{
	// 現在のマウス速度を取得.
	int Speed = 0;
	SystemParametersInfoA( SPI_GETMOUSESPEED, 0, &Speed, 0 );

	// 復元用ファイルが残っている場合は前回異常終了しているため、
	// 保存していた速度を元の速度として採用して復元する.
	//	※現在の速度は前回の実行で変更されたままの可能性があるため使用しない.
	if ( FileManager::FileCheck( RECOVERY_FILE_PATH ) ) {
		int SavedSpeed = Speed;
		FileManager::BinaryLoad( RECOVERY_FILE_PATH, SavedSpeed );
		if ( MOUSE_SPEED_MIN <= SavedSpeed && SavedSpeed <= MOUSE_SPEED_MAX ) Speed = SavedSpeed;
		ApplyMouseSpeed( Speed );
	}
	s_OriginalSpeed = Speed;

	// 異常終了時に復元できるように元の速度を保存しておく.
	//	※このファイルは正常終了時に削除される.
	FileManager::BinarySave( RECOVERY_FILE_PATH, Speed );

	// クラッシュ時に復元するためのハンドラを登録.
	s_PrevExceptionFilter	= SetUnhandledExceptionFilter( CrashExceptionFilter );
	s_PrevTerminateHandler	= std::set_terminate( TerminateHandler );
	std::signal( SIGABRT, AbortSignalHandler );

	// 監視プロセスの起動.
	LaunchWatchdog( Speed );

	return Speed;
}

//---------------------------.
// マウス速度を元に戻し、復元保証を終了する.
//---------------------------.
void MouseSpeedGuard::Stop()
{
	// マウス速度を元に戻す.
	ApplyMouseSpeed( s_OriginalSpeed );

	// 正常終了のため復元用ファイルを削除する.
	//	※これにより監視プロセスは何もせずに終了する.
	DeleteFileA( RECOVERY_FILE_PATH );
}
