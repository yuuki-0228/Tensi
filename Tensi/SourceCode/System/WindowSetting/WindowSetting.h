#pragma once
#include "..\SystemBase.h"

/************************************************
*	�E�B���h�E�ݒ�N���X.
*		���c�F�P.
**/
class CWindowSetting final
	: public CSystemBase
{
public:
	CWindowSetting();
	~CWindowSetting();

protected:
	// �������֐�.
	virtual void Init() override;

private:
	D3DXVECTOR3		m_Color;				// �w�i�̐F.
	D3DXVECTOR2		m_Size;					// �E�B���h�E�T�C�Y.
	D3DXPOSITION2	m_Pos;					// �E�B���h�E�̕\���ʒu.
	std::string		m_AppName;				// �A�v����.
	std::string		m_WndName;				// �E�B���h�E��.
	bool			m_IsDispCenter;			// �^�񒆂ɕ\�����邩.
	bool			m_IsDispMouseCursor;	// �}�E�X�J�[�\����\�����邩.
	bool			m_IsFPSRender;			// ��ʂ�FPS��`�悷�邩.
	bool			m_IsFullScreenLock;		// �t���X�N���[���𖳌������邩.
	bool			m_IsSizeLock;			// �T�C�Y���Œ艻����.
	bool			m_IsStartFullScreen;	// �t���X�N���[���ŋN�����邩.
	bool			m_IsMaxLock;			// �ő剻�{�^���𖳌�������.
	bool			m_IsMinLock;			// �ŏ����{�^���𖳌�������.
	bool			m_IsPopUpWnd;			// �|�b�v�A�b�v�E�B���h�E(�g����)�ɂ��邩.
	bool			m_IsShowOnTop;			// �őO�ʂŌŒ艻����.
	bool			m_IsLogStop;			// ���O�t�@�C�����~���邩.
	bool			m_IsInputNotActiveStop;	// �A�N�e�B�u�E�B���h�E�ł͖����ꍇ������~�����邩.
	bool			m_IsDispCloseMessage;	// �E�B���h�E����郁�b�Z�[�W�{�b�N�X��\�����邩.
	std::string		m_Version;				// �A�v���o�[�W����.
};
