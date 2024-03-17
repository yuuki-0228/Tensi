#pragma once
#include "file.h"
#include <vector>

namespace SaveCreate {
	// メイン
	int main();

	// .hファイルの作成
	void h( const std::vector<std::string>& File );

	// .cppファイルの作成
	void cpp( const std::vector<std::string>& File );

	// リザルト
	void result();
}