#include "SuperBall.h"

namespace {
	const float BALL_SIZE = 64.0f; // �{�[���̃T�C�Y.
}

CSuperBall::CSuperBall()
{
}

CSuperBall::~CSuperBall()
{
}

//---------------------------.
// ������.
//---------------------------.
bool CSuperBall::Init()
{
	// �摜�̎擾.
	m_pSprite = SpriteResource::GetSprite( "Ball", &m_SpriteState );
	m_SpriteState.AnimState.IsSetAnimNumber = true;
	m_SpriteState.AnimState.AnimNumber		= 2;

	// �I�u�W�F�N�g�̏�����.
	m_CollSize			= BALL_SIZE;
	m_SpeedRate			= 0.99999f;
	m_GravityPower		= 0.9f;
	m_IsDisp			= false;
	m_ObjectTag			= EObjectTag::SuperBall;

	WindowObjectInit();
	InitCollision();
	return true;
}

//---------------------------.
// �X�V.
//---------------------------.
void CSuperBall::Update( const float& DeltaTime )
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
void CSuperBall::LateUpdate( const float& DeltaTime )
{
	WindowObjectLateUpdate();
}

//---------------------------.
// �`��.
//---------------------------.
void CSuperBall::Render()
{
	if ( m_IsDisp == false ) return;
	m_pSprite->RenderUI( &m_SpriteState );
}

//---------------------------.
// �����蔻��֐�.
//---------------------------.
void CSuperBall::Collision( CActor* pActor )
{
	if ( pActor->GetObjectTag() != EObjectTag::Player ) return;

	// �������Ă��邩.
	if ( Coll2D::IsSphere2DToSphere2D(
		m_pCollisions->GetCollision<CSphere2D>(),
		pActor->GetCollision<CSphere2D>() ) == false ) return;

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
void CSuperBall::InitCollision()
{
	m_pCollisions = std::make_unique<CCollisions>();
	m_pCollisions->InitCollision( ECollType::Sphere2D );
	m_pCollisions->GetCollision<CSphere2D>()->SetSprite( m_pSprite, &m_SpriteState );
	m_pCollisions->GetCollision<CSphere2D>()->SetRadius( 32.0f );
}

//---------------------------.
// �����蔻��̍X�V.
//---------------------------.
void CSuperBall::UpdateCollision()
{
}

//---------------------------.
// ��ɓ����������̍X�V.
//---------------------------.
void CSuperBall::HitUpUpdate()
{
	// ���˂�����.
	m_MoveVector = Math::GetReflectVector( m_MoveVector, { 0.0f, 1.0f, 0.0f } );
}

//---------------------------.
// ���ɓ����������̍X�V.
//---------------------------.
void CSuperBall::HitDownUpdate()
{
	// ���˂�����.
	m_MoveVector = Math::GetReflectVector( m_MoveVector, { 0.0f, -1.0f, 0.0f } );

	// �������̗͂����炷.
	m_MoveVector.x *= 0.8f;
}

//---------------------------.
// ���ɓ����������̍X�V.
//---------------------------.
void CSuperBall::HitLeftUpdate()
{
	// ���˂�����.
	m_MoveVector = Math::GetReflectVector( m_MoveVector, { -1.0f, 0.0f, 0.0f } );
}

//---------------------------.
// �E�ɓ����������̍X�V.
//---------------------------.
void CSuperBall::HitRightUpdate()
{
	// ���˂�����.
	m_MoveVector = Math::GetReflectVector( m_MoveVector, { 1.0f, 0.0f, 0.0f } );
}
