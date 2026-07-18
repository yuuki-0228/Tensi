#include "error.h"
#include "..\key.h"
#include "..\file.h"
#include <filesystem>
#include <fstream>
#include <memory>
#include <vector>
#include <sstream>
#include <iostream>
#include <algorithm>

namespace {
#ifdef _DEBUG
	const std::string DATA_PATH			= "..\\..\\..\\Data\\";
	const std::string REMOVE_FILE_PATH	= "..\\remove_file.txt";
#else
	const std::string DATA_PATH			= "Data\\";
	const std::string REMOVE_FILE_PATH	= "Library\\encrypt\\remove_file.txt";
#endif
}

int main() {
	fprintf( stderr, "------ Encrypt Start ------\n" );
	encrypt::Edit::RemoveResultFile( DATA_PATH );

	// アーカイブ除外リストの読み込み.
	const std::vector<std::string> RemoveList = encrypt::Edit::LoadRemoveFileList( REMOVE_FILE_PATH );

	try {
		fprintf( stderr, "暗号化アーカイブを作成しています。\n" );

		for ( const auto& Entry : std::filesystem::directory_iterator( DATA_PATH ) ) {
			const std::string FilePath = Entry.path().string();
			const std::string FileName = Entry.path().filename().string();

			// Debugフォルダは開発専用のためリリースデータに含めない.
			if ( FileName == "Debug" ) {
				fprintf( stderr, "  ignore  : %s\n", FileName.c_str() );
				continue;
			}

			// 除外リストにあるものとData直下のファイルはそのままコピーする.
			const bool IsRemove = std::find( RemoveList.begin(), RemoveList.end(), FileName ) != RemoveList.end();
			if ( IsRemove || Entry.is_regular_file() ) {
				if ( FAILED( encrypt::Edit::CopyResultTree( FilePath ) ) ) throw FilePath;
				fprintf( stderr, "  copy    : %s\n", FileName.c_str() );
				continue;
			}

			// フォルダを暗号化してアーカイブにする.
			const int hr = encrypt::Edit::CreateArchiveFile( FilePath );
			if ( FAILED( hr ) ) throw FilePath;
			if ( hr == S_FALSE ) {
				fprintf( stderr, "  skip    : %s ( 空フォルダ )\n", FileName.c_str() );
			}
			else {
				fprintf( stderr, "  archive : %s%s\n", FileName.c_str(), encrypt::ARCHIVE_EXT.c_str() );
			}
		}
		fprintf( stderr, "暗号化アーカイブ作成が終了しました。\n" );
	} catch ( const std::filesystem::filesystem_error& e ) {
		// エラーメッセージを表示.
		return ErrorMessage( "ファイルの読み込み 失敗", e.path1().string().c_str() );
	} catch ( const std::string& Path ) {
		// エラーメッセージを表示.
		return ErrorMessage( "ファイルの読み込み 失敗", Path );
	}

	fprintf( stderr, "  作成された「RData」を「Data」にリネームしてください\n" );
	fprintf( stderr, "------ Encrypt End --------\n" );
	return S_OK;
}
