#include "Main.h"
#include "..\Common\DirectX\DirectX9.h"
#include "..\Common\DirectX\DirectX11.h"
#include "..\Common\WindowTextRenderer\WindowTextRenderer.h"
#include "..\Common\SoundManeger\SoundManeger.h"
#include "..\Object\Camera\CameraManager\CameraManager.h"
#include "..\Object\Light\Light.h"
#include "..\Object\Collision\CollisionRenderer\CollisionRenderer.h"
#include "..\Resource\LoadManager\LoadManager.h"
#include "..\Scene\SceneManager\SceneManager.h"
#include "..\Utility\FrameRate\FrameRate.h"
#include "..\Utility\ImGuiManager\ImGuiManager.h"
#include "..\Utility\Random\Random.h"
#include "..\Utility\Input\Input.h"
#include "..\Utility\FileManager\FileManager.h"
#include "..\Utility\FileManager\DragAndDrop\DragAndDrop.h"
#include "..\Utility\FileManager\DragAndDrop\DragAndDrop.h"
#include "..\Utility\Message\Message.h"
#include "..\Utility\WindowManager\WindowManager.h"
#include "..\Utility\MenuManager\MenuManager.h"
#include "..\System\SystemWindowManager\SystemWindowManager.h"
#include <dwmapi.h>

// ImGUi�Ŏg�p.
extern LRESULT ImGui_ImplWin32_WndProcHandler( HWND, UINT, WPARAM, LPARAM );

static const decltype( WM_USER ) WM_NOTIFYICON = WM_USER + 100;

// �T�u�E�B���h�E���펞�N���b�N����𖳂�����
#define SUB_WND_ALWAY_CLICK_LOSE

namespace{
	constexpr char		PARAMETER_FILE_PATH[]		= "Data\\Parameter\\";								// �p�����[�^�[�t�@�C���p�X.
	constexpr char		WINDOW_SETTING_FILE_PATH[]	= "Data\\Parameter\\Config\\WindowSetting.json";	// �E�B���h�E�̐ݒ�̃t�@�C���p�X.
	constexpr float		FPS_RENDER_SIZE				= 0.5f;												// FPS�`��T�C�Y.
	constexpr float		FPS_RENDER_POS[2]			= { 0.0f, 1.0f };									// FPS�`��ʒu.
	constexpr float		FPS_RENDER_COLOR[4]			= { 0.9f, 0.2f, 0.2f, 1.0f };						// FPS�`��F.

	// �t�F�[�h�̏��.
	enum enFadeState : unsigned char {
		None,
		FadeIn,		// �t�F�[�h�C����.
		FadeOut		// �t�F�[�h�A�E�g��.
	} typedef EFadeState;
}

CMain::CMain()
	: m_hWnd			( nullptr )
	, m_hSubWnd			( nullptr )
	, m_hDc				( nullptr )
	, m_hSubDc			( nullptr )
	, m_pFrameRate		( nullptr )
	, m_pLoadManager	( nullptr )
	, m_IsGameLoad		( false )
	, m_IsFPSRender		( false )
#ifdef _DEBUG
	, m_IsWindowTop		( false )
#else
	, m_IsWindowTop		( true )
#endif
{
	m_pFrameRate	= std::make_unique<CFrameRate>( FPS );
	m_pLoadManager	= std::make_unique<CLoadManager>();
}

CMain::~CMain()
{
	DeleteDC(		m_hSubDc	);
	DeleteDC(		m_hDc		);
	DeleteObject(	m_hSubWnd	);
	DeleteObject(	m_hWnd		);
}

//---------------------------.
// �X�V����.
//---------------------------.
void CMain::Update( const float& DeltaTime )
{
	// �N�����̃E�B���h�E�̏�����.
	if ( m_IsWindowInit == false ) WindowInit();

	// �őO�ʂŌŒ肵������.
	if ( m_IsWindowTop ) SetWindowPos( m_hWnd, HWND_TOPMOST, 0, 0, 0, 0, ( SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW ) );
#ifdef _DEBUG
	// �őO�ʂŌŒ������.
	if ( KeyInput::IsANDKeyDown( 'W', 'T' ) ) {
		m_IsWindowTop = !m_IsWindowTop;
		if ( m_IsWindowTop == false ) {
			SetWindowPos( m_hWnd, HWND_NOTOPMOST,	0, 0, 0, 0, ( SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW ) );
			SetWindowPos( m_hWnd, HWND_TOP,			0, 0, 0, 0, ( SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW ) );
		}
	}
#endif

	// �E�B���h�E�̃N���b�N����̍X�V.
	ClickUpdate();

	// �X�V����.
	DirectX11::CheckActiveWindow();
	ImGuiManager::SetingNewFrame();
	WindowManager::Update();
	Input::Update();
	SceneManager::Update( DeltaTime );
	CameraManager::Update( DeltaTime );
	Light::Update();
	Message::Update( DeltaTime );
	SystemWindowManager::Update( DeltaTime );

	// �o�b�N�o�b�t�@���N���A�ɂ���.
	DirectX11::ClearBackBuffer( 0 );

	// �`�揈��.
	SceneManager::Render();
	FPSRender();

	DirectX11::Present( 0 );

	// �T�u�E�B���h�E�̃o�b�N�o�b�t�@���N���A�ɂ���.
	DirectX11::ClearBackBuffer( 1 );

	// �T�u�E�B���h�E�̕`�揈��.
	SceneManager::SubRender();

	CollisionRenderer::Render();
	ImGuiManager::Render();

	// ��ʂɕ\��.
	DirectX11::Present( 1 );

	// TODO: ������x�ĂԂƂ�������N���Ȃ�?
	DirectX11::Present( 0 );

	// ����̃��O���o��.
	Input::KeyLogOutput();

	// �z�C�[���l��������.
	Input::SetMouseWheelDelta( 0 );
}

//---------------------------.
// �\�z����.
//---------------------------.
HRESULT CMain::Create()
{
	// DirectX9�̍\�z.
	if ( FAILED( DirectX9::Create( m_hWnd	)	) ) return E_FAIL;
	// DirectX11�̍\�z.
	if ( FAILED( DirectX11::Create( { m_hWnd, m_hSubWnd } ) ) ) return E_FAIL;
	// �I�[�f�B�I�C���^�[�t�F�[�X�̍\�z.
	if ( FAILED( SoundManager::Create()		) ) return E_FAIL;
	// �����̏�����.
	if ( FAILED( Random::Init()				) ) return E_FAIL;
	// ���̓N���X�̏�����.
	if ( FAILED( Input::Init()					) ) return E_FAIL;
	// �f�o�b�N�e�L�X�g�̏�����.
	if ( FAILED( WindowTextRenderer::Init()			) ) return E_FAIL;
	// ImGui�̏�����.
	if ( FAILED( ImGuiManager::Init( m_hWnd )	) ) return E_FAIL;
	// �E�B���h�E�}�l�[�W���[�̏�����.
	if ( FAILED( WindowManager::Init()			) ) return E_FAIL;
	// ���j���[�̏�����.
	if ( FAILED( MenuManager::Init( m_hWnd )	) ) return E_FAIL;

	// �[�x�𖳂���.
	DirectX11::SetDepth( false );

	// �t���X�N���[���ŋN�����邩.
	json WndSetting = FileManager::JsonLoad( WINDOW_SETTING_FILE_PATH );
	if ( WndSetting["IsStartFullScreen"] ) {
		// �t���X�N���[���ɐݒ�.
		DirectX11::SetFullScreen( true );
	}

	// ���\�[�X�̓ǂݍ���.
	m_pLoadManager->LoadResource( m_hWnd );

	// ���[�U�[���̎擾.
#ifndef _DEBUG
	char UserName[256];
	DWORD Size = sizeof( UserName );
	if ( GetUserNameA( UserName, &Size ) == 0 ) {
		//�擾�Ɏ��s����
		puts( "���[�U�[���̎擾�Ɏ��s���܂����B" );
	}

	// �V���[�g�J�b�g�̍쐬.
	const std::string WorkFile		= std::filesystem::current_path().string();
	const std::string ExePath		= WorkFile + "\\Slime.exe";
	const std::string ShortcutPath	= "C:\\Users\\" + std::string( UserName ) + "\\AppData\\Roaming\\Microsoft\\Windows\\Start Menu\\Programs\\Startup\\Slime.lnk";
	WindowManager::CreateShortcut(
		StringConversion::to_wString( ShortcutPath ).c_str(),
		StringConversion::to_wString( ExePath ).c_str(),
		NULL,
		NULL,
		StringConversion::to_wString( WorkFile ).c_str() );
#endif // #ifndef _DEBUG.
	
	const std::string appv = WndSetting["Version"];
	const std::string fp   = PARAMETER_FILE_PATH + appv + std::string( ".bin" );
#ifdef _DEBUG
	// �o�[�W�����t�@�C���̍쐬
	FileManager::BinarySave( fp.c_str(), appv );
#else
	if ( FileManager::FileCheck( fp ) == false ) {
		InfoMessage( "�Â��o�[�W�����̃f�[�^�ł�.." );
	}
#endif

	// �E�B���h�E�n���h���̐ݒ�.
	Input::SethWnd( m_hWnd );
	DragAndDrop::SethWnd( m_hWnd );
	return S_OK;
}

//---------------------------.
// ���b�Z�[�W���[�v.
//---------------------------.
void CMain::Loop()
{
	// ���b�Z�[�W���[�v.
	MSG msg = { 0 };
	ZeroMemory( &msg, sizeof( msg ) );

	while ( msg.message != WM_QUIT ) {
		if ( PeekMessage( &msg, nullptr, 0, 0, PM_REMOVE ) ) {
			TranslateMessage( &msg );
			DispatchMessage( &msg );
		}
		else {
			// �t���[�����[�g�̑ҋ@����.
			if ( m_pFrameRate->Wait() ) continue;

			// ���[�h���̍X�V����.
			m_IsGameLoad = m_pLoadManager->ThreadRelease();
			//�X�V����.
			if ( m_IsGameLoad ) {
				Update( static_cast<float>( m_pFrameRate->GetDeltaTime() ) );
			}
		}
	}
	Log::PushLog( "------ ���C�����[�v�I�� ------" );
}

//---------------------------.
// �E�B���h�E�������֐�.
//---------------------------.
HRESULT CMain::InitWindow( HINSTANCE hInstance )
{
	// �E�B���h�E�̐ݒ�̎擾.
	json WndSetting = FileManager::JsonLoad( WINDOW_SETTING_FILE_PATH );
	
	// �A�v����/�E�B���h�E�����擾.
	const std::wstring wAppName = StringConversion::to_wString( WndSetting["Name"]["App"], ECodePage::UTF8 );
	const std::wstring wWndName = StringConversion::to_wString( WndSetting["Name"]["Wnd"], ECodePage::UTF8 );

	// FPS��`�悷�邩.
	m_IsFPSRender = WndSetting["IsFPSRender"];

	// �E�B���h�E�̒�`.
	WNDCLASSEX wc;
	ZeroMemory( &wc, sizeof( wc ) );// ������(0��ݒ�).

	wc.cbSize			= sizeof( wc );
	wc.style			= CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	wc.lpfnWndProc		= MsgProc;// WndProc;
	wc.hInstance		= hInstance;
	wc.hIcon			= LoadIcon( hInstance, _T( "ICON" ) );
	wc.hCursor			= LoadCursor( nullptr, IDC_ARROW );
	wc.hbrBackground	= (HBRUSH)GetStockObject( LTGRAY_BRUSH );
	wc.lpszClassName	= wAppName.c_str();
	wc.hIconSm			= wc.hIcon;
	
	// �E�B���h�E�N���X��Windows�ɓo�^.
	if ( !RegisterClassEx( &wc ) ) {
		ErrorMessage( "�E�B���h�E�N���X�̓o�^�Ɏ��s" );
		return E_FAIL;
	}

	// �E�B���h�E�̈�̒���.
	RECT	rect;			// ��`�\����.
	DWORD	dwStyle;		// �E�B���h�E�X�^�C��.
	DWORD	dwExStyle;		// �E�B���h�E�̊g���X�^�C��.

	int sizex = GetSystemMetrics( SM_CXVIRTUALSCREEN );
	int sizey = GetSystemMetrics( SM_CYVIRTUALSCREEN );
	int dispx = GetSystemMetrics( SM_XVIRTUALSCREEN );
	int dispy = GetSystemMetrics( SM_YVIRTUALSCREEN );

	rect.left	= dispx;											// ��.
	rect.top	= dispy;											// ��.
	rect.right	= sizex + dispx;									// �E.
	rect.bottom = sizey + dispy;									// ��.
	dwStyle		= WS_OVERLAPPEDWINDOW;								// �E�B���h�E���.
	dwExStyle	= WS_EX_LAYERED | WS_EX_TOOLWINDOW;					// �E�B���h�E�g���@�\.
	if ( WndSetting["IsSizeLock"]	) dwStyle ^= WS_THICKFRAME;		// �T�C�Y�̕ύX���֎~���邩.
	if ( WndSetting["IsMaxLock"]	) dwStyle ^= WS_MAXIMIZEBOX;	// �g�剻���֎~���邩.
	if ( WndSetting["IsMinLock"]	) dwStyle ^= WS_MINIMIZEBOX;	// �g�剻���֎~���邩.
	if ( WndSetting["IsPopUpWnd"]	) dwStyle  = WS_POPUP;			// �g�����E�B���h�E.

	if ( AdjustWindowRect(
		&rect,	// (in)��ʃT�C�Y����������`�\����.(out)�v�Z����.
		dwStyle,// �E�B���h�E���.
		FALSE ) == 0 )// ���j���[�������ǂ����w��.
	{
		ErrorMessage( "�E�B���h�E�̈�̒����Ɏ��s" );
		return E_FAIL;
	}

	// �E�B���h�E�̍쐬.
	m_hWnd = CreateWindowEx(
		dwExStyle,					// �g���@�\( ���߃E�B���h�E:WS_EX_LAYERED ).
		wAppName.c_str(),			// �A�v����.
		wWndName.c_str(),			// �E�B���h�E�^�C�g��.
		dwStyle,					// �E�B���h�E���(����).
		rect.left, rect.top,		// �\���ʒux,y���W.
		sizex,						// �E�B���h�E��.
		sizey,						// �E�B���h�E����.
		nullptr,					// �e�E�B���h�E�n���h��.
		nullptr,					// ���j���[�ݒ�.
		hInstance,					// �C���X�^���X�ԍ�.
		nullptr );					// �E�B���h�E�쐬���ɔ�������C�x���g�ɓn���f�[�^.
	if ( !m_hWnd ) {
		ErrorMessage( "�E�B���h�E�쐬���s" );
		return E_FAIL;
	}
	WindowManager::SetWnd( m_hWnd );

	// �T�u�E�B���h�E�̍쐬.
	m_hSubWnd = CreateWindowEx(
		dwExStyle,					// �g���@�\( ���߃E�B���h�E:WS_EX_LAYERED ).
		wAppName.c_str(),			// �A�v����.
		wWndName.c_str(),			// �E�B���h�E�^�C�g��.
		dwStyle,					// �E�B���h�E���(����).
		rect.left, rect.top,		// �\���ʒux,y���W.
		sizex,						// �E�B���h�E��.
		sizey,						// �E�B���h�E����.
		nullptr,					// �e�E�B���h�E�n���h��.
		nullptr,					// ���j���[�ݒ�.
		hInstance,					// �C���X�^���X�ԍ�.
		nullptr );					// �E�B���h�E�쐬���ɔ�������C�x���g�ɓn���f�[�^.

	if ( !m_hSubWnd ) {
		ErrorMessage( "�T�u�E�B���h�E�쐬���s" );
		return E_FAIL;
	}
	WindowManager::SetSubWnd( m_hSubWnd );

	// DC�̎擾.
	m_hDc		= GetDC( m_hWnd );
	m_hSubDc	= GetDC( m_hSubWnd );

	// �^�X�N���C�̍쐬.
	NOTIFYICONDATA nid;
	nid.cbSize				= sizeof( NOTIFYICONDATA );				// ���̍\���̂̃o�C�g��.
	nid.hWnd				= m_hWnd;								// �E�B���h�E�n���h��.
	nid.uID					= 0;									// �����̃A�C�R����\�������Ƃ��̎���ID.
	nid.uFlags				= NIF_MESSAGE | NIF_ICON | NIF_TIP;		// ���̍\���̂̂ǂ̃����o���L�����������t���O.
	nid.uCallbackMessage	= WM_NOTIFYICON;						// WM_USER�ȍ~�̒萔.
	nid.hIcon				= LoadIcon( hInstance, _T( "ICON" ) );	// �^�X�N���C�̃A�C�R��.
	_tcscpy( nid.szTip, wAppName.c_str() );							// �^�X�N���C�ɕ\�����镶��.
	int ret = ( int ) Shell_NotifyIcon( NIM_ADD, &nid );			// �e�N�X���C�̊J�n.

	// �E�B���h�E�̕\��.
	ShowWindow( m_hWnd, SW_SHOW );
	ShowWindow( m_hSubWnd, SW_SHOW );
	UpdateWindow( m_hWnd );
	UpdateWindow( m_hSubWnd );

	// �Ŕw�ʂɈړ�������.
	SetWindowPos( m_hWnd,	 HWND_BOTTOM, 0, 0, 0, 0, ( SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW ) );
	SetWindowPos( m_hSubWnd, HWND_BOTTOM, 0, 0, 0, 0, ( SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW ) );

	// �}�E�X�J�[�\����\�����邩.
	if ( WndSetting["IsDispMouseCursor"] == false ) {
		DirectX11::SetIsDispMouseCursor( false );
		ShowCursor( FALSE );
	}
	return S_OK;
}

//---------------------------.
// �E�B���h�E�֐�(���b�Z�[�W���̏���).
//---------------------------.
LRESULT CALLBACK CMain::MsgProc(
	HWND hWnd, UINT uMsg,
	WPARAM wParam, LPARAM lParam )
{
	// ImGui�̃E�B���h�E�𓮂�������T�C�Y�ύX��������ł���悤�ɂ���.
	if ( ImGui_ImplWin32_WndProcHandler( hWnd, uMsg, wParam, lParam ) ) return true;
	IMGUI_API

	switch ( uMsg ) {
	case WM_CREATE:
	{
		// �w�肵���F�̔w�i�𓧖��ɂ���.
//		SetLayeredWindowAttributes( hWnd, RGB( 0, 0, 0 ), 0, LWA_COLORKEY );
		break;
	}
	case WM_KEYDOWN:// �L�[�{�[�h�������ꂽ�Ƃ�.
	{
		// �E�B���h�E�̐ݒ�̎擾.
		json WndSetting = FileManager::JsonLoad( WINDOW_SETTING_FILE_PATH );
		if ( WndSetting["IsDispCloseMessage"] == false ) break;
		
		// �L�[�ʂ̏���.
		switch ( static_cast<char>( wParam ) ) {
		case VK_ESCAPE:	// ESC�L�[.
			if ( MessageBox( nullptr, _T( "�Q�[�����I�����܂����H" ), _T( "�x��" ), MB_YESNO ) == IDYES ) {
				PostQuitMessage( 0 );
			}
		}
		break;
	}
	case WM_MOUSEWHEEL:
	{
		// �}�E�X�z�C�[���̓���.
		const int Delta = GET_WHEEL_DELTA_WPARAM(wParam);
		Input::SetMouseWheelDelta( Delta );
		break;
	}
	case  WM_LBUTTONDBLCLK:
		Input::SetIsLeftDoubleClick( true );
		break;
	case  WM_MBUTTONDBLCLK:
		Input::SetIsCenterDoubleClick( true );
		break;
	case  WM_RBUTTONDBLCLK:
		Input::SetIsRightDoubleClick( true );
		break;
	case WM_DROPFILES:
	{
		// �h���b�v�����t�@�C���p�X��ǉ����Ă���.
		HDROP	hDrop = (HDROP) wParam;
		int		Num = DragQueryFile( hDrop, -1, NULL, 0 );
		for ( int i = 0; i < Num; ++i ) {
			// �ړ���̃t�@�C���p�X���쐬.
			TCHAR DropPath[100];
			DragQueryFile( hDrop, i, DropPath, sizeof( DropPath ) / sizeof( TCHAR ) );
			std::string sDropPath = StringConversion::to_String( DropPath );
			size_t		Num = sDropPath.find( "Desktop\\" );
			std::filesystem::create_directories( "Data\\DropFile" );
			std::string FilePath = "Data\\DropFile\\" + sDropPath.erase( 0, Num + sizeof( "Desktop\\" ) - 1 );

			// �t�@�C�����ꎞ�I�Ɉړ������t�@�C���p�X��ۑ�����.
			MoveFile( DropPath, StringConversion::to_wString( FilePath ).c_str() );
			DragAndDrop::AddFilePath( FilePath );
		}
		DragFinish( hDrop );
		break;
	}
	case WM_CLOSE:
		// �E�B���h�E�̔j��.
		DestroyWindow( hWnd );
		break;
	case WM_DESTROY:
		// �^�X�N���C���\���ɂ���.
		NOTIFYICONDATA nid;
		nid.cbSize	= sizeof( NOTIFYICONDATA );
		nid.hWnd	= hWnd;
		nid.uID		= 0;
		nid.uFlags	= 0;
		Shell_NotifyIcon( NIM_DELETE, &nid );

		// �I��.
		PostQuitMessage( 0 );
		break;
	case WM_NOTIFYICON:
		// ���j���[�̕\��.
		if ( wParam == 0 && lParam == WM_RBUTTONDOWN ) MenuManager::Disp();
		break;
	case WM_COMMAND:
		// ���j���[�̑I��.
		MenuManager::SelectMenu( LOWORD( wParam ) );
		break;
	case WM_SIZE:
		break;
	case WM_WINDOWPOSCHANGING:
		// �T�u�E�B���h�E�̈ʒu���X�V���ꂽ���ɍŔw�ʂɕύX����
		if ( hWnd != WindowManager::GetSubWnd() ) break;
		( ( LPWINDOWPOS )lParam )->hwndInsertAfter = HWND_BOTTOM;
		break;
	}

	// ���C���ɕԂ����.
	return DefWindowProc( hWnd, uMsg, wParam, lParam );
}

//---------------------------.
// �E�B���h�E�N�����̏�����( �N�������̂� ).
//---------------------------.
void CMain::WindowInit()
{
	// �E�B���h�E�ɃK���X���ʂ�t�^����.
	MARGINS Margins = { -1 };
	DwmExtendFrameIntoClientArea( m_hWnd,	 &Margins );
	DwmExtendFrameIntoClientArea( m_hSubWnd, &Margins );
	
	// �T�u�E�B���h�E�̓������ʂ𖳌��ɂ���.
	LONG SublStyle = GetWindowLong( m_hSubWnd, GWL_EXSTYLE );
	SublStyle ^= WS_EX_LAYERED;
	SetWindowLong( m_hSubWnd, GWL_EXSTYLE, SublStyle );
#ifdef SUB_WND_ALWAY_CLICK_LOSE
	// �T�u�͏펞�N���b�N�̔���𖳂���
	SublStyle |= WS_EX_TRANSPARENT | WS_EX_LAYERED | WS_EX_TOOLWINDOW;
	SetWindowLong( m_hSubWnd, GWL_EXSTYLE, SublStyle );
#endif

	// �E�B���h�E�̓������ʂ𖳌��ɂ���.
	LONG MainlStyle = GetWindowLong( m_hWnd, GWL_EXSTYLE );
	MainlStyle ^= WS_EX_LAYERED;
	SetWindowLong( m_hWnd, GWL_EXSTYLE, MainlStyle );

	// �E�B���h�E�̏��������s����.
	m_IsWindowInit = true;
}

//---------------------------.
// FPS�̕`��.
//---------------------------.
void CMain::FPSRender()
{
#ifdef _DEBUG
	if ( DebugKeyInput::IsANDKeyDown( VK_CONTROL, VK_F12 )	) m_IsFPSRender = !m_IsFPSRender;
	if ( m_IsFPSRender == false								) return;

	// �\�����镶���̍쐬.
	std::string FPSText =
		"    FPS    : " + std::to_string( static_cast<int>( m_pFrameRate->GetFPS() ) ) + '\n' +
		" DeltaTime : " + std::to_string( m_pFrameRate->GetDeltaTime() );

	// FPS�̕`��.
	WindowTextRenderer::DebugRender( FPSText, D3DXPOSITION3( FPS_RENDER_POS ), FPS_RENDER_SIZE, D3DXCOLOR4( FPS_RENDER_COLOR ) );

//	const D3DXVECTOR2 Pos = Input::GetMousePosition();
//
//	COLORREF MainColor = GetPixel( m_hDc, static_cast< int >( Pos.x ), static_cast< int >( Pos.y ) );
//	std::string Text = "<Main> R:" + std::to_string( GetRValue( MainColor ) ) + ", G:" + std::to_string( GetGValue( MainColor ) ) + ", B:" + std::to_string( GetBValue( MainColor ) );
//	WindowTextRenderer::DebugRender( Text, D3DXPOSITION3( 0.0f, 50.0f, 0.0f ), FPS_RENDER_SIZE, D3DXCOLOR4( FPS_RENDER_COLOR ) );
//
//	COLORREF SubColor = GetPixel( m_hSubDc, static_cast< int >( Pos.x ), static_cast< int >( Pos.y ) );
//	Text = "<Sub>  R:" + std::to_string( GetRValue( SubColor ) ) + ", G:" + std::to_string( GetGValue( SubColor ) ) + ", B:" + std::to_string( GetBValue( SubColor ) );
//	WindowTextRenderer::DebugRender( Text, D3DXPOSITION3( 0.0f, 80.0f, 0.0f ), FPS_RENDER_SIZE, D3DXCOLOR4( FPS_RENDER_COLOR ) );

#endif	// #ifdef _DEBUG.
}

//---------------------------.
// �E�B���h�E�̃N���b�N����̍X�V.
//---------------------------.
void CMain::ClickUpdate()
{
	// �}�E�X�̍��W�̎擾.
	const D3DXVECTOR2 Pos = Input::GetMousePosition();

	// ���C���E�B���h�E�̃}�E�X�̉��̃J���[�̎擾.
	COLORREF MainColor = GetPixel( m_hDc, static_cast<int>( Pos.x ), static_cast<int>( Pos.y ) );
	
	// �^�����̏ꍇ�N���b�N����𖳂���.
	if ( GetRValue( MainColor ) == 0 && GetGValue( MainColor ) == 0 && GetBValue( MainColor ) == 0 ) {
		LONG lStyle  = GetWindowLong( m_hWnd, GWL_EXSTYLE );
		lStyle		|= WS_EX_TRANSPARENT | WS_EX_LAYERED | WS_EX_TOOLWINDOW;
		SetWindowLong( m_hWnd, GWL_EXSTYLE, lStyle );
	}
	else {
		LONG lStyle  = GetWindowLong( m_hWnd, GWL_EXSTYLE );
		lStyle		&= WS_EX_TRANSPARENT & WS_EX_LAYERED | WS_EX_TOOLWINDOW;
		SetWindowLong( m_hWnd, GWL_EXSTYLE, lStyle );
	}

#ifndef SUB_WND_ALWAY_CLICK_LOSE
	// �T�u�E�B���h�E�̃}�E�X�̉��̃J���[�̎擾.
	COLORREF SubColor = GetPixel( m_hSubDc, static_cast<int>( Pos.x ), static_cast<int>( Pos.y ) );
	
	// �^�����̏ꍇ�N���b�N����𖳂���.
	if ( GetRValue( SubColor ) == 0 && GetGValue( SubColor ) == 0 && GetBValue( SubColor ) == 0 ) {
		LONG lStyle  = GetWindowLong( m_hSubWnd, GWL_EXSTYLE );
		lStyle		|= WS_EX_TRANSPARENT | WS_EX_LAYERED | WS_EX_TOOLWINDOW;
		SetWindowLong( m_hSubWnd, GWL_EXSTYLE, lStyle );
	}
	else {
		LONG lStyle  = GetWindowLong( m_hSubWnd, GWL_EXSTYLE );
		lStyle		&= WS_EX_TRANSPARENT & WS_EX_LAYERED | WS_EX_TOOLWINDOW;
		SetWindowLong( m_hSubWnd, GWL_EXSTYLE, lStyle );
	}
#endif
}
