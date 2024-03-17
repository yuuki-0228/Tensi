#include "FileManager.h"

//----------------------------.
// テキストファイルの読み込み.
//	"//" : コメントアウト.
//	"#"  : 区切り文字(',')までコメントアウト.
//----------------------------.
std::vector<std::string> FileManager::TextLoad(
	const std::string&	FilePath,
	const bool			IsCommentOut,
	const char			Delim )
{
	std::vector<std::string> OutList;

	// 暗号化されているか取得.
#ifndef _DEBUG
	std::string fp = encrypt::GetEncryptionFilePath( FilePath );
	if ( encrypt::GetIsEncryption( fp ) ) return FileManager::EFile::ETextLoad( fp, IsCommentOut, Delim );
#endif

	// ファイルを開く.
	std::fstream i( FilePath );
	if ( !i ) return OutList;

	// 一行づつ読み込む.
	std::string Line;
	while ( std::getline( i, Line ) ){
		// コメントアウト処理.
		if ( IsCommentOut == true ){
			if ( Line.substr( 0, 2 ) == "//" ) continue;
		}

		// 文字を一時的に格納する.
		std::string Buff = "";
		std::istringstream stream( Line );
		if ( IsCommentOut == false ){
			OutList.emplace_back( Line );
			continue;
		}

		// 区切り文字(',')で区切って文字を取得.
		while ( std::getline( stream, Buff, Delim ) ){
			// 一部分のコメントアウト処理.
			if ( IsCommentOut == true ){
				if ( Buff.find( '#' ) != std::string::npos ) continue;
			}

			// 読み込みリストに追加.
			OutList.emplace_back( Buff );
		}
	}
	// ファイルを閉じる.
	i.close();
	return OutList;
}

//----------------------------.
// テキストファイルに書き込む.
//----------------------------.
HRESULT FileManager::TextSave( const std::string& FilePath, const std::string& Data )
{
	// ファイルを開く.
	std::ofstream o( FilePath, std::ios::trunc );
	if ( !o ) {
		// 開けないためファイルディレクトリを作成する.
		FileManager::CreateFileDirectory( FilePath );

		// 書き込みなおす.
		FileManager::TextSave( FilePath, Data );
		return S_OK;
	}

	// 書き込み.
	o << Data;

	// ファイルを閉じる.
	o.close();
	return S_OK;
}

//---------------------------.
// jsonファイルを開く.
//---------------------------.
json FileManager::JsonLoad( const std::string& FilePath )
{
	json Out;

	// 暗号化されているか取得.
#ifndef _DEBUG
	std::string fp = encrypt::GetEncryptionFilePath( FilePath );
	if ( encrypt::GetIsEncryption( fp ) ) return FileManager::EFile::EJsonLoad( fp );
#endif

	// ファイルを開く.
	std::ifstream i( FilePath );
	if ( !i ) return Out;

	// json型に変更.
	i >> Out;

	// ファイルを閉じる.
	i.close();
	return Out;
}

//---------------------------.
// jsonファイルで書き込む.
//---------------------------.
HRESULT FileManager::JsonSave( const std::string& FilePath, const json& Data )
{
	// ファイルを開く.
	std::ofstream o( FilePath );
	if ( !o ) {
		// 開けないためファイルディレクトリを作成する.
		FileManager::CreateFileDirectory( FilePath );

		// 書き込みなおす.
		FileManager::JsonSave( FilePath, Data );
		return S_OK;
	}

	// 書き込み.
	o << std::setw( 2 ) << Data << std::endl;

	// ファイルを閉じる.
	o.close();
	return S_OK;
}

//---------------------------.
// json を std::unordered_map に変換.
//---------------------------.
std::unordered_map<std::string, std::string> FileManager::JsonToMap( const json& Json )
{
	std::unordered_map<std::string, std::string> Out;
	for ( auto& [Key, Value] : Json.items() ) Out[Key] = Value;
	return Out;
}

//---------------------------.
// std::unordered_map を json に変換.
//---------------------------.
json FileManager::MapToJson( const std::unordered_map<std::string, std::string> Map )
{
	json Out;
	for ( auto&[Key, Value] : Map ) {
		// 文字列から型を推測してその型に変換して保存する.
		if (		Value == "nullptr"						) Out[Key] = nullptr;
		else if (	Value == "true"							) Out[Key] = true;
		else if (	Value == "false"						) Out[Key] = false;
		else if (	Value.find_first_not_of( "0123456789.f" ) == std::string::npos ) {
			if (	Value.find( "." ) != std::string::npos	) Out[Key] = std::stof( Value );
			else											  Out[Key] = std::stoi( Value );
		} else												  Out[Key] = Value;
	}
	return Out;
}
json FileManager::MapToJson( const std::unordered_map<std::string, std::vector<std::string>> Map )
{
	json Out;
	for ( auto&[Key, vValue] : Map ) {
		for ( auto& Value : vValue ) {
			// 文字列から型を推測してその型に変換して保存する.
			if (		Value == "nullptr"						) Out[Key].emplace_back( nullptr );
			else if (	Value == "true"							) Out[Key].emplace_back( true );
			else if (	Value == "false"						) Out[Key].emplace_back( false );
			else if (	Value.find_first_not_of( "0123456789.f" ) == std::string::npos ) {
				if (	Value.find( "." ) != std::string::npos	) Out[Key].emplace_back( std::stof( Value ) );
				else											  Out[Key].emplace_back( std::stoi( Value ) );
			} else												  Out[Key].emplace_back( Value );
		}
	}
	return Out;
}

//---------------------------.
// 足りないファイルディレクトリを作成.
//---------------------------.
HRESULT FileManager::CreateFileDirectory( const std::string& FilePath ) {
	auto fp = FilePath.rfind( "\\" );
	if ( fp != std::string::npos ) {
		std::filesystem::create_directories( FilePath.substr( 0, FilePath.rfind( "\\" ) ) );
	}
	return S_OK;
}

//----------------------------.
// ファイルが存在するか調べる.
//----------------------------.
bool FileManager::FileCheck( const std::string& FilePath )
{
	return std::filesystem::is_regular_file( FilePath );
}

#ifndef _DEBUG
//----------------------------.
// 暗号化テキストファイルの読み込み.
//----------------------------.
std::vector<std::string> FileManager::EFile::ETextLoad(
	const std::string&	FilePath,
	const bool			IsCommentOut,
	const char			Delim )
{
	std::vector<std::string> OutList;

	// ファイルを開く.
	if ( encrypt::GetIsEncryption( FilePath ) == false ) return OutList;
	auto rf = encrypt::GetRestoreFile( FilePath );
	if ( rf.first == nullptr ) return OutList;
	encrypt::membuf mb( rf );
	std::istream i( &mb );
	if ( !i ) return OutList;

	// 一行づつ読み込む.
	std::string Line;
	while ( std::getline( i, Line ) ){
		// コメントアウト処理.
		if ( IsCommentOut == true ){
			if ( Line.substr( 0, 2 ) == "//" ) continue;
		}

		// 文字を一時的に格納する.
		std::string Buff = "";
		std::istringstream stream( Line );
		if ( IsCommentOut == false ){
			OutList.emplace_back( Line );
			continue;
		}

		// 区切り文字(',')で区切って文字を取得.
		while ( std::getline( stream, Buff, Delim ) ){
			// 一部分のコメントアウト処理.
			if ( IsCommentOut == true ){
				if ( Buff.find( '#' ) != std::string::npos ) continue;
			}

			// 読み込みリストに追加.
			OutList.emplace_back( Buff );
		}
	}
	// ファイルを閉じる.
	mb.DestroyFile();
	return OutList;
}

//---------------------------.
// 暗号化jsonファイルを開く.
//---------------------------.
json FileManager::EFile::EJsonLoad( const std::string& FilePath )
{
	json Out;

	// ファイルを開く.
	if ( encrypt::GetIsEncryption( FilePath ) == false ) return Out;
	auto rf = encrypt::GetRestoreFile( FilePath );
	if ( rf.first == nullptr ) return Out;
	encrypt::membuf mb( rf );
	std::istream i( &mb );
	if ( !i ) return Out;

	// json型に変更.
	i >> Out;

	// ファイルを閉じる.
	mb.DestroyFile();
	return Out;
}
#endif
