#pragma once
#include <Windows.h>
#include <vector>
#include <string>
#include <cstdint>

namespace SecretKey{
	// 暗号化対応ファイル.
	//	<<拡張子1, 拡張子2>, ファイルID(00~99)>
	const std::vector<std::pair<std::pair<std::string, std::string>, std::string>> FILE_LIST = {
		std::make_pair( std::make_pair( "png",	"PNG"	), "00" ),
		std::make_pair( std::make_pair( "bmp",	"BMP"	), "01" ),
		std::make_pair( std::make_pair( "jpg",	"JPG"	), "02" ),
	//	std::make_pair( std::make_pair( "midi",	"MIDI"	), "03" ),	// mciSendString：非対応
	//	std::make_pair( std::make_pair( "mp3",	"MP3"	), "04" ),	// mciSendString：非対応
	//	std::make_pair( std::make_pair( "wav",	"WAV"	), "05" ),	// mciSendString：非対応
		std::make_pair( std::make_pair( "ogg",	"OGG"	), "06" ),
		std::make_pair( std::make_pair( "x",	"X"		), "07" ),
	//	std::make_pair( std::make_pair( "obj",	"OBJ"	), "08" ),	// 非対応
	//	std::make_pair( std::make_pair( "fbx",	"FBX"	), "09" ),	// 非対応
	//	std::make_pair( std::make_pair( "mmd",	"MMD"	), "10" ),	// 非対応
		std::make_pair( std::make_pair( "efk",	"EFK"	), "11" ),
		std::make_pair( std::make_pair( "ttf",	"TTF"	), "12" ),
		std::make_pair( std::make_pair( "otf",	"OTF"	), "13" ),
		std::make_pair( std::make_pair( "txt",	"TXT"	), "14" ),
		std::make_pair( std::make_pair( "csv",	"CSV"	), "15" ),
		std::make_pair( std::make_pair( "json",	"JSON"	), "16" ),
	//	std::make_pair( std::make_pair( "ini",	"INI"	), "17" ),	// imguiと衝突するため暗号化しない
	//	std::make_pair( std::make_pair( "toml",	"TOML"	), "18" ),	// 非対応
	//	std::make_pair( std::make_pair( "bin",	"BIN"	), "19" ),	// 出力で使用するため暗号化しない
		std::make_pair( std::make_pair( "hlsl",	"HLSL"	), "20" ),
		std::make_pair( std::make_pair( "hlsli","HLSLI"	), "21" ),
		std::make_pair( std::make_pair( "pptx",	"PPTX"	), "22" ),
		std::make_pair( std::make_pair( "docx",	"DOCX"	), "23" ),
		std::make_pair( std::make_pair( "xlsx",	"XLSX"	), "24" ),
		std::make_pair( std::make_pair( "pdf",	"PDF"	), "25" ),
	};

	// 対応したファイルIDを取得.
	std::string GetFileId( const std::string& Ext );
	const std::string ErrorId = "-1";

	// 対応したパスの取得.
	std::string GetFileExt( const std::string& FId );

	// ファイルパスから暗号化シードを生成する.
	//	暗号化ファイル名のベース名からシードを作るため、暗号化側と復号側で一致する.
	std::uint32_t MakeSeed( const std::string& FilePath );

	// 暗号化.
	//	seed をファイルごとに変えることで、ファイル単位で鍵ストリームが変わり、
	//	既知平文攻撃( 同じ位置の平文が既知でも他ファイルへ流用できない )に耐性を持つ.
	void Encryption( char* data, const DWORD& size, std::uint32_t seed = 0 );

	// 暗号化を復元
	void Restore( char* data, const DWORD& size, std::uint32_t seed = 0 );
}
