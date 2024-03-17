#pragma once
#include "..\SaveData.h"

/************************************************
*	���ʂ̃Z�[�u�f�[�^�N���X
*	�ucreator�v�ɂ���Ď����ō쐬����Ă��܂�
*/
class CCommonSaveData final
{
public:
	CCommonSaveData();
	~CCommonSaveData();

	// �ۑ�
	void Save();
	// �ǂݍ���
	void Load();

	// �Z�[�u�f�[�^�̃A�h���X���X�g�̎擾
	SaveData::SSetSaveDataList* GetPointerSaveData() { return &m_pSaveData; }

private:
	// �Z�[�u�f�[�^���X�g
	struct stSaveDataList {
		STransform PlayerTransform;
		STransform BallTransform;
		STransform WaterFallTransform;
		STransform WateringCanTransform;
		float WateringCanWaterValue;
	} typedef SSaveDataList;

private:
	SaveData::SSetSaveDataList	m_pSaveData; // �Z�[�u�f�[�^�̃A�h���X���X�g
	SSaveDataList				m_SaveData;	 // �Z�[�u�f�[�^
};
