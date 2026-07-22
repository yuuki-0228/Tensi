#include "RoomCode.h"
#ifdef ENABLE_NETWORK
#include <cstdio>
#include <cctype>

namespace
{
	constexpr unsigned long long MASK48 = 0xFFFFFFFFFFFFULL;	// 48bitマスク.
}

//---------------------------.
// 部屋番号の作成.
//---------------------------.
std::string RoomCode::Encode( const std::string& Ip, unsigned short Port )
{
	// IPアドレスの解析.
	unsigned int a = 0, b = 0, c = 0, d = 0;
	if ( sscanf_s( Ip.c_str(), "%u.%u.%u.%u", &a, &b, &c, &d ) != 4 )	return "";
	if ( a > 255 || b > 255 || c > 255 || d > 255 )							return "";

	// IP(32bit) + ポート(16bit) を48bitにまとめる.
	const unsigned long long IpValue =
		( static_cast<unsigned long long>( a ) << 24 ) |
		( static_cast<unsigned long long>( b ) << 16 ) |
		( static_cast<unsigned long long>( c ) <<  8 ) |
		( static_cast<unsigned long long>( d ) );
	const unsigned long long Value = ( IpValue << 16 ) | static_cast<unsigned long long>( Port );

	// スクランブルしてチェックサムを付与する( 計56bit ).
	const unsigned long long Scrambled	= Scramble( Value );
	const unsigned long long Code		= ( Scrambled << 8 ) | static_cast<unsigned long long>( CalcChecksum( Scrambled ) );

	// 数字17桁にして 6-6-5 で区切る.
	char Buf[32];
	std::snprintf( Buf, sizeof( Buf ), "%017llu", Code );
	std::string Text( Buf );
	return Text.substr( 0, 6 ) + "-" + Text.substr( 6, 6 ) + "-" + Text.substr( 12, 5 );
}

//---------------------------.
// 部屋番号の復元.
//---------------------------.
bool RoomCode::Decode( const std::string& Code, std::string* pOutIp, unsigned short* pOutPort )
{
	// 数字だけを取り出す.
	std::string Digits;
	for ( const char c : Code ) {
		if ( std::isdigit( static_cast<unsigned char>( c ) ) )	Digits += c;
	}
	if ( Digits.length() != CODE_DIGITS )	return false;

	// 数値に戻す( 17桁は64bitに収まる ).
	unsigned long long Value = 0;
	for ( const char c : Digits ) {
		Value = Value * 10 + static_cast<unsigned long long>( c - '0' );
	}
	if ( Value > ( ( MASK48 << 8 ) | 0xFF ) )	return false;

	// チェックサムの確認.
	const unsigned long long	Scrambled	= Value >> 8;
	const unsigned char			Checksum	= static_cast<unsigned char>( Value & 0xFF );
	if ( CalcChecksum( Scrambled ) != Checksum )	return false;

	// 逆スクランブルしてIPアドレスとポート番号に分解する.
	const unsigned long long	Raw		= Unscramble( Scrambled );
	const unsigned long long	IpValue	= Raw >> 16;
	const unsigned short		Port	= static_cast<unsigned short>( Raw & 0xFFFF );

	if ( pOutIp != nullptr ) {
		char Buf[32];
		std::snprintf( Buf, sizeof( Buf ), "%u.%u.%u.%u",
			static_cast<unsigned int>( ( IpValue >> 24 ) & 0xFF ),
			static_cast<unsigned int>( ( IpValue >> 16 ) & 0xFF ),
			static_cast<unsigned int>( ( IpValue >>  8 ) & 0xFF ),
			static_cast<unsigned int>(   IpValue         & 0xFF ) );
		*pOutIp = Buf;
	}
	if ( pOutPort != nullptr )	*pOutPort = Port;
	return true;
}

//---------------------------.
// 48bitの左ローテーション.
//---------------------------.
unsigned long long RoomCode::Rotate48( unsigned long long Value, int Shift )
{
	Shift %= 48;
	if ( Shift == 0 )	return Value & MASK48;
	return ( ( Value << Shift ) | ( Value >> ( 48 - Shift ) ) ) & MASK48;
}

//---------------------------.
// スクランブル.
//---------------------------.
unsigned long long RoomCode::Scramble( unsigned long long Value )
{
	unsigned long long v = Value & MASK48;
	v ^= SCRAMBLE_KEY1;
	v  = Rotate48( v, SCRAMBLE_ROT1 );
	v ^= SCRAMBLE_KEY2;
	v  = Rotate48( v, SCRAMBLE_ROT2 );
	return v;
}

//---------------------------.
// 逆スクランブル.
//---------------------------.
unsigned long long RoomCode::Unscramble( unsigned long long Value )
{
	unsigned long long v = Value & MASK48;
	v  = Rotate48( v, 48 - SCRAMBLE_ROT2 );
	v ^= SCRAMBLE_KEY2;
	v  = Rotate48( v, 48 - SCRAMBLE_ROT1 );
	v ^= SCRAMBLE_KEY1;
	return v;
}

//---------------------------.
// チェックサムの計算.
//---------------------------.
unsigned char RoomCode::CalcChecksum( unsigned long long Value )
{
	unsigned char Sum = CHECKSUM_SALT;
	for ( int i = 0; i < 6; ++i ) {
		Sum = static_cast<unsigned char>( Sum + ( ( Value >> ( i * 8 ) ) & 0xFF ) );
		Sum = static_cast<unsigned char>( ( Sum << 1 ) | ( Sum >> 7 ) );
	}
	return Sum;
}

#endif	// #ifdef ENABLE_NETWORK.
