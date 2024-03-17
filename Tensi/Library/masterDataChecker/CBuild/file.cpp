#include "file.h"
#include "error.h"

//----------------------------.
// テキストファイルの読み込み.
//----------------------------.
std::vector<std::string> FileManager::TextLoad( const std::string& FilePath )
{
	std::vector<std::string> OutList;

	// ファイルを開く.
	std::fstream i( FilePath );
	if ( !i ) return OutList;

	// 一行づつ読み込む.
	std::string Line;
	while ( std::getline( i, Line ) ) {
		// 文字を一時的に格納する.
		std::string Buff = "";
		std::istringstream stream( Line );
		OutList.emplace_back( Line );
	}

	// ファイルを閉じる.
	i.close();
	return OutList;
}

//---------------------------.
// テキストファイルの書き込む.
//---------------------------.
bool FileManager::TextSave( const std::string& FilePath, const std::string& Data )
{
	// ファイルを開く.
	std::ofstream o( FilePath, std::ios::trunc );
	if ( !o ) {
		ErrorMessage( "ファイルの書き込み 失敗", FilePath );
		return false;
	}

	// 書き込み.
	o << Data;

	// ファイルを閉じる.
	o.close();
	return true;
}

//---------------------------.
// jsonファイルの読み込み.
//---------------------------.
json FileManager::JsonLoad( const std::string& FilePath ) {
	// ファイルを開く.
	json j;
	std::ifstream i( FilePath );
	if ( !i ) {
		ErrorMessage( "jsonファイルの読み込み 失敗", FilePath );
		return j;
	}

	// json型に変更.
	i >> j;

	// ファイルを閉じる.
	i.close();
	return j;
}

//---------------------------.
// 足りないファイルディレクトリを作成する.
//---------------------------.
void FileManager::CreateFileDirectory( const std::string& FilePath )
{
	auto fp = FilePath.rfind( "\\" );
	if ( fp != std::string::npos ) {
		std::filesystem::create_directories( FilePath.substr( 0, FilePath.rfind( "\\" ) ) );
	}
}
