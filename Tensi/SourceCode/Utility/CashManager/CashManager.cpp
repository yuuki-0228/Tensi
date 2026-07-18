#include "CashManager.h"
#ifdef ENABLE_CASH
#include <encrypt/file.h>

CashManager::CashManager()
	: m_Cashs	()
{
}

CashManager::~CashManager()
{
}

//---------------------------.
// インスタンスの取得.
//---------------------------.
CashManager* CashManager::GetInstance()
{
	static std::unique_ptr<CashManager> pInstance = std::make_unique<CashManager>();
	return pInstance.get();
}

//---------------------------.
// キャッシュデータを読み込む
//---------------------------.
HRESULT CashManager::Load()
{
	CashManager* pI = GetInstance();

	std::vector<std::pair<std::string, json>> FileList;
	auto FileLoad = [&]( const std::string& EntryPath )
	{
		const std::string Extension = std::filesystem::path( EntryPath ).extension().string();	// 拡張子.
		const std::string FilePath  = EntryPath;				// ファイルパス.
		const std::string FileName  = std::filesystem::path( EntryPath ).stem().string();			// ファイル名.

		// binファイルか.
		if ( Extension != ".bin" && Extension != ".BIN"		) return;

		// キャッシュデータの追加
		pI->m_Cashs[FileName] = std::make_pair( std::any(), FilePath );

		Log::PushLogInfo( FilePath + " 読み込み : 成功" );
	};

	Log::PushLogInfo( "------ キャッシュデータ読み込み開始 ------" );
	try {
		const std::vector<std::string> Files = encrypt::EnumerateDataFiles( pI->CAHS_FILE_PATH );
		std::for_each( Files.begin(), Files.end(), FileLoad );
	} catch ( const std::filesystem::filesystem_error& e ) {
		// ファイルが見つからないエラーは無視する.
		if ( std::string( e.what() ).find( "The system cannot find the path specified" ) == std::string::npos ) {

			// エラーメッセージを表示.
			ErrorMessage( "ファイルの読み込み 失敗" );
			return E_FAIL;
		}
	}
	Log::PushLogInfo( "------ キャッシュデータ読み込み開始 ------" );
	return S_OK;
}

//---------------------------.
// キーが存在するか
//---------------------------.
bool CashManager::ContainsKey( const std::string& key )
{
	CashManager* pI = GetInstance();

	return pI->m_Cashs.find( key ) != pI->m_Cashs.end();
}

//---------------------------.
// キャッシュデータを削除
//---------------------------.
void CashManager::Remove( const std::string& key )
{
	CashManager* pI = GetInstance();

	if ( pI->m_Cashs[key].second != "" ) {
		FileManager::FileDelete( pI->m_Cashs[key].second );
	}

	pI->m_Cashs.erase( key );
}

//---------------------------.
// キャッシュデータを全て削除
//---------------------------.
void CashManager::Clear()
{
	GetInstance()->m_Cashs.clear();
}

#endif