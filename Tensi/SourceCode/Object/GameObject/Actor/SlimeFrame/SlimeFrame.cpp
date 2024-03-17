#include "SlimeFrame.h"
#include "..\..\..\..\Resource\SpriteResource\SpriteResource.h"
#include "..\..\..\..\Utility\WindowManager\WindowManager.h"
#include "..\..\..\..\Utility\Input\Input.h"
#include <dwmapi.h>

CSlimeFrame::CSlimeFrame()
	: m_pBack		( nullptr )
	, m_pIcon		( nullptr )
	, m_BackState	()
	, m_IconState	()
	, m_hWnd		( NULL )
{
}

CSlimeFrame::~CSlimeFrame()
{
	Reset();
}

//---------------------------.
// ������.
//---------------------------.
bool CSlimeFrame::Init()
{
	m_pBack = SpriteResource::GetSprite( "SlimeFrameBack", &m_BackState );
	m_pIcon = SpriteResource::GetSprite( "SlimeFrameIcon", &m_IconState );
	return true;
}

//---------------------------.
// �X�V.
//---------------------------.
void CSlimeFrame::Update( const float& DeltaTime )
{
	SelectWindow();	// �E�B���h�E�̑I��.

	if ( m_hWnd == NULL ) return;

	// �E�B���h�E�̏����擾.
	RECT Rect;
	const bool IsWndOpen	= GetWindowRect( m_hWnd, &Rect );
	const bool IsInWndSmall = IsIconic( m_hWnd );
	DwmGetWindowAttribute( m_hWnd, DWMWA_EXTENDED_FRAME_BOUNDS, &Rect, sizeof( RECT ) );

	// �\���̐ݒ�.
	m_BackState.IsDisp = !IsInWndSmall;
	m_IconState.IsDisp = !IsInWndSmall;

	// �X���C���t���[������������.
	if ( KeyInput::IsANDKeyDown( VK_CONTROL, VK_RBUTTON ) || ( IsWndOpen == false && IsInWndSmall == false ) ) {
		Reset();
		return;
	};

	// �w�i�̐ݒ�.
	m_BackState.Transform.Position.x = static_cast<float>( Rect.left ) - 7.0f;
	m_BackState.Transform.Position.y = static_cast<float>( Rect.top  ) - 7.0f;
	m_BackState.Transform.Scale.x	 = static_cast<float>( Rect.right  - Rect.left ) + 14.0f;
	m_BackState.Transform.Scale.y	 = static_cast<float>( Rect.bottom - Rect.top  ) + 14.0f;
	m_BackState.Color.w = 0.85f;

	// �A�C�R���̐ݒ�.
	m_IconState.Transform.Position.x = static_cast<float>( Rect.right ) + 7.0f;
	m_IconState.Transform.Position.y = static_cast<float>( Rect.top   ) + 37.0f;
	m_IconState.Color.w				 = 0.85f;
}

//---------------------------.
// �`��.
//---------------------------.
void CSlimeFrame::Render()
{
	if ( m_hWnd == NULL ) return;

	m_pIcon->RenderUI( &m_IconState );
}
void CSlimeFrame::SubRender()
{
	if ( m_hWnd == NULL ) return;

	m_pBack->RenderUI( &m_BackState );
}

//---------------------------.
// �E�B���h�E�̑I��.
//---------------------------.
void CSlimeFrame::SelectWindow()
{
	if ( m_hWnd								 != NULL  ) return;
	if ( KeyInput::IsKeyPress( VK_CONTROL ) == false ) return;
	if ( KeyInput::IsKeyDown(  VK_LBUTTON ) == false ) return;

	// �E�B���h�E�ɓ������Ă��邩���ׂ�.
	m_hWnd = WindowManager::GetMouseDownWindow();
	if ( m_hWnd == NULL ) return;
	SetWindowPos( m_hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW );
	return;
}

//---------------------------.
// �E�B���h�E�����ɖ߂�.
//---------------------------.
void CSlimeFrame::Reset()
{
	if ( m_hWnd == NULL ) return;

	SetWindowPos( m_hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW );
	SetWindowPos( m_hWnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW );
	m_hWnd = NULL;
}
