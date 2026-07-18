#include "file.h"
#include "CBuild/error.h"
#include "key.h"
#include <filesystem>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <mutex>
#include <set>
#include <algorithm>
#include <cstring>

namespace {
	const std::string NORMAL_FILE	= "Data";	// データが入っているファイル名
	const std::string RESULT_FILE	= "RData";	// 暗号化したデータを一時的に入れるファイル名
	const std::string TMP_FILE		= "Tmp\\";	// 一時的に入れるファイル名

	// アーカイブのマジックナンバー.
	const char ARCHIVE_MAGIC[4] = { 'G', 'D', 'A', '1' };

	// アーカイブ内のエントリ情報.
	struct SArchiveEntry {
		std::string			GdaPath;	// エントリが入っているアーカイブのパス.
		unsigned long long	Offset;		// アーカイブ内のデータ位置.
		unsigned long long	Size;		// データサイズ.
	};
	std::unordered_map<std::string, SArchiveEntry>	ArchiveEntries;				// エントリ一覧.
	bool											IsArchiveMounted = false;	// マウント済みか.
	std::mutex										ArchiveMutex;				// アーカイブ操作用ミューテックス.

	// パスの正規化( '/' を '\' にする ).
	std::string NormalizePath( const std::string& FilePath ) {
		std::string Out = FilePath;
		for ( auto& c : Out ) {
			if ( c == '/' ) c = '\\';
		}
		return Out;
	}

	// ディスクから復元したファイルを取得( 前方宣言 ).
	std::pair<char*, DWORD> RestoreFileFromDisk( const std::wstring& FilePath );
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
// アーカイブをマウントする.
//---------------------------.
void encrypt::archive::Mount()
{
	std::unique_lock<std::mutex> lock( ArchiveMutex );
	if ( IsArchiveMounted ) return;
	IsArchiveMounted = true;

	// Data直下のアーカイブを全て読み込む.
	if ( std::filesystem::exists( NORMAL_FILE ) == false ) return;
	for ( const auto& Entry : std::filesystem::directory_iterator( NORMAL_FILE ) ) {
		if ( Entry.is_regular_file() == false )					continue;
		if ( Entry.path().extension().string() != ARCHIVE_EXT )	continue;

		const std::string GdaPath = Entry.path().string();
		std::ifstream File( GdaPath, std::ios::in | std::ios::binary );
		if ( !File ) continue;

		// マジックナンバーの確認.
		char Magic[sizeof( ARCHIVE_MAGIC )] = {};
		File.read( Magic, sizeof( Magic ) );
		if ( !File || memcmp( Magic, ARCHIVE_MAGIC, sizeof( Magic ) ) != 0 ) continue;

		// エントリ数の読み込み.
		unsigned int EntryCount = 0;
		File.read( reinterpret_cast<char*>( &EntryCount ), sizeof( EntryCount ) );

		// インデックスの読み込み.
		for ( unsigned int i = 0; i < EntryCount; i++ ) {
			unsigned int PathLen = 0;
			File.read( reinterpret_cast<char*>( &PathLen ), sizeof( PathLen ) );
			if ( !File || PathLen == 0 ) break;

			std::vector<char> Path( PathLen );
			File.read( Path.data(), PathLen );

			SArchiveEntry e;
			e.GdaPath = GdaPath;
			File.read( reinterpret_cast<char*>( &e.Offset ),	sizeof( e.Offset ) );
			File.read( reinterpret_cast<char*>( &e.Size ),		sizeof( e.Size ) );
			if ( !File ) break;

			// パスを復元して登録する.
			SecretKey::Restore( Path.data(), PathLen );
			ArchiveEntries[std::string( Path.data(), PathLen )] = e;
		}
	}
}

//---------------------------.
// アーカイブ内にファイルがあるか取得.
//---------------------------.
bool encrypt::archive::Exists( const std::string& FilePath )
{
	Mount();
	std::unique_lock<std::mutex> lock( ArchiveMutex );
	return ArchiveEntries.find( NormalizePath( FilePath ) ) != ArchiveEntries.end();
}

//---------------------------.
// アーカイブ内のファイルを取得.
//---------------------------.
std::pair<char*, DWORD> encrypt::archive::Load( const std::string& FilePath )
{
	Mount();

	// エントリ情報の取得.
	SArchiveEntry e;
	{
		std::unique_lock<std::mutex> lock( ArchiveMutex );
		auto itr = ArchiveEntries.find( NormalizePath( FilePath ) );
		if ( itr == ArchiveEntries.end() ) return std::make_pair( nullptr, 0 );
		e = itr->second;
	}

	// アーカイブから該当部分だけ読み込む.
	std::ifstream File( e.GdaPath, std::ios::in | std::ios::binary );
	if ( !File ) return std::make_pair( nullptr, 0 );

	char* pBuf = new char[e.Size + 1];
	pBuf[e.Size] = '\0';
	File.seekg( static_cast<std::streamoff>( e.Offset ) );
	File.read( pBuf, static_cast<std::streamsize>( e.Size ) );
	if ( !File ) {
		delete[] pBuf;
		return std::make_pair( nullptr, 0 );
	}
	return std::make_pair( pBuf, static_cast<DWORD>( e.Size ) );
}

//---------------------------.
// アーカイブ内にあるファイルか取得.
//---------------------------.
bool encrypt::GetIsArchiveFile( const std::string& FilePath )
{
	return archive::Exists( FilePath );
}

//---------------------------.
// 指定フォルダ以下のファイルを列挙する
//---------------------------.
std::vector<std::string> encrypt::EnumerateDataFiles( const std::string& DirPath )
{
	std::vector<std::string>	Out;
	std::set<std::string>		Registered;

	// 検索フォルダの正規化( 末尾の区切りを削除 ).
	std::string Dir = NormalizePath( DirPath );
	while ( Dir.empty() == false && Dir.back() == '\\' ) Dir.pop_back();
	if ( Dir.empty() ) return Out;

	// 実フォルダから列挙する.
	if ( std::filesystem::exists( Dir ) ) {
		for ( const auto& Entry : std::filesystem::recursive_directory_iterator( Dir ) ) {
			if ( Entry.is_regular_file() == false ) continue;
			const std::string p = NormalizePath( Entry.path().string() );
			if ( Registered.count( p ) ) continue;
			Registered.insert( p );
			Out.emplace_back( p );
		}
	}

	// アーカイブから列挙する.
	archive::Mount();
	{
		std::unique_lock<std::mutex> lock( ArchiveMutex );
		const std::string Prefix = Dir + "\\";
		for ( const auto& Itr : ArchiveEntries ) {
			const std::string& Key = Itr.first;
			if ( Key.compare( 0, Prefix.length(), Prefix ) != 0 ) continue;
			if ( Registered.count( Key ) ) continue;
			Registered.insert( Key );
			Out.emplace_back( Key );
		}
	}

	// 実フォルダ走査と同じ順になるように並び替える.
	std::sort( Out.begin(), Out.end() );
	return Out;
}

//---------------------------.
// 復元したファイルの取得.
//---------------------------.
std::pair<char*, DWORD> encrypt::GetRestoreFile( const std::string& FilePath )
{
	// 実ファイルが無い場合はアーカイブから取得する.
	const std::wstring wPath = encrypt::Edit::to_wString( FilePath );
	if ( ::GetFileAttributes( wPath.c_str() ) == INVALID_FILE_ATTRIBUTES &&
		 archive::Exists( FilePath ) )
	{
		auto rf = archive::Load( FilePath );

		// 暗号化ファイルのみ復元する.
		if ( rf.first != nullptr && GetIsEncryption( NormalizePath( FilePath ) ) ) {
			SecretKey::Restore( rf.first, rf.second );
		}
		return rf;
	}

	return RestoreFileFromDisk( wPath );
}
std::pair<char*, DWORD> encrypt::GetRestoreFile( const std::wstring& FilePath )
{
	// アーカイブ対応の読み込みを行うためstring版に委譲する.
	return GetRestoreFile( encrypt::Edit::to_String( FilePath ) );
}

//---------------------------.
// ディスクから復元したファイルを取得.
//---------------------------.
namespace {
std::pair<char*, DWORD> RestoreFileFromDisk( const std::wstring& FilePath )
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

	// 暗号化ファイルのみ復元する.
	const std::string sPath = NormalizePath( encrypt::Edit::to_String( FilePath ) );
	if ( encrypt::GetIsEncryption( sPath ) ) {
		SecretKey::Restore( pBuf, dwFileSize );
	}

	if ( hFile != NULL &&
		hFile != INVALID_HANDLE_VALUE )
	{
		::CloseHandle( hFile );
		hFile = NULL;
	}
	return std::make_pair( pBuf, dwFileSize );
}
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
// std::wstring を std::string に変換.
//---------------------------.
std::string encrypt::Edit::to_String( const std::wstring& s )
{
	if ( s.empty() ) return std::string();

	// 文字列を変換する.
	int			Size = WideCharToMultiByte( CP_ACP, 0, &s[0], ( int ) s.size(), NULL, 0, NULL, NULL );
	std::string	Out( Size, 0 );
	WideCharToMultiByte( CP_ACP, 0, &s[0], ( int ) s.size(), &Out[0], Size, NULL, NULL );
	return Out;
}

//---------------------------.
// アーカイブ除外リストの読み込み.
//---------------------------.
std::vector<std::string> encrypt::Edit::LoadRemoveFileList( const std::string& FilePath )
{
	std::vector<std::string> Out;

	// ファイルを開く.
	std::ifstream i( FilePath, std::ios::in | std::ios::binary );
	if ( !i ) return Out;

	// ファイル全体を読み込む.
	std::string Text( ( std::istreambuf_iterator<char>( i ) ), std::istreambuf_iterator<char>() );

	// BOMがある場合は削除する.
	if ( Text.length() >= 3 &&
		static_cast<unsigned char>( Text[0] ) == 0xEF &&
		static_cast<unsigned char>( Text[1] ) == 0xBB &&
		static_cast<unsigned char>( Text[2] ) == 0xBF )
	{
		Text.erase( 0, 3 );
	}

	// UTF-8で保存されている場合はCP932( ファイル名と同じ文字コード )に変換する.
	bool HasMultiByte = false;
	for ( const char& c : Text ) {
		if ( static_cast<unsigned char>( c ) >= 0x80 ) { HasMultiByte = true; break; }
	}
	if ( HasMultiByte && Text.empty() == false ) {
		const int wlen = MultiByteToWideChar( CP_UTF8, MB_ERR_INVALID_CHARS, &Text[0], ( int ) Text.size(), NULL, 0 );
		if ( wlen > 0 ) {
			std::wstring wText( wlen, 0 );
			MultiByteToWideChar( CP_UTF8, MB_ERR_INVALID_CHARS, &Text[0], ( int ) Text.size(), &wText[0], wlen );
			Text = to_String( wText );
		}
	}

	// 区切り文字( ',' )で区切って除外名を取得する.
	std::string Line;
	std::istringstream TextStream( Text );
	while ( std::getline( TextStream, Line ) ) {
		std::istringstream stream( Line );
		std::string Buff;
		while ( std::getline( stream, Buff, ',' ) ) {
			// 前後の空白・改行を削除する.
			while ( Buff.empty() == false &&
				( Buff.back() == '\r' || Buff.back() == ' ' || Buff.back() == '\t' ) ) Buff.pop_back();
			while ( Buff.empty() == false &&
				( Buff.front() == ' ' || Buff.front() == '\t' ) ) Buff.erase( 0, 1 );
			if ( Buff.empty() ) continue;
			Out.emplace_back( Buff );
		}
	}
	return Out;
}

//---------------------------.
// フォルダをアーカイブ( .gda )にしてリザルトファイルに作成.
//---------------------------.
int encrypt::Edit::CreateArchiveFile( const std::string& DirPath )
{
	// アーカイブに入れるファイル情報.
	struct SArchiveSource {
		std::string			Key;	// アーカイブ内のパス.
		std::vector<char>	Data;	// ファイルデータ.
	};
	std::vector<SArchiveSource> Sources;

	// フォルダ内のファイルを集める.
	for ( const auto& Entry : std::filesystem::recursive_directory_iterator( DirPath ) ) {
		if ( Entry.is_regular_file() == false ) continue;

		const std::string FilePath	= Entry.path().string();
		const std::string Extension	= Entry.path().extension().string();
		const std::string FileName	= Entry.path().stem().string();

		// ファイルの読み込み.
		std::ifstream File( FilePath, std::ios::in | std::ios::binary );
		if ( !File ) return ErrorMessage( "ファイルの読み込み失敗", FilePath );
		std::vector<char> Data( ( std::istreambuf_iterator<char>( File ) ), std::istreambuf_iterator<char>() );

		// 暗号化対象なら暗号化してパスを変換する.
		std::string EntryPath = FilePath;
		if ( SecretKey::GetFileId( Extension ) != SecretKey::ErrorId &&
			 FileName.substr( 0, 1 ) != SKIP_FILE )
		{
			if ( Data.empty() == false ) {
				SecretKey::Encryption( Data.data(), static_cast<DWORD>( Data.size() ) );
			}
			EntryPath = GetEncryptionFilePath( FilePath );
		}

		// 「Data\」から始まるパスをキーにする.
		const std::string DataDir = NORMAL_FILE + "\\";
		auto pos = EntryPath.find( DataDir );
		if ( pos != std::string::npos ) EntryPath = EntryPath.substr( pos );

		Sources.emplace_back( SArchiveSource{ EntryPath, std::move( Data ) } );
	}

	// 空フォルダの場合アーカイブを作成しない.
	if ( Sources.empty() ) return S_FALSE;

	// 出力パスの作成( RData\フォルダ名.gda ).
	std::string resultPath = DirPath + ARCHIVE_EXT;
	auto pos = resultPath.find( NORMAL_FILE );
	auto len = NORMAL_FILE.length();
	if ( pos != std::string::npos ) {
		resultPath.replace( pos, len, RESULT_FILE );
	}

	// アーカイブファイルを開く.
	CreateFileDirectory( resultPath );
	std::ofstream Out( resultPath, std::ios::out | std::ios::binary | std::ios::trunc );
	if ( !Out ) return ErrorMessage( "アーカイブの作成失敗", resultPath );

	// ヘッダの書き込み.
	const unsigned int EntryCount = static_cast<unsigned int>( Sources.size() );
	Out.write( ARCHIVE_MAGIC, sizeof( ARCHIVE_MAGIC ) );
	Out.write( reinterpret_cast<const char*>( &EntryCount ), sizeof( EntryCount ) );

	// インデックスサイズを計算してデータの開始位置を求める.
	unsigned long long Offset = sizeof( ARCHIVE_MAGIC ) + sizeof( EntryCount );
	for ( const auto& s : Sources ) {
		Offset += sizeof( unsigned int ) + s.Key.length() + sizeof( unsigned long long ) * 2;
	}

	// インデックスの書き込み( パスは暗号化する ).
	for ( const auto& s : Sources ) {
		const unsigned int			PathLen	= static_cast<unsigned int>( s.Key.length() );
		const unsigned long long	Size	= static_cast<unsigned long long>( s.Data.size() );
		std::vector<char> Path( s.Key.begin(), s.Key.end() );
		SecretKey::Encryption( Path.data(), static_cast<DWORD>( PathLen ) );

		Out.write( reinterpret_cast<const char*>( &PathLen ), sizeof( PathLen ) );
		Out.write( Path.data(), PathLen );
		Out.write( reinterpret_cast<const char*>( &Offset ), sizeof( Offset ) );
		Out.write( reinterpret_cast<const char*>( &Size ), sizeof( Size ) );
		Offset += Size;
	}

	// データの書き込み.
	for ( const auto& s : Sources ) {
		if ( s.Data.empty() ) continue;
		Out.write( s.Data.data(), s.Data.size() );
	}
	return S_OK;
}

//---------------------------.
// リザルトファイルにフォルダ・ファイルをそのままコピーを作成.
//---------------------------.
int encrypt::Edit::CopyResultTree( const std::string& Path )
{
	// リザルトファイルパスの作成.
	std::string resultPath = Path;
	auto pos = resultPath.find( NORMAL_FILE );
	auto len = NORMAL_FILE.length();
	if ( pos != std::string::npos ) {
		resultPath.replace( pos, len, RESULT_FILE );
	}

	// フォルダ・ファイルをそのままコピーする.
	CreateFileDirectory( resultPath );
	std::filesystem::copy( Path, resultPath,
		std::filesystem::copy_options::recursive | std::filesystem::copy_options::overwrite_existing );
	return S_OK;
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
