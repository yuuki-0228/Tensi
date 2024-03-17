#include "DragAndDrop.h"

DragAndDrop::DragAndDrop()
	: m_hWnd			()
	, m_FilePathList	()
{
}

DragAndDrop::~DragAndDrop()
{
}

//---------------------------.
// インスタンスの取得.
//---------------------------.
DragAndDrop* DragAndDrop::GetInstance()
{
	static std::unique_ptr<DragAndDrop> pInstance = std::make_unique<DragAndDrop>();
	return pInstance.get();
}

//---------------------------.
// ドラック&ドロップを使用する.
//---------------------------.
void DragAndDrop::Open()
{
	// ドラック&ドロップを使用する.
	DragAcceptFiles( GetInstance()->m_hWnd, TRUE );
}

//---------------------------.
// ドラック&ドロップを使用しない.
//---------------------------.
void DragAndDrop::Close()
{
	DragAndDrop* pI = GetInstance();

	// ドラック&ドロップを使用しない.
	DragAcceptFiles( pI->m_hWnd, FALSE );

	// 保存しているファイルパスを削除する.
	pI->m_FilePathList.clear();
}
