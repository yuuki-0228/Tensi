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

	// Jsonファイルのデータ位置指定定数
	constexpr int COMMENT	= 0; // コメントの位置.
	constexpr int DATA		= 1; // データの位置.

	// 配列のサイズを取得
	int GetSize( const json& j ) {
		// 一番最初にコメントがあるため要素数を1減らしておく.
		return static_cast<int>( j.size() ) - 1;
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

	std::vector<std::pair<std::pair<std::string, std::string>, json>> List;
}

//-----------------.
// メイン
//-----------------.
int MDataCreate::main()
{
	PrintOutput( "1>MasterDatas.h\n" );

	std::vector<std::pair<std::string, json>> ContainerList;
	auto ContainerLoad = [&]( const std::filesystem::directory_entry& Entry )
	{
		const std::string Extension = Entry.path().extension().string();	// 拡張子.
		const std::string FilePath = Entry.path().string();					// ファイルパス.
		const std::string FileName = Entry.path().stem().string();			// ファイル名.

		// jsonファイルか.
		if ( Extension != ".json" && Extension != ".JSON"	) return;
		if ( FileName.substr( 0, 1 ) == "$"					) return;

		// ファイルの追加
		ContainerList.emplace_back( std::make_pair( FileName, FileManager::JsonLoad( FilePath ) ) );
	};

	try {
		std::filesystem::recursive_directory_iterator Dir_itr( CONTAINER_PATH ), End_itr;
		std::for_each( Dir_itr, End_itr, ContainerLoad );
	} catch ( const std::filesystem::filesystem_error& e ) {
		// エラーメッセージを表示.
		return ErrorMessage( "1>Error : ファイルの読み込み 失敗", e.path1().string().c_str() );
	}

	h( ContainerList );
	PrintOutput( "1>MasterDatas.cpp\n" );
	cpp( List );

	return S_OK;
}

//-----------------.
// .hファイルの作成
//-----------------.
void MDataCreate::h( const std::vector<std::pair<std::string, json>>& Container )
{
	std::string Text = "";

	Text +=
		"#pragma once\n"
		"#include \"..\\..\\SystemSetting.h\"\n"
		"#ifdef ENABLE_MASTER_DATA\n"
		"#include \"..\\..\\Global.h\"\n"
		"#include \"..\\FileManager\\FileManager.h\"\n"
		"#include <unordered_map>\n"
		"#include <any>\n"
		"\n"
		"/************************************************\n"
		"*	マスターデータ一覧\n"
		"*	「creator」によって自動で作成されています\n"
		"**/\n"
		"namespace MasterData {\n"
		"	using ulong = unsigned long;"
		"\n";

	for ( auto& [Name, File] : Container ) {
		std::string structName = Name;
		if ( structName[0] == 'M' ) structName.replace( 0, 1, "Master" );
		structName += "Data";

		List.emplace_back( std::make_pair( std::make_pair( Name, structName ), File ) );

		Text +=
			"	struct " + structName + "\n"
			"	{\n"
			"		// ID\n"
			"		ulong Id;\n";

		for ( auto& [Key, Value] : File.items() ) {
			if ( Key == "ID" || Key == "Id" || Key == "id"	) continue;
			if ( Key.find( "_enum" ) != std::string::npos	) continue;
			const auto size		= GetSize( Value );
			const auto comment	= GetString( Value[COMMENT] );
			const auto numf		= std::count( comment.begin(), comment.end(), '<' );
			const auto nume		= std::count( comment.begin(), comment.end(), '>' );
			const auto isPair	= numf == 2 && nume == 2;
			const auto isId		= comment.find( "id" ) != std::string::npos || comment.find("Id") != std::string::npos || comment.find( "iD" ) != std::string::npos || comment.find( "ID" ) != std::string::npos;
			const auto Enum		= File.find( Key + "_enum" );

			Text += "		// " + comment + "\n";

			if ( Enum != File.end() ) {
				Text += "		enum class " + Key + "s {\n";

				for ( auto& l : Enum.value() ) {
					Text += "			" + std::string( l ) + ",\n";
				}

				Text += "		};\n";

				if ( size > 1 ) {
					Text += "		std::vector<" + Key + "s> " + Key + ";\n";
				}
				else {
					Text += "		" + Key + "s " + Key + ";\n";
				}
			}
			else if ( isId ) {
				if ( size > 1 ) {
					Text += "		std::vector<ulong> " + Key + ";\n";
				} else {
					Text += "		ulong " + Key + ";\n";
				}
			}
			else {
				if ( size > 1 ) {
					Text += "		std::vector<" + GetMold( Value[DATA], isPair ) + "> " + Key + ";\n";
				}
				else {
					Text += "		" + GetMold( Value[DATA], isPair ) + " " + Key + ";\n";
				}
			}
		}

		Text +=
			"\n"
			"		" + structName + "();\n"
			"		" + structName + "( const Json& j );\n"
			"	} typedef " + Name + "Data;\n"
			"\n";

		// ディレクトリを作成しておく
		FileManager::CreateFileDirectory( DATA_PATH + Name + "\\" );
	}

	Text +=
		"}\n"
		"using namespace MasterData;\n"
		"\n"
		"namespace MasterDataUtility {\n"
		"	std::unordered_map<std::string, std::unordered_map<ulong, std::any>> CreateCache( const std::vector<std::pair<std::string, Json>>& data );\n"
		"	void CacheSetup( std::unordered_map<std::string, std::unordered_map<ulong, std::any>>& out, const std::string& Container, const Json& File );\n"
		"}\n"
		"#endif\n";

	// ファイルの作成.
	FileManager::TextSave( H_PATH, Text );
}

//-----------------.
// .cppファイルの作成
//-----------------.
void MDataCreate::cpp( const std::vector<std::pair<std::pair<std::string, std::string>, json>>& List )
{
	std::string Text = "";

	Text +=
		"#include \"MasterDatas.h\"\n"
		"#ifdef ENABLE_MASTER_DATA\n"
		"\n"
		"namespace {\n"
		"	constexpr int _X		= 0; // Xの位置\n"
		"	constexpr int _Y		= 1; // Yの位置\n"
		"	constexpr int _Z		= 2; // Zの位置\n"
		"	constexpr int _W		= 3; // Wの位置\n"
		"	constexpr int _FIRST	= 0; // 最初の位置\n"
		"	constexpr int _SECOND	= 1; // 次の位置\n"
		"\n"
		"	// 配列のサイズを取得\n"
		"	int GetSize( const json& j ) {\n"
		"		return static_cast<int>( j.size() );\n"
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

	for ( auto& [Data, File] : List ) {
		auto& [Name, Struct] = Data;
		Text +=
			"//----------------------------.\n"
			"// " + Struct + ".\n"
			"//----------------------------.\n";

		Text +=
			Struct + "::" + Struct + "()\n"
			"	: Id	()\n";

		for ( auto& [Key, Value] : File.items() ) {
			if ( Key == "ID" || Key == "Id" || Key == "id" ) continue;
			if ( Key.find( "_enum" ) != std::string::npos ) continue;
			Text += "	, " + Key + "	()\n";
		}

		Text +=
			"{\n"
			"}\n";

		Text +=
			Struct + "::" + Struct + "( const Json& j )\n"
			"{\n"
			"	Id = j[\"Id\"].Get<ulong>();\n";

		for ( auto& [Key, Value] : File.items() ) {
			if ( Key == "ID" || Key == "Id" || Key == "id" ) continue;
			if ( Key.find( "_enum" ) != std::string::npos ) continue;
			const auto size		= GetSize( Value );
			const auto comment	= GetString( Value[COMMENT] );
			const auto numf		= std::count( comment.begin(), comment.end(), '<' );
			const auto nume		= std::count( comment.begin(), comment.end(), '>' );
			const auto isPair	= numf == 2 && nume == 2;
			const auto m		= GetMold( Value[DATA], isPair );
			const auto isId		= comment.find( "id" ) != std::string::npos || comment.find("Id") != std::string::npos || comment.find( "iD" ) != std::string::npos || comment.find( "ID" ) != std::string::npos;
			const auto Enum		= File.find( Key + "_enum" );

			// pairの要素の型を取得
			std::string fm = "";
			std::string sm = "";
			if ( m.find( "std::pair" ) != std::string::npos ) {
				fm = GetMold( Value[DATA][0], isPair );
				sm = GetMold( Value[DATA][1], isPair );
			}

			if ( size > 1 ) {
				Text +=
					"	const int " + Key + "Size = GetSize( j[\"" + Key + "\"] );\n"
					"	" + Key + ".resize( " + Key + "Size );\n"
					"	for ( int i = 0; i < " + Key + "Size; ++i ) {\n";

				if ( m == "std::string" ){
					Text += "		" + Key + "[i] = GetString( j[\"" + Key + "\"][i].Get<std::string>() );\n";
				}
				else if ( m == "D3DXVECTOR2" ) {
					Text += "		" + Key + "[i].x = j[\"" + Key + "\"][i][_X].Get<float>();\n";
					Text += "		" + Key + "[i].y = j[\"" + Key + "\"][i][_Y].Get<float>();\n";
				}
				else if ( m == "D3DXVECTOR3" ) {
					Text += "		" + Key + "[i].x = j[\"" + Key + "\"][i][_X].Get<float>();\n";
					Text += "		" + Key + "[i].y = j[\"" + Key + "\"][i][_Y].Get<float>();\n";
					Text += "		" + Key + "[i].z = j[\"" + Key + "\"][i][_Z].Get<float>();\n";
				}
				else if ( m == "D3DXVECTOR4" ) {
					Text += "		" + Key + "[i].x = j[\"" + Key + "\"][i][_X].Get<float>();\n";
					Text += "		" + Key + "[i].y = j[\"" + Key + "\"][i][_Y].Get<float>();\n";
					Text += "		" + Key + "[i].z = j[\"" + Key + "\"][i][_Z].Get<float>();\n";
					Text += "		" + Key + "[i].w = j[\"" + Key + "\"][i][_W].Get<float>();\n";
				}
				else if ( m.find( "std::pair" ) != std::string::npos ) {
					// first
					if ( fm == "std::string" ) {
						Text += "		" + Key + "[i].first  = GetString( j[\"" + Key + "\"][i][_FIRST].Get<std::string>() );\n";
					} else {
						Text += "		" + Key + "[i].first  = j[\"" + Key + "\"][i][_FIRST].Get<" + fm + ">();\n";
					}
					// second
					if ( sm == "std::string" ) {
						Text += "		" + Key + "[i].second = GetString( j[\"" + Key + "\"][i][_SECOND].Get<std::string>() );\n";
					} else {
						Text += "		" + Key + "[i].second = j[\"" + Key + "\"][i][_SECOND].Get<" + sm + ">();\n";
					}
				}
				else if ( Enum != File.end() ) {
					Text += "		" + Key + "[i] = " + "static_cast<" + Key + "s>( " + "j[\"" + Key + "\"][i].Get<int>() );\n";
				}
				else if ( isId ) {
					Text += "		" + Key + "[i] = j[\"" + Key + "\"][i].Get<ulong>();\n";
				}
				else {
					Text += "		" + Key + "[i] = j[\"" + Key + "\"][i].Get<" + m + ">();\n";
				}

				Text += "	}\n";
			}
			else {
				if ( m == "std::string" ){
					Text += "	" + Key + " = GetString( j[\"" + Key + "\"].Get<std::string>() );\n";
				}
				else if ( m == "D3DXVECTOR2" ) {
					Text += "	" + Key + ".x = j[\"" + Key + "\"][_X].Get<float>();\n";
					Text += "	" + Key + ".y = j[\"" + Key + "\"][_Y].Get<float>();\n";
				}
				else if ( m == "D3DXVECTOR3" ) {
					Text += "	" + Key + ".x = j[\"" + Key + "\"][_X].Get<float>();\n";
					Text += "	" + Key + ".y = j[\"" + Key + "\"][_Y].Get<float>();\n";
					Text += "	" + Key + ".z = j[\"" + Key + "\"][_Z].Get<float>();\n";
				}
				else if ( m == "D3DXVECTOR4" ) {
					Text += "	" + Key + ".x = j[\"" + Key + "\"][_X].Get<float>();\n";
					Text += "	" + Key + ".y = j[\"" + Key + "\"][_Y].Get<float>();\n";
					Text += "	" + Key + ".z = j[\"" + Key + "\"][_Z].Get<float>();\n";
					Text += "	" + Key + ".w = j[\"" + Key + "\"][_W].Get<float>();\n";
				}
				else if ( m.find( "std::pair" ) != std::string::npos ) {
					// first
					if ( fm == "std::string" ) {
						Text += "	" + Key + ".first  = GetString( j[\"" + Key + "\"][_FIRST].Get<std::string>() );\n";
					} else {
						Text += "	" + Key + ".first  = j[\"" + Key + "\"][_FIRST].Get<" + fm + ">();\n";
					}
					// second
					if ( sm == "std::string" ) {
						Text += "	" + Key + ".second = GetString( j[\"" + Key + "\"][_SECOND].Get<std::string>() );\n";
					} else {
						Text += "	" + Key + ".second = j[\"" + Key + "\"][_SECOND].Get<" + sm + ">();\n";
					}
				}
				else if ( Enum != File.end() ) {
					Text += "	" + Key + " = " + "static_cast<" + Key + "s>( " + "j[\"" + Key + "\"].Get<int>() );\n";
				}
				else if ( isId ) {
					Text += "	" + Key + " = j[\"" + Key + "\"].Get<ulong>();\n";
				}
				else {
					Text += "	" + Key + " = j[\"" + Key + "\"].Get<" + m + ">();\n";
				}
			}
		}

		Text +=
			"}\n"
			"\n";
	}

	Text +=
		"//----------------------------.\n"
		"// 読み込み\n"
		"//----------------------------.\n"
		"std::unordered_map<std::string, std::unordered_map<ulong, std::any>> MasterDataUtility::CreateCache(\n"
		"	const std::vector<std::pair<std::string, Json>>& data )\n"
		"{\n"
		"	std::unordered_map<std::string, std::unordered_map<ulong, std::any>> out;\n"
		"\n"
		"	for ( auto& [Container, File] : data )\n"
		"	{\n"
		"		if ( File.is_array() ) {\n"
		"			for ( auto& Data : File ) {\n"
		"				CacheSetup( out, Container, Data );\n"
		"			}\n"
		"		}\n"
		"		else {\n"
		"			CacheSetup( out, Container, File );\n"
		"		}\n"
		"	}\n"
		"	return out;\n"
		"}\n"
		"void MasterDataUtility::CacheSetup( std::unordered_map<std::string, std::unordered_map<ulong, std::any>>& out, const std::string& Container, const Json& File )\n"
		"{\n"
		"	const ulong Id = File[\"Id\"].Get<ulong>();\n";

	bool isFirst = true;
	for ( auto& [Data, File] : List ) {
		auto& [Name, Struct] = Data;

		if ( isFirst ) {
			Text += "	if (	  Container == \"" + Name + "\" ) out[typeid( " + Struct + " ).name()][Id] = " + Struct + "( File );\n";
			isFirst = false;
		}
		else {
			Text += "	else if ( Container == \"" + Name + "\" ) out[typeid( " + Struct + " ).name()][Id] = " + Struct + "( File );\n";
		}
	}

	Text +=
		"}\n"
		"#endif";

	// ファイルの作成.
	FileManager::TextSave( CPP_PATH, Text );
}

//-----------------.
// リザルトの表示
//-----------------.
void MDataCreate::result()
{
	auto h   = "1>Creator -> " + std::filesystem::absolute( H_PATH	 ).string() + "\n";
	auto cpp = "1>Creator -> " + std::filesystem::absolute( CPP_PATH ).string() + "\n";
	PrintOutput( h );
	PrintOutput( cpp );
}