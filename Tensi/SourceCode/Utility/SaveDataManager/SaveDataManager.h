#pragma once
#include "..\..\Global.h"
#include "SaveData.h"
#include "FlowerSaveData/FlowerSaveData.h"
#include "WeedSaveData/WeedSaveData.h"

class CCommonSaveData;

/************************************************
*	セーブデータクラス.
**/
class SaveDataManager final
{
public:
	SaveDataManager();
	~SaveDataManager();

	// データの設定.
	static SaveData::SSetSaveDataList* SetSaveData();

	// 保存.
	static void Save();
	// 読み込み(true:初回起動).
	static bool Load();

	// ログイン情報の保存.
	static void LoginSave();
	// ログイン情報の読み込み(true:初ログイン)(pDay:前回のログイン日)
	static bool LoginLoad( std::tm* pDay = nullptr );

	// 花のセーブデータの設定.
	static void SetFlowerSaveData( CFlowerSaveData::SaveFunction Save, CFlowerSaveData::LoadFunction Load );
	// 草のセーブデータの設定.
	static void SetWeedSaveData( CWeedSaveData::SaveFunction Save, CWeedSaveData::LoadFunction Load );

	// ロード時に行う関数の追加.
	static void PushLoadFunction( const std::function<void()> f ) { GetInstance()->m_LoadFunction.emplace_back( f ); }

private:
	// インスタンスの取得.
	static SaveDataManager* GetInstance();

private:
	std::vector<std::function<void()>>	m_LoadFunction;		// ロード時に行う関数.

	std::unique_ptr<CCommonSaveData>	m_CommonSaveData;	// 共通のセーブデータ
	std::unique_ptr<CFlowerSaveData>	m_FlowerSaveData;	// 花のセーブデータ
	std::unique_ptr<CWeedSaveData>		m_WeedSaveData;		// 草のセーブデータ
};