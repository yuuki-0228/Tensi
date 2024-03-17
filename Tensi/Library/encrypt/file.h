#pragma once
#include <Windows.h>
#include <string>
#include <streambuf>
#include <utility>

namespace encrypt {
	const std::string ENCRYPTION_FILE	= "#";	// 暗号化しているファイル装飾
	const std::string SKIP_FILE			= "$";	// 暗号化をしないファイル装飾

	// 復元したファイルの取得.
	std::pair<char*, DWORD> GetRestoreFile( const std::string& FilePath );
	std::pair<char*, DWORD> GetRestoreFile( const std::wstring& FilePath );

	// 暗号化したファイル名を取得
	//	暗号化していないファイルの場合そのまま返す
	std::string  GetEncryptionFilePath( const std::string&  NormalFilePath );

	// 暗号化されているか取得.
	bool GetIsEncryption( const std::string& FilePath );

	// 暗号化前の拡張しを取得
	std::string GetExtension( const std::string& FilePath );

	// 一時フォルダに復元ファイルを作成.
	std::string CreateTmpEncryptionFile( const std::string& FilePath );

	// 一時フォルダを削除.
	void RemoveTmpFile();

	// CBuildプログラム用
	namespace Edit {
		// リザルトファイルを空にする
		void RemoveResultFile( const std::string& FilePath );

		// 暗号化ファイルの作成.
		int CreateEncryptionFile( const std::string& FilePath );

		// リザルトファイルにコピーを作成.
		int CopyResultFile( const std::string& FilePath );

		// std::string を std::wstring に変換.
		std::wstring to_wString( const std::string& s );

		// ファイルディレクトリを作成する.
		void CreateEncryptionFileDirectory( const std::string& FilePath );
	}

	// std::istream変換用
	struct membuf : std::streambuf {
		membuf() {
			pbase = nullptr;
			this->setg( pbase, pbase, pbase );
		}
		membuf( std::pair<char*, DWORD> rf ) {
			pbase = rf.first;
			this->setg( pbase, pbase, pbase + rf.second );
		}
		membuf( char* base, std::ptrdiff_t n ) {
			pbase = base;
			this->setg( pbase, pbase, pbase + n );
		}
		~membuf() {
			DestroyFile();
		}

		// ファイルの破棄.
		void DestroyFile();
	private:
		char* pbase;
	};
}