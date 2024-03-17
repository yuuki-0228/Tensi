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
	// �e�L�X�g�t�@�C���̓ǂݍ���.
	std::vector<std::string> TextLoad( const std::string& FilePath );

	// �e�L�X�g�t�@�C���̏�������.
	bool TextSave( const std::string& FilePath, const std::string& Data );

	// json�t�@�C���̓ǂݍ���
	json JsonLoad( const std::string& FilePath );

	// ����Ȃ��t�@�C���f�B���N�g�����쐬����.
	void CreateFileDirectory( const std::string& FilePath );
}