#include "SuperBall.h"

namespace {
	const float BALL_SIZE = 64.0f; // ボールのサイズ.
}

CSuperBall::CSuperBall()
{
}

CSuperBall::~CSuperBall()
{
}

//---------------------------.
// 初期化.
//---------------------------.
bool CSuperBall::Init()
{
	// 画像の取得.
	m_pSprite = SpriteResource::GetSprite( "Ball", &m_SpriteState );
	m_SpriteState.AnimState.IsSetAnimNumber = true;
	m_SpriteState.AnimState.AnimNumber		= 2;

	// オブジェクトの初期化.
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
// 更新.
//---------------------------.
void CSuperBall::Update( const float& DeltaTime )
{
	m_DeltaTime = DeltaTime;
	if ( m_IsDisp == false ) return;

	// ウィンドウオブジェクトの更新.
	WindowObjectUpdate();

	UpdateCollision();
	if ( m_SpriteState.IsDisp == false ) return;
	ActorCollisionManager::PushObject( this, ECollObjType::BeAHit );
}

//---------------------------.
// 当たり判定終了後呼び出される更新.
//---------------------------.
void CSuperBall::LateUpdate( const float& DeltaTime )
{
	WindowObjectLateUpdate();
}

//---------------------------.
// 描画.
//---------------------------.
void CSuperBall::Render()
{
	if ( m_IsDisp == false ) return;
	m_pSprite->RenderUI( &m_SpriteState );
}

//---------------------------.
// 当たり判定関数.
//---------------------------.
void CSuperBall::Collision( CActor* pActor )
{
	if ( pActor->GetObjectTag() != EObjectTag::Player ) return;

	// 当たっているか.
	if ( Coll2D::IsSphere2DToSphere2D(
		m_pCollisions->GetCollision<CSphere2D>(),
		pActor->GetCollision<CSphere2D>() ) == false ) return;

	// ベクトルの作成.
	D3DXVECTOR3 Vector = m_SpriteState.Transform.Position - pActor->GetPosition();
	D3DXVec3Normalize( &Vector, &Vector );
	Vector.y -= 0.5f;

	// ベクトルの追加.
	m_MoveVector += Vector * 3.0f;
}

//---------------------------.
// 当たり判定の初期化.
//---------------------------.
void CSuperBall::InitCollision()
{
	m_pCollisions = std::make_unique<CCollisions>();
	m_pCollisions->InitCollision( ECollType::Sphere2D );
	m_pCollisions->GetCollision<CSphere2D>()->SetSprite( m_pSprite, &m_SpriteState );
	m_pCollisions->GetCollision<CSphere2D>()->SetRadius( 32.0f );
}

//---------------------------.
// 当たり判定の更新.
//---------------------------.
void CSuperBall::UpdateCollision()
{
}

//---------------------------.
// 上に当たった時の更新.
//---------------------------.
void CSuperBall::HitUpUpdate()
{
	// 反射させる.
	m_MoveVector = Math::GetReflectVector( m_MoveVector, { 0.0f, 1.0f, 0.0f } );
}

//---------------------------.
// 下に当たった時の更新.
//---------------------------.
void CSuperBall::HitDownUpdate()
{
	// 反射させる.
	m_MoveVector = Math::GetReflectVector( m_MoveVector, { 0.0f, -1.0f, 0.0f } );

	// 横方向の力を減らす.
	m_MoveVector.x *= 0.8f;
}

//---------------------------.
// 左に当たった時の更新.
//---------------------------.
void CSuperBall::HitLeftUpdate()
{
	// 反射させる.
	m_MoveVector = Math::GetReflectVector( m_MoveVector, { -1.0f, 0.0f, 0.0f } );
}

//---------------------------.
// 右に当たった時の更新.
//---------------------------.
void CSuperBall::HitRightUpdate()
{
	// 反射させる.
	m_MoveVector = Math::GetReflectVector( m_MoveVector, { 1.0f, 0.0f, 0.0f } );
}
