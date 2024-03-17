#pragma once
#include "..\..\..\Global.h"

/************************************************
*	�h���b�N���h���b�v�N���X.
*		���c�F�P.
**/
class DragAndDrop final
{
public:
	DragAndDrop();
	~DragAndDrop();

	// �h���b�N&�h���b�v���g�p����.
	static void Open();
	// �h���b�N&�h���b�v���g�p���Ȃ�.
	static void Close();
	
	// �t�@�C�����h���b�v���ꂽ��.
	static bool GetIsDrop() { return !GetInstance()->m_FilePathList.empty(); }
	// �t�@�C���p�X���擾.
	static std::vector<std::string> GetFilePath() { return GetInstance()->m_FilePathList; }

	// �t�@�C���p�X��ǉ�.
	static void AddFilePath( const std::string& FilePath ) { GetInstance()->m_FilePathList.emplace_back( FilePath ); }

	// �E�B���h�E�n���h����ݒ�.
	static void SethWnd( const HWND& hWnd ) { GetInstance()->m_hWnd = hWnd; }

private:
	// �C���X�^���X�̎擾.
	static DragAndDrop* GetInstance();

private:
	HWND						m_hWnd;			// �E�B���h�E�n���h��.
	std::vector<std::string>	m_FilePathList;	// �t�@�C���p�X���X�g.

private:
	// �R�s�[�E���[�u�R���X�g���N�^, ������Z�q�̍폜.
	DragAndDrop( const DragAndDrop& )					= delete;
	DragAndDrop( DragAndDrop&& )						= delete;
	DragAndDrop& operator = ( const DragAndDrop& )	= delete;
	DragAndDrop& operator = ( DragAndDrop&& )			= delete;
};