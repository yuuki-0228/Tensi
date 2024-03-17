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

	// �t�@�C������Z�[�u�f�[�^�̏����擾����
	std::vector<std::pair<std::string, std::string>> GetSaveData( const std::vector<std::string>& File ) {
		std::vector<std::pair<std::string, std::string>> saveData;
		saveData.reserve( File.size() );
		for ( auto& f : File ) {
			// �^�̎擾
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

			// ���O�̎擾
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
// ���C��
//-----------------.
int SaveCreate::main()
{
	fprintf( stderr, "1>CommonSaveData.h\n" );

	auto file = FileManager::TextLoad( STRUCT_DATA_PATH );
	if ( file.size() == 0 ) {
		return ErrorMessage( "1>Error : �t�@�C���̓ǂݍ��ݎ��s", "SaveData.h" );
	}

	h( file );
	fprintf( stderr, "1>CommonSaveData.cpp\n" );
	cpp( file );
	return S_OK;
}

//-----------------.
// .h�t�@�C���̍쐬
//-----------------.
void SaveCreate::h( const std::vector<std::string>& File ) {
	std::string Text = "";

	// �Z�[�u�f�[�^�̏����擾
	auto saveData = GetSaveData( File );

	Text +=
		"#pragma once\n"
		"#include \"..\\SaveData.h\"\n"
		"\n"
		"/************************************************\n"
		"*	���ʂ̃Z�[�u�f�[�^�N���X\n"
		"*	�ucreator�v�ɂ���Ď����ō쐬����Ă��܂�\n"
		"*/\n"
		"class CCommonSaveData final\n"
		"{\n"
		"public:\n"
		"	CCommonSaveData();\n"
		"	~CCommonSaveData();\n"
		"\n"
		"	// �ۑ�\n"
		"	void Save();\n"
		"	// �ǂݍ���\n"
		"	void Load();\n"
		"\n"
		"	// �Z�[�u�f�[�^�̃A�h���X���X�g�̎擾\n"
		"	SaveData::SSetSaveDataList* GetPointerSaveData() { return &m_pSaveData; }\n"
		"\n"
		"private:\n"
		"	// �Z�[�u�f�[�^���X�g\n"
		"	struct stSaveDataList {\n";

	for ( auto [mold, name] : saveData ) {
		Text += "		" + mold + " " + name + ";\n";
	}

	Text +=
		"	} typedef SSaveDataList;\n"
		"\n"
		"private:\n"
		"	SaveData::SSetSaveDataList	m_pSaveData; // �Z�[�u�f�[�^�̃A�h���X���X�g\n"
		"	SSaveDataList				m_SaveData;	 // �Z�[�u�f�[�^\n"
		"};\n";
	
	// �t�@�C���̍쐬.
	FileManager::TextSave( H_PATH, Text );
}

//-----------------.
// .cpp�t�@�C���̍쐬
//-----------------.
void SaveCreate::cpp( const std::vector<std::string>& File ) {
	std::string Text = "";

	// �Z�[�u�f�[�^�̏����擾
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
		"// �ۑ�.\n"
		"//---------------------------.\n"
		"void CCommonSaveData::Save()\n"
		"{\n"
		"	// �Z�[�u�f�[�^�̍X�V\n";

	for ( auto [mold, name] : saveData ) {
		Text += "	m_SaveData." + name + " = *m_pSaveData." + name + ";\n";
	}

	Text +=
		"\n"
		"	// �|�C���^�͕ۑ�����ƃo�O�邽��null�ɂ���\n";

	for ( auto [mold, name] : saveData ) {
		if ( mold == "STransform" ) {
			Text += "	m_SaveData." + name + ".pParent = nullptr;\n";
		}
	}

	Text +=
		"\n"
		"	// �o�C�i���f�[�^�̕ۑ�\n"
		"	FileManager::BinarySave( FILE_PATH, m_SaveData );\n"
		"}\n"
		"\n"
		"//---------------------------.\n"
		"// �ǂݍ���.\n"
		"//---------------------------.\n"
		"void CCommonSaveData::Load()\n"
		"{\n"
		"	// �o�C�i���f�[�^�̓ǂݍ���\n"
		"	FileManager::BinaryLoad( FILE_PATH, m_SaveData );\n"
		"\n"
		"	// �ǂݍ��񂾃f�[�^�ŏ�����\n";

	for ( auto [mold, name] : saveData ) {
		Text += "	*m_pSaveData." + name + " = m_SaveData." + name + ";\n";
	}

	Text += "}\n";

	// �t�@�C���̍쐬.
	FileManager::TextSave( CPP_PATH, Text );
}

//-----------------.
// ���U���g�̕\��
//-----------------.
void SaveCreate::result()
{
	auto h   = "1>Creator -> " + std::filesystem::absolute( H_PATH	 ).string() + "\n";
	auto cpp = "1>Creator -> " + std::filesystem::absolute( CPP_PATH ).string() + "\n";
	fprintf( stderr, h.c_str()		);
	fprintf( stderr, cpp.c_str()	);
}