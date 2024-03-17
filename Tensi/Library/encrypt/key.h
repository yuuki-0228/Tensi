#pragma once
#include <Windows.h>
#include <vector>
#include <string>

namespace SecretKey{
	// �Í������Ɏg�p����閧��(8bit)
	//	1Byte���Ƃɔ閧�����g�p���ĈÍ�������
	//	���������w�肵�Ă���ꍇ���ԂɎg�p���Ă���
	const std::vector<char> SECRET_KEY = {
		(char) 0b10010101,
		(char) 0b01011101,
		(char) 0b11010110,
		(char) 0b00100101,
	};
	
	// �o�^����Ă���閧���̐�.
	const int KEY_NUM = static_cast<int>( SECRET_KEY.size() );

	// �Í����Ή��t�@�C��.
	//	<<�g���q1, �g���q2>, �t�@�C��ID(00~99)>
	const std::vector<std::pair<std::pair<std::string, std::string>, std::string>> FILE_LIST = {
		std::make_pair( std::make_pair( "png",	"PNG"	), "00" ),
		std::make_pair( std::make_pair( "bmp",	"BMP"	), "01" ),
		std::make_pair( std::make_pair( "jpg",	"JPG"	), "02" ),
	//	std::make_pair( std::make_pair( "midi",	"MIDI"	), "03" ),	// mciSendString�F��Ή�
	//	std::make_pair( std::make_pair( "mp3",	"MP3"	), "04" ),	// mciSendString�F��Ή�
	//	std::make_pair( std::make_pair( "wav",	"WAV"	), "05" ),	// mciSendString�F��Ή�
	//	std::make_pair( std::make_pair( "ogg",	"OGG"	), "06" ),	// ��Ή�
		std::make_pair( std::make_pair( "x",	"X"		), "07" ),
	//	std::make_pair( std::make_pair( "obj",	"OBJ"	), "08" ),	// ��Ή�
	//	std::make_pair( std::make_pair( "fbx",	"FBX"	), "09" ),	// ��Ή�
	//	std::make_pair( std::make_pair( "mmd",	"MMD"	), "10" ),	// ��Ή�
		std::make_pair( std::make_pair( "efk",	"EFK"	), "11" ),
	//	std::make_pair( std::make_pair( "ttf",	"TTF"	), "12" ),	// �ǂݍ��݁F��Ή�
	//	std::make_pair( std::make_pair( "otf",	"OTF"	), "13" ),	// �ǂݍ��݁F��Ή�
		std::make_pair( std::make_pair( "txt",	"TXT"	), "14" ),
		std::make_pair( std::make_pair( "csv",	"CSV"	), "15" ),
		std::make_pair( std::make_pair( "json",	"JSON"	), "16" ),
	//	std::make_pair( std::make_pair( "ini",	"INI"	), "17" ),	// imgui�ƏՓ˂��邽�߈Í������Ȃ�
	//	std::make_pair( std::make_pair( "toml",	"TOML"	), "18" ),	// ��Ή�
	//	std::make_pair( std::make_pair( "bin",	"BIN"	), "19" ),	// �o�͂Ŏg�p���邽�߈Í������Ȃ�
		std::make_pair( std::make_pair( "hlsl",	"HLSL"	), "20" ),
		std::make_pair( std::make_pair( "hlsli","HLSLI"	), "21" ),
		std::make_pair( std::make_pair( "pptx",	"PPTX"	), "22" ),
		std::make_pair( std::make_pair( "docx",	"DOCX"	), "23" ),
		std::make_pair( std::make_pair( "xlsx",	"XLSX"	), "24" ),
		std::make_pair( std::make_pair( "pdf",	"PDF"	), "25" ),
	};

	// �Ή������t�@�C��ID���擾.
	std::string GetFileId( const std::string& Ext );
	const std::string ErrorId = "-1";

	// �Ή������p�X�̎擾.
	std::string GetFileExt( const std::string& FId );

	// �Í���.
	void Encryption( char* data, const DWORD& size );

	// �Í����𕜌�
	void Restore( char* data, const DWORD& size );
}