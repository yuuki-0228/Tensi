#pragma once
#include <string>

// 文字コード.
enum class enCodePage {
	ANSI,
	Macintosh,
	OEM,
	Symbol,
	UTF7,
	UTF8,

	None = 0
} typedef ECodePage;

/************************************************
*	文字列変換
**/
namespace StringConversion {
	// std::string を std::wstring に変換.
	std::wstring to_wString( const std::string& s, const ECodePage& CodePage = ECodePage::None );

	// std::wstring を std::string に変換.
	std::string to_String( const std::wstring& ws, const ECodePage& CodePage = ECodePage::None );
}