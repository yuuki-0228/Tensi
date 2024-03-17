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
		const std::string Extension = Entry.path().extension().string();	// �g���q.
		const std::string FilePath	= Entry.path().string();				// �t�@�C���p�X.
		const std::string FileName	= Entry.path().stem().string();			// �t�@�C����.

		// �g���q���Í�������t�@�C����.
		if ( Extension == "" ) {
			encrypt::Edit::CreateEncryptionFileDirectory( FilePath );
			return;
		}
		if ( SecretKey::GetFileId( Extension ) != SecretKey::ErrorId && FileName.substr( 0, 1 ) != encrypt::SKIP_FILE ) {
			// �Í����t�@�C���̍쐬
			if ( FAILED( encrypt::Edit::CreateEncryptionFile( FilePath ) ) ) throw FilePath;
		}
		else {
			// ���̂܂܃��U���g�t�@�C���ɃR�s�[���쐬
			if ( FAILED( encrypt::Edit::CopyResultFile( FilePath ) ) ) throw FilePath;
		}
	};

	try {
		fprintf( stderr, "�Í����t�H���_�쐬���Ă��܂��B\n" );
		fprintf( stderr, "encrypt ->�uData�v --[?][?][?][?]-->�uRData�v\n" );
		std::filesystem::recursive_directory_iterator Dir_itr( DATA_PATH ), End_itr;
		std::for_each( Dir_itr, End_itr, FileLoad );
		fprintf( stderr, "�Í����t�H���_�쐬���I�����܂����B\n" );
	} catch ( const std::filesystem::filesystem_error& e ) {
		// �G���[���b�Z�[�W��\��.
		return ErrorMessage( "�t�@�C���̓ǂݍ��� ���s", e.path1().string().c_str() );
	}

	fprintf( stderr, "  �쐬���ꂽ�uRData�v���uData�v�Ƀ��l�[�����Ă�������\n" );
	fprintf( stderr, "------ Encrypt End --------\n" );
	return S_OK;
}