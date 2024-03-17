#include "MDataCreate.h"
#include "string.h"
#include "error.h"

namespace {
	#ifdef _DEBUG
	const std::string CONTAINER_PATH	= "..\\..\\..\\Data\\Parameter\\MasterData\\Container";
	const std::string DATA_PATH			= "..\\..\\..\\Data\\Parameter\\MasterData\\";
	const std::string H_PATH			= "..\\..\\..\\SourceCode\\Utility\\MasterData\\dMasterDatas.h";
	const std::string CPP_PATH			= "..\\..\\..\\SourceCode\\Utility\\MasterData\\dMasterDatas.cpp";
#else
	const std::string CONTAINER_PATH	= "Data\\Parameter\\MasterData\\Container";
	const std::string DATA_PATH			= "Data\\Parameter\\MasterData\\";
	const std::string H_PATH			= "SourceCode\\Utility\\MasterData\\MasterDatas.h";
	const std::string CPP_PATH			= "SourceCode\\Utility\\MasterData\\MasterDatas.cpp";
#endif

	// Json�t�@�C���̃f�[�^�ʒu�w��萔
	constexpr int COMMENT	= 0; // �R�����g�̈ʒu.
	constexpr int DATA		= 1; // �f�[�^�̈ʒu.

	// �����������Ȃ��悤��std::string�ɕϊ����Ď擾
	std::string GetString( const std::string& string ) {
		// json��UTF8�Ȃ��߈�x�������ϊ�����.
		std::wstring wString = StringConversion::to_wString( string, ECodePage::UTF8 );
		return StringConversion::to_String( wString );
	}

	// �^�̎擾
	std::string GetMold( const json& j, const bool isPair = false ) {
		if ( j.is_string()			) return "std::string";
		if ( j.is_number_integer()	) return "int";
		if ( j.is_number_float()	) return "float";
		if ( j.is_boolean()			) return "bool";
		ErrorMessage( "1>Error : ��Ή��̌^�ł�", "json" );
		return "";
	}

	std::vector<std::pair<std::pair<std::string, std::string>, json>> List;
}

//-----------------.
// ���C��
//-----------------.
int MDataCreate::main()
{
	fprintf( stderr, "1>MasterDatas.h\n" );

	std::vector<std::pair<std::string, json>> ContainerList;
	auto ContainerLoad = [&]( const std::filesystem::directory_entry& Entry )
	{
		const std::string Extension = Entry.path().extension().string();	// �g���q.
		const std::string FilePath = Entry.path().string();					// �t�@�C���p�X.
		const std::string FileName = Entry.path().stem().string();			// �t�@�C����.

		// json�t�@�C����.
		if ( Extension != ".json" && Extension != ".JSON" ) return;

		// �t�@�C���̒ǉ�
		ContainerList.emplace_back( std::make_pair( FileName, FileManager::JsonLoad( FilePath ) ) );
	};

	try {
		std::filesystem::recursive_directory_iterator Dir_itr( CONTAINER_PATH ), End_itr;
		std::for_each( Dir_itr, End_itr, ContainerLoad );
	} catch ( const std::filesystem::filesystem_error& e ) {
		// �G���[���b�Z�[�W��\��.
		return ErrorMessage( "1>Error : �t�@�C���̓ǂݍ��� ���s", e.path1().string().c_str() );
	}

	h( ContainerList );
	fprintf( stderr, "1>MasterDatas.cpp\n" );
	cpp( List );

	return S_OK;
}

//-----------------.
// .h�t�@�C���̍쐬
//-----------------.
void MDataCreate::h( const std::vector<std::pair<std::string, json>>& Container )
{
	std::string Text = "";

	Text +=
		"#pragma once\n"
		"#include \"..\\..\\Global.h\"\n"
		"#include \"..\\FileManager\\FileManager.h\"\n"
		"#include <unordered_map>\n"
		"#include <any>\n"
		"\n"
		"/************************************************\n"
		"*	�}�X�^�[�f�[�^�ꗗ\n"
		"*	�ucreator�v�ɂ���Ď����ō쐬����Ă��܂�\n"
		"**/\n"
		"namespace {\n";

	for ( auto& [Name, File] : Container ) {
		std::string structName = Name;
		if ( structName[0] == 'M' ) structName.replace( 0, 1, "Master" );
		structName += "Data";

		List.emplace_back( std::make_pair( std::make_pair( Name, structName ), File ) );

		Text +=
			"	struct " + structName + "\n"
			"	{\n"
			"		// ID\n"
			"		unsigned int Id;\n";

		for ( auto& [Key, Value] : File.items() ) {
			if ( Key == "ID" || Key == "Id" || Key == "id"	) continue;
			if ( Key.find( "_enum" ) != std::string::npos	) continue;

			Text += "		// " + GetString( Value[COMMENT] ) + "\n";

			auto Enum = File.find( Key + "_enum" );
			if ( Enum == File.end() ) {
				Text += "		" + GetMold( Value[DATA] ) + " " + Key + ";\n";
			}
			else {
				Text += "		enum class " + Key + " {\n";

				for ( auto& l : Enum.value() ) {
					Text += "			" + std::string( l ) + ",\n";
				}

				Text += 
					"		};\n"
					"		" + Key + " " + Key + ";\n";
			}
		}

		Text += 
			"\n"
			"		" + structName + "( const json& j );\n"
			"	};\n"
			"\n";

		// �f�B���N�g�����쐬���Ă���
		FileManager::CreateFileDirectory( DATA_PATH + Name + "\\" );
	}

	Text +=
		"}\n"
		"\n"
		"namespace MasterData {\n"
		"	std::unordered_map<std::string, std::unordered_map<unsigned int, std::any>> CreateCache( const std::vector<std::pair<std::string, json>>& data );\n"
		"}\n";

	// �t�@�C���̍쐬.
	FileManager::TextSave( H_PATH, Text );
}

//-----------------.
// .cpp�t�@�C���̍쐬
//-----------------.
void MDataCreate::cpp( const std::vector<std::pair<std::pair<std::string, std::string>, json>>& List )
{
	std::string Text = "";

	Text +=
		"#include \"MasterDatas.h\"\n"
		"\n";

	for ( auto& [Data, File] : List ) {
		auto& [Name, Struct] = Data;
		Text +=
			Struct + "::" + Struct + "( const json& j )\n"
			"{\n";

		for ( auto& [Key, Value] : File.items() ) {
			if ( Key.find( "_enum" ) != std::string::npos ) continue;

			Text += "	" + Key + " = j[\"" + Key + "\"];\n";
		}

		Text +=
			"}\n"
			"\n";
	}

	Text +=
		"//----------------------------.\n"
		"// �ǂݍ���\n"
		"//----------------------------.\n"
		"std::unordered_map<std::string, std::unordered_map<unsigned int, std::any>> MasterData::CreateCache(\n"
		"	const std::vector<std::pair<std::string, json>>& data )\n"
		"{\n"
		"	std::unordered_map<std::string, std::unordered_map<unsigned int, std::any>> out;\n"
		"\n"
		"	for ( auto& [Container, File] : data )\n"
		"	{\n"
		"		for ( auto& f : File ) {\n";

	bool isFirst = true;
	for ( auto& [Data, File] : List ) {
		auto& [Name, Struct] = Data;

		if ( isFirst ) {
			Text += "			if (	  Container == \"" + Name + "\" ) out[\"" + Struct + "\"][f[\"Id\"]] = " + Struct + "( f );\n";
			isFirst = false;
		}
		else {
			Text += "			else if ( Container == \"" + Name + "\" ) out[\"" + Struct + "\"][f[\"Id\"]] = " + Struct + "( f );\n";
		}
	}

	Text +=
		"		}\n"
		"	}\n"
		"\n"
		"	return out;\n"
		"}\n";

	// �t�@�C���̍쐬.
	FileManager::TextSave( CPP_PATH, Text );
}

//-----------------.
// ���U���g�̕\��
//-----------------.
void MDataCreate::result()
{
	auto h   = "1>Creator -> " + std::filesystem::absolute( H_PATH	 ).string() + "\n";
	auto cpp = "1>Creator -> " + std::filesystem::absolute( CPP_PATH ).string() + "\n";
	fprintf( stderr, h.c_str()		);
	fprintf( stderr, cpp.c_str()	);
}