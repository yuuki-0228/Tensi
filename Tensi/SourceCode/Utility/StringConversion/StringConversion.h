#pragma once
#include <string>
#include <nameof\nameof.hpp>

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
*		﨑田友輝.
**/
namespace StringConversion {
	// std::string を std::wstring に変換.
	std::wstring to_wString( const std::string& s, const ECodePage& CodePage = ECodePage::None );

	// std::wstring を std::string に変換.
	std::string to_String( const std::wstring& ws, const ECodePage& CodePage = ECodePage::None );

	// 指定した幅になるように左側に文字を埋めた文字列に変換.
	std::string	 to_LeftPaddingString( const std::string&	s,	const int n, const char PaddedChar = ' ' );
	std::wstring to_LeftPaddingString( const std::wstring&	ws,	const int n, const char PaddedChar = ' ' );

	// 指定した幅になるように右側に文字を埋めた文字列に変換.
	std::string  to_RightPaddingString( const std::string&  s,	const int n, const char PaddedChar = ' ' );
	std::wstring to_RightPaddingString( const std::wstring& ws,	const int n, const char PaddedChar = ' ' );

	// 変数の型名を取得(全て省略).
	template<class T>
	inline std::string ShotType( const T& Var );

	// 変数の型名を取得(参照/修飾子を省略).
	template<class T>
	inline std::string Type( const T& Var );

	// 変数の型名を取得(全て).
	template<class T>
	inline std::string FullType( T Var );

	// 列挙体の名前の取得.
	template<class T>
	inline std::string Enum( const T& Enum );

	// 列挙体が宣言されているか取得.
	template<class T>
	inline bool EnumCheck( int i );

	// 列挙体の要素数を取得.
	template<class T>
	inline int EnumNum();
}

//---------------------------.
// 変数の型名を取得(全て省略).
//---------------------------.
template<class T>
std::string StringConversion::ShotType( const T& Var )
{
	return std::string( NAMEOF_SHORT_TYPE_EXPR( Var ) );
}

//---------------------------.
// 変数の型名を取得(参照/修飾子を省略).
//---------------------------.
template<class T>
inline std::string StringConversion::Type( const T& Var ) {
	return std::string( NAMEOF_TYPE_EXPR( Var ) );
}

//---------------------------.
// 変数の型名を取得(全て).
//---------------------------.
template<class T>
inline std::string StringConversion::FullType( T Var ) {
	return std::string( NAMEOF_FULL_TYPE_EXPR( Var ) );
}

//---------------------------.
// 列挙体の名前の取得.
//---------------------------.
template<class T>
inline std::string StringConversion::Enum( const T& Enum ) {
	return std::string( NAMEOF_ENUM( Enum ) );
}

//---------------------------.
// 列挙体が宣言されているか取得.
//---------------------------.
template<class T>
inline bool StringConversion::EnumCheck( int i ) {
	return NAMEOF_ENUM_CHECK( static_cast<T>( i ) );
}

//---------------------------.
// 列挙体の要素数を取得.
//---------------------------.
template<class T>
inline int StringConversion::EnumNum() {
	int		Cnt = 0;
	while ( EnumCheck<T>( Cnt ) ) Cnt++;
	return	Cnt;
}
