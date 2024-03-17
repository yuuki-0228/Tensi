#pragma once
#include <Windows.h>
#include <vector>
#include <string>

namespace SecretKey{
	// 暗号化時に使用する秘密鍵(8bit)
	//	1Byteごとに秘密鍵を使用して暗号化する
	//	複数鍵を指定している場合順番に使用していく
	const std::vector<char> SECRET_KEY = {
		(char) 0b10010101,
		(char) 0b01011101,
		(char) 0b11010110,
		(char) 0b00100101,
	};
	
	// 登録されている秘密鍵の数.
	const int KEY_NUM = static_cast<int>( SECRET_KEY.size() );

	// 暗号化対応ファイル.
	//	<<拡張子1, 拡張子2>, ファイルID(00~99)>
	const std::vector<std::pair<std::pair<std::string, std::string>, std::string>> FILE_LIST = {
		std::make_pair( std::make_pair( "png",	"PNG"	), "00" ),
		std::make_pair( std::make_pair( "bmp",	"BMP"	), "01" ),
		std::make_pair( std::make_pair( "jpg",	"JPG"	), "02" ),
	//	std::make_pair( std::make_pair( "midi",	"MIDI"	), "03" ),	// mciSendString：非対応
	//	std::make_pair( std::make_pair( "mp3",	"MP3"	), "04" ),	// mciSendString：非対応
	//	std::make_pair( std::make_pair( "wav",	"WAV"	), "05" ),	// mciSendString：非対応
	//	std::make_pair( std::make_pair( "ogg",	"OGG"	), "06" ),	// 非対応
		std::make_pair( std::make_pair( "x",	"X"		), "07" ),
	//	std::make_pair( std::make_pair( "obj",	"OBJ"	), "08" ),	// 非対応
	//	std::make_pair( std::make_pair( "fbx",	"FBX"	), "09" ),	// 非対応
	//	std::make_pair( std::make_pair( "mmd",	"MMD"	), "10" ),	// 非対応
		std::make_pair( std::make_pair( "efk",	"EFK"	), "11" ),
	//	std::make_pair( std::make_pair( "ttf",	"TTF"	), "12" ),	// 読み込み：非対応
	//	std::make_pair( std::make_pair( "otf",	"OTF"	), "13" ),	// 読み込み：非対応
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

	// 暗号化.
	void Encryption( char* data, const DWORD& size );

	// 暗号化を復元
	void Restore( char* data, const DWORD& size );
}