#pragma once
#include "file.h"
#include <vector>

namespace SaveCreate {
	// ���C��
	int main();

	// .h�t�@�C���̍쐬
	void h( const std::vector<std::string>& File );

	// .cpp�t�@�C���̍쐬
	void cpp( const std::vector<std::string>& File );

	// ���U���g
	void result();
}