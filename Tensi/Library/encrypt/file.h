#pragma once
#include <Windows.h>
#include <string>
#include <streambuf>
#include <utility>
#include <vector>

namespace encrypt {
	const std::string ENCRYPTION_FILE	= "#";	// 暗号化しているファイル装飾
	const std::string SKIP_FILE			= "$";	// 暗号化をしないファイル装飾

	const std::string ARCHIVE_EXT		= ".gda";	// アーカイブの拡張子.

	// アーカイブ(.gda)関係.
	//	Data直下にある「フォルダ名.gda」をマウントして中のファイルを取得する
	namespace archive {
		// アーカイブをマウントする( 初回のみ読み込み、以降は何もしない ).
		void Mount();

		// アーカイブ内にファイルがあるか取得.
		bool Exists( const std::string& FilePath );

		// アーカイブ内のファイルを取得( 復元はしない ).
		std::pair<char*, DWORD> Load( const std::string& FilePath );
	}

	// アーカイブ内にあるファイルか取得.
	bool GetIsArchiveFile( const std::string& FilePath );

	// 指定フォルダ以下のファイルを列挙する
	//	実フォルダとアーカイブの両方から列挙する( 重複は実フォルダを優先 )
	std::vector<std::string> EnumerateDataFiles( const std::string& DirPath );

	// 復元したファイルの取得.
	//	実ファイルが無い場合はアーカイブから取得する
	//	暗号化ファイル( #○○.bin )の場合のみ復元する
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

		// アーカイブ除外リスト( remove_file.txt )の読み込み.
		std::vector<std::string> LoadRemoveFileList( const std::string& FilePath );

		// フォルダをアーカイブ( .gda )にしてリザルトファイルに作成.
		int CreateArchiveFile( const std::string& DirPath );

		// リザルトファイルにフォルダ・ファイルをそのままコピーを作成.
		int CopyResultTree( const std::string& Path );

		// std::string を std::wstring に変換.
		std::wstring to_wString( const std::string& s );

		// std::wstring を std::string に変換.
		std::string to_String( const std::wstring& s );

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