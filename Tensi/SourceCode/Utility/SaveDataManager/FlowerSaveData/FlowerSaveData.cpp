#include "FlowerSaveData.h"
#include "Obfuscate/Obfuscate.h"
#include "..\..\..\Utility\FileManager\FileManager.h"
#include "..\..\..\Utility\ThreadManager\ThreadManager.h"

namespace {
	const std::string FILE_PATH = OBF( "Data\\DataCache\\flr.bin" );
	constexpr char SAVE_THREAD_TAG[]	= "SaveData";	// 書き込みの直列実行タグ( セーブ系で共通 ).
}

CFlowerSaveData::CFlowerSaveData()
	: m_SaveFunction	()
	, m_LoadFunction	()
{
}

CFlowerSaveData::~CFlowerSaveData()
{
}

//---------------------------.
// 保存.
//---------------------------.
void CFlowerSaveData::Save()
{
	std::vector<SFlowerData> Data = m_SaveFunction();

	// ポインタは保存するとバグるためnullにする
	for ( auto& d : Data ) d.Transform.pParent = nullptr;

	// バイナリデータの保存.
	//	書き込みはワーカースレッドで行う( 同タグの直列実行で書き込み順を保証する ).
	if ( ThreadManager::GetIsAvailable() ) {
		ThreadManager::StartSequential( SAVE_THREAD_TAG, [Data = std::move( Data )]() {
			FileManager::BinarySave( FILE_PATH.c_str(), Data );
		} );
	}
	else {
		// スレッドが使えない場合( アプリ終了時など )は同期で書き込む.
		FileManager::BinarySave( FILE_PATH.c_str(), Data );
	}
}

//---------------------------.
// 読み込み.
//---------------------------.
void CFlowerSaveData::Load()
{
	if ( FileManager::FileCheck( FILE_PATH ) == false ) return;

	std::vector<SFlowerData> Data;

	// バイナリデータの読み込み.
	FileManager::BinaryLoad( FILE_PATH.c_str(), Data );

	m_LoadFunction( Data );
}
