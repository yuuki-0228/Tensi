#pragma once
#include <Windows.h>
#include <string>
#include <streambuf>
#include <utility>

namespace encrypt {
	const std::string ENCRYPTION_FILE	= "#";	// �Í������Ă���t�@�C������
	const std::string SKIP_FILE			= "$";	// �Í��������Ȃ��t�@�C������

	// ���������t�@�C���̎擾.
	std::pair<char*, DWORD> GetRestoreFile( const std::string& FilePath );
	std::pair<char*, DWORD> GetRestoreFile( const std::wstring& FilePath );

	// �Í��������t�@�C�������擾
	//	�Í������Ă��Ȃ��t�@�C���̏ꍇ���̂܂ܕԂ�
	std::string  GetEncryptionFilePath( const std::string&  NormalFilePath );

	// �Í�������Ă��邩�擾.
	bool GetIsEncryption( const std::string& FilePath );

	// �Í����O�̊g�������擾
	std::string GetExtension( const std::string& FilePath );

	// �ꎞ�t�H���_�ɕ����t�@�C�����쐬.
	std::string CreateTmpEncryptionFile( const std::string& FilePath );

	// �ꎞ�t�H���_���폜.
	void RemoveTmpFile();

	// CBuild�v���O�����p
	namespace Edit {
		// ���U���g�t�@�C������ɂ���
		void RemoveResultFile( const std::string& FilePath );

		// �Í����t�@�C���̍쐬.
		int CreateEncryptionFile( const std::string& FilePath );

		// ���U���g�t�@�C���ɃR�s�[���쐬.
		int CopyResultFile( const std::string& FilePath );

		// std::string �� std::wstring �ɕϊ�.
		std::wstring to_wString( const std::string& s );

		// �t�@�C���f�B���N�g�����쐬����.
		void CreateEncryptionFileDirectory( const std::string& FilePath );
	}

	// std::istream�ϊ��p
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

		// �t�@�C���̔j��.
		void DestroyFile();
	private:
		char* pbase;
	};
}