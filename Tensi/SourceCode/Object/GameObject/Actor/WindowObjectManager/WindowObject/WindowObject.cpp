#include "WindowObject.h"
#include "..\..\..\..\..\Utility\Input\Input.h"
#include <dwmapi.h>

namespace {
	// �d�̓x�N�g��.
	const D3DXVECTOR3 GRAVITY_VECTOR = { 0.0f, 1.0f, 0.0f };
}

CWindowObject::CWindowObject()
	: m_pSprite				( nullptr )
	, m_SpriteState			()
	, m_VectorList			()
	, m_OldMoveVector		( INIT_FLOAT3 )
	, m_MoveVector			( INIT_FLOAT3 )
	, m_GrabPosition		( INIT_FLOAT2 )
	, m_StartPosition		( INIT_FLOAT3 )
	, m_EndPosition			( INIT_FLOAT3 )
	, m_DireSpeedRate		( 1.0f, 1.0f, 1.0f, 1.0f )
	, m_InWndHandle			( NULL )
	, m_LandingWnd			()
	, m_AddWndRect			()
	, m_CollSize			( INIT_FLOAT )
	, m_GravityPower		( 1.0f )
	, m_SpeedRate			( INIT_FLOAT )
	, m_CeilingPosY			( INIT_FLOAT )
	, m_IsLanding			( false )
	, m_IsFall				( false )
	, m_IsGrab				( false )
	, m_IsInWndSmall		( false )
	, m_IsTrashCan			( false )
	, m_IsDisp				( true )
	, m_AddCenterPosition	( INIT_FLOAT3 )
{
}

CWindowObject::~CWindowObject()
{
}

//---------------------------.
// �}�E�X�ɐG��Ă��邩
//---------------------------.
bool CWindowObject::GetIsMouseOver()
{
	// �ʒu�̎擾.
	D3DXPOSITION3*		 Pos		= &m_SpriteState.Transform.Position;
	const D3DXPOSITION2& MousePos	= Input::GetMousePosition();

	// �}�E�X���摜�̏�ɂ��邩�ǂ���.
	const D3DXPOSITION3& CPos		 = *Pos + m_AddCenterPosition;
	if ( CPos.x - m_CollSize / 2.0f <= MousePos.x && MousePos.x <= CPos.x + m_CollSize / 2.0f &&
		 CPos.y - m_CollSize / 2.0f <= MousePos.y && MousePos.y <= CPos.y + m_CollSize / 2.0f )
	{
		return true;
	}
	return false;
}

//---------------------------.
// �E�B���h�E�I�u�W�F�N�g�̏�����.
//---------------------------.
void CWindowObject::WindowObjectInit()
{
	// �摜�̎擾.
	m_AddCenterPosition = m_pSprite->GetSpriteState().AddCenterPos;

	// �^�X�N�o�[�̏����擾.
	const RECT& TaskBar = WindowManager::GetTaskBarRect();

	// �E�B���h�E�̃T�C�Y�𓖂��蔻��ɑΉ������T�C�Y�ɍX�V.
	m_WndRect.top		= static_cast<LONG>( m_CeilingPosY	);
	m_WndRect.bottom	= static_cast<LONG>( TaskBar.top	);

	// �E�B���h�E�̏��̕␳�l�̎擾.
	m_AddWndRect = WindowManager::GetAddWindowRect();

	// �E�B���h�E�I�u�W�F�N�g�}�l�[�W���[�ɓo�^.
	WindowObjectManager::PushObject( this, &m_SpriteState.Transform );
}

//---------------------------.
// �E�B���h�E�I�u�W�F�N�g�̍X�V.
//---------------------------.
void CWindowObject::WindowObjectUpdate()
{
	Grab();						// �͂�.
	GrabMoveUpdate();			// �͂�ł��鎞�̍X�V.
	Separate();					// �͂�ł��镨�𗣂�.
	PushGravity();				// �d�͂̒ǉ�.
	CreateMoveVector();			// �ړ��x�N�g���̍쐬.
	Move();						// �ړ�.

	WindowCollision();			// ���E�B���h�E�Ƃ̓����蔻��.
	InWindowCollision();		// ���ɓ����Ă���E�B���h�E�Ƃ̓����蔻��.
	ScreenCollision();			// �X�N���[������o�Ȃ��悤�ɂ��铖���蔻��.

	TeleportTrashCanCheck();	// ���ݔ��Ɉړ��ł��邩�̊m�F.
}

//---------------------------.
// �E�B���h�E�I�u�W�F�N�g�̓����蔻���̍X�V.
//---------------------------.
void CWindowObject::WindowObjectLateUpdate()
{
	// �ړ��x�N�g���̏�����.
	InitMoveVector();
}

//---------------------------.
// �͂�.
//---------------------------.
void CWindowObject::Grab()
{
	if ( m_SpriteState.IsDisp == false	) return;
	if ( m_IsGrab						) return;
	if ( GrabCheck()		  == false	) return;

	// �ʒu�̎擾.
	D3DXPOSITION3*		 Pos		= &m_SpriteState.Transform.Position;
	const D3DXPOSITION2& MousePos	= Input::GetMousePosition();

	// �}�E�X���摜�̏�ō��N���b�N�������ꂽ�u�Ԃ��ǂ���.
	if ( GetIsMouseOver() && KeyInput::IsKeyDown( VK_LBUTTON ) )
	{
		// �͂�ł����Ԃɂ���.
		m_IsGrab			= true;
		m_IsFall			= false;

		// �摜�̒�������̒͂񂾏ꏊ�̕␳�l��ۑ�.
		m_GrabPosition.x	= MousePos.x - Pos->x;
		m_GrabPosition.y	= MousePos.y - Pos->y;

		// �͂񂾎��̏�����.
		GrabInit();

		// �E�B���h�E�̒�����o��.
		EscapeWindow();
	}
}

//---------------------------.
// �͂�ł��鎞�̍X�V.
//---------------------------.
void CWindowObject::GrabMoveUpdate()
{
	if ( m_SpriteState.IsDisp	== false ) return;
	if ( m_IsGrab				== false ) return;

	// �ʒu�̎擾.
	const D3DXPOSITION2& OldMousePos = Input::GetMouseOldPosition();
	const D3DXPOSITION2& MousePos	 = Input::GetMousePosition();

	// �ʒu���}�E�X�̏ꏊ�ɂ���.
	m_SpriteState.Transform.Position.x = MousePos.x - m_GrabPosition.x;
	m_SpriteState.Transform.Position.y = MousePos.y - m_GrabPosition.y;

	// �}�E�X�̈ړ��x�N�g���̍쐬.
	m_OldMoveVector.x = ( MousePos.x - OldMousePos.x ) / 2.0f;
	m_OldMoveVector.y = ( MousePos.y - OldMousePos.y ) / 2.0f;

	// �݂͂̍X�V.
	GrabUpdate();
}

//---------------------------.
// �͂�ł��镨�𗣂�.
//---------------------------.
void CWindowObject::Separate()
{
	if ( m_IsGrab							== false ) return;
	if ( KeyInput::IsKeyUp( VK_LBUTTON )	== false ) return;
	
	// �݂͂���������.
	m_IsGrab		= false;
	m_GrabPosition	= INIT_FLOAT2;
	InWindowCheck();

	// ���������̏�����.
	SeparateInit();
}

//---------------------------.
// �E�B���h�E�̒��ɓ���邩�̃`�F�b�N.
//---------------------------.
void CWindowObject::InWindowCheck()
{
	const HWND& hWnd = WindowManager::GetMouseOverTheWindow();

	if ( hWnd != NULL ){
		// �E�B���h�E�̏���ۑ�.
		m_InWndHandle = hWnd;
	}
}

//---------------------------.
// ���ݔ��Ɉړ��ł��邩�m�F.
//---------------------------.
void CWindowObject::TeleportTrashCanCheck()
{
	if ( m_IsTrashCan == false ) return;

	// ���ݔ��̃E�B���h�E�n���h�����擾.
	const HWND& hWnd = WindowManager::GetFindWindow( "���ݔ�" );

	// ���ݔ�������������.
	if ( hWnd == NULL ) return;

	// ���ݔ��̒��ɓ����.
	m_IsTrashCan  = false;
	m_InWndHandle = hWnd;
}

//---------------------------.
// �E�B���h�E����o��.
//---------------------------.
void CWindowObject::EscapeWindow()
{
	if ( m_InWndHandle == NULL ) return;

	// ���ɓ����Ă�E�B���h�E�����폜.
	m_InWndHandle = NULL;

	// �`�悷��͈͂��폜.
	m_SpriteState.RenderArea = { 0.0f, 0.0f, -1.0f, -1.0f };
}

//---------------------------.
// �d�͂̒ǉ�.
//---------------------------.
void CWindowObject::PushGravity()
{
	if ( m_SpriteState.IsDisp == false ) return;

	m_VectorList.emplace_back( GRAVITY_VECTOR * m_GravityPower );
}

//---------------------------.
// �ړ��x�N�g���̍쐬.
//---------------------------.
void CWindowObject::CreateMoveVector()
{
	if ( m_SpriteState.IsDisp == false ) return;

	m_MoveVector = m_OldMoveVector * m_SpeedRate;
	if (		m_MoveVector.x > 0.0f ) m_MoveVector.x *= m_DireSpeedRate.x;
	else if (	m_MoveVector.x < 0.0f ) m_MoveVector.x *= m_DireSpeedRate.z;
	if (		m_MoveVector.y > 0.0f ) m_MoveVector.y *= m_DireSpeedRate.y;
	else if (	m_MoveVector.y < 0.0f ) m_MoveVector.y *= m_DireSpeedRate.w;

	for ( auto& v : m_VectorList ) {
		m_MoveVector += v;
	}
}

//---------------------------.
// �ړ��x�N�g���̏�����.
//---------------------------.
void CWindowObject::InitMoveVector()
{
	if ( m_SpriteState.IsDisp == false ) return;

	// �x�N�g�����X�g�̏�����.
	m_VectorList.clear();

	// �ړ��x�N�g����ۑ�.
	m_OldMoveVector = m_MoveVector;

	// �x�N�g��������������ꍇ����������.
	const float Length = D3DXVec3Length( &m_OldMoveVector );
	if ( Length <= 0.1f ) m_OldMoveVector = INIT_FLOAT3;
}

//---------------------------.
// �ړ�.
//---------------------------.
void CWindowObject::Move()
{
	if ( m_SpriteState.IsDisp == false ) return;

	// �ړ�.
	m_SpriteState.Transform.Position += m_MoveVector;
	m_Transform = m_SpriteState.Transform;

	// �ړ����̍X�V.
	MoveUpdate();

	// �������Ă��邩.
	if ( m_IsLanding == false && m_IsGrab == false && m_IsFall == false && m_MoveVector.y > 0.0f ) {
		m_IsFall		= true;
		m_StartPosition = m_SpriteState.Transform.Position;
	}

	// ������.
	if ( m_IsLanding == false ) return;
	m_IsLanding = false;
}

//---------------------------.
// ���E�B���h�E�̓����蔻��.
//---------------------------.
void CWindowObject::WindowCollision()
{
	if ( m_SpriteState.IsDisp == false	) return;
	if ( m_IsGrab						) return;

	// �ʒu�̎擾.
	D3DXPOSITION3* pPos = &m_SpriteState.Transform.Position;

	// ���E�B���h�E�̏����擾.
	const WindowManager::WndList& WindList = WindowManager::GetWindowList();

	// �E�B���h�E�ɓ������Ă��邩���ׂ�.
	const int			 InWndZOrder = WindowManager::GetWindowZOrder( m_InWndHandle );
	const D3DXPOSITION3& CPos		 = *pPos + m_AddCenterPosition;
	for ( auto& [w, hWnd] : WindList ) {
		// �{�b�N�X�ʂ����������Ă��邩���ׂ�.
		if ( ( CPos.x - m_CollSize / 2.0f < w.right	 ) && ( CPos.x + m_CollSize / 2.0f	> w.left ) &&
			 ( CPos.y - m_CollSize / 2.0f < w.bottom ) && ( CPos.y + m_CollSize / 2.0f	> w.top  ) )
		{
			// ���ɓ����Ă���E�B���h�E�Ƃ̓����蔻��𖳂���.
			if ( m_InWndHandle == hWnd ) continue;

			// ���ɓ����Ă���E�B���h�E��艺�̃E�B���h�E��.
			if ( m_InWndHandle != NULL ) {
				if ( InWndZOrder < WindowManager::GetWindowZOrder( hWnd ) ) continue;
			}

			// ������Ē��ɂ߂荞�񂾂�.
			const D3DXPOSITION3& OldPos = m_SpriteState.Transform.OldPosition + m_AddCenterPosition;
			if ( ( OldPos.x < w.right  ) && ( OldPos.x > w.left ) &&
				 ( OldPos.y < w.bottom ) && ( OldPos.y > w.top ) )
			{
				// �߂荞�񂾃E�B���h�E�̒��ɓ����.
				m_InWndHandle = hWnd;
				return;
			}

			// �O��̍��W�ňʒu�֌W���擾.
			EDirection Dire	= EDire::None;
			if (		w.left	<= OldPos.x && OldPos.x <= w.right	&&	w.bottom	<= OldPos.y ) Dire = EDirection::Up;
			else if (	w.left	<= OldPos.x	&& OldPos.x <= w.right	&&	w.top		>= OldPos.y ) Dire = EDirection::Down;
			else if (	w.top	<= OldPos.y	&& OldPos.y <= w.bottom	&&	w.right		<= OldPos.x ) Dire = EDirection::Left;
			else if (	w.top	<= OldPos.y && OldPos.y <= w.bottom	&&	w.left		>= OldPos.x ) Dire = EDirection::Right;
			else if (	w.right	<= OldPos.x && w.bottom	<= OldPos.y ) {
				const float DistanceR = OldPos.x - w.right;
				const float DistanceD = OldPos.y - w.bottom;
				if ( DistanceR > DistanceD ) Dire = EDirection::Left;
				else						 Dire = EDirection::Up;
			}
			else if (	w.right	<= OldPos.x	&& w.top	>= OldPos.y ) {
				const float DistanceR = OldPos.x - w.right;
				const float DistanceU = w.top	 - OldPos.y;
				if ( DistanceR > DistanceU ) Dire = EDirection::Left;
				else						 Dire = EDirection::Down;
			}
			else if (	w.left	>= OldPos.x && w.bottom	<= OldPos.y ) {
				const float DistanceL = w.left	 - OldPos.x;
				const float DistanceD = OldPos.y - w.bottom;
				if ( DistanceL > DistanceD ) Dire = EDirection::Right;
				else						 Dire = EDirection::Up;
			}
			else if (	w.left	>= OldPos.x && w.top	>= OldPos.y ) {
				const float DistanceL = w.left	- OldPos.x;
				const float DistanceU = w.top	- OldPos.y;
				if ( DistanceL > DistanceU ) Dire = EDirection::Right;
				else						 Dire = EDirection::Down;
			}

			// �ʒu�֌W�����ē����蔻��̓����ύX.
			switch ( Dire ) {
			case EDirection::Up:	HitUp( w );		break;
			case EDirection::Down:	HitDown( w );	break;
			case EDirection::Left:	HitLeft( w );	break;
			case EDirection::Right:	HitRight( w );	break;
			default:								break;
			}
		}
	}
}

//---------------------------.
// ���ɓ����Ă���E�B���h�E�Ƃ̓����蔻��.
//---------------------------.
void CWindowObject::InWindowCollision()
{
	if ( m_InWndHandle == NULL ) return;

	// �E�B���h�E�̏����擾.
	RECT WndRect;
	const bool IsOldSmall	= m_IsInWndSmall;
	const bool IsWndOpen	= GetWindowRect( m_InWndHandle, &WndRect );
	DwmGetWindowAttribute( m_InWndHandle, DWMWA_EXTENDED_FRAME_BOUNDS, &WndRect, sizeof( RECT ) );
	m_IsInWndSmall = IsIconic( m_InWndHandle );

	// �E�B���h�E���Ȃ����ߔ�\���ɂ���.
	m_SpriteState.IsDisp = !m_IsInWndSmall;

	// �E�B���h�E���j�����ꂽ.
	if ( IsWndOpen == false && IsOldSmall ) {
		EscapeWindow();
		m_IsTrashCan			= true;
		m_SpriteState.IsDisp	= false;
		return;
	}
	if ( IsWndOpen == false && m_IsInWndSmall == false ) {
		EscapeWindow();
		return;
	}
	
	// �E�B���h�E�̏��𒲐�.
	if ( m_SpriteState.IsDisp == false ) return;
	WndRect.left	+= m_AddWndRect.left;
	WndRect.top		+= m_AddWndRect.top;
	WndRect.right	+= m_AddWndRect.right;
	WndRect.bottom	+= m_AddWndRect.bottom;

	// �`��G���A�̐ݒ�.
	const bool IsWndBig = IsZoomed( m_InWndHandle );
	if ( IsWndBig == false ) {
		m_SpriteState.RenderArea.x	= static_cast<float>( WndRect.left					);
		m_SpriteState.RenderArea.y	= static_cast<float>( WndRect.top					);
		m_SpriteState.RenderArea.z	= static_cast<float>( WndRect.right - WndRect.left	);
		m_SpriteState.RenderArea.w	= static_cast<float>( WndRect.bottom - WndRect.top	);
	}
	else m_SpriteState.RenderArea	= { 0.0f, 0.0f, -1.0f, -1.0f };

	// �E�B���h�E����o�Ă��Ȃ����m�F.
	const D3DXPOSITION3& CPos = m_SpriteState.Transform.Position + m_AddCenterPosition;
	if (		CPos.y - m_CollSize / 2.0f	< WndRect.top		) HitUp(	WndRect, false );
	else if (	CPos.y + m_CollSize / 2.0f	> WndRect.bottom	) HitDown(	WndRect, false );
	if (		CPos.x - m_CollSize / 2.0f	< WndRect.left		) HitLeft(	WndRect, false );
	else if (	CPos.x + m_CollSize / 2.0f	> WndRect.right		) HitRight( WndRect, false );
}

//---------------------------.
// �X�N���[���̓����蔻��.
//---------------------------.
void CWindowObject::ScreenCollision()
{
	if ( m_SpriteState.IsDisp	== false ) return;
	if ( m_InWndHandle			!= NULL  ) return;

	// �X�N���[������o�Ă��Ȃ����m�F.
	const D3DXPOSITION3& CPos = m_SpriteState.Transform.Position + m_AddCenterPosition;
	if (		CPos.y - m_CollSize / 2.0f	< m_WndRect.top		) HitUp(	m_WndRect, false );
	else if (	CPos.y + m_CollSize / 2.0f	> m_WndRect.bottom	) HitDown(	m_WndRect, false );
	if (		CPos.x - m_CollSize / 2.0f	< m_WndRect.left	) HitLeft(	m_WndRect, false );
	else if (	CPos.x + m_CollSize / 2.0f	> m_WndRect.right	) HitRight( m_WndRect, false );
}

//---------------------------.
// �㑤�ɓ�������.
//---------------------------.
void CWindowObject::HitUp( const RECT& HitWnd, const bool IsFromOutside )
{
	// ���������ꏊ�ɑ�����.
	const float HitPos = static_cast<float>( IsFromOutside ? HitWnd.bottom : HitWnd.top );
	m_SpriteState.Transform.Position.y = HitPos - m_AddCenterPosition.y + m_CollSize / 2.0f;

	// ��ɓ����������̍X�V.
	HitUpUpdate();
}

//---------------------------.
// �����ɓ�������.
//---------------------------.
void CWindowObject::HitDown( const RECT& HitWnd, const bool IsFromOutside )
{
	// ���������ꏊ�ɑ�����.
	const float HitPos = static_cast<float>( IsFromOutside ? HitWnd.top : HitWnd.bottom );
	m_SpriteState.Transform.Position.y = HitPos - m_AddCenterPosition.y - m_CollSize / 2.0f;;
	
	// ���ɓ����������̍X�V.
	HitDownUpdate();

	// ���n������.
	Landing( HitWnd );
}

//---------------------------.
// �����ɓ�������.
//---------------------------.
void CWindowObject::HitLeft( const RECT& HitWnd, const bool IsFromOutside )
{
	// ���������ꏊ�ɑ�����.
	const float HitPos = static_cast<float>( IsFromOutside ? HitWnd.right : HitWnd.left );
	m_SpriteState.Transform.Position.x = HitPos - m_AddCenterPosition.x + m_CollSize / 2.0f;

	// ���ɓ����������̍X�V.
	HitLeftUpdate();
}

//---------------------------.
// �E���ɓ�������.
//---------------------------.
void CWindowObject::HitRight( const RECT& HitWnd, const bool IsFromOutside )
{
	// ���������ꏊ�ɑ�����.
	const float HitPos = static_cast<float>( IsFromOutside ? HitWnd.left : HitWnd.right );
	m_SpriteState.Transform.Position.x = HitPos - m_AddCenterPosition.x - m_CollSize / 2.0f;

	// �E�ɓ����������̍X�V.
	HitRightUpdate();
}

//---------------------------.
// ���n.
//---------------------------.
void CWindowObject::Landing( const RECT& HitWnd )
{
	// ���n������.
	m_IsLanding	 = true;
	m_LandingWnd = HitWnd;

	// �������Ă���.
	if ( m_IsFall == false ) return;
	m_IsFall = false;

	// ���n���̏�����.
	LandingInit();
}
