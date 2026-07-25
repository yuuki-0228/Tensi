#pragma once
#include <cstddef>
#include <string>

/************************************************
*	文字列リテラルのコンパイル時難読化.
*
*	OBF ( "..."  ) … char    文字列を暗号化し、実行時に std::string  を返す.
*	OBFW( L"..." ) … wchar_t 文字列を暗号化し、実行時に std::wstring を返す.
*
*	いずれもコンパイル時に XOR 暗号化してバイナリへ埋め込み、実行時にその場で復号する.
*	これにより、実行ファイルを strings 等でダンプしても平文の文字列が現れない.
*
*	使用例 :
*		std::string  path = OBF ( "Data\\Parameter\\Secret.json" );
*		std::wstring reg  = OBFW( L"Software\\Company\\App" );
*
*	注意 :
*		・戻り値は std::string / std::wstring( 実行時にヒープ確保して復号する ).
*		  constexpr 配列が要求される箇所には使えないため、
*		  const 文字列へ置き換えるか、その場で使用すること.
*		・機密性の高い文字列にのみ手動で適用する( 全文字列への一括適用はしない ).
**/
namespace Obfuscate
{
	// コンパイル時 XOR 暗号化文字列( char / wchar_t 共通 ).
	//	テンプレート引数 Key をリテラルごとに変え、同じ文字列でも埋め込みバイト列を変える.
	template<class T, std::size_t N, T Key>
	struct ObfString
	{
		// 暗号化済みの列( 平文はここには残らない ).
		T m_Data[N];

		// コンストラクタ : コンパイル時に 1 要素ずつ暗号化する.
		//	インデックス i ごとに鍵をずらし、単純な繰り返しパターンを避ける.
		constexpr ObfString( const T( &Str )[N] ) : m_Data{}
		{
			for ( std::size_t i = 0; i < N; ++i ) {
				m_Data[i] = static_cast<T>( Str[i] ^ static_cast<T>( Key + static_cast<T>( i ) ) );
			}
		}

		// 実行時に復号して basic_string を返す.
		//	volatile を挟み、最適化で平文へ畳み込まれてバイナリに再出現するのを防ぐ.
		std::basic_string<T> Decrypt() const
		{
			T buf[N];
			for ( std::size_t i = 0; i < N; ++i ) {
				volatile T c = m_Data[i];
				buf[i] = static_cast<T>( c ^ static_cast<T>( Key + static_cast<T>( i ) ) );
			}
			// 終端の NUL を除いて文字列化する.
			return std::basic_string<T>( buf, N - 1 );
		}
	};
}

// リテラルごとの鍵( 行番号から生成し、行ごとに暗号化結果が変わる ).
#define OBF_KEY( T ) static_cast<T>( ( __LINE__ * 131 + 83 ) & 0x7F )

// char 文字列をコンパイル時に暗号化し、実行時に復号した std::string を得る.
#define OBF( Str ) \
	( []() -> std::string { \
		constexpr ::Obfuscate::ObfString<char, sizeof( Str ) / sizeof( char ), OBF_KEY( char )> _obf( Str ); \
		return _obf.Decrypt(); \
	}() )

// wchar_t 文字列をコンパイル時に暗号化し、実行時に復号した std::wstring を得る.
#define OBFW( Str ) \
	( []() -> std::wstring { \
		constexpr ::Obfuscate::ObfString<wchar_t, sizeof( Str ) / sizeof( wchar_t ), OBF_KEY( wchar_t )> _obf( Str ); \
		return _obf.Decrypt(); \
	}() )
