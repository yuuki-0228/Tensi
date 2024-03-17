#pragma once
#include <string>

// •¶šƒR[ƒh.
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
*	•¶š—ñ•ÏŠ·
**/
namespace StringConversion {
	// std::string ‚ğ std::wstring ‚É•ÏŠ·.
	std::wstring to_wString( const std::string& s, const ECodePage& CodePage = ECodePage::None );

	// std::wstring ‚ğ std::string ‚É•ÏŠ·.
	std::string to_String( const std::wstring& ws, const ECodePage& CodePage = ECodePage::None );
}