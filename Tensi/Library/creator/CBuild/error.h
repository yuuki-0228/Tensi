#pragma once
#include <Windows.h>
#include <string>

// 標準エラー出力とデバッガーの出力ウィンドウの両方にメッセージを出力する
namespace {
	void PrintOutput( const std::string& msg ) {
		fprintf( stderr, "%s", msg.c_str() );
		OutputDebugStringA( msg.c_str() );
	}
	void PrintOutput( const std::wstring& msg ) {
		fwprintf( stderr, L"%s", msg.c_str() );
		OutputDebugStringW( msg.c_str() );
	}
}

// エラー関数
namespace {
	int ErrorMessage( const std::string& msg, const std::string& type ) {
		std::string errorMsg = msg + " : " + type + "\n";
		PrintOutput( errorMsg );
		return E_FAIL;
	}
	int ErrorMessage( const std::wstring& msg, const std::wstring& type ) {
		std::wstring errorMsg = msg + L" : " + type + L"\n";
		PrintOutput( errorMsg );
		return E_FAIL;
	}
}