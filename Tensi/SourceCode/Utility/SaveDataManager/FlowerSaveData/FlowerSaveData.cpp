#include "FlowerSaveData.h"
#include "..\..\..\Utility\FileManager\FileManager.h"

namespace {
	constexpr char FILE_PATH[] = "Data\\Parameter\\d\\f.bin";

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
// �ۑ�.
//---------------------------.
void CFlowerSaveData::Save()
{
	std::vector<SFlowerData> Data = m_SaveFunction();

	// �|�C���^�͕ۑ�����ƃo�O�邽��null�ɂ���
	for ( auto& d : Data ) d.Transform.pParent = nullptr;

	// �o�C�i���f�[�^�̕ۑ�.
	FileManager::BinarySave( FILE_PATH, Data );
}

//---------------------------.
// �ǂݍ���.
//---------------------------.
void CFlowerSaveData::Load()
{
	if ( FileManager::FileCheck( FILE_PATH ) == false ) return;

	std::vector<SFlowerData> Data;

	// �o�C�i���f�[�^�̓ǂݍ���.
	FileManager::BinaryLoad( FILE_PATH, Data );

	m_LoadFunction( Data );
}
