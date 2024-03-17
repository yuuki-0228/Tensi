#include "ConstCreate.h"
#include "string.h"
#include "error.h"

namespace {
#ifdef _DEBUG
	const std::string CONST_DATA_PATH	= "..\\..\\..\\Data\\Parameter\\Const";
	const std::string H_PATH			= "..\\..\\..\\SourceCode\\Utility\\Const\\dConst.h";
	const std::string CPP_PATH			= "..\\..\\..\\SourceCode\\Utility\\Const\\dConst.cpp";
#else
	const std::string CONST_DATA_PATH	= "Data\\Parameter\\Const";
	const std::string H_PATH			= "SourceCode\\Utility\\Const\\Const.h";
	const std::string CPP_PATH			= "SourceCode\\Utility\\Const\\Const.cpp";
#endif

	// �f�o�b�NJson�t�@�C���̃f�[�^�ʒu�w��萔
	constexpr int COMMENT	= 0; // �R�����g�̈ʒu.
	constexpr int DATA		= 1; // �f�[�^�̈ʒu.

	// �z��̃T�C�Y���擾
	int GetSize( const json& j ) {
		// ��ԍŏ��ɃR�����g�����邽�ߗv�f����1���炵�Ă���.
		return static_cast< int >( j.size() ) - 1;
	}

	// �����������Ȃ��悤��std::string�ɕϊ����Ď擾
	std::string GetString( const std::string& string ) {
		// json��UTF8�Ȃ��߈�x�������ϊ�����.
		std::wstring wString = StringConversion::to_wString( string, ECodePage::UTF8 );
		return StringConversion::to_String( wString );
	}

	// �^�̎擾
	std::string GetArrayMold( const json& j, const bool isPair );
	std::string GetMold( const json& j, const bool isPair = false ) {
		if ( j.is_string()			) return "std::string";
		if ( j.is_number_integer()	) return "int";
		if ( j.is_number_float()	) return "float";
		if ( j.is_boolean()			) return "bool";
		if ( j.is_array()			) return GetArrayMold( j, isPair );
		ErrorMessage( "1>Error : ��Ή��̌^�ł�", "json" );
		return "";
	}

	// �z��ɂȂ��Ă���^�̎擾.
	std::string GetArrayMold( const json& j, const bool isPair ) {
		if ( j.is_array() == false ) return GetMold( j );
		const auto size = j.size();

		// �v�f����1�̏ꍇ���̌^�ŕԂ�.
		if ( size == 1 ) return GetMold( j[0], isPair );

		// �v�f�S�Ă̌^���擾
		std::vector<std::string> ml;
		ml.reserve( size );
		for ( auto& l : j ) {
			ml.emplace_back( GetMold( l, isPair ) );
		}

		// D3DXVECTOR�n�̃`�F�b�N
		if ( isPair == false ) {
			const auto isAllFloat = std::all_of( ml.begin(), ml.end(), []( std::string s ) { return s == "float"; } );
			if ( size == 2 && isAllFloat ) return "D3DXVECTOR2";
			if ( size == 3 && isAllFloat ) return "D3DXVECTOR3";
			if ( size == 4 && isAllFloat ) return "D3DXVECTOR4";
		}

		// �^���o���o���Ȃ���std::pair<A, B>�ŕԂ�(�v�f����2��).
		if ( size == 2 ) {
			return "std::pair<" + GetMold( j[0], isPair ) + ", " + GetMold( j[1], isPair ) + ">";
		}
		ErrorMessage( "1>Error : ��Ή��̔z��ł�", "json" );
		return "";
	}


	// �^�ɂ������C���f���g�̎擾
	std::string GetMoldIndent( const json& j ) {
		if ( j.is_string()			) return "	";
		if ( j.is_number_integer()	) return "			";
		if ( j.is_number_float()	) return "		";
		if ( j.is_boolean()			) return "		";
		if ( j.is_array()			) return "	";
		return "";
	}
}

//-----------------.
// ���C��
//-----------------.
int ConstCreate::main()
{
	fprintf( stderr, "1>Const.h\n" );

	std::vector<std::pair<std::pair<std::string, std::string>, json>> jsonList;
	auto FileLoad = [&]( const std::filesystem::directory_entry& Entry )
	{
		const std::string Extension = Entry.path().extension().string();	// �g���q.
		const std::string FilePath = Entry.path().string();				// �t�@�C���p�X.
		const std::string FileName = Entry.path().stem().string();			// �t�@�C����.

		// �g���q���Í�������t�@�C����.
		if ( Extension != ".json" && Extension != ".JSON" ) return;

		// �t�@�C���̒ǉ�
		jsonList.emplace_back( std::make_pair( std::make_pair( FilePath, FileName ), FileManager::JsonLoad( FilePath ) ) );
	};

	try {
		std::filesystem::recursive_directory_iterator Dir_itr( CONST_DATA_PATH ), End_itr;
		std::for_each( Dir_itr, End_itr, FileLoad );
	} catch ( const std::filesystem::filesystem_error& e ) {
		// �G���[���b�Z�[�W��\��.
		return ErrorMessage( "Error : �t�@�C���̓ǂݍ��� ���s", e.path1().string().c_str() );
	}

	h( jsonList );
	fprintf( stderr, "1>Const.cpp\n" );
	cpp( jsonList );

	return S_OK;
}

//-----------------.
// .h�t�@�C���̍쐬
//-----------------.
void ConstCreate::h( const std::vector<std::pair<std::pair<std::string, std::string>, json>>& List ) {
	std::string Text = "";

	Text +=
		"#pragma once\n"
		"#include \"..\\..\\Global.h\"\n"
		"#include <vector>\n"
		"\n"
		"/************************************************\n"
		"*	�萔�֐�\n"
		"*	�ucreator�v�ɂ���Ď����ō쐬����Ă��܂�\n"
		"*/\n"
		"namespace ConstStructs {\n";

	std::vector<std::pair<std::string, std::string>> structList;
	for ( auto [File, Json] : List ) {
		auto [Path, Name] = File;
		const auto structName = "stConst" + Name;
		structList.emplace_back( std::make_pair( structName, Name ) );
		Text +=
			"	struct " + structName + " {\n";

		for ( auto& [Key, Value] : Json.items() ) {
			const auto size		= GetSize( Value );
			const auto comment	= GetString( Value[COMMENT] );
			const auto numf		= std::count( comment.begin(), comment.end(), '<' );
			const auto nume		= std::count( comment.begin(), comment.end(), '>' );
			const auto isPair	= numf == 2 && nume == 2;

			if ( size > 1 ) {
				Text += "		std::vector<" + GetMold( Value[DATA], isPair ) + "> " + Key + ";	// " + comment + "\n";
			}
			else {
				Text += "		" + GetMold( Value[DATA], isPair ) + GetMoldIndent( Value[DATA] ) + Key + ";	// " + comment + "\n";
			}
		}

		Text +=
			"\n"
			"		void Load() const;\n"
			"		" + structName + "() { Load(); }\n"
			"	};\n"
			"\n";
	}


	Text += 
		"}\n"
		"\n"
		"namespace Const{\n";

	for ( auto [Struct, Name] : structList ) {
		Text += "	const static ConstStructs::" + Struct + " " + Name + ";\n";
	}
	Text +=
		"	void Load();\n"
		"}\n";

	// �t�@�C���̍쐬.
	FileManager::TextSave( H_PATH, Text );
}

//-----------------.
// .cpp�t�@�C���̍쐬
//-----------------.
void ConstCreate::cpp( const std::vector<std::pair<std::pair<std::string, std::string>, json>>& List ) {
	std::string Text = "";

	Text +=
		"#include \"Const.h\"\n"
		"#include \"..\\FileManager\\FileManager.h\"\n"
		"\n"
		"namespace {\n"
		"	constexpr int _COMMENT	= 0; // �R�����g�̈ʒu\n"
		"	constexpr int _DATA		= 1; // �f�[�^�̈ʒu\n"
		"	constexpr int _X		= 0; // X�̈ʒu\n"
		"	constexpr int _Y		= 1; // Y�̈ʒu\n"
		"	constexpr int _Z		= 2; // Z�̈ʒu\n"
		"	constexpr int _W		= 2; // W�̈ʒu\n"
		"	constexpr int _FIRST	= 0; // �ŏ��̈ʒu\n"
		"	constexpr int _SECOND	= 1; // ���̈ʒu\n"
		"\n"
		"	// �z��̃T�C�Y���擾\n"
		"	int GetSize( const json& j ) {\n"
		"		// ��ԍŏ��ɃR�����g�����邽�ߗv�f����1���炵�Ă���\n"
		"		return static_cast<int>( j.size() ) - 1;\n"
		"	}\n"
		"\n"
		"	// �����������Ȃ��悤��std::string�ɕϊ����Ď擾\n"
		"	std::string GetString( const std::string& string ) {\n"
		"		// json��UTF8�Ȃ��߈�x�������ϊ�����\n"
		"		std::wstring wString = StringConversion::to_wString( string, ECodePage::UTF8 );\n"
		"		return StringConversion::to_String( wString );\n"
		"	}\n"
		"}\n"
		"\n";

	std::vector<std::string> structList;
	for ( auto [File, Json] : List ) {
		auto [Path, Name] = File;
		const auto structName = "stConst" + Name;
		structList.emplace_back( Name );

		std::string target		= "\\";
		std::string replacement = "\\\\";
		std::string::size_type pos = 0;
		while ( ( pos = Path.find( target, pos ) ) != std::string::npos ) {
			Path.replace( pos, target.length(), replacement );
			pos += replacement.length();
		}

		Text +=
			"void ConstStructs::" + structName + "::Load() const\n"
			"{\n"
			"	json j		= FileManager::JsonLoad( \"" + Path + "\" );\n"
			"	auto This	= const_cast<" + structName + "*>( this );\n"
			"\n"
			"	// ������\n";

		for ( auto& [Key, Value] : Json.items() ) {
			const auto size		= GetSize( Value );
			const auto comment	= GetString( Value[COMMENT] );
			const auto numf		= std::count( comment.begin(), comment.end(), '<' );
			const auto nume		= std::count( comment.begin(), comment.end(), '>' );
			const auto isPair	= numf == 2 && nume == 2;
			const auto m		= GetMold( Value[DATA], isPair );

			if ( size > 1 ) {
				Text +=
					"	const int " + Key + "_SIZE = GetSize( j[\"" + Key + "\"] );\n"
					"	This->" + Key + ".resize( " + Key + "_SIZE );\n"
					"	for ( int i = 0; i < " + Key + "_SIZE; ++i ) {\n"
					"		const int No = i + 1;\n";

				if ( m == "std::string" ) {
					Text += "		This->" + Key + "[i] = GetString( j[\"" + Key + "\"][No] ); \n";
				}
				else if ( m == "D3DXVECTOR2" ) {
					Text += "		This->" + Key + "[i].x = j[\"" + Key + "\"][No][_X];\n";
					Text += "		This->" + Key + "[i].y = j[\"" + Key + "\"][No][_Y];\n";
				}
				else if ( m == "D3DXVECTOR3" ) {
					Text += "		This->" + Key + "[i].x = j[\"" + Key + "\"][No][_X];\n";
					Text += "		This->" + Key + "[i].y = j[\"" + Key + "\"][No][_Y];\n";
					Text += "		This->" + Key + "[i].z = j[\"" + Key + "\"][No][_Z];\n";
				}
				else if ( m == "D3DXVECTOR4" ) {
					Text += "		This->" + Key + "[i].x = j[\"" + Key + "\"][No][_X];\n";
					Text += "		This->" + Key + "[i].y = j[\"" + Key + "\"][No][_Y];\n";
					Text += "		This->" + Key + "[i].z = j[\"" + Key + "\"][No][_Z];\n";
					Text += "		This->" + Key + "[i].w = j[\"" + Key + "\"][No][_W];\n";
				}
				else if ( m.find( "std::pair" ) != std::string::npos ) {
					// first
					if ( m.find( "<std::string, " ) != std::string::npos ) {
						Text += "		This->" + Key + "[i].first  = GetString( j[\"" + Key + "\"][No][_FIRST] );\n";
					}
					else {
						Text += "		This->" + Key + "[i].first  = j[\"" + Key + "\"][No][_FIRST];\n";
					}
					// second
					if ( m.find( ", std::string>" ) != std::string::npos ) {
						Text += "		This->" + Key + "[i].second = GetString( j[\"" + Key + "\"][No][_SECOND] );\n";
					}
					else {
						Text += "		This->" + Key + "[i].second = j[\"" + Key + "\"][No][_SECOND];\n";
					}
				}
				else {
					Text += "		This->" + Key + "[i] = j[\"" + Key + "\"][No];\n";
				}

				Text += "	}\n";
			}
			else {
				if ( m == "std::string" ){
					Text += "	This->" + Key + " = GetString( j[\"" + Key + "\"][_DATA] );\n";
				}
				else if ( m == "D3DXVECTOR2" ) {
					Text += "	This->" + Key + ".x = j[\"" + Key + "\"][_DATA][_X];\n";
					Text += "	This->" + Key + ".y = j[\"" + Key + "\"][_DATA][_Y];\n";
				}
				else if ( m == "D3DXVECTOR3" ) {
					Text += "	This->" + Key + ".x = j[\"" + Key + "\"][_DATA][_X];\n";
					Text += "	This->" + Key + ".y = j[\"" + Key + "\"][_DATA][_Y];\n";
					Text += "	This->" + Key + ".z = j[\"" + Key + "\"][_DATA][_Z];\n";
				}
				else if ( m == "D3DXVECTOR4" ) {
					Text += "	This->" + Key + ".x = j[\"" + Key + "\"][_DATA][_X];\n";
					Text += "	This->" + Key + ".y = j[\"" + Key + "\"][_DATA][_Y];\n";
					Text += "	This->" + Key + ".z = j[\"" + Key + "\"][_DATA][_Z];\n";
					Text += "	This->" + Key + ".w = j[\"" + Key + "\"][_DATA][_W];\n";
				}
				else if ( m.find( "std::pair" ) != std::string::npos ) {
					// first
					if ( m.find( "<std::string, " ) != std::string::npos ) {
						Text += "	This->" + Key + ".first  = GetString( j[\"" + Key + "\"][_DATA][_FIRST] );\n";
					} else {
						Text += "	This->" + Key + ".first  = j[\"" + Key + "\"][_DATA][_FIRST];\n";
					}
					// second
					if ( m.find( ", std::string>" ) != std::string::npos ) {
						Text += "	This->" + Key + ".second = GetString( j[\"" + Key + "\"][_DATA][_SECOND] );\n";
					} else {
						Text += "	This->" + Key + ".second = j[\"" + Key + "\"][_DATA][_SECOND];\n";
					}
				}
				else {
					Text += "	This->" + Key + " = j[\"" + Key + "\"][_DATA];\n";
				}
			}
		}

		Text +=
			"\n"
			"	Log::PushLog( \"" + Path + " �ǂݍ��� : ����\" );\n"
			"}\n"
			"\n";
	}

	Text += 
		"void Const::Load()\n"
		"{\n";
	for ( auto Name : structList ) {
		Text += "	" + Name + ".Load();\n";
	}
	Text += "}\n";

	// �t�@�C���̍쐬.
	FileManager::TextSave( CPP_PATH, Text );
}

//-----------------.
// ���U���g�̕\��
//-----------------.
void ConstCreate::result()
{
	auto h   = "1>Creator -> " + std::filesystem::absolute( H_PATH	 ).string() + "\n";
	auto cpp = "1>Creator -> " + std::filesystem::absolute( CPP_PATH ).string() + "\n";
	fprintf( stderr, h.c_str()		);
	fprintf( stderr, cpp.c_str()	);
}
