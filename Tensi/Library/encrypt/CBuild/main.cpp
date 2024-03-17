#include "error.h"
#include "..\key.h"
#include "..\file.h"
#include <filesystem>
#include <fstream>
#include <memory>
#include <vector>
#include <sstream>
#include <iostream>

namespace {
#ifdef _DEBUG
	const std::string DATA_PATH = "..\\..\\..\\Data\\";
#else
	const std::string DATA_PATH = "Data\\";
#endif
}

int main() {
	fprintf( stderr, "------ Encrypt Start ------\n" );
	encrypt::Edit::RemoveResultFile( DATA_PATH );
	auto FileLoad = [&]( const std::filesystem::directory_entry& Entry )
	{
		const std::string Extension = Entry.path().extension().string();	// 拡張子.
		const std::string FilePath	= Entry.path().string();				// ファイルパス.
		const std::string FileName	= Entry.path().stem().string();			// ファイル名.

		// 拡張子が暗号化するファイルか.
		if ( Extension == "" ) {
			encrypt::Edit::CreateEncryptionFileDirectory( FilePath );
			return;
		}
		if ( SecretKey::GetFileId( Extension ) != SecretKey::ErrorId && FileName.substr( 0, 1 ) != encrypt::SKIP_FILE ) {
			// 暗号化ファイルの作成
			if ( FAILED( encrypt::Edit::CreateEncryptionFile( FilePath ) ) ) throw FilePath;
		}
		else {
			// そのままリザルトファイルにコピーを作成
			if ( FAILED( encrypt::Edit::CopyResultFile( FilePath ) ) ) throw FilePath;
		}
	};

	try {
		fprintf( stderr, "暗号化フォルダ作成しています。\n" );
		fprintf( stderr, "encrypt ->「Data」 --[?][?][?][?]-->「RData」\n" );
		std::filesystem::recursive_directory_iterator Dir_itr( DATA_PATH ), End_itr;
		std::for_each( Dir_itr, End_itr, FileLoad );
		fprintf( stderr, "暗号化フォルダ作成が終了しました。\n" );
	} catch ( const std::filesystem::filesystem_error& e ) {
		// エラーメッセージを表示.
		return ErrorMessage( "ファイルの読み込み 失敗", e.path1().string().c_str() );
	}

	fprintf( stderr, "  作成された「RData」を「Data」にリネームしてください\n" );
	fprintf( stderr, "------ Encrypt End --------\n" );
	return S_OK;
}