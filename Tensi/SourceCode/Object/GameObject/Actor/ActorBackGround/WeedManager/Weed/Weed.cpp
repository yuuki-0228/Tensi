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
// 初期化.
//---------------------------.
bool CWeed::Init()
{
	// 画像の取得.
	m_pWeed = SpriteResource::GetSprite( "Weed", &m_WeedState );
	m_AddCenterPosition = m_pWeed->GetSpriteState().AddCenterPos;
	m_WeedSize			= m_pWeed->GetSpriteState().Disp.w;

	// オブジェクトの初期化.
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
// 更新.
//---------------------------.
void CWeed::Update( const float& DeltaTime )
{
	m_DeltaTime = DeltaTime;
	if ( m_IsDisp == false ) return;

	MouseShakeUpdate();		// マウスによる雑草の揺れの更新.
	GrabUptate();			// 雑草の掴みの更新.
	GrabUpdate();			// 雑草を掴んでいるときの更新.
	ComeOutAnimUptate();	// 雑草が抜けるアニメーションの更新.

	TransformUpdate( m_WeedState.Transform );
	UpdateCollision();
	if ( m_WeedState.IsDisp == false ) return;
	ActorCollisionManager::PushObject( this, ECollObjType::BeAHit );
}

//---------------------------.
// 描画.
//---------------------------.
void CWeed::SubRender()
{
	if ( m_IsDisp == false ) return;

	m_pWeed->RenderUI( &m_WeedState );
}

//---------------------------.
// 当たり判定関数.
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
// 草のデータの取得.
//---------------------------.
SWeedData CWeed::GetWeedData()
{
	return SWeedData(
		m_WeedState.Transform,
		m_IsDisp
	);
}

//---------------------------.
// 草のデータの設定.
//---------------------------.
void CWeed::SetWeedData( const SWeedData& t )
{
	m_IsDisp				= t.IsDisp;
	m_Transform				= t.Transform;
	m_WeedState.Transform	= t.Transform;
}

//---------------------------.
// 苗木を埋める.
//---------------------------.
void CWeed::Fill( const D3DXPOSITION3& Pos )
{
	if ( m_IsDisp ) return;

	// タスクバーのサイズを取得.
	const RECT& Rect = WindowManager::GetTaskBarRect();

	// 雑草の位置の設定.
	m_IsDisp				= true;
	m_Transform.Position	= Pos;
	m_Transform.Position.y	= static_cast<float>( Rect.top ) + 10.0f;
	m_WeedState.Transform	= m_Transform;
}

//---------------------------.
// 当たり判定の初期化.
//---------------------------.
void CWeed::InitCollision()
{
	m_pCollisions = std::make_unique<CCollisions>();
	m_pCollisions->InitCollision( ECollType::Sphere2D );
	m_pCollisions->GetCollision<CSphere2D>()->SetSprite( m_pWeed, &m_WeedState );
	m_pCollisions->GetCollision<CSphere2D>()->SetRadius( 20.0f );
}

//---------------------------.
// 当たり判定の更新.
//---------------------------.
void CWeed::UpdateCollision()
{
}

//---------------------------.
// マウスに触れたか.
//---------------------------.
bool CWeed::GetIsTouchMouse()
{
	// 位置の取得.
	D3DXPOSITION3*		 Pos	  = &m_WeedState.Transform.Position;
	const D3DXPOSITION2& MousePos = Input::GetMousePosition();

	// マウスが画像の上にあるかどうか.
	const D3DXPOSITION3& CPos = *Pos + m_AddCenterPosition;
	if ( CPos.x - m_WeedSize / 2.0f <= MousePos.x && MousePos.x <= CPos.x + m_WeedSize / 2.0f &&
		 CPos.y - m_WeedSize / 2.0f <= MousePos.y && MousePos.y <= CPos.y + m_WeedSize / 2.0f )
	{
		return true;
	}
	return false;
}

//---------------------------.
// 雑草を揺らす.
//---------------------------.
void CWeed::Shake( const D3DXPOSITION3& Pos, const D3DXPOSITION3& OldPos )
{
	if ( m_IsComeOut						) return;
	if ( m_WeedState.AnimState.IsAnimationX ) return;

	// 揺れ判定を起こさないか.
	if ( std::abs( Pos.x - OldPos.x ) <= Const::Weed.SKIP_SHAKE_SPEED ) return;

	// 右から触れられた
	if ( Pos.x <= OldPos.x ) {
		m_WeedState.AnimState.IsAnimationX = true;
		m_WeedState.AnimState.PatternNo.y = 0;
	}
	// 左から触れられた
	else {
		m_WeedState.AnimState.IsAnimationX = true;
		m_WeedState.AnimState.PatternNo.y = 1;
	}
}

//---------------------------.
// 雑草を掴む.
//---------------------------.
void CWeed::GrabUptate()
{
	if ( m_IsComeOut ) return;

	// 掴む.
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
	// 離す
	else 
	{
		if ( Input::GetIsLeftClickUp() ) {
			m_IsGrab = false;
			Input::ResetMouseSpeed();
		}
	}
}

//---------------------------.
// マウスによる雑草の揺れの更新.
//---------------------------.
void CWeed::MouseShakeUpdate()
{
	// マウスが画像の上にあるかどうか.
	if ( GetIsTouchMouse() == false ) return;
	
	// 揺らす.
	const D3DXPOSITION3& MousePos	 = Input::GetMousePosition3();
	const D3DXPOSITION3& OldMousePos = Input::GetMouseOldPosition3();
	Shake( MousePos, OldMousePos );
}

//---------------------------.
// 雑草を掴んでいる時の更新.
//---------------------------.
void CWeed::GrabUpdate()
{
	if ( m_IsGrab == false ) return;

	// 位置の取得.
	const D3DXPOSITION2& MousePos	= Input::GetMousePosition();
	D3DXPOSITION3*		 Pos		= &m_WeedState.Transform.Position;
	const D3DXPOSITION3& CPos		= *Pos + m_AddCenterPosition;

	// 雑草が抜けるか
	if ( CPos.y - MousePos.y >= Const::Weed.COME_OUT_HEIGHT ) {
		m_IsComeOut = true;
		m_IsGrab	= false;
		Input::ResetMouseSpeed();
	}
}

//---------------------------.
// 雑草が抜けるアニメーションの更新.
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