#include "MasterDatas.h"

//----------------------------.
// �ǂݍ���
//----------------------------.
std::unordered_map<std::string, std::unordered_map<unsigned int, std::any>> MasterData::CreateCache(
	const std::vector<std::pair<std::string, json>>& data )
{
	std::unordered_map<std::string, std::unordered_map<unsigned int, std::any>> out;

	for ( auto& [Container, File] : data )
	{
		for ( auto& f : File ) {
		}
	}

	return out;
}
