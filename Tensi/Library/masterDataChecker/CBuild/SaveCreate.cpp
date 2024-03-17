#include "SaveCreate.h"
#include "string.h"
#include "error.h"
#include "file.h"

namespace {
#ifdef _DEBUG
	const std::string STRUCT_DATA_PATH	= "..\\..\\..\\SourceCode\\Utility\\SaveDataManager\\SaveData.h";
	const std::string H_PATH			= "..\\..\\..\\SourceCode\\Utility\\SaveDataManager\\CommonSaveData\\dCommonSaveData.h";
	const std::string CPP_PATH			= "..\\..\\..\\SourceCode\\Utility\\SaveDataManager\\CommonSaveData\\dCommonSaveData.cpp";
#else
	const std::string STRUCT_DATA_PATH	= "SourceCode\\Utility\\SaveDataManager\\SaveData.h";
	const std::string H_PATH			= "SourceCode\\Utility\\SaveDataManager\\CommonSaveData\\CommonSaveData.h";
	const std::string CPP_PATH			= "SourceCode\\Utility\\SaveDataManager\\CommonSaveData\\CommonSaveData.cpp";
#endif

	// ファイルからセーブデータの情報を取得する
	std::vector<std::pair<std::string, std::string>> GetSaveData( const std::vector<std::string>& File ) {
		std::vector<std::pair<std::string, std::string>> saveData;
		saveData.reserve( File.size() );
		for ( auto& f : File ) {
			// 型の取得
			std::string mold = "";
			std::string from = "		";
			std::string to = "* ";
			std::string::size_type spos = f.find( from );
			if ( spos != std::string::npos ) {
				std::string::size_type epos = f.find( to, spos + from.length() );
				if ( epos != std::string::npos ) {
					mold = f.substr( spos + from.length(), epos - spos - from.length() );
				}
			}

			// 名前の取得
			std::string name = "";
			from = "* ";
			to = ";";
			spos = f.find( from );
			if ( spos != std::string::npos ) {
				std::string::size_type epos = f.find( to, spos + from.length() );
				if ( epos != std::string::npos ) {
					name = f.substr( spos + from.length(), epos - spos - from.length() );
				}
			}

			if ( mold != "" && name != "" ) {
				saveData.emplace_back( std::make_pair( mold, name ) );
			}
		}
		return saveData;
	}
}

//-----------------.
// メイン
//-----------------.
int SaveCreate::main()
{
	fprintf( stderr, "1>CommonSaveData.h\n" );

	auto file = FileManager::TextLoad( STRUCT_DATA_PATH );
	if ( file.size() == 0 ) {
		return ErrorMessage( "1>Error : ファイルの読み込み失敗", "SaveData.h" );
	}

	h( file );
	fprintf( stderr, "1>CommonSaveData.cpp\n" );
	cpp( file );
	return S_OK;
}

//-----------------.
// .hファイルの作成
//-----------------.
void SaveCreate::h( const std::vector<std::string>& File ) {
	std::string Text = "";

	// セーブデータの情報を取得
	auto saveData = GetSaveData( File );

	Text +=
		"#pragma once\n"
		"#include \"..\\SaveData.h\"\n"
		"\n"
		"/************************************************\n"
		"*	共通のセーブデータクラス\n"
		"*	「creator」によって自動で作成されています\n"
		"*/\n"
		"class CCommonSaveData final\n"
		"{\n"
		"public:\n"
		"	CCommonSaveData();\n"
		"	~CCommonSaveData();\n"
		"\n"
		"	// 保存\n"
		"	void Save();\n"
		"	// 読み込み\n"
		"	void Load();\n"
		"\n"
		"	// セーブデータのアドレスリストの取得\n"
		"	SaveData::SSetSaveDataList* GetPointerSaveData() { return &m_pSaveData; }\n"
		"\n"
		"private:\n"
		"	// セーブデータリスト\n"
		"	struct stSaveDataList {\n";

	for ( auto [mold, name] : saveData ) {
		Text += "		" + mold + " " + name + ";\n";
	}

	Text +=
		"	} typedef SSaveDataList;\n"
		"\n"
		"private:\n"
		"	SaveData::SSetSaveDataList	m_pSaveData; // セーブデータのアドレスリスト\n"
		"	SSaveDataList				m_SaveData;	 // セーブデータ\n"
		"};\n";
	
	// ファイルの作成.
	FileManager::TextSave( H_PATH, Text );
}

//-----------------.
// .cppファイルの作成
//-----------------.
void SaveCreate::cpp( const std::vector<std::string>& File ) {
	std::string Text = "";

	// セーブデータの情報を取得
	auto saveData = GetSaveData( File );

	Text +=
		"#include \"CommonSaveData.h\"\n"
		"#include \"..\\..\\FileManager\\FileManager.h\"\n"
		"\n"
		"namespace {\n"
		"	constexpr char FILE_PATH[] = \"Data\\\\Parameter\\\\d\\\\m.bin\";\n"
		"}\n"
		"\n"
		"CCommonSaveData::CCommonSaveData()\n"
		"	: m_pSaveData	()\n"
		"	, m_SaveData	()\n"
		"{\n"
		"}\n"
		"\n"
		"CCommonSaveData::~CCommonSaveData()\n"
		"{\n"
		"}\n"
		"\n"
		"//---------------------------.\n"
		"// 保存.\n"
		"//---------------------------.\n"
		"void CCommonSaveData::Save()\n"
		"{\n"
		"	// セーブデータの更新\n";

	for ( auto [mold, name] : saveData ) {
		Text += "	m_SaveData." + name + " = *m_pSaveData." + name + ";\n";
	}

	Text +=
		"\n"
		"	// ポインタは保存するとバグるためnullにする\n";

	for ( auto [mold, name] : saveData ) {
		if ( mold == "STransform" ) {
			Text += "	m_SaveData." + name + ".pParent = nullptr;\n";
		}
	}

	Text +=
		"\n"
		"	// バイナリデータの保存\n"
		"	FileManager::BinarySave( FILE_PATH, m_SaveData );\n"
		"}\n"
		"\n"
		"//---------------------------.\n"
		"// 読み込み.\n"
		"//---------------------------.\n"
		"void CCommonSaveData::Load()\n"
		"{\n"
		"	// バイナリデータの読み込み\n"
		"	FileManager::BinaryLoad( FILE_PATH, m_SaveData );\n"
		"\n"
		"	// 読み込んだデータで初期化\n";

	for ( auto [mold, name] : saveData ) {
		Text += "	*m_pSaveData." + name + " = m_SaveData." + name + ";\n";
	}

	Text += "}\n";

	// ファイルの作成.
	FileManager::TextSave( CPP_PATH, Text );
}

//-----------------.
// リザルトの表示
//-----------------.
void SaveCreate::result()
{
	auto h   = "1>Creator -> " + std::filesystem::absolute( H_PATH	 ).string() + "\n";
	auto cpp = "1>Creator -> " + std::filesystem::absolute( CPP_PATH ).string() + "\n";
	fprintf( stderr, h.c_str()		);
	fprintf( stderr, cpp.c_str()	);
}