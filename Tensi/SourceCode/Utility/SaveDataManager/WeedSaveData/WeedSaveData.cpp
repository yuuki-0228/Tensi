#include "WeedSaveData.h"
#include "..\..\..\Utility\FileManager\FileManager.h"

namespace {
	constexpr char FILE_PATH[] = "Data\\Parameter\\d\\w.bin";

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
// �ۑ�.
//---------------------------.
void CWeedSaveData::Save()
{
	std::vector<SWeedData> Data = m_SaveFunction();

	// �|�C���^�͕ۑ�����ƃo�O�邽��null�ɂ���
	for ( auto& s : Data ) s.Transform.pParent = nullptr;

	// �o�C�i���f�[�^�̕ۑ�.
	FileManager::BinarySave( FILE_PATH, Data );
}

//---------------------------.
// �ǂݍ���.
//---------------------------.
void CWeedSaveData::Load()
{
	if ( FileManager::FileCheck( FILE_PATH ) == false ) return;

	std::vector<SWeedData> Data;

	// �o�C�i���f�[�^�̓ǂݍ���.
	FileManager::BinaryLoad( FILE_PATH, Data );

	m_LoadFunction( Data );
}
