#pragma once
#include "file.h"
#include <vector>

namespace MDataCreate {
	// ���C��
	int main();

	// .h�t�@�C���̍쐬
	void h( const std::vector<std::pair<std::string, json>>& Container );

	// .cpp�t�@�C���̍쐬
	void cpp( const std::vector<std::pair<std::pair<std::string, std::string>, json>>& List );

	// ���U���g
	void result();
}