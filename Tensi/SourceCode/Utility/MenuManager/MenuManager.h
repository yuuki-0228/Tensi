#pragma once
#include "..\..\SystemSetting.h"
#ifdef ENABLE_MENU
#include "..\..\Global.h"

/************************************************
*	���j���[�N���X.
**/
class MenuManager
{
public:
	MenuManager();
	~MenuManager();

	// ������.
	static HRESULT Init( const HWND& hWnd );

	// �\��.
	static void Disp();
	// ���j���[�̑I��.
	static void SelectMenu( const WORD No );

private:
	// �C���X�^���X�̎擾.
	static MenuManager* GetInstance();

private:
	HWND	m_hWnd;				// �E�B���h�E�n���h��.
	HMENU	m_hMenu;			// ���j���[.
	bool	m_IsDispBall;		// �{�[����\�����Ă��邩.
	bool	m_IsDispHeavyBall;	// �d���{�[����\�����Ă��邩.
	bool	m_IsSuperBall;		// �X�[�p�[�{�[����\�����Ă��邩.
	bool	m_IsPlayer;			// �v���C���[��\�����Ă��邩.
};
#endif