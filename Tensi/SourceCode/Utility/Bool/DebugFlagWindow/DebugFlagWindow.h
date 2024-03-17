#pragma once
#include "..\..\..\Global.h"

/************************************************
*	���b�Z�[�W�E�B���h�E�N���X.
*		���c�F�P.
**/
class DebugFlagWindow final
{
public:
	DebugFlagWindow();
	~DebugFlagWindow();

	// �`��.
	static void Render();

private:
	// �C���X�^���X�̎擾.
	static DebugFlagWindow* GetInstance();

	static void FlagFind(); // �t���O��������.

private:
	std::string	m_FindString;			// �t���O�E�B���h�E�̌������镶��.
	CBool		m_IsDispNameNoneFlag;	// �����̃t���O��\�������邩.

private:
	// �R�s�[�E���[�u�R���X�g���N�^, ������Z�q�̍폜.
	DebugFlagWindow( const DebugFlagWindow & )				= delete;
	DebugFlagWindow& operator = ( const DebugFlagWindow & )	= delete;
	DebugFlagWindow( DebugFlagWindow && )						= delete;
	DebugFlagWindow& operator = ( DebugFlagWindow && )		= delete;
};
