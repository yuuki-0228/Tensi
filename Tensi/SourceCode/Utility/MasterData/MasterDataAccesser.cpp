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
// 読み込み
//----------------------------.
HRESULT MasterDataAccesser::Load()
{
	MasterDataAccesser* pI = GetInstance();

	std::vector<std::pair<std::string, json>> FileList;
	auto FileLoad = [&]( const std::filesystem::directory_entry& Entry )
	{
		const std::string Extension = Entry.path().extension().string();	// 拡張子.
		const std::string FilePath = Entry.path().string();					// ファイルパス.
		const std::string FileName = Entry.path().stem().string();			// ファイル名.

		// jsonファイルか.
		if ( FilePath.find( "Container" ) != std::string::npos ) return;
		if ( Extension != ".json" && Extension != ".JSON" ) return;

		// ファイルの追加
		auto spos = FilePath.rfind( "\\" );
		auto fpos = FilePath.rfind( "\\", spos - 1 );
		std::string Container = FilePath.substr( fpos + 1, spos - fpos - 1 );
		FileList.emplace_back( std::make_pair( Container, FileManager::JsonLoad( FilePath ) ) );
	};

	try {
		std::filesystem::recursive_directory_iterator Dir_itr( DATA_PATH ), End_itr;
		std::for_each( Dir_itr, End_itr, FileLoad );
	} catch ( const std::filesystem::filesystem_error& e ) {
		// ファイルが見つからないエラーは無視する.
		if ( std::string( e.what() ).find( "指定されたパスが見つかりません。" ) == std::string::npos ) {

			// エラーメッセージを表示.
			ErrorMessage( "ファイルの読み込み 失敗" );
			return E_FAIL;
		}
	}

	pI->m_MasterContainer = MasterData::CreateCache( FileList );
	return S_OK;
}

//----------------------------.
// インスタンスの取得.
//----------------------------.
MasterDataAccesser* MasterDataAccesser::GetInstance()
{
	static std::unique_ptr<MasterDataAccesser> pInstance = std::make_unique<MasterDataAccesser>();
	return pInstance.get();
}
