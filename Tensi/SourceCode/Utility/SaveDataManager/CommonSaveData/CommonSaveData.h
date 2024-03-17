#pragma once
#include "..\SaveData.h"

/************************************************
*	共通のセーブデータクラス
*	「creator」によって自動で作成されています
*/
class CCommonSaveData final
{
public:
	CCommonSaveData();
	~CCommonSaveData();

	// 保存
	void Save();
	// 読み込み
	void Load();

	// セーブデータのアドレスリストの取得
	SaveData::SSetSaveDataList* GetPointerSaveData() { return &m_pSaveData; }

private:
	// セーブデータリスト
	struct stSaveDataList {
		STransform PlayerTransform;
		STransform BallTransform;
		STransform WaterFallTransform;
		STransform WateringCanTransform;
		float WateringCanWaterValue;
	} typedef SSaveDataList;

private:
	SaveData::SSetSaveDataList	m_pSaveData; // セーブデータのアドレスリスト
	SSaveDataList				m_SaveData;	 // セーブデータ
};
