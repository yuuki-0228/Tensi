#include "FileManager.h"
#ifdef ENABLE_FILE
#include <time.h>

namespace {
	// 文字列の前後の空白を取り除く.
	std::string Trim( const std::string& Text )
	{
		constexpr char SPACE[] = " \t\r\n";
		const size_t Begin = Text.find_first_not_of( SPACE );
		if ( Begin == std::string::npos ) return "";
		const size_t End = Text.find_last_not_of( SPACE );
		return Text.substr( Begin, End - Begin + 1 );
	}

	// ログレベルを文字列に変換する.
	const char* LogLevelToString( const FileManager::ELogLevel Level )
	{
		switch ( Level ) {
		case FileManager::ELogLevel::Debug:		return "DEBUG";
		case FileManager::ELogLevel::Info:		return "INFO";
		case FileManager::ELogLevel::Warning:	return "WARN";
		case FileManager::ELogLevel::Error:		return "ERROR";
		case FileManager::ELogLevel::Fatal:		return "FATAL";
		}
		return "INFO";
	}
}

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

	// 実ファイルが無い場合はアーカイブから読み込む.
	if ( FileManager::FileCheck( FilePath ) == false &&
		 encrypt::GetIsArchiveFile( FilePath ) ) return FileManager::EFile::ETextLoad( FilePath, IsCommentOut, Delim );
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

//----------------------------.
// ログファイルの読み込み.
//----------------------------.
std::vector<std::string> FileManager::LogLoad( const std::string& FilePath )
{
	// コメント処理を行わず1行ずつ読み込む.
	return FileManager::TextLoad( FilePath, false );
}

//----------------------------.
// ログファイルの書き込み.
//	出力形式 : [時:分:秒] [レベル] [呼び出し] メッセージ.
//----------------------------.
HRESULT FileManager::LogSave(
	const std::string&	FilePath,
	const std::string&	Text,
	const ELogLevel		Level,
	const std::string&	Caller,
	const bool			IsAppend)
{
	// ファイルを開く.
	std::ofstream o( FilePath, IsAppend ? std::ios::app : std::ios::trunc );
	if ( !o ) {
		// 開けないためファイルディレクトリを作成する.
		FileManager::CreateFileDirectory( FilePath );

		// 書き込みなおす.
		return FileManager::LogSave( FilePath, Text, Level, Caller, IsAppend );
	}

	// 現在の時間を取得.
	time_t	nowTime = time( nullptr );
	tm		timeData;
	// ローカル時間に変換.
	localtime_s( &timeData, &nowTime );

	// [タイムスタンプ].
	o << "[";
	o << timeData.tm_year + 1900	<< "/";	// 1900 足すことで現在の年になる.
	o << timeData.tm_mon + 1		<< "/";	// 1 足すことで現在の月になる.
	o << timeData.tm_mday			<< " ";
	o << timeData.tm_hour			<< ":";
	o << timeData.tm_min			<< ":";
	o << timeData.tm_sec;
	o << "] ";

	// [レベル] [呼び出し] メッセージ.
	o << "[" << LogLevelToString( Level )	<< "] ";
	if ( !Caller.empty() ) o << "[" << Caller << "] ";
	o << Text << std::endl;

	// ファイルを閉じる.
	o.close();
	return S_OK;
}

//----------------------------.
// iniファイルの読み込み.
//----------------------------.
FileManager::IniData FileManager::IniLoad( const std::string& FilePath )
{
	IniData Out;

	// コメント処理を行わず1行ずつ読み込む.
	const std::vector<std::string> Lines = FileManager::TextLoad( FilePath, false );

	std::string Section = "";
	for ( const std::string& RawLine : Lines ) {
		// 前後の空白を取り除く.
		const std::string Line = Trim( RawLine );
		if ( Line.empty() ) continue;

		// コメント行は読み飛ばす.
		if ( Line[0] == ';' || Line[0] == '#' ) continue;

		// セクション行.
		if ( Line.front() == '[' && Line.back() == ']' ) {
			Section = Trim( Line.substr( 1, Line.size() - 2 ) );
			// 空のセクションも作成する.
			Out[Section];
			continue;
		}

		// キーと値に分割する.
		const size_t Pos = Line.find( '=' );
		if ( Pos == std::string::npos ) continue;
		const std::string Key	= Trim( Line.substr( 0, Pos ) );
		const std::string Value	= Trim( Line.substr( Pos + 1 ) );
		if ( Key.empty() ) continue;
		Out[Section][Key] = Value;
	}
	return Out;
}

//----------------------------.
// iniファイルの書き込み.
//----------------------------.
HRESULT FileManager::IniSave( const std::string& FilePath, const IniData& Data )
{
	std::ostringstream ss;

	// セクションに属さないキーを先に書き込む.
	const auto gitr = Data.find( "" );
	if ( gitr != Data.end() ) {
		for ( const auto& [Key, Value] : gitr->second ) ss << Key << "=" << Value << std::endl;
		ss << std::endl;
	}

	// セクションごとに書き込む.
	for ( const auto& [Section, Keys] : Data ) {
		if ( Section.empty() ) continue;
		ss << "[" << Section << "]" << std::endl;
		for ( const auto& [Key, Value] : Keys ) ss << Key << "=" << Value << std::endl;
		ss << std::endl;
	}
	return FileManager::TextSave( FilePath, ss.str() );
}

//---------------------------.
// jsonファイルを開く.
//---------------------------.
Json FileManager::JsonLoad( const std::string& FilePath )
{
	json Out;

	// 暗号化されているか取得.
#ifndef _DEBUG
	std::string fp = encrypt::GetEncryptionFilePath( FilePath );
	if ( encrypt::GetIsEncryption( fp ) ) return FileManager::EFile::EJsonLoad( fp );

	// 実ファイルが無い場合はアーカイブから読み込む.
	if ( FileManager::FileCheck( FilePath ) == false &&
		 encrypt::GetIsArchiveFile( FilePath ) ) return FileManager::EFile::EJsonLoad( FilePath );
#endif

	// ファイルを開く.
	std::ifstream i( FilePath );
	if ( !i ) return Out;

	// json型に変更.
	try {
		i >> Out;
	}
	catch ( ... ) {
		Log::PushLogError( "Json parse error : " + FilePath );
		Out = json();
	}

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

//----------------------------.
// ファイルを削除する
//----------------------------.
HRESULT FileManager::FileDelete(const std::string& FilePath)
{
	std::filesystem::remove(FilePath);
	return S_OK;
}

//----------------------------.
// ファイルを全て削除する
//----------------------------.
HRESULT FileManager::FileDeleteAll(const std::string& FilePath)
{
	std::filesystem::remove_all(FilePath);
	return S_OK;
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
	if ( encrypt::GetIsEncryption( FilePath ) == false &&
		 encrypt::GetIsArchiveFile( FilePath ) == false ) return OutList;
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
	if ( encrypt::GetIsEncryption( FilePath ) == false &&
		 encrypt::GetIsArchiveFile( FilePath ) == false ) return Out;
	auto rf = encrypt::GetRestoreFile( FilePath );
	if ( rf.first == nullptr ) return Out;
	encrypt::membuf mb( rf );
	std::istream i( &mb );
	if ( !i ) return Out;

	// json型に変更.
	try {
		i >> Out;
	}
	catch ( ... ) {
		Log::PushLogError( "Json parse error : " + FilePath );
		Out = json();
	}

	// ファイルを閉じる.
	mb.DestroyFile();
	return Out;
}
#endif
#endif