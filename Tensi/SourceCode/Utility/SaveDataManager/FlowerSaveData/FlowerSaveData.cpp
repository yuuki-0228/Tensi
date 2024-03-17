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
// 保存.
//---------------------------.
void CFlowerSaveData::Save()
{
	std::vector<SFlowerData> Data = m_SaveFunction();

	// ポインタは保存するとバグるためnullにする
	for ( auto& d : Data ) d.Transform.pParent = nullptr;

	// バイナリデータの保存.
	FileManager::BinarySave( FILE_PATH, Data );
}

//---------------------------.
// 読み込み.
//---------------------------.
void CFlowerSaveData::Load()
{
	if ( FileManager::FileCheck( FILE_PATH ) == false ) return;

	std::vector<SFlowerData> Data;

	// バイナリデータの読み込み.
	FileManager::BinaryLoad( FILE_PATH, Data );

	m_LoadFunction( Data );
}
