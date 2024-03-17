#pragma once
#include <nlohmnn\json.hpp>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <iostream>
#include <memory>
#include <string>

using json = nlohmann::json;

namespace FileManager {
	// テキストファイルの読み込み.
	std::vector<std::string> TextLoad( const std::string& FilePath );

	// テキストファイルの書き込む.
	bool TextSave( const std::string& FilePath, const std::string& Data );

	// jsonファイルの読み込み
	json JsonLoad( const std::string& FilePath );

	// 足りないファイルディレクトリを作成する.
	void CreateFileDirectory( const std::string& FilePath );
}