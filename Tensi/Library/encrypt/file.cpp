#include "file.h"
#include "CBuild/error.h"
#include "key.h"
#include <filesystem>

namespace {
	const std::string NORMAL_FILE	= "Data";	// �f�[�^�������Ă���t�@�C����
	const std::string RESULT_FILE	= "RData";	// �Í��������f�[�^���ꎞ�I�ɓ����t�@�C����
	const std::string TMP_FILE		= "Tmp\\";	// �ꎞ�I�ɓ����t�@�C����
}

//---------------------------.
// �t�@�C���f�B���N�g�����쐬����.
//---------------------------.
void CreateFileDirectory( const std::string& FilePath ) {
	auto fp = FilePath.rfind( "\\" );
	if ( fp != std::string::npos ) {
		std::filesystem::create_directories( FilePath.substr( 0, FilePath.rfind( "\\" ) ) );
	}
}

//---------------------------.
// ���������t�@�C���̎擾.
//---------------------------.
std::pair<char*, DWORD> encrypt::GetRestoreFile( const std::string& FilePath )
{
	return GetRestoreFile( encrypt::Edit::to_wString( FilePath ) );
}
std::pair<char*, DWORD> encrypt::GetRestoreFile( const std::wstring& FilePath )
{
	const wchar_t*	Path = FilePath.c_str();
	char*			pBuf = NULL;

	// �t�@�C���̓ǂݍ���
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
		ErrorMessage( L"�t�@�C���̓ǂݍ��ݎ��s", FilePath );
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
		ErrorMessage( L"�t�@�C���̓ǂݍ��ݎ��s", FilePath );
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
		ErrorMessage( L"�t�@�C���̓ǂݍ��ݎ��s", FilePath );
		return std::make_pair( nullptr, 0 );
	}

	// ����.
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
// �Í��������t�@�C�������擾
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
// �Í�������Ă��邩�擾.
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
// �Í����O�̊g�������擾
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
// �ꎞ�t�H���_�ɕ����t�@�C�����쐬.
//---------------------------.
std::string encrypt::CreateTmpEncryptionFile( const std::string& FilePath )
{
	auto rf = GetRestoreFile( FilePath );

	// �ꎞ�t�@�C���p�X���쐬
	std::string tfp = FilePath;
	auto dpos = tfp.find( "Data\\" );
	if ( dpos != std::string::npos ) {
		tfp.insert( dpos, TMP_FILE );
	}

	// �����t�@�C���̍쐬
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
// �ꎞ�t�H���_���폜.
//---------------------------.
void encrypt::RemoveTmpFile()
{
	std::filesystem::remove_all( "Data\\Tmp" );
}

//---------------------------.
// ���U���g�t�@�C������ɂ���
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
// �Í����t�@�C���̍쐬.
//---------------------------.
int encrypt::Edit::CreateEncryptionFile( const std::string& FilePath )
{
	std::wstring	w	 = encrypt::Edit::to_wString( FilePath );
	const wchar_t*	Path = w.c_str();
	char*			pBuf = NULL;

	// �t�@�C���̓ǂݍ���
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
		return ErrorMessage( "�t�@�C���̓ǂݍ��ݎ��s", FilePath );
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
		return ErrorMessage( "�t�@�C���̓ǂݍ��ݎ��s", FilePath );
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
		return ErrorMessage( "�t�@�C���̓ǂݍ��ݎ��s", FilePath );
	}

	// �Í���.
	SecretKey::Encryption( pBuf, dwFileSize );

	// ���U���g�t�@�C���p�X�̍쐬
	std::string resultPath = GetEncryptionFilePath( FilePath );
	auto pos = resultPath.find( NORMAL_FILE );
	auto len = NORMAL_FILE.length();
	if ( pos != std::string::npos ) {
		resultPath.replace( pos, len, RESULT_FILE );
	}

	// ���U���g�t�@�C���̍쐬
	std::wstring	rw	  = encrypt::Edit::to_wString( resultPath );
	const wchar_t*	rPath = rw.c_str();
	CreateFileDirectory( resultPath );
	HANDLE hFile2 = ::CreateFile( rPath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL );
	::WriteFile( hFile2, pBuf, dwFileSize, NULL, NULL );

	// �j��.
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
// ���U���g�t�@�C���̍쐬.
//---------------------------.
int encrypt::Edit::CopyResultFile( const std::string& FilePath )
{
	// ���U���g�t�@�C���p�X�̍쐬
	std::string resultPath = FilePath;
	std::string t = "Data";
	auto pos = resultPath.find( t );
	auto len = t.length();
	if ( pos != std::string::npos ) {
		resultPath.replace( pos, len, RESULT_FILE );
	}

	// ���U���g�t�@�C���̍쐬
	CreateFileDirectory( resultPath );
	std::filesystem::copy( FilePath, resultPath );
	return S_OK;
}

//---------------------------.
// std::string �� std::wstring �ɕϊ�.
//---------------------------.
std::wstring encrypt::Edit::to_wString( const std::string& s )
{
	// �������ϊ�����.
	int				Size = MultiByteToWideChar( CP_ACP, 0, &s[0], ( int ) s.size(), NULL, 0 );
	std::wstring	wOut( Size, 0 );
	MultiByteToWideChar( CP_ACP, 0, &s[0], ( int ) s.size(), &wOut[0], Size );
	return wOut;
}

//---------------------------.
// �t�@�C���f�B���N�g�����쐬����.
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
// �t�@�C���̔j��.
//---------------------------.
void encrypt::membuf::DestroyFile()
{
	if ( pbase != NULL )
	{
		delete[] pbase;
		pbase = NULL;
	}
}
