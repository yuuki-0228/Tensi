#include "SaveDataManager.h"
#include "CommonSaveData/CommonSaveData.h"
#include "..\..\Utility\FileManager\FileManager.h"
#include "..\..\Utility\TimeManager\TimeManager.h"

namespace {
	constexpr char FILE_PATH[]		 = "Data\\Parameter\\d\\m.bin";
	constexpr char LOGIN_FILE_PATH[] = "Data\\Parameter\\d\\l.bin";

}

SaveDataManager::SaveDataManager()
	: m_LoadFunction	()
	, m_CommonSaveData	( std::make_unique<CCommonSaveData>()	)
	, m_FlowerSaveData	( std::make_unique<CFlowerSaveData>()	)
	, m_WeedSaveData	( std::make_unique<CWeedSaveData>()		)
{
}

SaveDataManager::~SaveDataManager()
{
}

//----------------------------.
// インスタンスの取得.
//----------------------------.
SaveDataManager* SaveDataManager::GetInstance()
{
	static std::unique_ptr<SaveDataManager> pInstance = std::make_unique<SaveDataManager>();
	return pInstance.get();
}

//---------------------------.
// データの設定.
//---------------------------.
SaveData::SSetSaveDataList* SaveDataManager::SetSaveData()
{
	return GetInstance()->m_CommonSaveData->GetPointerSaveData();
}

//---------------------------.
// 保存.
//---------------------------.
void SaveDataManager::Save()
{
	SaveDataManager* pI = GetInstance();

	// セーブデータの保存.
	pI->m_CommonSaveData->Save();
	pI->m_FlowerSaveData->Save();
	pI->m_WeedSaveData->Save();
}

//---------------------------.
// 読み込み.
//---------------------------.
bool SaveDataManager::Load()
{
	SaveDataManager* pI = GetInstance();

	// 初回ログイン
	if ( FileManager::FileCheck( FILE_PATH ) == false ) return true;
	
	// セーブデータの読み込み.
	pI->m_CommonSaveData->Load();
	pI->m_FlowerSaveData->Load();
	pI->m_WeedSaveData->Load();

	// ロード時に行う関数の実行.
	for ( auto& f : pI->m_LoadFunction ) f();
	return false;
}

//---------------------------.
// ログイン情報の保存.
//---------------------------.
void SaveDataManager::LoginSave()
{
	const std::tm& Now = TimeManager::GetTime();

	FileManager::BinarySave( LOGIN_FILE_PATH, Now );
}

//---------------------------.
// ログイン情報の読み込み.
//---------------------------.
bool SaveDataManager::LoginLoad( std::tm* pDay )
{
	SaveDataManager* pI = GetInstance();

	if ( FileManager::FileCheck( LOGIN_FILE_PATH ) == false ) {
		// ログイン情報が存在しないため初回ログインとする
		return true;
	}

	// バイナリデータの読み込み.
	std::tm Day;
	FileManager::BinaryLoad( LOGIN_FILE_PATH, Day );
	if ( pDay != nullptr ) {
		*pDay = Day;
	}

	// 読み込んだ日時を元に今日ログインしたか調べる
	const std::tm& Now = TimeManager::GetTime();
	if ( TimeManager::GetIsSameDay( Now, Day ) ) {
		return false;
	}
	return true;
}

//---------------------------.
// 花のセーブデータの設定.
//---------------------------.
void SaveDataManager::SetFlowerSaveData( CFlowerSaveData::SaveFunction Save, CFlowerSaveData::LoadFunction Load )
{
	SaveDataManager* pI = GetInstance();

	pI->m_FlowerSaveData->SetSaveFunction( Save );
	pI->m_FlowerSaveData->SetLoadFunction( Load );
}

//---------------------------.
// 草のセーブデータの設定.
//---------------------------.
void SaveDataManager::SetWeedSaveData( CWeedSaveData::SaveFunction Save, CWeedSaveData::LoadFunction Load )
{
	SaveDataManager* pI = GetInstance();

	pI->m_WeedSaveData->SetSaveFunction( Save );
	pI->m_WeedSaveData->SetLoadFunction( Load );
}
