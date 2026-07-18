#include "MasterDataAccesser.h"
#ifdef ENABLE_MASTER_DATA
#include <encrypt/file.h>

namespace {
	const std::string FILE_PATH = "Data\\Parameter\\MasterData";
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

	std::vector<std::pair<std::string, Json>> FileList;
	auto FileLoad = [&]( const std::string& EntryPath )
	{
		const std::string Extension = std::filesystem::path( EntryPath ).extension().string();	// 拡張子.
		const std::string FilePath = EntryPath;					// ファイルパス.
		const std::string FileName = std::filesystem::path( EntryPath ).stem().string();			// ファイル名.

		// jsonファイルか.
		if ( FilePath.find( "Container" ) != std::string::npos ) return;
		if ( Extension != ".json" && Extension != ".JSON" ) return;

		// ファイルの追加
		auto spos = FilePath.rfind( "\\" );
		auto fpos = FilePath.rfind( "\\", spos - 1 );
		std::string Container = FilePath.substr( fpos + 1, spos - fpos - 1 );
		FileList.emplace_back( std::make_pair( Container, FileManager::JsonLoad( FilePath ) ) );

		Log::PushLogInfo( FilePath + " 読み込み : 成功" );
	};

	Log::PushLogInfo( "------マスターデータ読み込み開始 ------" );
	try {
		const std::vector<std::string> Files = encrypt::EnumerateDataFiles( FILE_PATH );
		std::for_each( Files.begin(), Files.end(), FileLoad );
	} catch ( const std::filesystem::filesystem_error& e ) {
		// ファイルが見つからないエラーは無視する.
		if ( std::string( e.what() ).find( "The system cannot find the path specified" ) == std::string::npos ) {

			// エラーメッセージを表示.
			ErrorMessage( "ファイルの読み込み 失敗" );
			return E_FAIL;
		}
	}
	Log::PushLogInfo( "------ マスターデータ読み込み開始 ------" );

	pI->m_MasterContainer = MasterDataUtility::CreateCache( FileList );
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

#endif