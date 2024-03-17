#pragma once
#include "..\..\Global.h"

// �E�B���h�E�N���X�萔.
namespace WindowClassType {
	const std::string NOTEPAD_CLASS		= "Notepad";			// ������.
	const std::string IE_BROWSER_CLASS	= "IEFrame";			// IE�u���E�U.
	const std::string BROWSER_CLASS		= "Chrome_WidgetWin_1";	// GoogleChrome / MicrosoftEdge.
	const std::string EXPLORER_CLASS	= "CabinetWClass";		// �G�N�X�v���[��.
}
namespace {
	// �A�C�R���C���f�b�N�X�^.
	using ICOINDEX = int;
}

/************************************************
*	�E�B���h�E�̏��}�l�[�W���[�N���X.
**/
class WindowManager
{
public:
	using IconList		= std::vector<std::pair<D3DXPOSITION2, std::string>>;
	using IconFindMap	= std::unordered_map<std::string, ICOINDEX>;
	using IconPosMap	= std::unordered_map<ICOINDEX, D3DXPOSITION2>;
	using IconNameMap	= std::unordered_map<ICOINDEX, std::string>;
	using WndList		= std::vector<std::pair<RECT, HWND>>;
	using WndFindMap	= std::unordered_map<std::string, HWND>;
	using WndRectMap	= std::unordered_map<HWND, RECT>;
	using WndNameMap	= std::unordered_map<HWND, std::string>;
	using WndZOrderMap	= std::unordered_map<HWND, int>;

public:
	WindowManager();
	~WindowManager();

	// ������.
	static HRESULT Init();
	// �X�V.
	static void Update();

	// �f�X�N�g�b�v�̃n���h���̎擾.
	static HWND GetDesktop() { return GetInstance()->m_hDesktop; }
	// ���̃Q�[���̃E�B���h�E�n���h���̎擾.
	static HWND GetWnd() { return GetInstance()->m_hMyWindow; }
	static HWND GetSubWnd() { return GetInstance()->m_hMySubWindow; }

	// ���̃Q�[���̃E�B���h�E�̏����擾.
	static RECT GetMyWndSize() { return GetInstance()->m_MyWindowRect; }

	// �^�X�N�o�[�̏����擾.
	static RECT GetTaskBarRect();
	// �^�X�N�o�[�̃E�B���h�E�n���h���̎擾.
	static HWND GetTaskBarWnd();

	// �E�B���h�E�̏��̕␳�l�̎擾.
	static RECT GetAddWindowRect() { return GetInstance()->m_AddWindowRect; }

	// �}�E�X�����̃E�B���h�E�̏�ɂ���ꍇ���̃E�B���h�E�̃n���h�����擾(��ɂȂ��ꍇ:NULL).
	static HWND GetMouseOverTheWindow();
	// �}�E�X�����̃E�B���h�E�̏�ɂ��邩�擾.
	static bool GetIsMouseOverTheWindow();

	// �w�肵�����W�����̃E�B���h�E�̏�ɂ���ꍇ���̃E�B���h�E�̃n���h�����擾(��ɂȂ��ꍇ:NULL).
	static HWND GetPointOverTheWindow( const D3DXPOSITION3& Pos );
	static HWND GetPointOverTheWindow( const D3DXPOSITION2& Pos );
	// �w�肵�����W�����̃E�B���h�E�̏�ɂ��邩�擾.
	static bool GetIsPointOverTheWindow( const D3DXPOSITION3& Pos );
	static bool GetIsPointOverTheWindow( const D3DXPOSITION2& Pos );

	// �E�B���h�E�̖��O����n���h�����擾.
	static HWND GetFindWindow( const std::string& Name );
	// �f�X�N�g�b�v�̃A�C�R��������A�C�R���C���f�b�N�X���擾.
	static ICOINDEX GetFindDesktopIcon( const std::string& Name );

	// �E�B���h�E�̏�񃊃X�g���擾.
	//	<�h���b�v�V���h�E���������E�B���h�E�̏��, �E�B���h�E�n���h��>.
	static WndList GetWindowList();
	// �f�X�N�g�b�v�̃A�C�R���̈ʒu���X�g�̎擾.
	//	<�A�C�R���̈ʒu, �A�C�R����>.
	static IconList	GetDesktopIconList();

	// �E�B���h�E�̏��̎擾.
	static RECT GetWindowSize( const HWND& hWnd );
	// �h���b�v�V���h�E���������E�B���h�E�̏��̎擾.
	static RECT GetWindowNoShadowSize( const HWND& hWnd );
	// �E�B���h�E���̎擾.
	static std::string GetWindowName( const HWND& hWnd );
	// �E�B���h�E�N���X���̎擾.
	static std::string GetWindowClassName( const HWND& hWnd );
	// �E�B���h�E��Z�I�[�_�[�̎擾( ���������������㑤 ).
	static int GetWindowZOrder( const HWND& hWnd );

	// �f�X�N�g�b�v�̃A�C�R���̈ʒu���擾.
	static D3DXPOSITION2 GetDesktopIconPosition( const ICOINDEX Index );
	// �f�X�N�g�b�v�̃A�C�R���̖��O���擾.
	static std::string GetDesktopIconName( const ICOINDEX Index );
	// �f�X�N�g�b�v�̃A�C�R���̃t�@�C���p�X���擾.
	static std::string GetDesktopIconPath( const ICOINDEX Index );

	// �}�E�X�̉��ɂ���E�B���h�E�̎擾.
	static HWND GetMouseDownWindow() { return GetInstance()->m_MouseDownWindow; }

	// �f�X�N�g�b�v�̃A�C�R���̈ʒu��ݒ�.
	static void SetDesktopIconPosition( const ICOINDEX Index, const int x, const int y );
	static void SetDesktopIconPosition( const ICOINDEX Index, const D3DXPOSITION2& Pos );
	static void SetDesktopIconPosition( const ICOINDEX Index, const D3DXPOSITION3& Pos );
	// �f�X�N�g�b�v�̃A�C�R���̈ʒu��ǉ�.
	static void AddDesktopIconPosition( const ICOINDEX Index, const int x, const int y );
	static void AddDesktopIconPosition( const ICOINDEX Index, const D3DXPOSITION2& Pos );
	static void AddDesktopIconPosition( const ICOINDEX Index, const D3DXPOSITION3& Pos );

	// �E�B���h�E�̈ʒu��ݒ�.
	static void SetWindowPosition( const HWND& hWnd, const int x, const int y );
	static void SetWindowPosition( const HWND& hWnd, const D3DXPOSITION2& Pos );
	static void SetWindowPosition( const HWND& hWnd, const D3DXPOSITION3& Pos );
	// �E�B���h�E�̈ʒu��ǉ�.
	static void AddWindowPosition( const HWND& hWnd, const int x, const int y );
	static void AddWindowPosition( const HWND& hWnd, const D3DXPOSITION2& Pos );
	static void AddWindowPosition( const HWND& hWnd, const D3DXPOSITION3& Pos );

	// �E�B���h�E�̃T�C�Y��ݒ�.
	static void SetWindowSize( const HWND& hWnd, const int x, const int y );
	static void SetWindowSize( const HWND& hWnd, const D3DXSCALE2& Size );
	static void SetWindowSize( const HWND& hWnd, const D3DXSCALE3& Size );
	// �E�B���h�E�̃T�C�Y��ǉ�.
	static void AddWindowSize( const HWND& hWnd, const int x, const int y );
	static void AddWindowSize( const HWND& hWnd, const D3DXSCALE2& Size );
	static void AddWindowSize( const HWND& hWnd, const D3DXSCALE3& Size );

	// �E�B���h�E���őO�ʂɈړ�.
	static void SetWindowTop( const HWND& hWnd, const bool IsLock = false );
	// �E�B���h�E���Ŕw�ʂɈړ�.
	static void SetWindowBottom( const HWND& hWnd );
	// �E�B���h�E���w�肵���E�B���h�E�̉��Ɉړ�.
	static void SetWindowSelectDown( const HWND& hWnd, const HWND& hSelectWnd );
	// �E�B���h�E���w�肵���E�B���h�E�̏�Ɉړ�.
	static void SetWindowSelectUp( const HWND& hWnd, const HWND& hSelectWnd );

	// �E�B���h�E�̔j��( "hWnd" �� "NULL" �̏ꍇ���̃E�B���h�E��j������ ).
	static void WindowDelete( const HWND& hWnd = NULL );

	// �E�B���h�E����������.
	static void WindowFlash( const HWND& hWnd );

	// �E�B���h�E�n���h���̐ݒ�.
	static void SetWnd( HWND hWnd ) { GetInstance()->m_hMyWindow = hWnd; }
	static void SetSubWnd( HWND hWnd ) { GetInstance()->m_hMySubWindow = hWnd; }

	// �A�v���̃V���[�g�J�b�g���쐬����.
	static bool CreateShortcut(
		LPCTSTR pszLink,				LPCTSTR pszFile,
		LPCTSTR pszDescription = NULL,	LPCTSTR pszArgs		= NULL,
		LPCTSTR pszWorkingDir  = NULL,	LPCTSTR pszIconPath	= NULL,
		int nIcon = 0, int nShowCmd = SW_SHOWNORMAL );

private:
	// �C���X�^���X�̎擾.
	static WindowManager* GetInstance();

	// �E�B���h�E�̍X�V.
	static void WindowListUpdate();

	// �f�X�N�g�b�v�̃A�C�R���̈ʒu�̍X�V.
	static void DesktopIconUpdate();
	
	// �^�X�N�o�[�̍X�V.
	static void TaskBarUpdate();

private:
	HWND			m_hDesktop;					// �f�X�N�g�b�v�̃n���h��.
	HWND			m_hMyWindow;				// ���̃Q�[���̃E�B���h�E�̃n���h��.
	HWND			m_hMySubWindow;				// ���̃Q�[���̃T�u�E�B���h�E�̃n���h��.
	HWND			m_MouseDownWindow;			// �}�E�X�J�[�\���̉��ɂ���E�B���h�E.
	RECT			m_MyWindowRect;				// ���̃Q�[���̃E�B���h�E�̏��.
	RECT			m_AddWindowRect;			// �E�B���h�E�̏��ɒǉ�����␳�l.
	APPBARDATA		m_TaskBar;					// �^�X�N�o�[�̏��.
	DWORD			m_DProcessID;				// �f�X�N�g�b�v�̃v���Z�XID.
	HANDLE			m_DProcessHandle;			// �f�X�N�g�b�v�̃v���Z�X�̃n���h��.
	LPVOID			m_DProcessMemory;			// �f�X�N�g�b�v�̃v���Z�X�̃�����.
	WndList			m_WindowList;				// �E�B���h�E�̏�񃊃X�g.
	WndFindMap		m_WindowFindMap;			// �E�B���h�E�̖��O�Ńn���h���擾�p���X�g.
	WndRectMap		m_WindowRectMap;			// �E�B���h�E���擾�p���X�g.
	WndRectMap		m_WindowNoShadowRectMap;	// �h���b�v�V���h�E���������E�B���h�E���擾�p���X�g.
	WndNameMap		m_WindowNameMap;			// �E�B���h�E���擾�p���X�g.
	WndNameMap		m_WindowClassMap;			// �E�B���h�E�N���X���擾�p���X�g.
	WndZOrderMap	m_WindowZOrderMap;			// �E�B���h�E��Z�I�[�_�[�擾�p���X�g.
	IconList		m_IconList;					// �A�C�R���̈ʒu���X�g.
	IconFindMap		m_IconFindMap;				// �A�C�R���̖��O�ŃA�C�R���̈ʒu�擾�p���X�g.
	IconPosMap		m_IconPosMap;				// �A�C�R���̃C���f�b�N�X�擾�p���X�g.
	IconNameMap		m_IconNameMap;				// �A�C�R���̃C���f�b�N�X�擾�p���X�g.
	std::string		m_UserName;					// ���[�U�[��.
	bool			m_IsWindowUpdate;			// �E�B���h�E���X�g���X�V������.
	bool			m_IsDesktopIconUpdate;		// �f�X�N�g�b�v�̃A�C�R���̈ʒu���X�V������.
	bool			m_IsTaskBarUpdate;			// �^�X�N�o�[�̍X�V��������.
};