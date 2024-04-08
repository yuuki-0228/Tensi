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

	// ログの入力.
	static HRESULT PushLog( const char*			Log );
	static HRESULT PushLog( const std::string&	Log );

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
