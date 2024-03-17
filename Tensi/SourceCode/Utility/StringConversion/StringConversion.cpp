#include "StringConversion.h"
#include <Windows.h>
#include <iostream>
#include <iomanip>
#include <sstream>

//---------------------------.
// std::string �� std::wstring �ɕϊ�.
//---------------------------.
std::wstring StringConversion::to_wString( const std::string& s, const ECodePage& CodePage )
{
	// �����R�[�h�̐ݒ�.
	int Code = CP_ACP;
	switch ( CodePage ) {
	case ECodePage::Macintosh:	Code = CP_MACCP;	break;
	case ECodePage::OEM:		Code = CP_OEMCP;	break;
	case ECodePage::Symbol:		Code = CP_SYMBOL;	break;
	case ECodePage::UTF7:		Code = CP_UTF7;		break;
	case ECodePage::UTF8:		Code = CP_UTF8;		break;
	default:					Code = CP_ACP;		break;
	}

	// �������ϊ�����.
	int				Size = MultiByteToWideChar( Code, 0, &s[0], ( int )s.size(), NULL, 0 );
	std::wstring	wOut( Size, 0 );
	MultiByteToWideChar( Code, 0, &s[0], ( int )s.size(), &wOut[0], Size );
	return wOut;
}

//---------------------------.
// std::wstring �� std::string �ɕϊ�.
//---------------------------.
std::string StringConversion::to_String( const std::wstring& ws, const ECodePage& CodePage )
{
	// �����R�[�h�̐ݒ�.
	int Code = CP_ACP;
	switch ( CodePage ) {
	case ECodePage::Macintosh:	Code = CP_MACCP;	break;
	case ECodePage::OEM:		Code = CP_OEMCP;	break;
	case ECodePage::Symbol:		Code = CP_SYMBOL;	break;
	case ECodePage::UTF7:		Code = CP_UTF7;		break;
	case ECodePage::UTF8:		Code = CP_UTF8;		break;
	default:					Code = CP_ACP;		break;
	}

	// �������ϊ�����.
	const int	Size = WideCharToMultiByte( Code, 0, ws.c_str(), -1, (char*) NULL, 0, NULL, NULL );
	CHAR*		Byte = new CHAR[Size];
	WideCharToMultiByte( Code, 0, ws.c_str(), -1, Byte, Size, NULL, NULL );
	std::string Out( Byte, Byte + Size - 1 );
	delete[]	Byte;
	return Out;
}

//---------------------------.
// �����w�肵�ĕ�����𖄂߂�.
//---------------------------.
std::string StringConversion::to_LeftPaddingString( const std::string& s, const int n, const char PaddedChar )
{
	std::ostringstream ss;
	ss << std::right << std::setfill( PaddedChar ) << std::setw( n ) << s;
	return ss.str();
}
std::wstring StringConversion::to_LeftPaddingString( const std::wstring& ws, const int n, const char PaddedChar )
{
	const std::string& String			= StringConversion::to_String( ws );
	const std::string& PaddingString	= to_LeftPaddingString( String, n, PaddedChar );
	return StringConversion::to_wString( PaddingString );
}

//---------------------------.
// �����w�肵�ĕ�����𖄂߂�.
//---------------------------.
std::string StringConversion::to_RightPaddingString( const std::string& s, const int n, const char PaddedChar )
{
	std::ostringstream ss;
	ss << std::left << std::setfill( PaddedChar ) << std::setw( n ) << s;
	return ss.str();
}
std::wstring StringConversion::to_RightPaddingString( const std::wstring& ws, const int n, const char PaddedChar )
{
	const std::string& String			= StringConversion::to_String( ws );
	const std::string& PaddingString	= to_RightPaddingString( String, n, PaddedChar );
	return StringConversion::to_wString( PaddingString );
}