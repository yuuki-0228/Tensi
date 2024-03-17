#pragma once
#include <string>

// �����R�[�h.
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
*	������ϊ�
**/
namespace StringConversion {
	// std::string �� std::wstring �ɕϊ�.
	std::wstring to_wString( const std::string& s, const ECodePage& CodePage = ECodePage::None );

	// std::wstring �� std::string �ɕϊ�.
	std::string to_String( const std::wstring& ws, const ECodePage& CodePage = ECodePage::None );
}