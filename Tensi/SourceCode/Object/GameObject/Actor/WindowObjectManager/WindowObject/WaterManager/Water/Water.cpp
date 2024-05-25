#include "Water.h"

namespace {
	const float OBJ_SIZE = 4.0f;	// 水のサイズ.
}

CWater::CWater()
{
}

CWater::~CWater()
{
}

//---------------------------.
// 初期化.
//---------------------------.
bool CWater::Init()
{
	// 画像の取得.
	m_pSprite = SpriteResource::GetSprite( "Water", &m_SpriteState );
	m_SpriteState.Color		= Color4::Cyan;
	m_SpriteState.Color.w	= Color::ALPHA_MID;
	
	// オブジェクトの初期化.
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
// 更新.
//---------------------------.
void CWater::Update( const float& DeltaTime )
{
	m_DeltaTime = DeltaTime;
	if ( m_IsDisp == false ) return;

	// ウィンドウオブジェクトの更新.
	WindowObjectUpdate();

	TransformUpdate( m_SpriteState.Transform );
	UpdateCollision();
	if ( m_SpriteState.IsDisp == false ) return;
	ActorCollisionManager::PushObject( this, ECollObjType::Hit );
}

//---------------------------.
// 当たり判定終了後呼び出される更新.
//---------------------------.
void CWater::LateUpdate( const float& DeltaTime )
{
	WindowObjectLateUpdate();
}

//---------------------------.
// 描画.
//---------------------------.
void CWater::Render()
{
	if ( m_IsDisp == false ) return;
	m_pSprite->RenderUI( &m_SpriteState );
}

//---------------------------.
// 当たり判定関数.
//---------------------------.
void CWater::Collision( CActor* pActor )
{

}

//---------------------------.
// 水の設定.
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
// 当たり判定の初期化.
//---------------------------.
void CWater::InitCollision()
{
	m_pCollisions = std::make_unique<CCollisions>();
	m_pCollisions->InitCollision( ECollType::Box2D );
	m_pCollisions->GetCollision<CBox2D>()->SetSprite( m_pSprite, &m_SpriteState );
}

//---------------------------.
// 当たり判定の更新.
//---------------------------.
void CWater::UpdateCollision()
{
}

//---------------------------.
// 上に当たった時の更新.
//---------------------------.
void CWater::HitUpUpdate()
{
	m_MoveVector.y = 0.0f;
	m_OldMoveVector.y *= 0.5f;
}

//---------------------------.
// 下に当たった時の更新.
//---------------------------.
void CWater::HitDownUpdate()
{
	m_IsDisp = false;

	// 横方向の力を減らす.
	m_MoveVector.x *= 0.7f;
}

//---------------------------.
// 左に当たった時の更新.
//---------------------------.
void CWater::HitLeftUpdate()
{
	m_MoveVector.x = 0.0f;
	m_OldMoveVector.x *= 0.5f;
}

//---------------------------.
// 右に当たった時の更新.
//---------------------------.
void CWater::HitRightUpdate()
{
	m_MoveVector.x = 0.0f;
	m_OldMoveVector.x *= 0.5f;
}