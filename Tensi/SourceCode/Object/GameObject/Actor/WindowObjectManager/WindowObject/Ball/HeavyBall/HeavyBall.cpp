#include "HeavyBall.h"
#include "..\..\Player\Player.h"

namespace {
	const float BALL_SIZE = 64.0f;	// ボールのサイズ.
}

CHeavyBall::CHeavyBall()
{
}

CHeavyBall::~CHeavyBall()
{
}

//---------------------------.
// 初期化.
//---------------------------.
bool CHeavyBall::Init()
{
	// 画像の取得.
	m_pSprite		= SpriteResource::GetSprite( "Ball", &m_SpriteState );
	m_SpriteState.AnimState.IsSetAnimNumber = true;
	m_SpriteState.AnimState.AnimNumber		= true;

	// オブジェクトの初期化.
	m_CollSize			= BALL_SIZE;
	m_SpeedRate			= 0.996f;
	m_GravityPower		= 1.25f;
	m_DireSpeedRate.x	= 0.8f;
	m_DireSpeedRate.z	= 0.8f;
	m_DireSpeedRate.w	= 0.8f;
	m_IsDisp			= false;
	m_ObjectTag			= EObjectTag::HeavyBall;
	m_IsUseImpactSquash	= false;	// 壁への衝突で拡縮させない.
	m_Weight			= 10;		// 重さを設定する( 最も重い ).
	m_GrabDownSpeed		= 1.0f;		// 掴んでいる時に毎フレームマウスを下に下げる速度.
	
	WindowObjectInit();
	InitCollision();
	return true;
}

//---------------------------.
// 更新.
//---------------------------.
void CHeavyBall::Update( const float& DeltaTime )
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
void CHeavyBall::LateUpdate( const float& DeltaTime )
{
	WindowObjectLateUpdate();
}

//---------------------------.
// 描画.
//---------------------------.
void CHeavyBall::Render()
{
	if ( m_IsDisp == false ) return;
	m_pSprite->RenderUI( &m_SpriteState );
}

//---------------------------.
// 当たり判定関数.
//---------------------------.
void CHeavyBall::Collision( CActor* pActor )
{
	if ( pActor->GetObjectTag() != EObjectTag::Player ) return;

	// 当たっているか.
	if ( Coll2D::IsSphere2DToSphere2D(
		m_pCollisions->GetCollision<CSphere2D>(),
		pActor->GetCollision<CSphere2D>() ) == false ) return;

	// 速度の確認.
	if ( m_MoveVector.y > 25.0f ) {
		dynamic_cast<CPlayer*>( pActor )->Collapsed();
	}

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
void CHeavyBall::InitCollision()
{
	m_pCollisions = std::make_unique<CCollisions>();
	m_pCollisions->InitCollision( ECollType::Sphere2D );
	m_pCollisions->GetCollision<CSphere2D>()->SetSprite( m_pSprite, &m_SpriteState );
	m_pCollisions->GetCollision<CSphere2D>()->SetRadius( 32.0f );
}

//---------------------------.
// 当たり判定の更新.
//---------------------------.
void CHeavyBall::UpdateCollision()
{
}

//---------------------------.
// 上に当たった時の更新.
//---------------------------.
void CHeavyBall::HitUpUpdate()
{
	// 反射させる.
	m_MoveVector = Math::GetReflectVector( m_MoveVector, { 0.0f, 1.0f, 0.0f } );
}

//---------------------------.
// 下に当たった時の更新.
//---------------------------.
void CHeavyBall::HitDownUpdate()
{
	// 反射させる.
	m_MoveVector = Math::GetReflectVector( m_MoveVector, { 0.0f, -1.0f, 0.0f } );

	// 横方向の力を減らす.
	m_MoveVector.x *= 0.8f;
}

//---------------------------.
// 左に当たった時の更新.
//---------------------------.
void CHeavyBall::HitLeftUpdate()
{
	// 反射させる.
	m_MoveVector = Math::GetReflectVector( m_MoveVector, { -1.0f, 0.0f, 0.0f } );
}

//---------------------------.
// 右に当たった時の更新.
//---------------------------.
void CHeavyBall::HitRightUpdate()
{
	// 反射させる.
	m_MoveVector = Math::GetReflectVector( m_MoveVector, { 1.0f, 0.0f, 0.0f } );
}
