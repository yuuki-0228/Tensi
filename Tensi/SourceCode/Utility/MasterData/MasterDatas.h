#pragma once
#include "..\..\Global.h"
#include "..\FileManager\FileManager.h"
#include <unordered_map>
#include <any>

/************************************************
*	マスターデータ一覧
*	「creator」によって自動で作成されています
**/
namespace {
}

namespace MasterData {
	std::unordered_map<std::string, std::unordered_map<unsigned int, std::any>> CreateCache( const std::vector<std::pair<std::string, json>>& data );
}
