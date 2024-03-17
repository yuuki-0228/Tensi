#pragma once
#include "..\..\Global.h"
#include "..\FileManager\FileManager.h"
#include <unordered_map>
#include <any>

/************************************************
*	�}�X�^�[�f�[�^�ꗗ
*	�ucreator�v�ɂ���Ď����ō쐬����Ă��܂�
**/
namespace {
}

namespace MasterData {
	std::unordered_map<std::string, std::unordered_map<unsigned int, std::any>> CreateCache( const std::vector<std::pair<std::string, json>>& data );
}
