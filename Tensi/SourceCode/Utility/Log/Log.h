#pragma once
#include <Windows.h>
#include <memory>
#include <string>
#include <time.h>

/************************************************
*	ログクラス.
**/
class Log final
{
public:
	Log();
	~Log();

	// ログテキストを作成.
	//	アプリ起動時に作成する.
	static HRESULT OpenLogText();

	// ログテキストを閉じる.
	static HRESULT CloseLogText();

	// ログファイルの破棄.
	static HRESULT DeleteLogText();

	// ログの入力の実体.
	//	直接呼ばず、下の PushLog～ マクロ( Log::PushLogInfo( "..." ) の形 )で呼び出す.
	static HRESULT PushLogDebugImpl		( const std::string& Log, const char* Caller );	// デバッグ (Releaseでは書き出さない).
	static HRESULT PushLogInfoImpl		( const std::string& Log, const char* Caller );	// 情報.
	static HRESULT PushLogWarningImpl	( const std::string& Log, const char* Caller );	// 警告.
	static HRESULT PushLogErrorImpl		( const std::string& Log, const char* Caller );	// エラー.
	static HRESULT PushLogFatalImpl		( const std::string& Log, const char* Caller );	// 致命的なエラー.

private:
	// インスタンスの取得.
	static Log* GetInstance();

private:
	bool m_Stop;	// ログを停止するか.

private:
	// コピー・ムーブコンストラクタ, 代入演算子の削除.
	Log( const Log& )					= delete;
	Log& operator = ( const Log& )	= delete;
	Log( Log&& )						= delete;
	Log& operator = ( Log&& )			= delete;
};

// ログの入力.
//	呼び出し元(クラス::関数名)を LOG_CALLER で自動的に付与する.
//	使用例 : Log::PushLogInfo( "メッセージ" );
//	出力例 : [12:34:56] [INFO] [CMain::InitWindow] メッセージ.
// Releaseでは呼び出し元の関数名( __FUNCTION__ )をバイナリに残さないため空にする.
//	Debugのみ関数名を付与し、ログの可読性を保つ.
#ifdef _DEBUG
#define LOG_CALLER __FUNCTION__
#else
#define LOG_CALLER ""
#endif

#define PushLogDebug( Text )	PushLogDebugImpl	( Text, LOG_CALLER )
#define PushLogInfo( Text )		PushLogInfoImpl		( Text, LOG_CALLER )
#define PushLogWarning( Text )	PushLogWarningImpl	( Text, LOG_CALLER )
#define PushLogError( Text )	PushLogErrorImpl	( Text, LOG_CALLER )
#define PushLogFatal( Text )	PushLogFatalImpl	( Text, LOG_CALLER )
