#pragma once
#include "..\..\..\Global.h"
#include "..\ImGuiManager.h"

/************************************************
*	�f�o�b�O�E�B���h�E�N���X.
*		���c�F�P.
**/
class DebugWindow final
{
public:
	DebugWindow();
	~DebugWindow();

	// �`��.
	static void Render();

	// �E�B���h�E�ɏo�͂���.
	static void PushProc( const char* Name, const std::function<void()>& Proc );

private:
	// �C���X�^���X�̎擾.
	static DebugWindow* GetInstance();

private:
	std::queue<std::function<void()>>	m_WatchQueue;		// �E�H�b�`�K�w�p�̊֐��̃L���[.
	std::queue<std::function<void()>>	m_MenuQueue;		// �f�o�b�O���j���[�K�w�p�̊֐��̃L���[.

private:
	// �R�s�[�E���[�u�R���X�g���N�^, ������Z�q�̍폜.
	DebugWindow( const DebugWindow & )				= delete;
	DebugWindow& operator = ( const DebugWindow & )	= delete;
	DebugWindow( DebugWindow && )						= delete;
	DebugWindow& operator = ( DebugWindow && )		= delete;
};
