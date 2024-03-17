#include "HeavyBall.h"
#include "..\Player\Player.h"
#include "..\..\..\..\..\Utility\Input\Input.h"

namespace {
	const float BALL_SIZE = 64.0f;	// �{�[���̃T�C�Y.
}

CHeavyBall::CHeavyBall()
{
}

CHeavyBall::~CHeavyBall()
{
}

//---------------------------.
// ������.
//---------------------------.
bool CHeavyBall::Init()
{
	// �摜�̎擾.
	m_pSprite		= SpriteResource::GetSprite( "Ball", &m_SpriteState );
	m_SpriteState.AnimState.IsSetAnimNumber = true;
	m_SpriteState.AnimState.AnimNumber		= true;

	// �I�u�W�F�N�g�̏�����.
	m_CollSize			= BALL_SIZE;
	m_SpeedRate			= 0.996f;
	m_GravityPower		= 1.25f;
	m_DireSpeedRate.x	= 0.8f;
	m_DireSpeedRate.z	= 0.8f;
	m_DireSpeedRate.w	= 0.8f;
	m_IsDisp			= false;
	m_ObjectTag			= EObjectTag::HeavyBall;
	
	WindowObjectInit();
	InitCollision();
	return true;
}

//---------------------------.
// �X�V.
//---------------------------.
void CHeavyBall::Update( const float& DeltaTime )
{
	m_DeltaTime = DeltaTime;
	if ( m_IsDisp == false ) return;

	// �E�B���h�E�I�u�W�F�N�g�̍X�V.
	WindowObjectUpdate();

	UpdateCollision();
	if ( m_SpriteState.IsDisp == false ) return;
	ActorCollisionManager::PushObject( this, ECollObjType::BeAHit );
}

//---------------------------.
// �����蔻��I����Ăяo�����X�V.
//---------------------------.
void CHeavyBall::LateUpdate( const float& DeltaTime )
{
	WindowObjectLateUpdate();
}

//---------------------------.
// �`��.
//---------------------------.
void CHeavyBall::Render()
{
	if ( m_IsDisp == false ) return;
	m_pSprite->RenderUI( &m_SpriteState );
}

//---------------------------.
// �����蔻��֐�.
//---------------------------.
void CHeavyBall::Collision( CActor* pActor )
{
	if ( pActor->GetObjectTag() != EObjectTag::Player ) return;

	// �������Ă��邩.
	if ( Coll2D::IsSphere2DToSphere2D(
		m_pCollisions->GetCollision<CSphere2D>(),
		pActor->GetCollision<CSphere2D>() ) == false ) return;

	// ���x�̊m�F.
	if ( m_MoveVector.y > 25.0f ) {
		dynamic_cast<CPlayer*>( pActor )->Collapsed();
	}

	// �x�N�g���̍쐬.
	D3DXVECTOR3 Vector = m_SpriteState.Transform.Position - pActor->GetPosition();
	D3DXVec3Normalize( &Vector, &Vector );
	Vector.y -= 0.5f;

	// �x�N�g���̒ǉ�.
	m_MoveVector += Vector * 3.0f;
}

//---------------------------.
// �����蔻��̏�����.
//---------------------------.
void CHeavyBall::InitCollision()
{
	m_pCollisions = std::make_unique<CCollisions>();
	m_pCollisions->InitCollision( ECollType::Sphere2D );
	m_pCollisions->GetCollision<CSphere2D>()->SetSprite( m_pSprite, &m_SpriteState );
	m_pCollisions->GetCollision<CSphere2D>()->SetRadius( 32.0f );
}

//---------------------------.
// �����蔻��̍X�V.
//---------------------------.
void CHeavyBall::UpdateCollision()
{
}

//---------------------------.
// �݂͂̍X�V.
//---------------------------.
void CHeavyBall::GrabUpdate()
{
	// �}�E�X�����������ɉ�����.
	const D3DXVECTOR2& Pos = Input::GetMousePosition();
	SetCursorPos( static_cast<int>( Pos.x ), static_cast<int>( Pos.y + 1.0f ) );

	// �}�E�X�̈ړ����x��������.
	Input::SetMouseSpeed( 2 );
}

//---------------------------.
// ���������̏�����.
//---------------------------.
void CHeavyBall::SeparateInit()
{
	Input::ResetMouseSpeed();
}

//---------------------------.
// ��ɓ����������̍X�V.
//---------------------------.
void CHeavyBall::HitUpUpdate()
{
	// ���˂�����.
	m_MoveVector = Math::GetReflectVector( m_MoveVector, { 0.0f, 1.0f, 0.0f } );
}

//---------------------------.
// ���ɓ����������̍X�V.
//---------------------------.
void CHeavyBall::HitDownUpdate()
{
	// ���˂�����.
	m_MoveVector = Math::GetReflectVector( m_MoveVector, { 0.0f, -1.0f, 0.0f } );

	// �������̗͂����炷.
	m_MoveVector.x *= 0.8f;
}

//---------------------------.
// ���ɓ����������̍X�V.
//---------------------------.
void CHeavyBall::HitLeftUpdate()
{
	// ���˂�����.
	m_MoveVector = Math::GetReflectVector( m_MoveVector, { -1.0f, 0.0f, 0.0f } );
}

//---------------------------.
// �E�ɓ����������̍X�V.
//---------------------------.
void CHeavyBall::HitRightUpdate()
{
	// ���˂�����.
	m_MoveVector = Math::GetReflectVector( m_MoveVector, { 1.0f, 0.0f, 0.0f } );
}
