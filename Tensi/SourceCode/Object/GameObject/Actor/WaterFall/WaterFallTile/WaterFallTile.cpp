#include "WaterFallTile.h"
#include "..\..\..\..\..\Resource\SpriteResource\SpriteResource.h"
#include "..\..\..\..\..\Utility\WindowManager\WindowManager.h"

namespace {
	constexpr int WATERFALL_PATTERN_NO	= 0;	// 滝のパターンNo
	constexpr int SPLASH_PATTERN_NO		= 1;	// 水しぶきのパターンNo
}

CWaterFallTile::CWaterFallTile()
	: m_pWaterFall		( nullptr )
	, m_WaterFallState	()
	, m_SplashState		()
	, m_ImageSize		( INIT_FLOAT )
{
}

CWaterFallTile::~CWaterFallTile()
{
}

//---------------------------.
// 初期化.
//---------------------------.
bool CWaterFallTile::Init()
{
	// 画像の取得.
	m_pWaterFall	= SpriteResource::GetSprite( "WaterFall", &m_WaterFallState );
	m_SplashState	= m_WaterFallState;	
	m_ImageSize		= m_pWaterFall->GetSpriteState().Disp.w;

	// オブジェクトの初期化.
	m_IsDisp								= false;
	m_SplashState.IsDisp					= false;
	m_WaterFallState.AnimState.IsAnimationX	= true;
	m_SplashState.AnimState.IsAnimationX	= true;
	m_ObjectTag								= EObjectTag::WaterFall;
	m_WaterFallState.AnimState.PatternNo.y	= WATERFALL_PATTERN_NO;
	m_SplashState.AnimState.PatternNo.y		= SPLASH_PATTERN_NO;

	const RECT& Rect = WindowManager::GetTaskBarRect();
	m_WaterFallState.RenderArea.w	= static_cast<float>( Rect.top );
	m_SplashState.RenderArea.w		= m_WaterFallState.RenderArea.w;

	InitCollision();
	return true;
}

//---------------------------.
// 更新.
//---------------------------.
void CWaterFallTile::Update( const float& DeltaTime )
{
	m_DeltaTime = DeltaTime;
	if ( m_IsDisp == false ) return;

	UpdateCollision();
	if ( m_WaterFallState.IsDisp == false ) return;
	ActorCollisionManager::PushObject( this, ECollObjType::Hit );
}

//---------------------------.
// 描画.
//---------------------------.
void CWaterFallTile::SubRender()
{
	if ( m_IsDisp == false ) return;
	m_pWaterFall->RenderUI( &m_SplashState		);
	m_pWaterFall->RenderUI( &m_WaterFallState	);
}

//---------------------------.
// 当たり判定関数.
//---------------------------.
void CWaterFallTile::Collision( CActor* pActor )
{
}

//---------------------------.
// タイルの設定
//---------------------------.
void CWaterFallTile::Setting( const D3DXPOSITION3& Pos, const bool IsWater )
{
	m_IsDisp							= true;
	m_WaterFallState.Transform.Position = Pos;
	m_SplashState.Transform.Position	= Pos;
	m_SplashState.IsDisp				= IsWater;
}

//---------------------------.
// 当たり判定の初期化.
//---------------------------.
void CWaterFallTile::InitCollision()
{
	m_pCollisions = std::make_unique<CCollisions>();
	m_pCollisions->InitCollision( ECollType::Box2D );
	m_pCollisions->GetCollision<CBox2D>()->SetSprite( m_pWaterFall, &m_WaterFallState );
}

//---------------------------.
// 当たり判定の更新.
//---------------------------.
void CWaterFallTile::UpdateCollision()
{
}
