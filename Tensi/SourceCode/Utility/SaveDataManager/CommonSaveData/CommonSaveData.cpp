#include "CommonSaveData.h"
#include "..\..\FileManager\FileManager.h"

namespace {
	constexpr char FILE_PATH[] = "Data\\Parameter\\d\\m.bin";
}

CCommonSaveData::CCommonSaveData()
	: m_pSaveData	()
	, m_SaveData	()
{
}

CCommonSaveData::~CCommonSaveData()
{
}

//---------------------------.
// 保存.
//---------------------------.
void CCommonSaveData::Save()
{
	// セーブデータの更新
	m_SaveData.PlayerTransform = *m_pSaveData.PlayerTransform;
	m_SaveData.BallTransform = *m_pSaveData.BallTransform;
	m_SaveData.WaterFallTransform = *m_pSaveData.WaterFallTransform;
	m_SaveData.WateringCanTransform = *m_pSaveData.WateringCanTransform;
	m_SaveData.WateringCanWaterValue = *m_pSaveData.WateringCanWaterValue;

	// ポインタは保存するとバグるためnullにする
	m_SaveData.PlayerTransform.pParent = nullptr;
	m_SaveData.BallTransform.pParent = nullptr;
	m_SaveData.WaterFallTransform.pParent = nullptr;
	m_SaveData.WateringCanTransform.pParent = nullptr;

	// バイナリデータの保存
	FileManager::BinarySave( FILE_PATH, m_SaveData );
}

//---------------------------.
// 読み込み.
//---------------------------.
void CCommonSaveData::Load()
{
	// バイナリデータの読み込み
	FileManager::BinaryLoad( FILE_PATH, m_SaveData );

	// 読み込んだデータで初期化
	*m_pSaveData.PlayerTransform = m_SaveData.PlayerTransform;
	*m_pSaveData.BallTransform = m_SaveData.BallTransform;
	*m_pSaveData.WaterFallTransform = m_SaveData.WaterFallTransform;
	*m_pSaveData.WateringCanTransform = m_SaveData.WateringCanTransform;
	*m_pSaveData.WateringCanWaterValue = m_SaveData.WateringCanWaterValue;
}
