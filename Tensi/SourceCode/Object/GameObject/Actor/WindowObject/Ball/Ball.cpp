#include "Ball.h"
#include "..\..\..\..\..\Utility\SaveDataManager\SaveDataManager.h"

namespace {
	const float BALL_SIZE = 64.0f; // �{�[���̃T�C�Y.
}

CBall::CBall()
{
}

CBall::~CBall()
{
}

//---------------------------.
// ������.
//---------------------------.
bool CBall::Init()
{
	// �摜�̎擾.
	m_pSprite = SpriteResource::GetSprite( "Ball", &m_SpriteState );
	m_SpriteState.AnimState.IsSetAnimNumber = true;
	m_SpriteState.AnimState.AnimNumber		= 0;

	// �I�u�W�F�N�g�̏�����.
	m_CollSize			= BALL_SIZE;
	m_SpeedRate			= 0.996f;
	m_IsDisp			= false;
	m_ObjectTag			= EObjectTag::Ball;

	SaveDataManager::SetSaveData()->BallTransform = &m_SpriteState.Transform;

	WindowObjectInit();
	InitCollision();
	return true;
}

//---------------------------.
// �X�V.
//---------------------------.
void CBall::Update( const float& DeltaTime )
{
	m_DeltaTime = DeltaTime;
	if ( m_IsDisp == false ) return;

	// �E�B���h�E�I�u�W�F�N�g�̍X�V.
	WindowObjectUpdate();

	TransformUpdate( m_SpriteState.Transform );
	UpdateCollision();
	if ( m_SpriteState.IsDisp == false ) return;
	ActorCollisionManager::PushObject( this, ECollObjType::Both );
}

//---------------------------.
// �����蔻��I����Ăяo�����X�V.
//---------------------------.
void CBall::LateUpdate( const float& DeltaTime )
{
	WindowObjectLateUpdate();
}

//---------------------------.
// �`��.
//---------------------------.
void CBall::Render()
{
	if ( m_IsDisp == false ) return;
	m_pSprite->RenderUI( &m_SpriteState );
}

//---------------------------.
// �����蔻��֐�.
//---------------------------.
void CBall::Collision( CActor* pActor )
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
void CBall::InitCollision()
{
	m_pCollisions = std::make_unique<CCollisions>();
	m_pCollisions->InitCollision( ECollType::Sphere2D );
	m_pCollisions->GetCollision<CSphere2D>()->SetSprite( m_pSprite, &m_SpriteState );
	m_pCollisions->GetCollision<CSphere2D>()->SetRadius( 32.0f );
}

//---------------------------.
// �����蔻��̍X�V.
//---------------------------.
void CBall::UpdateCollision()
{
}

//---------------------------.
// ��ɓ����������̍X�V.
//---------------------------.
void CBall::HitUpUpdate()
{
	// ���˂�����.
	m_MoveVector = Math::GetReflectVector( m_MoveVector, { 0.0f, 1.0f, 0.0f } );
}

//---------------------------.
// ���ɓ����������̍X�V.
//---------------------------.
void CBall::HitDownUpdate()
{
	// ���˂�����.
	m_MoveVector = Math::GetReflectVector( m_MoveVector, { 0.0f, -1.0f, 0.0f } );

	// �������̗͂����炷.
	m_MoveVector.x *= 0.8f;
}

//---------------------------.
// ���ɓ����������̍X�V.
//---------------------------.
void CBall::HitLeftUpdate()
{
	// ���˂�����.
	m_MoveVector = Math::GetReflectVector( m_MoveVector, { -1.0f, 0.0f, 0.0f } );
}

//---------------------------.
// �E�ɓ����������̍X�V.
//---------------------------.
void CBall::HitRightUpdate()
{
	// ���˂�����.
	m_MoveVector = Math::GetReflectVector( m_MoveVector, { 1.0f, 0.0f, 0.0f } );
}
