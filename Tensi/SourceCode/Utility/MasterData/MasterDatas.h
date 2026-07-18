#pragma once
#include "..\..\SystemSetting.h"
#ifdef ENABLE_MASTER_DATA
#include "..\..\Global.h"
#include "..\FileManager\FileManager.h"
#include <unordered_map>
#include <any>

/************************************************
*	マスターデータ一覧
*	「creator」によって自動で作成されています
**/
namespace MasterData {
	using ulong = unsigned long;
}
using namespace MasterData;

namespace MasterDataUtility {
	std::unordered_map<std::string, std::unordered_map<ulong, std::any>> CreateCache( const std::vector<std::pair<std::string, Json>>& data );
	void CacheSetup( std::unordered_map<std::string, std::unordered_map<ulong, std::any>>& out, const std::string& Container, const Json& File );
}
#endif
