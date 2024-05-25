#include "Water.h"

namespace {
	const float OBJ_SIZE = 4.0f;	// ���̃T�C�Y.
}

CWater::CWater()
{
}

CWater::~CWater()
{
}

//---------------------------.
// ������.
//---------------------------.
bool CWater::Init()
{
	// �摜�̎擾.
	m_pSprite = SpriteResource::GetSprite( "Water", &m_SpriteState );
	m_SpriteState.Color		= Color4::Cyan;
	m_SpriteState.Color.w	= Color::ALPHA_MID;
	
	// �I�u�W�F�N�g�̏�����.
	m_CollSize			= OBJ_SIZE;
	m_SpeedRate			= 0.9996f;
	m_GravityPower		= 0.3f;
	m_IsDisp			= false;
	m_ObjectTag			= EObjectTag::Water;

	WindowObjectInit();
	InitCollision();
	return true;
}

//---------------------------.
// �X�V.
//---------------------------.
void CWater::Update( const float& DeltaTime )
{
	m_DeltaTime = DeltaTime;
	if ( m_IsDisp == false ) return;

	// �E�B���h�E�I�u�W�F�N�g�̍X�V.
	WindowObjectUpdate();

	TransformUpdate( m_SpriteState.Transform );
	UpdateCollision();
	if ( m_SpriteState.IsDisp == false ) return;
	ActorCollisionManager::PushObject( this, ECollObjType::Hit );
}

//---------------------------.
// �����蔻��I����Ăяo�����X�V.
//---------------------------.
void CWater::LateUpdate( const float& DeltaTime )
{
	WindowObjectLateUpdate();
}

//---------------------------.
// �`��.
//---------------------------.
void CWater::Render()
{
	if ( m_IsDisp == false ) return;
	m_pSprite->RenderUI( &m_SpriteState );
}

//---------------------------.
// �����蔻��֐�.
//---------------------------.
void CWater::Collision( CActor* pActor )
{

}

//---------------------------.
// ���̐ݒ�.
//---------------------------.
void CWater::SetWater( const D3DXPOSITION3& Pos, const D3DXVECTOR3& Vec3, const HWND hWnd )
{
	if ( m_IsDisp ) return;

	m_IsDisp							= true;
	m_SpriteState.Transform.Position	= Pos;
	m_MoveVector						= Vec3;
	m_OldMoveVector						= Vec3;
	m_InWndHandle						= hWnd;
}

//---------------------------.
// �����蔻��̏�����.
//---------------------------.
void CWater::InitCollision()
{
	m_pCollisions = std::make_unique<CCollisions>();
	m_pCollisions->InitCollision( ECollType::Box2D );
	m_pCollisions->GetCollision<CBox2D>()->SetSprite( m_pSprite, &m_SpriteState );
}

//---------------------------.
// �����蔻��̍X�V.
//---------------------------.
void CWater::UpdateCollision()
{
}

//---------------------------.
// ��ɓ����������̍X�V.
//---------------------------.
void CWater::HitUpUpdate()
{
	m_MoveVector.y = 0.0f;
	m_OldMoveVector.y *= 0.5f;
}

//---------------------------.
// ���ɓ����������̍X�V.
//---------------------------.
void CWater::HitDownUpdate()
{
	m_IsDisp = false;

	// �������̗͂����炷.
	m_MoveVector.x *= 0.7f;
}

//---------------------------.
// ���ɓ����������̍X�V.
//---------------------------.
void CWater::HitLeftUpdate()
{
	m_MoveVector.x = 0.0f;
	m_OldMoveVector.x *= 0.5f;
}

//---------------------------.
// �E�ɓ����������̍X�V.
//---------------------------.
void CWater::HitRightUpdate()
{
	m_MoveVector.x = 0.0f;
	m_OldMoveVector.x *= 0.5f;
}