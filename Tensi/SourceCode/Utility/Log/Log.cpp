#include "Log.h"
#include "..\FileManager\FileManager.h"
#include <iostream>
#include <fstream>
#include <sstream>

namespace {
	constexpr char LOG_TEXT_FILE_PATH[]			= "Data\\$system.log";								// ログテキストのファイルパス.
	constexpr char WINDOW_SETTING_FILE_PATH[]	= "Data\\Parameter\\Config\\WindowSetting.json";	// ウィンドウの設定のファイルパス.
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
	return FileManager::LogSave( LOG_TEXT_FILE_PATH, Log, FileManager::ELogLevel::Debug, Caller );
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
	return FileManager::LogSave( LOG_TEXT_FILE_PATH, Log, FileManager::ELogLevel::Info, Caller );
}

//----------------------------.
// ログの入力 : 警告.
//----------------------------.
HRESULT Log::PushLogWarningImpl( const std::string& Log, const char* Caller )
{
	if ( GetInstance()->m_Stop ) return S_OK;
	return FileManager::LogSave( LOG_TEXT_FILE_PATH, Log, FileManager::ELogLevel::Warning, Caller );
}

//----------------------------.
// ログの入力 : エラー.
//----------------------------.
HRESULT Log::PushLogErrorImpl( const std::string& Log, const char* Caller )
{
	if ( GetInstance()->m_Stop ) return S_OK;
	return FileManager::LogSave( LOG_TEXT_FILE_PATH, Log, FileManager::ELogLevel::Error, Caller );
}

//----------------------------.
// ログの入力 : 致命的なエラー.
//----------------------------.
HRESULT Log::PushLogFatalImpl( const std::string& Log, const char* Caller )
{
	if ( GetInstance()->m_Stop ) return S_OK;
	return FileManager::LogSave( LOG_TEXT_FILE_PATH, Log, FileManager::ELogLevel::Fatal, Caller );
}
