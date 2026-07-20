#include "Log.h"
#include "..\FileManager\FileManager.h"
#include "..\ThreadManager\ThreadManager.h"
#include <iostream>
#include <fstream>
#include <sstream>

namespace {
	constexpr char LOG_TEXT_FILE_PATH[]			= "Data\\$system.log";								// ログテキストのファイルパス.
	constexpr char WINDOW_SETTING_FILE_PATH[]	= "Data\\Parameter\\Config\\WindowSetting.json";	// ウィンドウの設定のファイルパス.
#ifdef ENABLE_THREAD
	constexpr char LOG_THREAD_TAG[]				= "Log";											// 書き込みの直列実行タグ.
#endif // ENABLE_THREAD

	// ログをファイルへ書き込む.
	//	ファイルの開閉を伴い重いことがあるため、ワーカースレッドの直列ジョブで行う.
	//	( 同タグの直列実行のため書き込み順は呼び出し順のまま保たれる ).
	HRESULT PushLogToFile( const std::string& Text, const FileManager::ELogLevel Level, const std::string& Caller )
	{
#ifdef ENABLE_THREAD
		if ( ThreadManager::GetIsAvailable() ) {
			ThreadManager::StartSequential( LOG_THREAD_TAG, [Text, Level, Caller]() {
				FileManager::LogSave( LOG_TEXT_FILE_PATH, Text, Level, Caller );
			} );
			return S_OK;
		}
#endif // ENABLE_THREAD
		// スレッドが使えない場合( アプリ終了時など )は同期で書き込む.
		return FileManager::LogSave( LOG_TEXT_FILE_PATH, Text, Level, Caller );
	}
}

Log::Log()
	: m_Stop	( false )
{
}

Log::~Log()
{
}

//----------------------------.
// インスタンスの取得.
//----------------------------.
Log* Log::GetInstance()
{
	static std::unique_ptr<Log> pInstance = std::make_unique<Log>();
	return pInstance.get();
}

//----------------------------.
// ログテキストを作成.
//----------------------------.
HRESULT Log::OpenLogText()
{
	// ログを停止するか取得.
	Json WndSetting			= FileManager::JsonLoad( WINDOW_SETTING_FILE_PATH );
	GetInstance()->m_Stop	= WndSetting["IsLogStop"].Get( false );
	if ( GetInstance()->m_Stop ) return S_OK;

	// アプリ起動ログを日付付きで新規作成する.
	return FileManager::LogSave( LOG_TEXT_FILE_PATH, "App launch", FileManager::ELogLevel::Info, __FUNCTION__, false );
}

//----------------------------.
// ログテキストを閉じる.
//----------------------------.
HRESULT Log::CloseLogText()
{
	if ( GetInstance()->m_Stop ) return S_OK;

	// アプリ終了ログを追記する.
	return FileManager::LogSave( LOG_TEXT_FILE_PATH, "App End", FileManager::ELogLevel::Info, __FUNCTION__ );
}

//----------------------------.
// ログファイルの破棄.
//----------------------------.
HRESULT Log::DeleteLogText()
{
	std::filesystem::remove( LOG_TEXT_FILE_PATH );
	return S_OK;
}

//----------------------------.
// ログの入力 : デバッグ.
//	Releaseでは書き出さない.
//----------------------------.
HRESULT Log::PushLogDebugImpl( const std::string& Log, const char* Caller )
{
#ifdef _DEBUG
	if ( GetInstance()->m_Stop ) return S_OK;
	return PushLogToFile( Log, FileManager::ELogLevel::Debug, Caller );
#else
	return S_OK;
#endif
}

//----------------------------.
// ログの入力 : 情報.
//----------------------------.
HRESULT Log::PushLogInfoImpl( const std::string& Log, const char* Caller )
{
	if ( GetInstance()->m_Stop ) return S_OK;
	return PushLogToFile( Log, FileManager::ELogLevel::Info, Caller );
}

//----------------------------.
// ログの入力 : 警告.
//----------------------------.
HRESULT Log::PushLogWarningImpl( const std::string& Log, const char* Caller )
{
	if ( GetInstance()->m_Stop ) return S_OK;
	return PushLogToFile( Log, FileManager::ELogLevel::Warning, Caller );
}

//----------------------------.
// ログの入力 : エラー.
//----------------------------.
HRESULT Log::PushLogErrorImpl( const std::string& Log, const char* Caller )
{
	if ( GetInstance()->m_Stop ) return S_OK;
	return PushLogToFile( Log, FileManager::ELogLevel::Error, Caller );
}

//----------------------------.
// ログの入力 : 致命的なエラー.
//----------------------------.
HRESULT Log::PushLogFatalImpl( const std::string& Log, const char* Caller )
{
	if ( GetInstance()->m_Stop ) return S_OK;
	return PushLogToFile( Log, FileManager::ELogLevel::Fatal, Caller );
}
