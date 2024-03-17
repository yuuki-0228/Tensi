#include "Main.h"
#include "..\Global.h"
#include "..\Common\DirectX\DirectX11.h"
#include <crtdbg.h>	//_ASSERT_EXPR()�ŕK�v.

//�O���[�o���ϐ�.
std::unique_ptr<CMain>	g_pCMain = nullptr;

//================================================
//	���C���֐�.
//================================================
INT WINAPI WinMain(
	HINSTANCE hInstance,	//�C���X�^���X�ԍ�(�E�B���h�E�̔ԍ�).
	HINSTANCE hPrevInstance,
	PSTR lpCmdLine,
	INT nCmdShow )
{
#ifdef _DEBUG
	// ���������[�N���o
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif	// #ifdef _DEBUG.

	Log::OpenLogText();

	// ���������N���X�錾.
	g_pCMain = std::make_unique<CMain>();

	if ( g_pCMain != nullptr ) {

		// �E�B���h�E�쐬����������.
		if ( SUCCEEDED( g_pCMain->InitWindow( hInstance ) ) )
		{
			// Dx11�p�̏�����.
			if ( SUCCEEDED( g_pCMain->Create() ) )
			{
				// ���b�Z�[�W���[�v.
				g_pCMain->Loop();
			}
		}
	}
	Log::CloseLogText();

	// �}�E�X��\������.
	ShowCursor( TRUE );
	return 0;
}
