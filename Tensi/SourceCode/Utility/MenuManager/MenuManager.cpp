#include "MenuManager.h"
#ifdef ENABLE_MENU
#include "..\..\Object\GameObject\Actor\WindowObjectManager\WindowObject\WindowObject.h"
#include "..\..\Object\GameObject\Actor\WindowObjectManager\WindowObjectManager.h"

MenuManager::MenuManager()
	: m_hWnd			( NULL )
	, m_hMenu			( NULL )
	, m_IsDispBall		( false )
	, m_IsDispHeavyBall	( false )
	, m_IsSuperBall		( false )
	, m_IsPlayer		( true )
{
}

MenuManager::~MenuManager()
{
	// ���j���[�̔j��.
	DestroyMenu( m_hMenu );
}

//---------------------------.
// �C���X�^���X�̎擾.
//---------------------------.
MenuManager* MenuManager::GetInstance()
{
	static std::unique_ptr<MenuManager> pInstance = std::make_unique<MenuManager>();
	return pInstance.get();
}

//---------------------------.
// ������.
//---------------------------.
HRESULT MenuManager::Init( const HWND& hWnd )
{
	MenuManager* pI = GetInstance();

	// �E�B���h�E�n���h���̕ۑ�.
	pI->m_hWnd	= hWnd;

	// ���j���[�̍쐬.
	pI->m_hMenu = CreatePopupMenu();

	// �T�u���j���[�̃e�L�X�g.
	const std::vector<WCHAR*> Text = {
		( WCHAR* ) L"�{�[���̕\��/��\��",
		( WCHAR* ) L"�d���{�[���̕\��/��\��",
		( WCHAR* ) L"�X�[�p�[�{�[���̕\��/��\��",
		( WCHAR* ) L"�X���C���N�̕\��/��\��",
		( WCHAR* ) L"�΂��΂��I"
	};

	// �T�u���j���[�̍쐬.
	const int Size = static_cast<int>( Text.size() );
	std::vector<MENUITEMINFO> SubMenu;
	SubMenu.resize( Size );
	for ( int i = 0; i < Size; ++i ) {
		SubMenu[i].cbSize		= sizeof( MENUITEMINFO );
		SubMenu[i].fMask		= MIIM_STRING | MIIM_ID;
		SubMenu[i].wID			= i;
		SubMenu[i].dwTypeData	= Text[i];
		SubMenu[i].cch			= 4;
		InsertMenuItem( pI->m_hMenu, i, TRUE, &SubMenu[i] );
	}
	return S_OK;
}

//---------------------------.
// ���j���[�̕\��.
//---------------------------.
void MenuManager::Disp()
{
	MenuManager* pI = GetInstance();

	// �}�E�X�J�[�\�����W�擾.
	POINT Pos;
	GetCursorPos( &Pos );

	// ���j���[�̕\��.
	SetForegroundWindow( pI->m_hWnd );
	TrackPopupMenuEx( pI->m_hMenu, TPM_LEFTALIGN, Pos.x, Pos.y, pI->m_hWnd, NULL );
}

//---------------------------.
// ���j���[�̑I��.
//---------------------------.
void MenuManager::SelectMenu( const WORD No )
{
	MenuManager* pI = GetInstance();

	switch ( No ) {
	case 0:
		// �{�[���̕\��/��\���̐ݒ�.
		pI->m_IsDispBall = !pI->m_IsDispBall;
		WindowObjectManager::GetObjectPtr( EObjectTag::NormalBall, 0 )->SetIsDisp( pI->m_IsDispBall );
		break;
	case 1:
		// �d���{�[���̕\��/��\���̐ݒ�.
		pI->m_IsDispHeavyBall = !pI->m_IsDispHeavyBall;
		WindowObjectManager::GetObjectPtr( EObjectTag::HeavyBall, 0 )->SetIsDisp( pI->m_IsDispHeavyBall );
		break;
	case 2:
		// �X�[�p�[�{�[���̕\��/��\���̐ݒ�.
		pI->m_IsSuperBall = !pI->m_IsSuperBall;
		WindowObjectManager::GetObjectPtr( EObjectTag::SuperBall, 0 )->SetIsDisp( pI->m_IsSuperBall );
		break;
	case 3:
		// �v���C���[�̕\��/��\���̐ݒ�.
		pI->m_IsPlayer = !pI->m_IsPlayer;
		WindowObjectManager::GetObjectPtr( EObjectTag::Player, 0 )->SetIsDisp( pI->m_IsPlayer );
		break;
	case 4:
		// �E�B���h�E�̔j��.
		DestroyWindow( pI->m_hWnd );
		break;
	}
}

#endif