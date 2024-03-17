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
// �C���X�^���X�̎擾.
//---------------------------.
DragAndDrop* DragAndDrop::GetInstance()
{
	static std::unique_ptr<DragAndDrop> pInstance = std::make_unique<DragAndDrop>();
	return pInstance.get();
}

//---------------------------.
// �h���b�N&�h���b�v���g�p����.
//---------------------------.
void DragAndDrop::Open()
{
	// �h���b�N&�h���b�v���g�p����.
	DragAcceptFiles( GetInstance()->m_hWnd, TRUE );
}

//---------------------------.
// �h���b�N&�h���b�v���g�p���Ȃ�.
//---------------------------.
void DragAndDrop::Close()
{
	DragAndDrop* pI = GetInstance();

	// �h���b�N&�h���b�v���g�p���Ȃ�.
	DragAcceptFiles( pI->m_hWnd, FALSE );

	// �ۑ����Ă���t�@�C���p�X���폜����.
	pI->m_FilePathList.clear();
}
