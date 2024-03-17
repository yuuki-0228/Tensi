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

	// デバックJsonファイルのデータ位置指定定数
	constexpr int COMMENT	= 0; // コメントの位置.
	constexpr int DATA		= 1; // データの位置.

	// 配列のサイズを取得
	int GetSize( const json& j ) {
		// 一番最初にコメントがあるため要素数を1減らしておく.
		return static_cast< int >( j.size() ) - 1;
	}

	// 文字化けしないようにstd::stringに変換して取得
	std::string GetString( const std::string& string ) {
		// jsonはUTF8なため一度文字列を変換する.
		std::wstring wString = StringConversion::to_wString( string, ECodePage::UTF8 );
		return StringConversion::to_String( wString );
	}

	// 型の取得
	std::string GetArrayMold( const json& j, const bool isPair );
	std::string GetMold( const json& j, const bool isPair = false ) {
		if ( j.is_string()			) return "std::string";
		if ( j.is_number_integer()	) return "int";
		if ( j.is_number_float()	) return "float";
		if ( j.is_boolean()			) return "bool";
		if ( j.is_array()			) return GetArrayMold( j, isPair );
		ErrorMessage( "1>Error : 非対応の型です", "json" );
		return "";
	}

	// 配列になっている型の取得.
	std::string GetArrayMold( const json& j, const bool isPair ) {
		if ( j.is_array() == false ) return GetMold( j );
		const auto size = j.size();

		// 要素数が1つの場合その型で返す.
		if ( size == 1 ) return GetMold( j[0], isPair );

		// 要素全ての型を取得
		std::vector<std::string> ml;
		ml.reserve( size );
		for ( auto& l : j ) {
			ml.emplace_back( GetMold( l, isPair ) );
		}

		// D3DXVECTOR系のチェック
		if ( isPair == false ) {
			const auto isAllFloat = std::all_of( ml.begin(), ml.end(), []( std::string s ) { return s == "float"; } );
			if ( size == 2 && isAllFloat ) return "D3DXVECTOR2";
			if ( size == 3 && isAllFloat ) return "D3DXVECTOR3";
			if ( size == 4 && isAllFloat ) return "D3DXVECTOR4";
		}

		// 型がバラバラなためstd::pair<A, B>で返す(要素数は2つ).
		if ( size == 2 ) {
			return "std::pair<" + GetMold( j[0], isPair ) + ", " + GetMold( j[1], isPair ) + ">";
		}
		ErrorMessage( "1>Error : 非対応の配列です", "json" );
		return "";
	}


	// 型にあったインデントの取得
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
// メイン
//-----------------.
int ConstCreate::main()
{
	fprintf( stderr, "1>Const.h\n" );

	std::vector<std::pair<std::pair<std::string, std::string>, json>> jsonList;
	auto FileLoad = [&]( const std::filesystem::directory_entry& Entry )
	{
		const std::string Extension = Entry.path().extension().string();	// 拡張子.
		const std::string FilePath = Entry.path().string();				// ファイルパス.
		const std::string FileName = Entry.path().stem().string();			// ファイル名.

		// 拡張子が暗号化するファイルか.
		if ( Extension != ".json" && Extension != ".JSON" ) return;

		// ファイルの追加
		jsonList.emplace_back( std::make_pair( std::make_pair( FilePath, FileName ), FileManager::JsonLoad( FilePath ) ) );
	};

	try {
		std::filesystem::recursive_directory_iterator Dir_itr( CONST_DATA_PATH ), End_itr;
		std::for_each( Dir_itr, End_itr, FileLoad );
	} catch ( const std::filesystem::filesystem_error& e ) {
		// エラーメッセージを表示.
		return ErrorMessage( "Error : ファイルの読み込み 失敗", e.path1().string().c_str() );
	}

	h( jsonList );
	fprintf( stderr, "1>Const.cpp\n" );
	cpp( jsonList );

	return S_OK;
}

//-----------------.
// .hファイルの作成
//-----------------.
void ConstCreate::h( const std::vector<std::pair<std::pair<std::string, std::string>, json>>& List ) {
	std::string Text = "";

	Text +=
		"#pragma once\n"
		"#include \"..\\..\\Global.h\"\n"
		"#include <vector>\n"
		"\n"
		"/************************************************\n"
		"*	定数関数\n"
		"*	「creator」によって自動で作成されています\n"
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

	// ファイルの作成.
	FileManager::TextSave( H_PATH, Text );
}

//-----------------.
// .cppファイルの作成
//-----------------.
void ConstCreate::cpp( const std::vector<std::pair<std::pair<std::string, std::string>, json>>& List ) {
	std::string Text = "";

	Text +=
		"#include \"Const.h\"\n"
		"#include \"..\\FileManager\\FileManager.h\"\n"
		"\n"
		"namespace {\n"
		"	constexpr int _COMMENT	= 0; // コメントの位置\n"
		"	constexpr int _DATA		= 1; // データの位置\n"
		"	constexpr int _X		= 0; // Xの位置\n"
		"	constexpr int _Y		= 1; // Yの位置\n"
		"	constexpr int _Z		= 2; // Zの位置\n"
		"	constexpr int _W		= 2; // Wの位置\n"
		"	constexpr int _FIRST	= 0; // 最初の位置\n"
		"	constexpr int _SECOND	= 1; // 次の位置\n"
		"\n"
		"	// 配列のサイズを取得\n"
		"	int GetSize( const json& j ) {\n"
		"		// 一番最初にコメントがあるため要素数を1減らしておく\n"
		"		return static_cast<int>( j.size() ) - 1;\n"
		"	}\n"
		"\n"
		"	// 文字化けしないようにstd::stringに変換して取得\n"
		"	std::string GetString( const std::string& string ) {\n"
		"		// jsonはUTF8なため一度文字列を変換する\n"
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
			"	// 初期化\n";

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
			"	Log::PushLog( \"" + Path + " 読み込み : 成功\" );\n"
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

	// ファイルの作成.
	FileManager::TextSave( CPP_PATH, Text );
}

//-----------------.
// リザルトの表示
//-----------------.
void ConstCreate::result()
{
	auto h   = "1>Creator -> " + std::filesystem::absolute( H_PATH	 ).string() + "\n";
	auto cpp = "1>Creator -> " + std::filesystem::absolute( CPP_PATH ).string() + "\n";
	fprintf( stderr, h.c_str()		);
	fprintf( stderr, cpp.c_str()	);
}
