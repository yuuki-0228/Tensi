#include "MasterDataAccesser.h"

namespace {
	const std::string DATA_PATH = "..\\..\\..\\Data\\Parameter\\MasterData";
}

MasterDataAccesser::MasterDataAccesser()
{
}

MasterDataAccesser::~MasterDataAccesser()
{
}

//----------------------------.
// �ǂݍ���
//----------------------------.
HRESULT MasterDataAccesser::Load()
{
	MasterDataAccesser* pI = GetInstance();

	std::vector<std::pair<std::string, json>> FileList;
	auto FileLoad = [&]( const std::filesystem::directory_entry& Entry )
	{
		const std::string Extension = Entry.path().extension().string();	// �g���q.
		const std::string FilePath = Entry.path().string();					// �t�@�C���p�X.
		const std::string FileName = Entry.path().stem().string();			// �t�@�C����.

		// json�t�@�C����.
		if ( FilePath.find( "Container" ) != std::string::npos ) return;
		if ( Extension != ".json" && Extension != ".JSON" ) return;

		// �t�@�C���̒ǉ�
		auto spos = FilePath.rfind( "\\" );
		auto fpos = FilePath.rfind( "\\", spos - 1 );
		std::string Container = FilePath.substr( fpos + 1, spos - fpos - 1 );
		FileList.emplace_back( std::make_pair( Container, FileManager::JsonLoad( FilePath ) ) );
	};

	try {
		std::filesystem::recursive_directory_iterator Dir_itr( DATA_PATH ), End_itr;
		std::for_each( Dir_itr, End_itr, FileLoad );
	} catch ( const std::filesystem::filesystem_error& e ) {
		// �t�@�C����������Ȃ��G���[�͖�������.
		if ( std::string( e.what() ).find( "�w�肳�ꂽ�p�X��������܂���B" ) == std::string::npos ) {

			// �G���[���b�Z�[�W��\��.
			ErrorMessage( "�t�@�C���̓ǂݍ��� ���s" );
			return E_FAIL;
		}
	}

	pI->m_MasterContainer = MasterData::CreateCache( FileList );
	return S_OK;
}

//----------------------------.
// �C���X�^���X�̎擾.
//----------------------------.
MasterDataAccesser* MasterDataAccesser::GetInstance()
{
	static std::unique_ptr<MasterDataAccesser> pInstance = std::make_unique<MasterDataAccesser>();
	return pInstance.get();
}
