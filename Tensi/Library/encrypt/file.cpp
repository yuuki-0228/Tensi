#include "file.h"
#include "CBuild/error.h"
#include "key.h"
#include <filesystem>

namespace {
	const std::string NORMAL_FILE	= "Data";	// データが入っているファイル名
	const std::string RESULT_FILE	= "RData";	// 暗号化したデータを一時的に入れるファイル名
	const std::string TMP_FILE		= "Tmp\\";	// 一時的に入れるファイル名
}

//---------------------------.
// ファイルディレクトリを作成する.
//---------------------------.
void CreateFileDirectory( const std::string& FilePath ) {
	auto fp = FilePath.rfind( "\\" );
	if ( fp != std::string::npos ) {
		std::filesystem::create_directories( FilePath.substr( 0, FilePath.rfind( "\\" ) ) );
	}
}

//---------------------------.
// 復元したファイルの取得.
//---------------------------.
std::pair<char*, DWORD> encrypt::GetRestoreFile( const std::string& FilePath )
{
	return GetRestoreFile( encrypt::Edit::to_wString( FilePath ) );
}
std::pair<char*, DWORD> encrypt::GetRestoreFile( const std::wstring& FilePath )
{
	const wchar_t*	Path = FilePath.c_str();
	char*			pBuf = NULL;

	// ファイルの読み込み
	HANDLE hFile = ::CreateFile( Path, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL );
	if ( hFile == INVALID_HANDLE_VALUE )
	{
		if ( hFile != NULL &&
			hFile != INVALID_HANDLE_VALUE )
		{
			::CloseHandle( hFile );
			hFile = NULL;
		}
		if ( pBuf != NULL )
		{
			delete[] pBuf;
			pBuf = NULL;
		}
		ErrorMessage( L"ファイルの読み込み失敗", FilePath );
		return std::make_pair( nullptr, 0 );
	}

	DWORD dwFileSize = ::GetFileSize( hFile, NULL );
	if ( dwFileSize == -1 )
	{
		if ( hFile != NULL &&
			hFile != INVALID_HANDLE_VALUE )
		{
			::CloseHandle( hFile );
			hFile = NULL;
		}
		if ( pBuf != NULL )
		{
			delete[] pBuf;
			pBuf = NULL;
		}
		ErrorMessage( L"ファイルの読み込み失敗", FilePath );
		return std::make_pair( nullptr, 0 );
	}

	pBuf = new char[dwFileSize + 1];

	DWORD dwActualRead;
	int rc = ::ReadFile( hFile, pBuf, dwFileSize, &dwActualRead, NULL );
	if ( rc == 0 ||
		dwFileSize != dwActualRead )
	{
		if ( hFile != NULL &&
			hFile != INVALID_HANDLE_VALUE )
		{
			::CloseHandle( hFile );
			hFile = NULL;
		}

		if ( pBuf != NULL )
		{
			delete[] pBuf;
			pBuf = NULL;
		}
		ErrorMessage( L"ファイルの読み込み失敗", FilePath );
		return std::make_pair( nullptr, 0 );
	}

	// 復元.
	SecretKey::Restore( pBuf, dwFileSize );

	if ( hFile != NULL &&
		hFile != INVALID_HANDLE_VALUE )
	{
		::CloseHandle( hFile );
		hFile = NULL;
	}
	return std::make_pair( pBuf, dwFileSize );
}

//---------------------------.
// 暗号化したファイル名を取得
//---------------------------.
std::string encrypt::GetEncryptionFilePath( const std::string& NormalFilePath )
{
	std::string efp = NormalFilePath;

	auto spos = efp.rfind( "\\" );
	if ( spos != std::string::npos ) {
		if ( efp.substr( spos + 1, 1 ) == SKIP_FILE ) return NormalFilePath;
		efp.insert( spos + 1, ENCRYPTION_FILE );
	}

	auto epos = efp.rfind( "." );
	if ( epos != std::string::npos ) {
		auto ext = efp.substr( epos + 1 );
		auto eid = SecretKey::GetFileId( ext );
		if ( eid == SecretKey::ErrorId ) return NormalFilePath;
		efp.insert( epos, eid );

		auto path = efp.substr( 0, epos + eid.length() );
		efp = path + ".bin";
	}

	return efp;
}

//---------------------------.
// 暗号化されているか取得.
//---------------------------.
bool encrypt::GetIsEncryption( const std::string& FilePath )
{
	auto spos = FilePath.rfind( "\\" );
	auto epos = FilePath.rfind( "." );
	if ( spos != std::string::npos && epos != std::string::npos ) {
		if ( FilePath.substr( spos + 1, 1 ) == ENCRYPTION_FILE &&
			 FilePath.substr( epos + 1 )    == "bin" )
		{
			return true;
		}
	}
	return false;
}

//---------------------------.
// 暗号化前の拡張しを取得
//---------------------------.
std::string encrypt::GetExtension( const std::string& FilePath )
{
	auto epos = FilePath.rfind( "." );
	if ( epos != std::string::npos ) {
		auto eid = FilePath.substr( epos - 2, 2 );
		return SecretKey::GetFileExt( eid );
	}

	return SecretKey::ErrorId;
}

//---------------------------.
// 一時フォルダに復元ファイルを作成.
//---------------------------.
std::string encrypt::CreateTmpEncryptionFile( const std::string& FilePath )
{
	auto rf = GetRestoreFile( FilePath );

	// 一時ファイルパスを作成
	std::string tfp = FilePath;
	auto dpos = tfp.find( "Data\\" );
	if ( dpos != std::string::npos ) {
		tfp.insert( dpos, TMP_FILE );
	}

	// 復元ファイルの作成
	std::wstring wtfp = encrypt::Edit::to_wString( tfp );
	CreateFileDirectory( tfp );
	HANDLE hFile = ::CreateFile( wtfp.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL );
	::WriteFile( hFile, rf.first, rf.second, NULL, NULL );

	if ( hFile != NULL &&
		hFile != INVALID_HANDLE_VALUE )
	{
		::CloseHandle( hFile );
		hFile = NULL;
	}
	if ( rf.first != NULL )
	{
		delete[]  rf.first;
		rf.first = NULL;
	}
	return tfp;
}

//---------------------------.
// 一時フォルダを削除.
//---------------------------.
void encrypt::RemoveTmpFile()
{
	std::filesystem::remove_all( "Data\\Tmp" );
}

//---------------------------.
// リザルトファイルを空にする
//---------------------------.
void encrypt::Edit::RemoveResultFile( const std::string& FilePath )
{
	std::string f = FilePath;
	auto pos = f.find( NORMAL_FILE );
	auto len = NORMAL_FILE.length();
	if ( pos != std::string::npos ) {
		f.replace( pos, len, RESULT_FILE );
	}
	std::filesystem::remove_all( f );
}

//---------------------------.
// 暗号化ファイルの作成.
//---------------------------.
int encrypt::Edit::CreateEncryptionFile( const std::string& FilePath )
{
	std::wstring	w	 = encrypt::Edit::to_wString( FilePath );
	const wchar_t*	Path = w.c_str();
	char*			pBuf = NULL;

	// ファイルの読み込み
	HANDLE hFile = ::CreateFile( Path, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL );
	if ( hFile == INVALID_HANDLE_VALUE )
	{
		if ( hFile != NULL &&
			hFile != INVALID_HANDLE_VALUE )
		{
			::CloseHandle( hFile );
			hFile = NULL;
		}
		if ( pBuf != NULL )
		{
			delete[] pBuf;
			pBuf = NULL;
		}
		return ErrorMessage( "ファイルの読み込み失敗", FilePath );
	}

	DWORD dwFileSize = ::GetFileSize( hFile, NULL );
	if ( dwFileSize == -1 )
	{
		if ( hFile != NULL &&
			hFile != INVALID_HANDLE_VALUE )
		{
			::CloseHandle( hFile );
			hFile = NULL;
		}
		if ( pBuf != NULL )
		{
			delete[] pBuf;
			pBuf = NULL;
		}
		return ErrorMessage( "ファイルの読み込み失敗", FilePath );
	}

	pBuf = new char[dwFileSize + 1];

	DWORD dwActualRead;
	int rc = ::ReadFile( hFile, pBuf, dwFileSize, &dwActualRead, NULL );
	if ( rc == 0 ||
		dwFileSize != dwActualRead )
	{
		if ( hFile != NULL &&
			hFile != INVALID_HANDLE_VALUE )
		{
			::CloseHandle( hFile );
			hFile = NULL;
		}

		if ( pBuf != NULL )
		{
			delete[] pBuf;
			pBuf = NULL;
		}
		return ErrorMessage( "ファイルの読み込み失敗", FilePath );
	}

	// 暗号化.
	SecretKey::Encryption( pBuf, dwFileSize );

	// リザルトファイルパスの作成
	std::string resultPath = GetEncryptionFilePath( FilePath );
	auto pos = resultPath.find( NORMAL_FILE );
	auto len = NORMAL_FILE.length();
	if ( pos != std::string::npos ) {
		resultPath.replace( pos, len, RESULT_FILE );
	}

	// リザルトファイルの作成
	std::wstring	rw	  = encrypt::Edit::to_wString( resultPath );
	const wchar_t*	rPath = rw.c_str();
	CreateFileDirectory( resultPath );
	HANDLE hFile2 = ::CreateFile( rPath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL );
	::WriteFile( hFile2, pBuf, dwFileSize, NULL, NULL );

	// 破棄.
	if ( hFile != NULL &&
		hFile != INVALID_HANDLE_VALUE )
	{
		::CloseHandle( hFile );
		hFile = NULL;
	}
	if ( hFile2 != NULL &&
		hFile2 != INVALID_HANDLE_VALUE )
	{
		::CloseHandle( hFile2 );
		hFile2 = NULL;
	}
	if ( pBuf != NULL )
	{
		delete[] pBuf;
		pBuf = NULL;
	}
	return S_OK;
}

//---------------------------.
// リザルトファイルの作成.
//---------------------------.
int encrypt::Edit::CopyResultFile( const std::string& FilePath )
{
	// リザルトファイルパスの作成
	std::string resultPath = FilePath;
	std::string t = "Data";
	auto pos = resultPath.find( t );
	auto len = t.length();
	if ( pos != std::string::npos ) {
		resultPath.replace( pos, len, RESULT_FILE );
	}

	// リザルトファイルの作成
	CreateFileDirectory( resultPath );
	std::filesystem::copy( FilePath, resultPath );
	return S_OK;
}

//---------------------------.
// std::string を std::wstring に変換.
//---------------------------.
std::wstring encrypt::Edit::to_wString( const std::string& s )
{
	// 文字列を変換する.
	int				Size = MultiByteToWideChar( CP_ACP, 0, &s[0], ( int ) s.size(), NULL, 0 );
	std::wstring	wOut( Size, 0 );
	MultiByteToWideChar( CP_ACP, 0, &s[0], ( int ) s.size(), &wOut[0], Size );
	return wOut;
}

//---------------------------.
// ファイルディレクトリを作成する.
//---------------------------.
void encrypt::Edit::CreateEncryptionFileDirectory( const std::string& FilePath )
{
	std::string resultPath = FilePath;
	std::string t = "Data";
	auto pos = resultPath.find( t );
	auto len = t.length();
	if ( pos != std::string::npos ) {
		resultPath.replace( pos, len, RESULT_FILE );
	}

	std::filesystem::create_directories( resultPath );
}

//---------------------------.
// ファイルの破棄.
//---------------------------.
void encrypt::membuf::DestroyFile()
{
	if ( pbase != NULL )
	{
		delete[] pbase;
		pbase = NULL;
	}
}
