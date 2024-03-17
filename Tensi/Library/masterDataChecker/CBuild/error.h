#pragma once
#include <Windows.h>
#include <string>

// ƒGƒ‰[ŠÖ”
namespace {
	int ErrorMessage( const std::string& msg, const std::string& type ) {
		std::string errorMsg = msg + " : " + type + "\n";
		fprintf( stderr, errorMsg.c_str() );
		return E_FAIL;
	}
	int ErrorMessage( const std::wstring& msg, const std::wstring& type ) {
		std::wstring errorMsg = msg + L" : " + type + L"\n";
		fwprintf( stderr, errorMsg.c_str() );
		return E_FAIL;
	}
}