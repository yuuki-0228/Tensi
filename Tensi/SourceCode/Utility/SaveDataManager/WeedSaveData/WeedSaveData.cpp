#include "WeedSaveData.h"
#include "..\..\..\Utility\FileManager\FileManager.h"
#include "..\..\..\Utility\ThreadManager\ThreadManager.h"

namespace {
	constexpr char FILE_PATH[]			= "Data\\DataCache\\wed.bin";
	constexpr char SAVE_THREAD_TAG[]	= "SaveData";	// 書き込みの直列実行タグ( セーブ系で共通 ).
}

CWeedSaveData::CWeedSaveData()
	: m_SaveFunction	()
	, m_LoadFunction	()
{
}

CWeedSaveData::~CWeedSaveData()
{
}

//---------------------------.
// 保存.
//---------------------------.
void CWeedSaveData::Save()
{
	std::vector<SWeedData> Data = m_SaveFunction();

	// ポインタは保存するとバグるためnullにする
	for ( auto& s : Data ) s.Transform.pParent = nullptr;

	// バイナリデータの保存.
	//	書き込みはワーカースレッドで行う( 同タグの直列実行で書き込み順を保証する ).
	if ( ThreadManager::GetIsAvailable() ) {
		ThreadManager::StartSequential( SAVE_THREAD_TAG, [Data = std::move( Data )]() {
			FileManager::BinarySave( FILE_PATH, Data );
		} );
	}
	else {
		// スレッドが使えない場合( アプリ終了時など )は同期で書き込む.
		FileManager::BinarySave( FILE_PATH, Data );
	}
}

//---------------------------.
// 読み込み.
//---------------------------.
void CWeedSaveData::Load()
{
	if ( FileManager::FileCheck( FILE_PATH ) == false ) return;

	std::vector<SWeedData> Data;

	// バイナリデータの読み込み.
	FileManager::BinaryLoad( FILE_PATH, Data );

	m_LoadFunction( Data );
}
