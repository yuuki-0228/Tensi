#include "Weed.h"
#include "..\..\..\WindowObjectManager\WindowObjectManager.h"
#include "..\..\..\..\..\..\Resource\SpriteResource\SpriteResource.h"
#include "..\..\..\..\..\..\Utility\Input\Input.h"
#include "..\..\..\..\..\..\Utility\WindowManager\WindowManager.h"
#include "..\..\..\..\..\..\Utility\Const\Const.h"

CWeed::CWeed()
	: m_pWeed				( nullptr )
	, m_WeedState			()
	, m_AddCenterPosition	( INIT_FLOAT3 )
	, m_WeedSize			( INIT_FLOAT )
	, m_IsGrab				( false )
	, m_IsComeOut			( false )
{
}

CWeed::~CWeed()
{
}

//---------------------------.
// ������.
//---------------------------.
bool CWeed::Init()
{
	// �摜�̎擾.
	m_pWeed = SpriteResource::GetSprite( "Weed", &m_WeedState );
	m_AddCenterPosition = m_pWeed->GetSpriteState().AddCenterPos;
	m_WeedSize			= m_pWeed->GetSpriteState().Disp.w;

	// �I�u�W�F�N�g�̏�����.
	m_IsDisp							= false;
	m_IsGrab							= false;
	m_IsComeOut							= false;
	m_WeedState.AnimState.AnimPlayMax	= 1;
	m_ObjectTag							= EObjectTag::Weed;

	const RECT& Rect = WindowManager::GetTaskBarRect();
	m_WeedState.RenderArea.w = static_cast<float>( Rect.top );

	InitCollision();
	return true;
}

//---------------------------.
// �X�V.
//---------------------------.
void CWeed::Update( const float& DeltaTime )
{
	m_DeltaTime = DeltaTime;
	if ( m_IsDisp == false ) return;

	MouseShakeUpdate();		// �}�E�X�ɂ��G���̗h��̍X�V.
	GrabUptate();			// �G���݂̒͂̍X�V.
	GrabUpdate();			// �G����͂�ł���Ƃ��̍X�V.
	ComeOutAnimUptate();	// �G����������A�j���[�V�����̍X�V.

	TransformUpdate( m_WeedState.Transform );
	UpdateCollision();
	if ( m_WeedState.IsDisp == false ) return;
	ActorCollisionManager::PushObject( this, ECollObjType::BeAHit );
}

//---------------------------.
// �`��.
//---------------------------.
void CWeed::SubRender()
{
	if ( m_IsDisp == false ) return;

	m_pWeed->RenderUI( &m_WeedState );
}

//---------------------------.
// �����蔻��֐�.
//---------------------------.
void CWeed::Collision( CActor* pActor )
{
	const EObjectTag Tag = pActor->GetObjectTag();
	if ( Tag != EObjectTag::Player && Tag != EObjectTag::NormalBall ) return;

	if ( Coll2D::IsSphere2DToSphere2D(
		m_pCollisions->GetCollision<CSphere2D>(),
		pActor->GetCollision<CSphere2D>() ) == false ) return;

	const D3DXPOSITION3& Pos	= pActor->GetPosition();
	const D3DXPOSITION3& OldPos = pActor->GetTransform().OldPosition;

	Shake( Pos, OldPos );
}

//---------------------------.
// ���̃f�[�^�̎擾.
//---------------------------.
SWeedData CWeed::GetWeedData()
{
	return SWeedData(
		m_WeedState.Transform,
		m_IsDisp
	);
}

//---------------------------.
// ���̃f�[�^�̐ݒ�.
//---------------------------.
void CWeed::SetWeedData( const SWeedData& t )
{
	m_IsDisp				= t.IsDisp;
	m_Transform				= t.Transform;
	m_WeedState.Transform	= t.Transform;
}

//---------------------------.
// �c�؂𖄂߂�.
//---------------------------.
void CWeed::Fill( const D3DXPOSITION3& Pos )
{
	if ( m_IsDisp ) return;

	// �^�X�N�o�[�̃T�C�Y���擾.
	const RECT& Rect = WindowManager::GetTaskBarRect();

	// �G���̈ʒu�̐ݒ�.
	m_IsDisp				= true;
	m_Transform.Position	= Pos;
	m_Transform.Position.y	= static_cast<float>( Rect.top ) + 10.0f;
	m_WeedState.Transform	= m_Transform;
}

//---------------------------.
// �����蔻��̏�����.
//---------------------------.
void CWeed::InitCollision()
{
	m_pCollisions = std::make_unique<CCollisions>();
	m_pCollisions->InitCollision( ECollType::Sphere2D );
	m_pCollisions->GetCollision<CSphere2D>()->SetSprite( m_pWeed, &m_WeedState );
	m_pCollisions->GetCollision<CSphere2D>()->SetRadius( 20.0f );
}

//---------------------------.
// �����蔻��̍X�V.
//---------------------------.
void CWeed::UpdateCollision()
{
}

//---------------------------.
// �}�E�X�ɐG�ꂽ��.
//---------------------------.
bool CWeed::GetIsTouchMouse()
{
	// �ʒu�̎擾.
	D3DXPOSITION3*		 Pos	  = &m_WeedState.Transform.Position;
	const D3DXPOSITION2& MousePos = Input::GetMousePosition();

	// �}�E�X���摜�̏�ɂ��邩�ǂ���.
	const D3DXPOSITION3& CPos = *Pos + m_AddCenterPosition;
	if ( CPos.x - m_WeedSize / 2.0f <= MousePos.x && MousePos.x <= CPos.x + m_WeedSize / 2.0f &&
		 CPos.y - m_WeedSize / 2.0f <= MousePos.y && MousePos.y <= CPos.y + m_WeedSize / 2.0f )
	{
		return true;
	}
	return false;
}

//---------------------------.
// �G����h�炷.
//---------------------------.
void CWeed::Shake( const D3DXPOSITION3& Pos, const D3DXPOSITION3& OldPos )
{
	if ( m_IsComeOut						) return;
	if ( m_WeedState.AnimState.IsAnimationX ) return;

	// �h�ꔻ����N�����Ȃ���.
	if ( std::abs( Pos.x - OldPos.x ) <= Const::Weed.SKIP_SHAKE_SPEED ) return;

	// �E����G���ꂽ
	if ( Pos.x <= OldPos.x ) {
		m_WeedState.AnimState.IsAnimationX = true;
		m_WeedState.AnimState.PatternNo.y = 0;
	}
	// ������G���ꂽ
	else {
		m_WeedState.AnimState.IsAnimationX = true;
		m_WeedState.AnimState.PatternNo.y = 1;
	}
}

//---------------------------.
// �G����͂�.
//---------------------------.
void CWeed::GrabUptate()
{
	if ( m_IsComeOut ) return;

	// �͂�.
	if ( m_IsGrab == false )
	{
		if ( GetIsTouchMouse() &&
			 Input::GetIsLeftClickDown() &&
			 WindowManager::GetIsMouseOverTheWindow() == false &&
			 WindowObjectManager::GetIsMouseOverTheObject () == false )
		{
			m_IsGrab = true;
			Input::SetMouseSpeed( 1 );
		}
	}
	// ����
	else 
	{
		if ( Input::GetIsLeftClickUp() ) {
			m_IsGrab = false;
			Input::ResetMouseSpeed();
		}
	}
}

//---------------------------.
// �}�E�X�ɂ��G���̗h��̍X�V.
//---------------------------.
void CWeed::MouseShakeUpdate()
{
	// �}�E�X���摜�̏�ɂ��邩�ǂ���.
	if ( GetIsTouchMouse() == false ) return;
	
	// �h�炷.
	const D3DXPOSITION3& MousePos	 = Input::GetMousePosition3();
	const D3DXPOSITION3& OldMousePos = Input::GetMouseOldPosition3();
	Shake( MousePos, OldMousePos );
}

//---------------------------.
// �G����͂�ł��鎞�̍X�V.
//---------------------------.
void CWeed::GrabUpdate()
{
	if ( m_IsGrab == false ) return;

	// �ʒu�̎擾.
	const D3DXPOSITION2& MousePos	= Input::GetMousePosition();
	D3DXPOSITION3*		 Pos		= &m_WeedState.Transform.Position;
	const D3DXPOSITION3& CPos		= *Pos + m_AddCenterPosition;

	// �G���������邩
	if ( CPos.y - MousePos.y >= Const::Weed.COME_OUT_HEIGHT ) {
		m_IsComeOut = true;
		m_IsGrab	= false;
		Input::ResetMouseSpeed();
	}
}

//---------------------------.
// �G����������A�j���[�V�����̍X�V.
//---------------------------.
void CWeed::ComeOutAnimUptate()
{
	if ( m_IsComeOut == false ) return;

	m_WeedState.Transform.Position.y -= Const::Weed.COME_OUT_ANIM_MOVE_SPEED * m_DeltaTime;
	m_WeedState.Color.w -= Const::Weed.COME_OUT_ANIM_ALPHA_SPEED * m_DeltaTime;

	if ( m_WeedState.Color.w < Color::ALPHA_MIN )
	{
		m_WeedState.Color.w = Color::ALPHA_MAX;
		m_IsDisp = false;
	}
}