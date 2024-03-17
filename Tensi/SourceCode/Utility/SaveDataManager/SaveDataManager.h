#pragma once
#include "..\..\Global.h"
#include "SaveData.h"
#include "FlowerSaveData/FlowerSaveData.h"
#include "WeedSaveData/WeedSaveData.h"

class CCommonSaveData;

/************************************************
*	�Z�[�u�f�[�^�N���X.
**/
class SaveDataManager final
{
public:
	SaveDataManager();
	~SaveDataManager();

	// �f�[�^�̐ݒ�.
	static SaveData::SSetSaveDataList* SetSaveData();

	// �ۑ�.
	static void Save();
	// �ǂݍ���(true:����N��).
	static bool Load();

	// ���O�C�����̕ۑ�.
	static void LoginSave();
	// ���O�C�����̓ǂݍ���(true:�����O�C��)(pDay:�O��̃��O�C����)
	static bool LoginLoad( std::tm* pDay = nullptr );

	// �Ԃ̃Z�[�u�f�[�^�̐ݒ�.
	static void SetFlowerSaveData( CFlowerSaveData::SaveFunction Save, CFlowerSaveData::LoadFunction Load );
	// ���̃Z�[�u�f�[�^�̐ݒ�.
	static void SetWeedSaveData( CWeedSaveData::SaveFunction Save, CWeedSaveData::LoadFunction Load );

	// ���[�h���ɍs���֐��̒ǉ�.
	static void PushLoadFunction( const std::function<void()> f ) { GetInstance()->m_LoadFunction.emplace_back( f ); }

private:
	// �C���X�^���X�̎擾.
	static SaveDataManager* GetInstance();

private:
	std::vector<std::function<void()>>	m_LoadFunction;		// ���[�h���ɍs���֐�.

	std::unique_ptr<CCommonSaveData>	m_CommonSaveData;	// ���ʂ̃Z�[�u�f�[�^
	std::unique_ptr<CFlowerSaveData>	m_FlowerSaveData;	// �Ԃ̃Z�[�u�f�[�^
	std::unique_ptr<CWeedSaveData>		m_WeedSaveData;		// ���̃Z�[�u�f�[�^
};