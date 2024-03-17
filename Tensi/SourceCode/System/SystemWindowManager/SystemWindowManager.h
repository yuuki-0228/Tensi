#pragma once
#include "..\..\Global.h"

class InputSetting;
class CWindowSetting;
class CBugReport;
class CSprite;

/************************************************
*	�V�X�e���ݒ������E�B���h�E�}�l�[�W���[.
*		���c�F�P.
**/
class SystemWindowManager final
{
public:
	SystemWindowManager();
	~SystemWindowManager();

	// �X�V.
	static void Update( const float& DeltaTime );
	// �`��.
	static void Render();

private:
	// �C���X�^���X�̎擾.
	static SystemWindowManager* GetInstance();

private:
	std::unique_ptr<InputSetting>	m_pInputSetting;	// ���͐ݒ���s���E�B���h�E.
	std::unique_ptr<CWindowSetting> m_pWindowSetting;	// �E�B���h�E�ݒ���s���E�B���h�E.
	std::unique_ptr<CBugReport>		m_pBugReport;		// �o�O�񍐂��s���E�B���h�E.
	CSprite*						m_pButton;			// �{�^��.

private:
	// �R�s�[�E���[�u�R���X�g���N�^, ������Z�q�̍폜.
	SystemWindowManager( const SystemWindowManager & )				= delete;
	SystemWindowManager& operator = ( const SystemWindowManager & )	= delete;
	SystemWindowManager( SystemWindowManager && )						= delete;
	SystemWindowManager& operator = ( SystemWindowManager && )		= delete;
};
