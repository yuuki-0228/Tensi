#include "MasterDatas.h"
#ifdef ENABLE_MASTER_DATA

namespace {
	constexpr int _X		= 0; // Xの位置
	constexpr int _Y		= 1; // Yの位置
	constexpr int _Z		= 2; // Zの位置
	constexpr int _W		= 3; // Wの位置
	constexpr int _FIRST	= 0; // 最初の位置
	constexpr int _SECOND	= 1; // 次の位置

	// 配列のサイズを取得
	int GetSize( const json& j ) {
		return static_cast<int>( j.size() );
	}

	// 文字化けしないようにstd::stringに変換して取得
	std::string GetString( const std::string& string ) {
		// jsonはUTF8なため一度文字列を変換する
		std::wstring wString = StringConversion::to_wString( string, ECodePage::UTF8 );
		return StringConversion::to_String( wString );
	}
}

//----------------------------.
// 読み込み
//----------------------------.
std::unordered_map<std::string, std::unordered_map<ulong, std::any>> MasterDataUtility::CreateCache(
	const std::vector<std::pair<std::string, Json>>& data )
{
	std::unordered_map<std::string, std::unordered_map<ulong, std::any>> out;

	for ( auto& [Container, File] : data )
	{
		if ( File.is_array() ) {
			for ( auto& Data : File ) {
				CacheSetup( out, Container, Data );
			}
		}
		else {
			CacheSetup( out, Container, File );
		}
	}
	return out;
}
void MasterDataUtility::CacheSetup( std::unordered_map<std::string, std::unordered_map<ulong, std::any>>& out, const std::string& Container, const Json& File )
{
	const ulong Id = File["Id"].Get<ulong>();
}
#endif