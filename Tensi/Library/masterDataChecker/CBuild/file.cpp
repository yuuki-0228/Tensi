#include "file.h"
#include "error.h"

//----------------------------.
// �e�L�X�g�t�@�C���̓ǂݍ���.
//----------------------------.
std::vector<std::string> FileManager::TextLoad( const std::string& FilePath )
{
	std::vector<std::string> OutList;

	// �t�@�C�����J��.
	std::fstream i( FilePath );
	if ( !i ) return OutList;

	// ��s�Âǂݍ���.
	std::string Line;
	while ( std::getline( i, Line ) ) {
		// �������ꎞ�I�Ɋi�[����.
		std::string Buff = "";
		std::istringstream stream( Line );
		OutList.emplace_back( Line );
	}

	// �t�@�C�������.
	i.close();
	return OutList;
}

//---------------------------.
// �e�L�X�g�t�@�C���̏�������.
//---------------------------.
bool FileManager::TextSave( const std::string& FilePath, const std::string& Data )
{
	// �t�@�C�����J��.
	std::ofstream o( FilePath, std::ios::trunc );
	if ( !o ) {
		ErrorMessage( "�t�@�C���̏������� ���s", FilePath );
		return false;
	}

	// ��������.
	o << Data;

	// �t�@�C�������.
	o.close();
	return true;
}

//---------------------------.
// json�t�@�C���̓ǂݍ���.
//---------------------------.
json FileManager::JsonLoad( const std::string& FilePath ) {
	// �t�@�C�����J��.
	json j;
	std::ifstream i( FilePath );
	if ( !i ) {
		ErrorMessage( "json�t�@�C���̓ǂݍ��� ���s", FilePath );
		return j;
	}

	// json�^�ɕύX.
	i >> j;

	// �t�@�C�������.
	i.close();
	return j;
}

//---------------------------.
// ����Ȃ��t�@�C���f�B���N�g�����쐬����.
//---------------------------.
void FileManager::CreateFileDirectory( const std::string& FilePath )
{
	auto fp = FilePath.rfind( "\\" );
	if ( fp != std::string::npos ) {
		std::filesystem::create_directories( FilePath.substr( 0, FilePath.rfind( "\\" ) ) );
	}
}
