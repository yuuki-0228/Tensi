#pragma once
#include "file.h"
#include <vector>

namespace ConstCreate{
	// メイン
	int main();

	// .hファイルの作成
	void h( const std::vector<std::pair<std::pair<std::string, std::string>, json>>& List );

	// .cppファイルの作成
	void cpp( const std::vector<std::pair<std::pair<std::string, std::string>, json>>& List );

	// リザルト
	void result();
}