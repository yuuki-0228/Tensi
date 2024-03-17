#include "Log.h"
#include "..\FileManager\FileManager.h"
#include <iostream>
#include <fstream>
#include <sstream>

namespace {
	typedef tm TIME_DATA;
	constexpr char LOG_TEXT_FILE_PATH[]			= "Data\\$Log.txt";									// ログテキストのファイルパス.
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
	json WndSetting			= FileManager::JsonLoad( WINDOW_SETTING_FILE_PATH );
	GetInstance()->m_Stop	= WndSetting["IsLogStop"];
	if ( GetInstance()->m_Stop ) return S_OK;

	// ファイルを開く.
	std::ofstream LogText;
	LogText.open( LOG_TEXT_FILE_PATH, std::ios::trunc );

	// ファイルが読み込めてなかったら終了.
	if( !LogText.is_open() ) return E_FAIL;

	// 現在の時間を獲得.
	time_t		nowTime	= time( nullptr );
	TIME_DATA	timeData;	
	// ローカル時間に変換.
	localtime_s( &timeData, &nowTime );

	// 現在の時間をテキストに入力.
	LogText << "[";
	LogText << timeData.tm_year + 1900	<< "/";	// 1900 足すことで現在の年になる.
	LogText << timeData.tm_mon + 1		<< "/";	// 1 足すことで現在の年になる.
	LogText << timeData.tm_mday			<< " ";
	LogText << timeData.tm_hour			<< ":";
	LogText << timeData.tm_min;
	LogText << "]" << " >> App launch" << std::endl << std::endl;

	// ファイルを閉じる.
	LogText.close();
	return S_OK;
}

//----------------------------.
// ログテキストを閉じる.
//----------------------------.
HRESULT Log::CloseLogText()
{
	if ( GetInstance()->m_Stop ) return S_OK;

	// ファイルを読み込む.
	std::ofstream LogText( LOG_TEXT_FILE_PATH, std::ios::app );

	// ファイルが読み込めてなかったら終了.
	if( !LogText.is_open() ) return E_FAIL;

	// 現在の時間を獲得.
	time_t		nowTime = time( nullptr );
	TIME_DATA	timeData;
	// ローカル時間に変換.
	localtime_s( &timeData, &nowTime );

	// 現在の時間をテキストに入力.
	LogText << std::endl		<<  "[";
	LogText << timeData.tm_hour	<< ":";
	LogText << timeData.tm_min	<< ":";
	LogText << timeData.tm_sec;
	LogText << "]" << " >> App End" << std::endl;

	// ファイルを閉じる.
	LogText.close();

	return S_OK;
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
// ログの入力.
//----------------------------.
HRESULT Log::PushLog( const char* Log )
{
	if ( GetInstance()->m_Stop ) return S_OK;

	// ファイルを開く.
	std::ofstream LogText( LOG_TEXT_FILE_PATH, std::ios::app );

	// ファイルが読み込めてなかったら終了.
	if( !LogText.is_open() ) return E_FAIL;

	// 現在の時間を獲得.
	time_t		nowTime = time( nullptr );
	TIME_DATA	timeData;
	// ローカル時間に変換.
	localtime_s( &timeData, &nowTime );

	// 現在の時間をテキストに入力.
	LogText << "[";
	LogText << timeData.tm_hour << ":";
	LogText << timeData.tm_min	<< ":";
	LogText << timeData.tm_sec;
	// ログの入力.
	LogText << "]" << " >> " << Log << std::endl;

	// ファイルを閉じる.
	LogText.close();

	return S_OK;
}
HRESULT Log::PushLog( const std::string& Log )
{
	if ( GetInstance()->m_Stop ) return S_OK;

	// ファイルを開く.
	std::ofstream LogText( LOG_TEXT_FILE_PATH, std::ios::app );

	// ファイルが読み込めてなかったら終了.
	if( !LogText.is_open() ) return E_FAIL;

	// 現在の時間を獲得.
	time_t		nowTime = time( nullptr );
	TIME_DATA	timeData;
	// ローカル時間に変換.
	localtime_s( &timeData, &nowTime );

	// 現在の時間をテキストに入力.
	LogText << "[";
	LogText << timeData.tm_hour << ":";
	LogText << timeData.tm_min	<< ":";
	LogText << timeData.tm_sec;
	// ログの入力.
	LogText << "]" << " >> " << Log.c_str() << std::endl;

	// ファイルを閉じる.
	LogText.close();

	return S_OK;
}
