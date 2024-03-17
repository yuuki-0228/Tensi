#pragma once
#include "..\Global.h"
#include "..\Common\Sprite\Sprite.h"

class CFrameRate;
class CLoadManager;
class CMaskFade;

/************************************************
*	���C���N���X.
**/
class CMain final
{
public:
	CMain();
	~CMain();

	void	Update(		const float& DeltaTime );	// �X�V����.
	HRESULT Create();								// �\�z����.
	void	Loop();									// ���C�����[�v.

	// �E�B���h�E�������֐�.
	HRESULT InitWindow( HINSTANCE hInstance );

private:
	// �E�B���h�E�֐�(���b�Z�[�W���̏���).
	static LRESULT CALLBACK MsgProc(
		HWND hWnd, UINT uMsg,
		WPARAM wParam, LPARAM lParam );
	
	// �E�B���h�E�N�����̏�����( �N�������̂� ).
	void WindowInit();
	// FPS�̕`��.
	void FPSRender();

	// �E�B���h�E�̃N���b�N����̍X�V.
	void ClickUpdate();

private:
	HWND							m_hWnd;			// �E�B���h�E�n���h��.
	HWND							m_hSubWnd;		// �T�u�E�B���h�E�n���h��.
	HDC								m_hDc;			// �E�B���h�E��DC.
	HDC								m_hSubDc;		// �T�u�E�B���h�E��DC.
	std::unique_ptr<CFrameRate>		m_pFrameRate;	// �t���[�����[�g.
	std::unique_ptr<CLoadManager>	m_pLoadManager;	// ���[�h�}�l�[�W���[.
	bool							m_IsGameLoad;	// �ǂݍ��݂��I��������.
	bool							m_IsFPSRender;	// FPS��\�������邩.
	bool							m_IsWindowInit;	// �E�B���h�E�N�����̏��������s������.
	bool							m_IsWindowTop;	// �E�B���h�E���őO�ʂŌŒ肷�邩.
};