#pragma once
#include "..\Global.h"
#include "..\Utility\FileManager\FileManager.h"
#include "..\Utility\ImGuiManager\ImGuiManager.h"
#include <functional>

/************************************************
*	�V�X�e���ݒ�̃x�[�X�N���X.
*		���c�F�P.
**/
class CSystemBase
{
public:
	CSystemBase();
	virtual ~CSystemBase();

	// �`��.
	virtual void Render() final;
	// �\��.
	void Open() { m_IsOpen = true; }

protected:
	// �������֐�.
	//	m_RenderFunc�ɕ`�揈�����L�q����.
	virtual void Init() {};

	// �E�B���h�E���̏�����.
	//	Init�֐��̍Ō�ɌĂ�.
	virtual void InitWndName( const std::string& Name ) final;

protected:
	std::function<void()>	m_RenderFunc;	// �`�揈��.
	CBool					m_IsOpen;		// �J�����ǂ���.

private:
	std::string				m_WndName;		// �E�B���h�E��.
	bool					m_IsOldOpen;	// �O��̊J���Ă��邩.
};
