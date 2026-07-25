#include "key.h"

namespace {
	// マスター鍵の長さ( ChaCha20 の鍵は 256bit = 32byte ).
	constexpr int MASTER_KEY_LEN = 32;

	// 難読化用の基準マスク( コンパイル時定数 ).
	constexpr unsigned char MASK = 0x5A;

	// XOR マスク済みマスター鍵.
	//	平文の鍵を連続配列で持たず、実行時に MASK で復元する( strings 等での静的抽出対策 ).
	//	実鍵[i] = 格納値[i] ^ ( MASK + i ) として実行時に求める.
	const unsigned char g_MaskedKey[MASTER_KEY_LEN] = {
		0x3C, 0xA1, 0x7F, 0x08, 0xD5, 0x62, 0xBE, 0x14,
		0x9A, 0x4D, 0xE7, 0x21, 0x88, 0xF3, 0x5B, 0xC6,
		0x0E, 0x77, 0xB2, 0x39, 0xD4, 0x6A, 0x91, 0xC0,
		0x2F, 0xE5, 0x1D, 0xA8, 0x53, 0xBC, 0x74, 0x9E,
	};

	// マスター鍵の 1 バイトを実行時に復元する.
	//	volatile を挟み、最適化で平文へ畳み込まれてバイナリに再出現するのを防ぐ.
	inline unsigned char MasterKeyByte( int i )
	{
		volatile unsigned char m = g_MaskedKey[i % MASTER_KEY_LEN];
		return static_cast<unsigned char>( m ^ static_cast<unsigned char>( MASK + i ) );
	}

	// 32bit 左ローテート.
	inline std::uint32_t RotL32( std::uint32_t v, int c )
	{
		return ( v << c ) | ( v >> ( 32 - c ) );
	}

	// ChaCha のクォーターラウンド.
	inline void QuarterRound( std::uint32_t& a, std::uint32_t& b, std::uint32_t& c, std::uint32_t& d )
	{
		a += b; d ^= a; d = RotL32( d, 16 );
		c += d; b ^= c; b = RotL32( b, 12 );
		a += b; d ^= a; d = RotL32( d,  8 );
		c += d; b ^= c; b = RotL32( b,  7 );
	}

	// ChaCha20 ブロック関数( 64 バイトの鍵ストリームを生成する ).
	void ChaCha20Block( const std::uint32_t key[8], std::uint32_t counter, const std::uint32_t nonce[3], unsigned char out[64] )
	{
		// 初期状態( 定数 "expand 32-byte k" + 鍵 + カウンタ + ノンス ).
		std::uint32_t st[16] = {
			0x61707865u, 0x3320646eu, 0x79622d32u, 0x6b206574u,
			key[0], key[1], key[2], key[3], key[4], key[5], key[6], key[7],
			counter, nonce[0], nonce[1], nonce[2],
		};

		std::uint32_t x[16];
		for ( int i = 0; i < 16; ++i ) x[i] = st[i];

		// 20 ラウンド( ダブルラウンド 10 回 ).
		for ( int i = 0; i < 10; ++i ) {
			QuarterRound( x[0], x[4], x[ 8], x[12] );
			QuarterRound( x[1], x[5], x[ 9], x[13] );
			QuarterRound( x[2], x[6], x[10], x[14] );
			QuarterRound( x[3], x[7], x[11], x[15] );
			QuarterRound( x[0], x[5], x[10], x[15] );
			QuarterRound( x[1], x[6], x[11], x[12] );
			QuarterRound( x[2], x[7], x[ 8], x[13] );
			QuarterRound( x[3], x[4], x[ 9], x[14] );
		}

		// 初期状態を加算して little-endian で書き出す.
		for ( int i = 0; i < 16; ++i ) {
			const std::uint32_t v = x[i] + st[i];
			out[i * 4 + 0] = static_cast<unsigned char>( v         & 0xFF );
			out[i * 4 + 1] = static_cast<unsigned char>( ( v >>  8 ) & 0xFF );
			out[i * 4 + 2] = static_cast<unsigned char>( ( v >> 16 ) & 0xFF );
			out[i * 4 + 3] = static_cast<unsigned char>( ( v >> 24 ) & 0xFF );
		}
	}

	// ChaCha20-CTR で鍵ストリームを XOR する( 暗号化・復号で対称 ).
	//	鍵    : 難読化済みマスター鍵を実行時に合成( little-endian ).
	//	ノンス: ファイル毎シード( パス由来 )とファイルサイズから生成する.
	//		size は暗号化・復号どちらでも同一ファイルなら必ず一致する値のため、
	//		desync の心配なくノンスの実効エントロピーを増やせる
	//		( 以前は 3 ワード目を 0 のまま捨てていた ).
	void XorStream( char* data, const DWORD& size, std::uint32_t seed )
	{
		std::uint32_t key[8];
		for ( int w = 0; w < 8; ++w ) {
			key[w] =  static_cast<std::uint32_t>( MasterKeyByte( w * 4 + 0 ) )
					| ( static_cast<std::uint32_t>( MasterKeyByte( w * 4 + 1 ) ) <<  8 )
					| ( static_cast<std::uint32_t>( MasterKeyByte( w * 4 + 2 ) ) << 16 )
					| ( static_cast<std::uint32_t>( MasterKeyByte( w * 4 + 3 ) ) << 24 );
		}

		const std::uint32_t sizeWord = static_cast<std::uint32_t>( size ) ^ 0xA5A5A5A5u;
		const std::uint32_t nonce[3] = { seed, RotL32( seed, 16 ) ^ sizeWord, sizeWord };
		std::uint32_t counter = 0;
		unsigned char ks[64];

		for ( DWORD i = 0; i < size; ) {
			ChaCha20Block( key, counter, nonce, ks );
			++counter;
			const DWORD n = ( size - i ) < 64 ? ( size - i ) : 64;
			for ( DWORD j = 0; j < n; ++j ) {
				data[i + j] ^= static_cast<char>( ks[j] );
			}
			i += n;
		}
	}
}

//---------------------------.
// 対応したファイルIDを取得.
//---------------------------.
std::string SecretKey::GetFileId( const std::string& Ext )
{
	std::string e = Ext;
	if ( e.substr( 0, 1 ) == "." ) e.erase( 0, 1 );

	for ( auto [List, id] : FILE_LIST ) {
		auto [e1, e2] = List;
		if ( e == e1 || e == e2 ) {
			return id;
		}
	}
	return ErrorId;
}

//---------------------------.
// 対応したパスの取得.
//---------------------------.
std::string SecretKey::GetFileExt( const std::string& FId )
{
	for ( auto [List, id] : FILE_LIST ) {
		if ( FId == id ) {
			return "." + List.first;
		}
	}
	return ErrorId;
}

//---------------------------.
// ファイルパスから暗号化シードを生成する.
//---------------------------.
//	フルパス( ディレクトリ込み )をハッシュに使うことで、同じファイル名が
//	別フォルダに存在してもシードが変わるようにする( 衝突耐性の強化 ).
//	暗号化側( encrypt.exe, ビルド時は "RData\..." )と復号側( ゲーム, 実行時は
//	リネーム後の "Data\..." )でフォルダ名だけが異なるため、"RData\" は "Data\"
//	に正規化してから使う( 両者で同一シードになるようにする ).
std::uint32_t SecretKey::MakeSeed( const std::string& FilePath )
{
	// パス区切りを統一する( '/' → '\' ).
	std::string path = FilePath;
	for ( auto& c : path ) {
		if ( c == '/' ) c = '\\';
	}

	// "RData\" は "Data\" に正規化する( ビルド時と実行時のフォルダ名差異を吸収 ).
	const std::size_t rpos = path.find( "RData\\" );
	if ( rpos != std::string::npos ) {
		path = path.substr( 0, rpos ) + "Data\\" + path.substr( rpos + 6 );
	}

	// FNV-1a 32bit でハッシュ化( フルパスが対象 ).
	std::uint32_t hash = 2166136261u;
	for ( char c : path ) {
		hash ^= static_cast<unsigned char>( c );
		hash *= 16777619u;
	}
	return hash;
}

//---------------------------.
// 暗号化.
//---------------------------.
void SecretKey::Encryption( char* data, const DWORD& size, std::uint32_t seed )
{
	XorStream( data, size, seed );
}

//---------------------------.
// 暗号化を元に戻す
//---------------------------.
void SecretKey::Restore( char* data, const DWORD& size, std::uint32_t seed )
{
	XorStream( data, size, seed );
}
