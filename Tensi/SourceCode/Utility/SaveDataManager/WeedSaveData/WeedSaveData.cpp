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
// 保存.
//---------------------------.
void CWeedSaveData::Save()
{
	std::vector<SWeedData> Data = m_SaveFunction();

	// ポインタは保存するとバグるためnullにする
	for ( auto& s : Data ) s.Transform.pParent = nullptr;

	// バイナリデータの保存.
	FileManager::BinarySave( FILE_PATH, Data );
}

//---------------------------.
// 読み込み.
//---------------------------.
void CWeedSaveData::Load()
{
	if ( FileManager::FileCheck( FILE_PATH ) == false ) return;

	std::vector<SWeedData> Data;

	// バイナリデータの読み込み.
	FileManager::BinaryLoad( FILE_PATH, Data );

	m_LoadFunction( Data );
}
