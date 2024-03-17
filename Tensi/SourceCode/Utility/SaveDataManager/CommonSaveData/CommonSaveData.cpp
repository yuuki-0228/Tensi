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
// �ۑ�.
//---------------------------.
void CCommonSaveData::Save()
{
	// �Z�[�u�f�[�^�̍X�V
	m_SaveData.PlayerTransform = *m_pSaveData.PlayerTransform;
	m_SaveData.BallTransform = *m_pSaveData.BallTransform;
	m_SaveData.WaterFallTransform = *m_pSaveData.WaterFallTransform;
	m_SaveData.WateringCanTransform = *m_pSaveData.WateringCanTransform;
	m_SaveData.WateringCanWaterValue = *m_pSaveData.WateringCanWaterValue;

	// �|�C���^�͕ۑ�����ƃo�O�邽��null�ɂ���
	m_SaveData.PlayerTransform.pParent = nullptr;
	m_SaveData.BallTransform.pParent = nullptr;
	m_SaveData.WaterFallTransform.pParent = nullptr;
	m_SaveData.WateringCanTransform.pParent = nullptr;

	// �o�C�i���f�[�^�̕ۑ�
	FileManager::BinarySave( FILE_PATH, m_SaveData );
}

//---------------------------.
// �ǂݍ���.
//---------------------------.
void CCommonSaveData::Load()
{
	// �o�C�i���f�[�^�̓ǂݍ���
	FileManager::BinaryLoad( FILE_PATH, m_SaveData );

	// �ǂݍ��񂾃f�[�^�ŏ�����
	*m_pSaveData.PlayerTransform = m_SaveData.PlayerTransform;
	*m_pSaveData.BallTransform = m_SaveData.BallTransform;
	*m_pSaveData.WaterFallTransform = m_SaveData.WaterFallTransform;
	*m_pSaveData.WateringCanTransform = m_SaveData.WateringCanTransform;
	*m_pSaveData.WateringCanWaterValue = m_SaveData.WateringCanWaterValue;
}
