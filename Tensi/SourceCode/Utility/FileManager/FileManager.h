#pragma once
#include "..\..\SystemSetting.h"
#ifdef ENABLE_FILE
#include "..\..\Global.h"
#include <nlohmnn\json.hpp>
#include <memory>
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include <encrypt/file.h>
#include <type_traits>
#include "Json\Json.h"

// json型を作成.
using json = nlohmann::json;

/************************************************
*	ファイル関係一覧.
**/
namespace FileManager {
	// テキストファイルの読み込み.
	//	"//" : コメントアウト.
	//	"#"  : 区切り文字(',')までコメントアウト.
	std::vector<std::string> TextLoad(
		const std::string&	FilePath,
		const bool			IsCommentOut = true,
		const char			Delim = ',' );

	// テキストファイルの書き込む.
	HRESULT TextSave( const std::string& FilePath, const std::string& Data );

	// ログファイルの読み込み (1行ずつ取得する).
	std::vector<std::string> LogLoad( const std::string& FilePath );

	// ログレベル.
	enum class ELogLevel {
		Debug,		// デバッグ情報.
		Info,		// 通常の情報.
		Warning,	// 警告.
		Error,		// エラー.
		Fatal,		// 致命的なエラー.
	};

	// ログファイルの書き込み.
	//	行頭に [時:分:秒] [レベル] [呼び出し] を付けて1行追記する.
	//	Level    : ログレベル.
	//	Caller   : 呼び出し元 (クラス::関数名など).
	//	IsAppend : 追記するか (false で新規作成).
	//	IsDate   : タイムスタンプに日付も付けるか (起動ログなどで使用).
	HRESULT LogSave(
		const std::string&	FilePath,
		const std::string&	Text,
		const ELogLevel		Level,
		const std::string&	Caller,
		const bool			IsAppend	= true);

	// iniデータ ( [セクション名][キー名] = 値 ).
	//	セクションに属さないキーは空文字("")のセクションに格納する.
	using IniData = std::unordered_map<std::string, std::unordered_map<std::string, std::string>>;

	// iniファイルの読み込み.
	//	";" "#" : 行頭コメントアウト.
	IniData IniLoad( const std::string& FilePath );

	// iniファイルの書き込み.
	//	※unordered_map のためセクション・キーの順序は保存されない.
	HRESULT IniSave( const std::string& FilePath, const IniData& Data );

	// iniデータから安全に値を取得する.
	template<class T>
	T IniGet( const IniData& Ini, const std::string& Section, const std::string& Key, const T& Default )
	{
		// セクションとキーを検索する.
		const auto sitr = Ini.find( Section );
		if ( sitr == Ini.end() ) return Default;
		const auto kitr = sitr->second.find( Key );
		if ( kitr == sitr->second.end() ) return Default;
		const std::string& Value = kitr->second;

		// 文字列はそのまま返す.
		if constexpr ( std::is_same_v<T, std::string> ) {
			return Value;
		}
		// bool は "true" / "false" にも対応する.
		else if constexpr ( std::is_same_v<T, bool> ) {
			if ( Value == "true"  || Value == "1" ) return true;
			if ( Value == "false" || Value == "0" ) return false;
			return Default;
		}
		// それ以外は文字列から変換する.
		else {
			std::istringstream ss( Value );
			T Out;
			if ( ss >> Out ) return Out;
			return Default;
		}
	}

	// バイナリデータでの読み込み.
	template<class T>
	HRESULT BinaryLoad( const char* FilePath, T& Out, const int& SeekPoint = 0 );
	template<class T>
	HRESULT BinaryLoad( const char* FilePath, std::vector<T>& OutList );

	// バイナリデータでの書き込む.
	template<class T>
	HRESULT BinarySave( const char* FilePath, const T& Data );
	template<class T>
	HRESULT BinarySave( const char* FilePath, const std::vector<T>& DataList );

	// json形式でjsonファイルを開く.
	//	戻り値の Json は nlohmann::json と相互変換できるラッパー.
	//	値の取得は Json::Get( Default ) / Json::Get<T>() を使用する.
	Json JsonLoad( const std::string& FilePath );

	// json形式をjsonファイルで書き込む.
	HRESULT JsonSave( const std::string& FilePath, const json& Data );

	// 足りないファイルディレクトリを作成する.
	HRESULT CreateFileDirectory( const std::string& FilePath );

	// ファイルが存在するか調べる.
	bool FileCheck( const std::string& FilePath );

	// ファイルを削除する
	HRESULT FileDelete(const std::string& FilePath);
	// ファイルを全て削除する
	HRESULT FileDeleteAll(const std::string& FilePath);

#ifndef _DEBUG
	// 暗号化ファイルの読み込み
	namespace EFile {
		// テキストファイルの読み込み.
		std::vector<std::string> ETextLoad(
			const std::string& FilePath,
			const bool			IsCommentOut = true,
			const char			Delim = ',' );

		// バイナリデータでの読み込み.
		template<class T>
		HRESULT EBinaryLoad( const char* FilePath, T& Out, const int& SeekPoint = 0 );
		template<class T>
		HRESULT EBinaryLoad( const char* FilePath, std::vector<T>& OutList );
		// json形式でjsonファイルを開く.
		json EJsonLoad( const std::string& FilePath );
	}
#endif
}

//---------------------------.
// バイナリデータでの読み込み.
//---------------------------.
template<class T>
HRESULT FileManager::BinaryLoad( const char* FilePath, T& Out, const int& SeekPoint )
{
#ifndef _DEBUG
	// 暗号化されているか取得.
	std::string fp = encrypt::GetEncryptionFilePath( FilePath );
	if ( encrypt::GetIsEncryption( fp ) ) return FileManager::EFile::EBinaryLoad( fp.c_str(), Out, SeekPoint );

	// 実ファイルが無い場合はアーカイブから読み込む.
	if ( FileManager::FileCheck( FilePath ) == false &&
		 encrypt::GetIsArchiveFile( FilePath ) ) return FileManager::EFile::EBinaryLoad( FilePath, Out, SeekPoint );
#endif

	// ファイルを開く.
	std::fstream o( FilePath, std::ios::in | std::ios::binary );
	if ( !o ) return E_FAIL;

	// 読み込み.
	o.seekg( SeekPoint * sizeof( T ) );
	o.read( reinterpret_cast<char*>( std::addressof( Out ) ), sizeof( T ) );

	//ファイルを閉じる
	o.close();
	return S_OK;
}

//---------------------------.
// バイナリデータをvectorで読み込み.
//---------------------------.
template<class T>
HRESULT FileManager::BinaryLoad( const char* FilePath, std::vector<T>& OutList )
{
	// 暗号化されているか取得.
#ifndef _DEBUG
	std::string fp = encrypt::GetEncryptionFilePath( FilePath );
	if ( encrypt::GetIsEncryption( fp ) ) return FileManager::EFile::EBinaryLoad( fp.c_str(), OutList );

	// 実ファイルが無い場合はアーカイブから読み込む.
	if ( FileManager::FileCheck( FilePath ) == false &&
		 encrypt::GetIsArchiveFile( FilePath ) ) return FileManager::EFile::EBinaryLoad( FilePath, OutList );
#endif

	// ファイルを開く.
	std::ifstream o( FilePath, std::ios::in | std::ios::binary );
	if ( !o ) return E_FAIL;

	// 初期化.
	OutList.clear();
	int Size = 0;

	// 読み込み.
	o.read( reinterpret_cast<char*>( &Size ), sizeof( Size ) );
	OutList.resize( Size );
	o.read( reinterpret_cast<char*>( &OutList[0] ), Size * sizeof( T ) );

	// ファイルを閉じる.
	o.close();
	return S_OK;
}

//---------------------------.
// バイナリデータでの書き込み.
//---------------------------.
template<class T>
HRESULT FileManager::BinarySave( const char* FilePath, const T& Data )
{
	// ファイルを開く.
	std::fstream i( FilePath, std::ios::out | std::ios::binary | std::ios::trunc );
	if ( !i ) {
		// 開けないためファイルディレクトリを作成する.
		FileManager::CreateFileDirectory( FilePath );

		// 書き込みなおす.
		FileManager::BinarySave<T>( FilePath, Data );
		return S_OK;
	};

	// 書き込む.
	i.write( reinterpret_cast<const char*>( std::addressof( Data ) ), sizeof( T ) );

	//ファイルを閉じる
	i.close();
	return S_OK;
}

//---------------------------.
// バイナリデータをvectorで書き込み.
//---------------------------.
template<class T>
HRESULT FileManager::BinarySave( const char* FilePath, const std::vector<T>& DataList )
{
	// ファイルを開く.
	std::ofstream i( FilePath, std::ios::out | std::ios::binary );
	if ( !i ) {
		// 開けないためファイルディレクトリを作成する.
		FileManager::CreateFileDirectory( FilePath );

		// 書き込みなおす.
		FileManager::BinarySave<T>( FilePath, DataList );
	};

	// 書き込み.
	const int& Size = static_cast<int>( DataList.size() );
	i.write( reinterpret_cast<const char*>( &Size ), sizeof( Size ) );
	i.write( reinterpret_cast<const char*>( &DataList[0] ), Size * sizeof( T ) );

	// ファイルを閉じる.
	i.close();
	return S_OK;
}

#ifndef _DEBUG
//---------------------------.
// 暗号化バイナリデータでの読み込み.
//---------------------------.
template<class T>
HRESULT FileManager::EFile::EBinaryLoad( const char* FilePath, T& Out, const int& SeekPoint )
{
	// ファイルを開く.
	if ( encrypt::GetIsEncryption( FilePath ) == false &&
		 encrypt::GetIsArchiveFile( FilePath ) == false ) return E_FAIL;
	auto rf = encrypt::GetRestoreFile( FilePath );
	if ( rf.first == nullptr ) return E_FAIL;
	encrypt::membuf mb( rf );
	std::istream i( &mb );
	if ( !i ) return E_FAIL;

	// 読み込み.
	i.seekg( SeekPoint * sizeof( T ) );
	i.read( reinterpret_cast< char* >( std::addressof( Out ) ), sizeof( T ) );

	//ファイルを閉じる
	mb.DestroyFile();
	return S_OK;
}

//---------------------------.
// バイナリデータをvectorで読み込み.
//---------------------------.
template<class T>
HRESULT FileManager::EFile::EBinaryLoad( const char* FilePath, std::vector<T>& OutList )
{
	// ファイルを開く.
	if ( encrypt::GetIsEncryption( FilePath ) == false &&
		 encrypt::GetIsArchiveFile( FilePath ) == false ) return E_FAIL;
	auto rf = encrypt::GetRestoreFile( FilePath );
	if ( rf.first == nullptr ) return E_FAIL;
	encrypt::membuf mb( rf );
	std::istream i( &mb );
	if ( !i ) return E_FAIL;

	// 初期化.
	OutList.clear();
	int Size = 0;

	// 読み込み.
	i.read( reinterpret_cast< char* >( &Size ), sizeof( Size ) );
	OutList.resize( Size );
	i.read( reinterpret_cast< char* >( &OutList[0] ), Size * sizeof( T ) );

	// ファイルを閉じる.
	mb.DestroyFile();
	return S_OK;
}
#endif
#endif