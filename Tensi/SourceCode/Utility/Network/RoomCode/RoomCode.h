#pragma once
#include "..\..\..\SystemSettings.h"
#ifdef ENABLE_NETWORK
#include <string>

/************************************************
*	部屋番号の相互変換.
*	IPv4アドレスとポート番号( 計48bit )を定数ベースで
*	スクランブルし、数字だけの部屋番号文字列に変換する.
*	( 見た目からIPアドレスが分からない部屋番号にする ).
*
*	使い方.
*	・作成 : RoomCode::Encode( "192.168.0.10", 52400 ) で "123456-789012-34567" のような文字列を得る.
*	・復元 : RoomCode::Decode( 部屋番号, &Ip, &Port ) でIPアドレスとポート番号に戻す.
**/
class RoomCode final
{
public:
	// スクランブル用の定数( 変更すると過去の部屋番号と互換性が無くなる ).
	static constexpr unsigned long long	SCRAMBLE_KEY1	= 0x5A3C96E1B7D2ULL;	// XOR鍵1( 48bit ).
	static constexpr unsigned long long	SCRAMBLE_KEY2	= 0x1F8E6D4C2B0AULL;	// XOR鍵2( 48bit ).
	static constexpr int				SCRAMBLE_ROT1	= 13;					// ローテーション量1.
	static constexpr int				SCRAMBLE_ROT2	= 29;					// ローテーション量2.
	static constexpr unsigned char		CHECKSUM_SALT	= 0xA5;					// チェックサム用の値.
	static constexpr int				CODE_DIGITS		= 17;					// 部屋番号の桁数( 区切りを除く ).

public:
	// IPアドレスとポート番号から部屋番号を作成する.
	//	Ip : "192.168.0.10" のような文字列.
	//	失敗時は空文字列を返す.
	static std::string Encode( const std::string& Ip, unsigned short Port );

	// 部屋番号からIPアドレスとポート番号を復元する.
	//	区切りのハイフンや空白は無視される. 失敗時は false.
	static bool Decode( const std::string& Code, std::string* pOutIp, unsigned short* pOutPort );

private:
	// 48bitの左ローテーション.
	static unsigned long long Rotate48( unsigned long long Value, int Shift );
	// スクランブル.
	static unsigned long long Scramble( unsigned long long Value );
	// 逆スクランブル.
	static unsigned long long Unscramble( unsigned long long Value );
	// チェックサムの計算.
	static unsigned char CalcChecksum( unsigned long long Value );
};

#endif	// #ifdef ENABLE_NETWORK.
