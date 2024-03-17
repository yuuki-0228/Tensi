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
// �C���X�^���X�̎擾.
//----------------------------.
SaveDataManager* SaveDataManager::GetInstance()
{
	static std::unique_ptr<SaveDataManager> pInstance = std::make_unique<SaveDataManager>();
	return pInstance.get();
}

//---------------------------.
// �f�[�^�̐ݒ�.
//---------------------------.
SaveData::SSetSaveDataList* SaveDataManager::SetSaveData()
{
	return GetInstance()->m_CommonSaveData->GetPointerSaveData();
}

//---------------------------.
// �ۑ�.
//---------------------------.
void SaveDataManager::Save()
{
	SaveDataManager* pI = GetInstance();

	// �Z�[�u�f�[�^�̕ۑ�.
	pI->m_CommonSaveData->Save();
	pI->m_FlowerSaveData->Save();
	pI->m_WeedSaveData->Save();
}

//---------------------------.
// �ǂݍ���.
//---------------------------.
bool SaveDataManager::Load()
{
	SaveDataManager* pI = GetInstance();

	// ���񃍃O�C��
	if ( FileManager::FileCheck( FILE_PATH ) == false ) return true;
	
	// �Z�[�u�f�[�^�̓ǂݍ���.
	pI->m_CommonSaveData->Load();
	pI->m_FlowerSaveData->Load();
	pI->m_WeedSaveData->Load();

	// ���[�h���ɍs���֐��̎��s.
	for ( auto& f : pI->m_LoadFunction ) f();
	return false;
}

//---------------------------.
// ���O�C�����̕ۑ�.
//---------------------------.
void SaveDataManager::LoginSave()
{
	const std::tm& Now = TimeManager::GetTime();

	FileManager::BinarySave( LOGIN_FILE_PATH, Now );
}

//---------------------------.
// ���O�C�����̓ǂݍ���.
//---------------------------.
bool SaveDataManager::LoginLoad( std::tm* pDay )
{
	SaveDataManager* pI = GetInstance();

	if ( FileManager::FileCheck( LOGIN_FILE_PATH ) == false ) {
		// ���O�C����񂪑��݂��Ȃ����ߏ��񃍃O�C���Ƃ���
		return true;
	}

	// �o�C�i���f�[�^�̓ǂݍ���.
	std::tm Day;
	FileManager::BinaryLoad( LOGIN_FILE_PATH, Day );
	if ( pDay != nullptr ) {
		*pDay = Day;
	}

	// �ǂݍ��񂾓��������ɍ������O�C�����������ׂ�
	const std::tm& Now = TimeManager::GetTime();
	if ( TimeManager::GetIsSameDay( Now, Day ) ) {
		return false;
	}
	return true;
}

//---------------------------.
// �Ԃ̃Z�[�u�f�[�^�̐ݒ�.
//---------------------------.
void SaveDataManager::SetFlowerSaveData( CFlowerSaveData::SaveFunction Save, CFlowerSaveData::LoadFunction Load )
{
	SaveDataManager* pI = GetInstance();

	pI->m_FlowerSaveData->SetSaveFunction( Save );
	pI->m_FlowerSaveData->SetLoadFunction( Load );
}

//---------------------------.
// ���̃Z�[�u�f�[�^�̐ݒ�.
//---------------------------.
void SaveDataManager::SetWeedSaveData( CWeedSaveData::SaveFunction Save, CWeedSaveData::LoadFunction Load )
{
	SaveDataManager* pI = GetInstance();

	pI->m_WeedSaveData->SetSaveFunction( Save );
	pI->m_WeedSaveData->SetLoadFunction( Load );
}
