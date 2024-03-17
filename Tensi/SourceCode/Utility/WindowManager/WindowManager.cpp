#include "WindowManager.h"
#include "..\Input\Input.h"
#include <dwmapi.h>
#include <psapi.h>
#include <shlobj.h>

WindowManager::WindowManager()
	: m_hDesktop				( NULL )
	, m_hMyWindow				( NULL )
	, m_hMySubWindow			( NULL )
	, m_MouseDownWindow			( NULL )
	, m_MyWindowRect			()
	, m_AddWindowRect			()
	, m_TaskBar					()
	, m_DProcessID				( NULL )
	, m_DProcessHandle			( NULL )
	, m_DProcessMemory			( NULL )
	, m_WindowList				()
	, m_WindowFindMap			()
	, m_WindowRectMap			()
	, m_WindowNoShadowRectMap	()
	, m_WindowNameMap			()
	, m_WindowClassMap			()
	, m_WindowZOrderMap			()
	, m_IconList				()
	, m_IconFindMap				()
	, m_IconPosMap				()
	, m_IconNameMap				()
	, m_UserName				( "" )
	, m_IsWindowUpdate			( false )
	, m_IsDesktopIconUpdate		( false )
	, m_IsTaskBarUpdate			( false )
{
}

WindowManager::~WindowManager()
{
	// �㏈��.
	VirtualFreeEx( m_DProcessHandle, m_DProcessMemory, 0, MEM_RELEASE );
	CloseHandle( m_DProcessHandle );
}

//----------------------------.
// �C���X�^���X�̎擾.
//----------------------------.
WindowManager* WindowManager::GetInstance()
{
	static std::unique_ptr<WindowManager> pInstance = std::make_unique<WindowManager>();
	return pInstance.get();
}

//---------------------------.
// ������.
//---------------------------.
HRESULT WindowManager::Init()
{
	WindowManager* pI = GetInstance();

	// �E�B���h�E�̏��ɒǉ�����␳�l�̍쐬.
	GetWindowRect( pI->m_hMyWindow, &pI->m_MyWindowRect );
	pI->m_AddWindowRect.left	= -pI->m_MyWindowRect.left;
	pI->m_AddWindowRect.top		= -pI->m_MyWindowRect.top;
	pI->m_AddWindowRect.right	= -pI->m_MyWindowRect.left;
	pI->m_AddWindowRect.bottom	= -pI->m_MyWindowRect.top;

	// ���[�U�[���̎擾.
	char Name[256];
	DWORD Size = sizeof( Name );
	if ( GetUserNameA( Name, &Size ) == 0 ) return E_FAIL;
	pI->m_UserName = Name;

	// �f�X�N�g�b�v�̃n���h���̎擾.
	pI->m_hDesktop = FindWindow( L"Progman", L"Program Manager" );
	pI->m_hDesktop = FindWindowEx( pI->m_hDesktop, NULL, L"SHELLDLL_DefView", NULL );
	pI->m_hDesktop = FindWindowEx( pI->m_hDesktop, NULL, L"SysListView32", NULL );

	if ( pI->m_hDesktop == NULL ) {
		// �ǎ���ύX�����ꍇ��Program Manager��SHELLDLL_DefView��SysListView32�ł͂Ȃ�.
		// WorkerW��SHELLDLL_DefView��SysListView32�ɂȂ邱�Ƃ�����̂ŁA��������.
		// WorkerW�͕�������̂Ń��[�v���ĖړI�̕���������܂ŒT��.
		HWND hWorkerW		= NULL;
		HWND hShellViewWin	= NULL;

		do {
			hWorkerW		= FindWindowEx( NULL, hWorkerW, L"WorkerW", NULL );
			hShellViewWin	= FindWindowEx( hWorkerW, NULL, L"SHELLDLL_DefView", NULL );
		} while ( hShellViewWin == NULL && hWorkerW != NULL );
		pI->m_hDesktop = FindWindowEx( hShellViewWin, NULL, L"SysListView32", NULL );
	}
	if ( pI->m_hDesktop == NULL ) return E_FAIL;

	// �v���Z�XID�̎擾.
	GetWindowThreadProcessId( pI->m_hDesktop, &pI->m_DProcessID );
	if ( pI->m_DProcessID == NULL ) return E_FAIL;

	// �v���Z�X�̃n���h���̎擾.
	pI->m_DProcessHandle = OpenProcess( PROCESS_VM_OPERATION | PROCESS_VM_READ |
		PROCESS_VM_WRITE, false, pI->m_DProcessID );
	if ( pI->m_DProcessHandle == NULL ) return E_FAIL;

	// ���̃v���Z�X�Ń��������m�ۂ���.
	pI->m_DProcessMemory = VirtualAllocEx( pI->m_DProcessHandle, NULL, 4096,
		MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE );
	if ( pI->m_DProcessMemory == NULL ) return E_FAIL;
	return S_OK;
}

//---------------------------.
// �X�V.
//---------------------------.
void WindowManager::Update()
{
	WindowManager* pI = GetInstance();

	// �t���O�̏�����.
	pI->m_IsWindowUpdate		= false;
	pI->m_IsDesktopIconUpdate	= false;
	pI->m_IsTaskBarUpdate		= false;
}

//---------------------------.
// �^�X�N�o�[�̏����擾.
//---------------------------.
RECT WindowManager::GetTaskBarRect()
{
	WindowManager* pI = GetInstance();

	// �^�X�N�o�[���X�V���Ă��Ȃ��ꍇ�X�V����.
	if ( pI->m_IsTaskBarUpdate == false ) TaskBarUpdate();

	return GetInstance()->m_TaskBar.rc;
}

//---------------------------.
// �^�X�N�o�[�̃E�B���h�E�n���h���̎擾.
//---------------------------.
HWND WindowManager::GetTaskBarWnd()
{
	WindowManager* pI = GetInstance();

	// �^�X�N�o�[���X�V���Ă��Ȃ��ꍇ�X�V����.
	if ( pI->m_IsTaskBarUpdate == false ) TaskBarUpdate();

	return GetInstance()->m_TaskBar.hWnd;
}

//---------------------------.
// �}�E�X�����̃E�B���h�E�̏�ɂ���ꍇ���̃E�B���h�E�̃n���h�����擾(��ɂȂ��ꍇ:NULL).
//---------------------------.
HWND WindowManager::GetMouseOverTheWindow()
{
	const D3DXPOSITION3& MousePos = Input::GetMousePosition3();
	return GetPointOverTheWindow( MousePos );
}

//---------------------------.
// �}�E�X�����̃E�B���h�E�̏�ɂ��邩�擾.
//---------------------------.
bool WindowManager::GetIsMouseOverTheWindow()
{
	const D3DXPOSITION3& MousePos = Input::GetMousePosition3();
	return GetIsPointOverTheWindow( MousePos );
}

//---------------------------.
// �w�肵�����W�����̃E�B���h�E�̏�ɂ���ꍇ���̃E�B���h�E�̃n���h�����擾(��ɂȂ��ꍇ:NULL).
//---------------------------.
HWND WindowManager::GetPointOverTheWindow( const D3DXPOSITION3& Pos )
{
	WindowManager* pI = GetInstance();

	// �E�B���h�E���X�g���X�V���Ă��Ȃ��ꍇ�X�V����.
	if ( pI->m_IsWindowUpdate == false ) WindowListUpdate();

	// �E�B���h�E�ɓ������Ă��邩���ׂ�.
	for ( auto& [w, hWnd] : pI->m_WindowList ) {
		// �}�E�X���������Ă��邩���ׂ�.
		if ( ( Pos.x < w.right  ) && ( Pos.x > w.left ) &&
			 ( Pos.y < w.bottom ) && ( Pos.y > w.top  ) )
		{
			return hWnd;
		}
	}
	return NULL;
}
HWND WindowManager::GetPointOverTheWindow( const D3DXPOSITION2& Pos )
{
	return GetPointOverTheWindow( { Pos.x, Pos.y, INIT_FLOAT } );
}

//---------------------------.
// �w�肵�����W�����̃E�B���h�E�̏�ɂ��邩�擾.
//---------------------------.
bool WindowManager::GetIsPointOverTheWindow( const D3DXPOSITION3& Pos )
{
	return GetPointOverTheWindow( Pos ) != NULL;
}
bool WindowManager::GetIsPointOverTheWindow( const D3DXPOSITION2& Pos )
{
	return GetIsPointOverTheWindow( { Pos.x, Pos.y, INIT_FLOAT } );
}

//---------------------------.
// �E�B���h�E�̖��O����n���h�����擾.
//---------------------------.
HWND WindowManager::GetFindWindow( const std::string& Name )
{
	WindowManager* pI = GetInstance();

	// �E�B���h�E���X�g���X�V���Ă��Ȃ��ꍇ�X�V����.
	if ( pI->m_IsWindowUpdate == false ) WindowListUpdate();
	return pI->m_WindowFindMap[Name];
}

//---------------------------.
// �f�X�N�g�b�v�̃A�C�R��������A�C�R���C���f�b�N�X���擾.
//---------------------------.
ICOINDEX WindowManager::GetFindDesktopIcon( const std::string& Name )
{
	WindowManager* pI = GetInstance();

	// �A�C�R�����X�g���X�V���Ă��Ȃ��ꍇ�X�V����.
	if ( pI->m_IsDesktopIconUpdate == false ) DesktopIconUpdate();
	return pI->m_IconFindMap[Name];
}

//---------------------------.
// �E�B���h�E�̏����擾.
//---------------------------.
WindowManager::WndList WindowManager::GetWindowList()
{
	WindowManager* pI = GetInstance();

	// �E�B���h�E�N���X���X�V���Ă��Ȃ��ꍇ�X�V����.
	if ( pI->m_IsWindowUpdate == false ) WindowListUpdate();
	return pI->m_WindowList;
}

//---------------------------.
// �f�X�N�g�b�v�̃A�C�R���̈ʒu���X�g�̎擾.
//---------------------------.
WindowManager::IconList WindowManager::GetDesktopIconList()
{
	WindowManager* pI = GetInstance();

	// �E�B���h�E�N���X���X�V���Ă��Ȃ��ꍇ�X�V����.
	if ( pI->m_IsDesktopIconUpdate == false ) DesktopIconUpdate();
	return pI->m_IconList;
}

//---------------------------.
// �E�B���h�E�̏��̎擾.
//---------------------------.
RECT WindowManager::GetWindowSize( const HWND& hWnd )
{
	WindowManager* pI = GetInstance();

	// �E�B���h�E�N���X���X�V���Ă��Ȃ��ꍇ�X�V����.
	if ( pI->m_IsWindowUpdate == false ) WindowListUpdate();
	return pI->m_WindowRectMap[hWnd];
}

//---------------------------.
// �h���b�v�V���h�E���������E�B���h�E�̏��̎擾.
//---------------------------.
RECT WindowManager::GetWindowNoShadowSize( const HWND& hWnd )
{
	WindowManager* pI = GetInstance();

	// �E�B���h�E�N���X���X�V���Ă��Ȃ��ꍇ�X�V����.
	if ( pI->m_IsWindowUpdate == false ) WindowListUpdate();
	return pI->m_WindowNoShadowRectMap[hWnd];
}

//---------------------------.
// �E�B���h�E���̎擾.
//---------------------------.
std::string WindowManager::GetWindowName( const HWND& hWnd )
{
	WindowManager* pI = GetInstance();

	// �E�B���h�E�N���X���X�V���Ă��Ȃ��ꍇ�X�V����.
	if ( pI->m_IsWindowUpdate == false ) WindowListUpdate();
	return pI->m_WindowNameMap[hWnd];
}

//---------------------------.
// �E�B���h�E�N���X���̎擾.
//---------------------------.
std::string WindowManager::GetWindowClassName( const HWND& hWnd )
{
	WindowManager* pI = GetInstance();

	// �E�B���h�E�N���X���X�V���Ă��Ȃ��ꍇ�X�V����.
	if ( pI->m_IsWindowUpdate == false ) WindowListUpdate();
	return pI->m_WindowClassMap[hWnd];
}

//---------------------------.
// Z�I�[�_�[�̎擾( ���������������㑤 ).
//---------------------------.
int WindowManager::GetWindowZOrder( const HWND& hWnd )
{
	WindowManager* pI = GetInstance();

	// �E�B���h�E�N���X���X�V���Ă��Ȃ��ꍇ�X�V����.
	if ( pI->m_IsWindowUpdate == false ) WindowListUpdate();
	return pI->m_WindowZOrderMap[hWnd];
}

//---------------------------.
// �A�C�R���̈ʒu���擾.
//---------------------------.
D3DXPOSITION2 WindowManager::GetDesktopIconPosition( const ICOINDEX Index )
{
	WindowManager* pI = GetInstance();

	// �A�C�R�����X�g���X�V���Ă��Ȃ��ꍇ�X�V����.
	if ( pI->m_IsDesktopIconUpdate == false ) DesktopIconUpdate();
	return pI->m_IconPosMap[Index];
}

//---------------------------.
// �A�C�R���̖��O���擾.
//---------------------------.
std::string WindowManager::GetDesktopIconName( const ICOINDEX Index )
{
	WindowManager* pI = GetInstance();

	// �A�C�R�����X�g���X�V���Ă��Ȃ��ꍇ�X�V����.
	if ( pI->m_IsDesktopIconUpdate == false ) DesktopIconUpdate();
	return pI->m_IconNameMap[Index];
}

//---------------------------.
// �f�X�N�g�b�v�̃A�C�R���̃t�@�C���p�X���擾.
//---------------------------.
std::string WindowManager::GetDesktopIconPath( const ICOINDEX Index )
{
	WindowManager* pI = GetInstance();

	// �A�C�R�����X�g���X�V���Ă��Ȃ��ꍇ�X�V����.
	if ( pI->m_IsDesktopIconUpdate == false ) DesktopIconUpdate();
	return "C:\\Users\\" + pI->m_UserName + "\\Desktop\\" + pI->m_IconNameMap[Index];
}

//---------------------------.
// �f�X�N�g�b�v�̃A�C�R���̈ʒu��ݒ�.
//---------------------------.
void WindowManager::SetDesktopIconPosition( const ICOINDEX Index, const int x, const int y )
{
	WindowManager* pI = GetInstance();

	// �ݒ肷��ʒu�𒲐�����.
	const int PosX = x - pI->m_AddWindowRect.left;
	const int PosY = y - pI->m_AddWindowRect.top;

	SendMessage( 
		pI->m_hDesktop,
		LVM_SETITEMPOSITION,
		Index,
		MAKELPARAM( x, y )
	);
}
void WindowManager::SetDesktopIconPosition( const ICOINDEX Index, const D3DXPOSITION2& Pos )
{
	SetDesktopIconPosition( Index, static_cast<int>( Pos.x ), static_cast<int>( Pos.y ) );
}
void WindowManager::SetDesktopIconPosition( const ICOINDEX Index, const D3DXPOSITION3& Pos )
{
	SetDesktopIconPosition( Index, static_cast<int>( Pos.x ), static_cast<int>( Pos.y ) );
}

//---------------------------.
// �f�X�N�g�b�v�̃A�C�R���̈ʒu��ǉ�.
//---------------------------.
void WindowManager::AddDesktopIconPosition( const ICOINDEX Index, const int x, const int y )
{
	WindowManager* pI = GetInstance();

	SetDesktopIconPosition( Index, static_cast<int>( pI->m_IconPosMap[Index].x + x ), static_cast<int>( pI->m_IconPosMap[Index].y + y ) );
}
void WindowManager::AddDesktopIconPosition( const ICOINDEX Index, const D3DXPOSITION2& Pos )
{
	AddDesktopIconPosition( Index, static_cast<int>( Pos.x ), static_cast<int>( Pos.y ) );
}
void WindowManager::AddDesktopIconPosition( const ICOINDEX Index, const D3DXPOSITION3& Pos )
{
	AddDesktopIconPosition( Index, static_cast<int>( Pos.x ), static_cast<int>( Pos.y ) );
}

//---------------------------.
// �E�B���h�E�̈ʒu��ݒ�.
//---------------------------.
void WindowManager::SetWindowPosition( const HWND& hWnd, const int x, const int y )
{
	SetWindowPos( hWnd, NULL, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER );
}
void WindowManager::SetWindowPosition( const HWND& hWnd, const D3DXPOSITION2& Pos )
{
	SetWindowPosition( hWnd, static_cast<int>( Pos.x ), static_cast<int>( Pos.y ) );
}
void WindowManager::SetWindowPosition( const HWND& hWnd, const D3DXPOSITION3& Pos )
{
	SetWindowPosition( hWnd, static_cast<int>( Pos.x ), static_cast<int>( Pos.y ) );
}

//---------------------------.
// �E�B���h�E�̈ʒu��ǉ�.
//---------------------------.
void WindowManager::AddWindowPosition( const HWND& hWnd, const int x, const int y )
{
	WindowManager* pI = GetInstance();

	SetWindowPosition( hWnd, pI->m_WindowRectMap[hWnd].left + x, pI->m_WindowRectMap[hWnd].top  + y );
}
void WindowManager::AddWindowPosition( const HWND& hWnd, const D3DXPOSITION2& Pos )
{
	AddWindowPosition( hWnd, static_cast<int>( Pos.x ), static_cast<int>( Pos.y ) );
}
void WindowManager::AddWindowPosition( const HWND& hWnd, const D3DXPOSITION3& Pos )
{
	AddWindowPosition( hWnd, static_cast<int>( Pos.x ), static_cast<int>( Pos.y ) );
}

//---------------------------.
// �E�B���h�E�̃T�C�Y��ݒ�.
//---------------------------.
void WindowManager::SetWindowSize( const HWND& hWnd, const int x, const int y )
{
	SetWindowPos( hWnd, NULL, 0, 0, x, y, SWP_NOMOVE | SWP_NOZORDER );
}
void WindowManager::SetWindowSize( const HWND& hWnd, const D3DXSCALE2& Size )
{
	SetWindowSize( hWnd, static_cast<int>( Size.x ), static_cast<int>( Size.y ) );
}
void WindowManager::SetWindowSize( const HWND& hWnd, const D3DXSCALE3& Size )
{
	SetWindowSize( hWnd, static_cast<int>( Size.x ), static_cast<int>( Size.y ) );
}

//---------------------------.
// �E�B���h�E�̃T�C�Y��ǉ�.
//---------------------------.
void WindowManager::AddWindowSize( const HWND& hWnd, const int x, const int y )
{
	WindowManager* pI = GetInstance();

	SetWindowSize( hWnd, pI->m_WindowRectMap[hWnd].right - pI->m_WindowRectMap[hWnd].left + x, pI->m_WindowRectMap[hWnd].bottom - pI->m_WindowRectMap[hWnd].top + y );
}
void WindowManager::AddWindowSize( const HWND& hWnd, const D3DXSCALE2& Size )
{
	AddWindowSize( hWnd, static_cast<int>( Size.x ), static_cast< int >( Size.y ) );
}
void WindowManager::AddWindowSize( const HWND& hWnd, const D3DXSCALE3& Size )
{
	AddWindowSize( hWnd, static_cast<int>( Size.x ), static_cast< int >( Size.y ) );
}

//---------------------------.
// �E�B���h�E���őO�ʂɈړ�.
//---------------------------.
void WindowManager::SetWindowTop( const HWND& hWnd, const bool IsLock )
{
	SetWindowPos( hWnd, IsLock ? HWND_TOPMOST : HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW );
	if ( IsLock ) return;
	SetWindowPos( hWnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW );
}

//---------------------------.
// �E�B���h�E���Ŕw�ʂɈړ�.
//---------------------------.
void WindowManager::SetWindowBottom( const HWND& hWnd )
{
	SetWindowPos( hWnd, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW );
}

//---------------------------.
// �E�B���h�E���w�肵���E�B���h�E�̉��Ɉړ�.
//---------------------------.
void WindowManager::SetWindowSelectDown( const HWND& hWnd, const HWND& hSelectWnd )
{
	SetWindowPos( hWnd, hSelectWnd, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW );
}

//---------------------------.
// �E�B���h�E���w�肵���E�B���h�E�̏�Ɉړ�.
//---------------------------.
void WindowManager::SetWindowSelectUp( const HWND& hWnd, const HWND& hSelectWnd )
{
	SetWindowPos( hWnd, GetWindow( hSelectWnd, GW_HWNDPREV ), 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW );
}

//---------------------------.
// �E�B���h�E�̔j��.
//---------------------------.
void WindowManager::WindowDelete( const HWND& hWnd )
{
	// �E�B���h�E�̔j��.
	if ( hWnd == NULL ) PostMessage( GetInstance()->m_hMyWindow, WM_CLOSE, 0, 0 );
	else				PostMessage( hWnd, WM_CLOSE, 0, 0 );
}

//---------------------------.
// �E�B���h�E����������.
//---------------------------.
void WindowManager::WindowFlash( const HWND& hWnd )
{
	FLASHWINFO fInfo {
		sizeof( FLASHWINFO ),
		hWnd,
		FLASHW_TRAY | FLASHW_TIMERNOFG
	};
	FlashWindowEx( &fInfo );
}

//---------------------------.
// �A�v���̃V���[�g�J�b�g���쐬����
//---------------------------.
bool WindowManager::CreateShortcut(
	LPCTSTR pszLink,		LPCTSTR pszFile,
	LPCTSTR pszDescription, LPCTSTR pszArgs,
	LPCTSTR pszWorkingDir,  LPCTSTR pszIconPath,
	int nIcon, int nShowCmd )
{
	HRESULT			hr;
	IShellLink*		pIShellLink;
	IPersistFile*	pIPersistFile;

	//IShellLink�̍쐬
	pIShellLink = NULL;
	hr = ::CoCreateInstance( CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, ( void** )&pIShellLink );
	if ( pIShellLink == NULL || FAILED( hr ) ) return	false;

	//IPersistFile�̎擾
	pIPersistFile = NULL;
	hr = pIShellLink->QueryInterface( IID_IPersistFile, ( void** )&pIPersistFile );
	if ( pIPersistFile == NULL || FAILED( hr ) )
	{
		pIShellLink->Release();
		return false;
	}

	//�V���[�g�J�b�g�ڍאݒ�
	hr = pIShellLink->SetPath( pszFile );
	if ( SUCCEEDED( hr ) && pszDescription	) hr = pIShellLink->SetDescription( pszDescription );
	if ( SUCCEEDED( hr ) && pszArgs			) hr = pIShellLink->SetArguments( pszArgs );
	if ( SUCCEEDED( hr ) && pszWorkingDir	) hr = pIShellLink->SetWorkingDirectory( pszWorkingDir );
	if ( SUCCEEDED( hr ) && pszIconPath		) hr = pIShellLink->SetIconLocation( pszIconPath, nIcon );
	if ( SUCCEEDED( hr )					) hr = pIShellLink->SetShowCmd( nShowCmd );

#ifndef UNICODE
	WCHAR*	pwszUnicode;
	int		nLen;

	//Unicode�ϊ�
	nLen = ::MultiByteToWideChar( CP_ACP, 0, pszLink, -1, NULL, 0 );
	pwszUnicode = new WCHAR[nLen + 1];
	if ( pwszUnicode == NULL )
	{
		pIPersistFile->Release();
		pIShellLink->Release();
		return	false;
	}
	nLen = ::MultiByteToWideChar( CP_ACP, 0, pszLink, -1, pwszUnicode, nLen + 1 );
	if ( nLen == 0 ) hr = E_FAIL;

	//�V���[�g�J�b�g�̕ۑ�
	if ( SUCCEEDED( hr ) ) hr = pIPersistFile->Save( pwszUnicode, TRUE );

	delete	pwszUnicode;
#else
	//�V���[�g�J�b�g�̕ۑ�
	if ( SUCCEEDED( hr ) ) hr = pIPersistFile->Save( pszLink, TRUE );
#endif

	pIPersistFile->Release();
	pIShellLink->Release();

	return SUCCEEDED( hr ) ? true : false;
}

//---------------------------.
// �E�B���h�E�̍X�V.
//---------------------------.
void WindowManager::WindowListUpdate()
{
	WindowManager* pI = GetInstance();

	// ���X�g�̏�����.
	pI->m_WindowList.clear();
	pI->m_WindowFindMap.clear();
	pI->m_WindowRectMap.clear();
	pI->m_WindowNameMap.clear();
	pI->m_WindowClassMap.clear();
	pI->m_WindowZOrderMap.clear();
	pI->m_MouseDownWindow = NULL;

	// �}�E�X�J�[�\���̈ʒu�̎擾.
	const D3DXPOSITION2& MousePos = Input::GetMousePosition();

	// ���C���E�B���h�E( �őO�ʂŌŒ� )��艺�̃E�B���h�E�𒲂ׂĂ���.
	HWND hCheckWnd	= pI->m_hMyWindow;
	int	 ZOrder		= 0;
	while ( 1 ) {
		// �m�F����E�B���h�E�̉��̃E�B���h�E���擾.
		hCheckWnd = GetWindow( hCheckWnd, GW_HWNDNEXT );

		// ���ɃE�B���h�E�������ꍇ�I������.
		if ( hCheckWnd == NULL ) break;

		// ���̃Q�[���̏ꍇ�X�L�b�v����.
		if ( hCheckWnd == pI->m_hMyWindow		) continue;
		if ( hCheckWnd == pI->m_hMySubWindow	) continue;

		// �E�B���h�E�����擾.
		WCHAR szName[256];
		GetWindowTextW( hCheckWnd, szName, 256 );

		// �f�X�N�g�b�v�E�B���h�E�}�l�[�W���[�̊m�F.
		int IsCloaked;
		DwmGetWindowAttribute( hCheckWnd, DWMWA_CLOAKED, &IsCloaked, sizeof( int ) );
		if ( IsCloaked != 0 ) continue;

		// �E�B���h�E�𒲂ׂ�.
		if ( ( wcslen( szName ) <= 0 ) ||
			!IsWindowVisible( hCheckWnd ) ||
			( GetParent( hCheckWnd ) != NULL ) ||
			( GetWindow( hCheckWnd, GW_OWNER ) != NULL ) ||
			( GetWindowLongPtrW( hCheckWnd, GWL_EXSTYLE ) & WS_EX_TOOLWINDOW ) )
		{
			continue;
		}

		// �k�����������ׂ�.
		if ( IsIconic( hCheckWnd ) ) continue;

		// �E�B���h�E�̏���ǉ�����.
		RECT NoShadowRect;
		RECT Rect;
		DwmGetWindowAttribute( hCheckWnd, DWMWA_EXTENDED_FRAME_BOUNDS, &NoShadowRect, sizeof( RECT ) );
		GetWindowRect( hCheckWnd, &Rect );

		// �E�B���h�E�̏��𒲐�����.
		NoShadowRect.left	+= pI->m_AddWindowRect.left;
		NoShadowRect.top	+= pI->m_AddWindowRect.top;
		NoShadowRect.right	+= pI->m_AddWindowRect.right;
		NoShadowRect.bottom	+= pI->m_AddWindowRect.bottom;
		Rect.left			+= pI->m_AddWindowRect.left;
		Rect.top			+= pI->m_AddWindowRect.top;
		Rect.right			+= pI->m_AddWindowRect.right;
		Rect.bottom			+= pI->m_AddWindowRect.bottom;

		// �N���X���̎擾.
		WCHAR szClass[256];
		GetClassNameW( hCheckWnd, szClass, 256 );

		// ���O�𕶎���ɕϊ�.
		const std::string WndName  = StringConversion::to_String( szName );
		const std::string WndClass = StringConversion::to_String( szClass );

		// �}�E�X�̉��ɂ��邩�m�F.
		if ( pI->m_MouseDownWindow == NULL ){
			if ( ( MousePos.x < NoShadowRect.right )  && ( MousePos.x > NoShadowRect.left ) &&
				 ( MousePos.y < NoShadowRect.bottom ) && ( MousePos.y > NoShadowRect.top  ) )
			{
				pI->m_MouseDownWindow = hCheckWnd;
			}
		}

		// �E�B���h�E��ۑ�.
		pI->m_WindowList.emplace_back( std::make_pair( NoShadowRect, hCheckWnd ) );
		pI->m_WindowFindMap[WndName]			= hCheckWnd;
		pI->m_WindowRectMap[hCheckWnd]			= Rect;
		pI->m_WindowNoShadowRectMap[hCheckWnd]	= NoShadowRect;
		pI->m_WindowNameMap[hCheckWnd]			= WndName;
		pI->m_WindowClassMap[hCheckWnd]			= WndClass;
		pI->m_WindowZOrderMap[hCheckWnd]		= ZOrder;
		ZOrder++;
	}

	// �E�B���h�E�̈ʒu�̍X�V���s����.
	pI->m_IsWindowUpdate = true;
}

//---------------------------.
// �f�X�N�g�b�v�̃A�C�R���̈ʒu�̍X�V.
//---------------------------.
void WindowManager::DesktopIconUpdate()
{
	WindowManager* pI = GetInstance();

	// �A�C�e�����擾.
	int nCount;
	nCount = ListView_GetItemCount( pI->m_hDesktop );
	pI->m_IconList.clear();
	pI->m_IconList.resize( nCount );

	// �ΏۃA�C�e���̎擾.
	for( int i = 0; i < nCount; i++ ) {
		// �A�C�R�����̎擾.
		TCHAR	buff[1024] = L"";	
		LVITEM	li;
		li.mask			= LVIF_TEXT;
		li.iItem		= i;
		li.iSubItem		= 0;
		li.cchTextMax	= 1024;
		li.pszText		= ( ( LPTSTR )pI->m_DProcessMemory ) + 1024;
		WriteProcessMemory( pI->m_DProcessHandle, pI->m_DProcessMemory, &li, sizeof( LVITEM ), NULL );
		SendMessage( pI->m_hDesktop, LVM_GETITEM, i, ( LPARAM )pI->m_DProcessMemory );
		ReadProcessMemory( pI->m_DProcessHandle, ( ( LPTSTR )pI->m_DProcessMemory ) + 1024, buff, 1024, NULL );

		// �A�C�R���̈ʒu�̎擾.
		POINT p;
		SendMessage( pI->m_hDesktop, LVM_GETITEMPOSITION, i, ( LPARAM )pI->m_DProcessMemory );
		ReadProcessMemory( pI->m_DProcessHandle, pI->m_DProcessMemory, &p, sizeof( POINT ), NULL );
		
		// �A�C�R���̈ʒu�𒲐�����.
		p.x += pI->m_AddWindowRect.left;
		p.y += pI->m_AddWindowRect.top;

		// �A�C�R������ۑ�.
		const D3DXPOSITION2 IconPos		= { static_cast<float>( p.x ), static_cast<float>( p.y ) };
		const std::string	IconName	= StringConversion::to_String( buff );
		pI->m_IconList[i]				= std::make_pair( IconPos, IconName );
		pI->m_IconFindMap[IconName]		= i;
		pI->m_IconPosMap[i]				= IconPos;
		pI->m_IconNameMap[i]			= IconName;
	}

	// �f�X�N�g�b�v�̃A�C�R���̈ʒu�̍X�V���s����.
	pI->m_IsDesktopIconUpdate = true;
	return;
}

//---------------------------.
// �^�X�N�o�[�̍X�V.
//---------------------------.
void WindowManager::TaskBarUpdate()
{
	WindowManager* pI = GetInstance();

	if ( pI->m_IsTaskBarUpdate ) return;

	ZeroMemory( &pI->m_TaskBar, sizeof( pI->m_TaskBar ) );
	pI->m_TaskBar.cbSize = sizeof( pI->m_TaskBar );
	SHAppBarMessage( ABM_GETTASKBARPOS, &pI->m_TaskBar );
}
