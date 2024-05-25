#include "WindowObject.h"
#include "..\..\..\..\..\Utility\Input\Input.h"
#include <dwmapi.h>

namespace {
	// 重力ベクトル.
	const D3DXVECTOR3 GRAVITY_VECTOR = { 0.0f, 1.0f, 0.0f };
}

CWindowObject::CWindowObject()
	: m_pSprite				( nullptr )
	, m_SpriteState			()
	, m_VectorList			()
	, m_OldMoveVector		( INIT_FLOAT3 )
	, m_MoveVector			( INIT_FLOAT3 )
	, m_GrabPosition		( INIT_FLOAT2 )
	, m_StartPosition		( INIT_FLOAT3 )
	, m_EndPosition			( INIT_FLOAT3 )
	, m_DireSpeedRate		( 1.0f, 1.0f, 1.0f, 1.0f )
	, m_InWndHandle			( NULL )
	, m_LandingWnd			()
	, m_AddWndRect			()
	, m_CollSize			( INIT_FLOAT )
	, m_GravityPower		( 1.0f )
	, m_SpeedRate			( INIT_FLOAT )
	, m_CeilingPosY			( INIT_FLOAT )
	, m_IsLanding			( false )
	, m_IsFall				( false )
	, m_IsGrab				( false )
	, m_IsInWndSmall		( false )
	, m_IsTrashCan			( false )
	, m_IsDisp				( true )
	, m_AddCenterPosition	( INIT_FLOAT3 )
{
}

CWindowObject::~CWindowObject()
{
}

//---------------------------.
// マウスに触れているか
//---------------------------.
bool CWindowObject::GetIsMouseOver()
{
	// 位置の取得.
	D3DXPOSITION3*		 Pos		= &m_SpriteState.Transform.Position;
	const D3DXPOSITION2& MousePos	= Input::GetMousePosition();

	// マウスが画像の上にあるかどうか.
	const D3DXPOSITION3& CPos		 = *Pos + m_AddCenterPosition;
	if ( CPos.x - m_CollSize / 2.0f <= MousePos.x && MousePos.x <= CPos.x + m_CollSize / 2.0f &&
		 CPos.y - m_CollSize / 2.0f <= MousePos.y && MousePos.y <= CPos.y + m_CollSize / 2.0f )
	{
		return true;
	}
	return false;
}

//---------------------------.
// ウィンドウオブジェクトの初期化.
//---------------------------.
void CWindowObject::WindowObjectInit()
{
	// 画像の取得.
	m_AddCenterPosition = m_pSprite->GetSpriteState().AddCenterPos;

	// タスクバーの情報を取得.
	const RECT& TaskBar = WindowManager::GetTaskBarRect();

	// ウィンドウのサイズを当たり判定に対応したサイズに更新.
	m_WndRect.top		= static_cast<LONG>( m_CeilingPosY	);
	m_WndRect.bottom	= static_cast<LONG>( TaskBar.top	);

	// ウィンドウの情報の補正値の取得.
	m_AddWndRect = WindowManager::GetAddWindowRect();

	// ウィンドウオブジェクトマネージャーに登録.
	WindowObjectManager::PushObject( this, &m_SpriteState.Transform );
}

//---------------------------.
// ウィンドウオブジェクトの更新.
//---------------------------.
void CWindowObject::WindowObjectUpdate()
{
	Grab();						// 掴む.
	GrabMoveUpdate();			// 掴んでいる時の更新.
	Separate();					// 掴んでいる物を離す.
	PushGravity();				// 重力の追加.
	CreateMoveVector();			// 移動ベクトルの作成.
	Move();						// 移動.

	WindowCollision();			// 他ウィンドウとの当たり判定.
	InWindowCollision();		// 中に入っているウィンドウとの当たり判定.
	ScreenCollision();			// スクリーンから出ないようにする当たり判定.

	TeleportTrashCanCheck();	// ごみ箱に移動できるかの確認.
}

//---------------------------.
// ウィンドウオブジェクトの当たり判定後の更新.
//---------------------------.
void CWindowObject::WindowObjectLateUpdate()
{
	// 移動ベクトルの初期化.
	InitMoveVector();
}

//---------------------------.
// 掴む.
//---------------------------.
void CWindowObject::Grab()
{
	if ( m_SpriteState.IsDisp == false	) return;
	if ( m_IsGrab						) return;
	if ( GrabCheck()		  == false	) return;

	// 位置の取得.
	D3DXPOSITION3*		 Pos		= &m_SpriteState.Transform.Position;
	const D3DXPOSITION2& MousePos	= Input::GetMousePosition();

	// マウスが画像の上で左クリックを押された瞬間かどうか.
	if ( GetIsMouseOver() && KeyInput::IsKeyDown( VK_LBUTTON ) )
	{
		// 掴んでいる状態にする.
		m_IsGrab			= true;
		m_IsFall			= false;

		// 画像の中央からの掴んだ場所の補正値を保存.
		m_GrabPosition.x	= MousePos.x - Pos->x;
		m_GrabPosition.y	= MousePos.y - Pos->y;

		// 掴んだ時の初期化.
		GrabInit();

		// ウィンドウの中から出す.
		EscapeWindow();
	}
}

//---------------------------.
// 掴んでいる時の更新.
//---------------------------.
void CWindowObject::GrabMoveUpdate()
{
	if ( m_SpriteState.IsDisp	== false ) return;
	if ( m_IsGrab				== false ) return;

	// 位置の取得.
	const D3DXPOSITION2& OldMousePos = Input::GetMouseOldPosition();
	const D3DXPOSITION2& MousePos	 = Input::GetMousePosition();

	// 位置をマウスの場所にする.
	m_SpriteState.Transform.Position.x = MousePos.x - m_GrabPosition.x;
	m_SpriteState.Transform.Position.y = MousePos.y - m_GrabPosition.y;

	// マウスの移動ベクトルの作成.
	m_OldMoveVector.x = ( MousePos.x - OldMousePos.x ) / 2.0f;
	m_OldMoveVector.y = ( MousePos.y - OldMousePos.y ) / 2.0f;

	// 掴みの更新.
	GrabUpdate();
}

//---------------------------.
// 掴んでいる物を離す.
//---------------------------.
void CWindowObject::Separate()
{
	if ( m_IsGrab							== false ) return;
	if ( KeyInput::IsKeyUp( VK_LBUTTON )	== false ) return;
	
	// 掴みを解除する.
	m_IsGrab		= false;
	m_GrabPosition	= INIT_FLOAT2;
	InWindowCheck();

	// 離した時の初期化.
	SeparateInit();
}

//---------------------------.
// ウィンドウの中に入れるかのチェック.
//---------------------------.
void CWindowObject::InWindowCheck()
{
	const HWND& hWnd = WindowManager::GetMouseOverTheWindow();

	if ( hWnd != NULL ){
		// ウィンドウの情報を保存.
		m_InWndHandle = hWnd;
	}
}

//---------------------------.
// ごみ箱に移動できるか確認.
//---------------------------.
void CWindowObject::TeleportTrashCanCheck()
{
	if ( m_IsTrashCan == false ) return;

	// ごみ箱のウィンドウハンドルを取得.
	const HWND& hWnd = WindowManager::GetFindWindow( "ごみ箱" );

	// ごみ箱が見つかったか.
	if ( hWnd == NULL ) return;

	// ごみ箱の中に入れる.
	m_IsTrashCan  = false;
	m_InWndHandle = hWnd;
}

//---------------------------.
// ウィンドウから出す.
//---------------------------.
void CWindowObject::EscapeWindow()
{
	if ( m_InWndHandle == NULL ) return;

	// 中に入ってるウィンドウ情報を削除.
	m_InWndHandle = NULL;

	// 描画する範囲を削除.
	m_SpriteState.RenderArea = { 0.0f, 0.0f, -1.0f, -1.0f };
}

//---------------------------.
// 重力の追加.
//---------------------------.
void CWindowObject::PushGravity()
{
	if ( m_SpriteState.IsDisp == false ) return;

	m_VectorList.emplace_back( GRAVITY_VECTOR * m_GravityPower );
}

//---------------------------.
// 移動ベクトルの作成.
//---------------------------.
void CWindowObject::CreateMoveVector()
{
	if ( m_SpriteState.IsDisp == false ) return;

	m_MoveVector = m_OldMoveVector * m_SpeedRate;
	if (		m_MoveVector.x > 0.0f ) m_MoveVector.x *= m_DireSpeedRate.x;
	else if (	m_MoveVector.x < 0.0f ) m_MoveVector.x *= m_DireSpeedRate.z;
	if (		m_MoveVector.y > 0.0f ) m_MoveVector.y *= m_DireSpeedRate.y;
	else if (	m_MoveVector.y < 0.0f ) m_MoveVector.y *= m_DireSpeedRate.w;

	for ( auto& v : m_VectorList ) {
		m_MoveVector += v;
	}
}

//---------------------------.
// 移動ベクトルの初期化.
//---------------------------.
void CWindowObject::InitMoveVector()
{
	if ( m_SpriteState.IsDisp == false ) return;

	// ベクトルリストの初期化.
	m_VectorList.clear();

	// 移動ベクトルを保存.
	m_OldMoveVector = m_MoveVector;

	// ベクトルが小さすぎる場合初期化する.
	const float Length = D3DXVec3Length( &m_OldMoveVector );
	if ( Length <= 0.1f ) m_OldMoveVector = INIT_FLOAT3;
}

//---------------------------.
// 移動.
//---------------------------.
void CWindowObject::Move()
{
	if ( m_SpriteState.IsDisp == false ) return;

	// 移動.
	m_SpriteState.Transform.Position += m_MoveVector;
	m_Transform = m_SpriteState.Transform;

	// 移動時の更新.
	MoveUpdate();

	// 落下しているか.
	if ( m_IsLanding == false && m_IsGrab == false && m_IsFall == false && m_MoveVector.y > 0.0f ) {
		m_IsFall		= true;
		m_StartPosition = m_SpriteState.Transform.Position;
	}

	// 初期化.
	if ( m_IsLanding == false ) return;
	m_IsLanding = false;
}

//---------------------------.
// 他ウィンドウの当たり判定.
//---------------------------.
void CWindowObject::WindowCollision()
{
	if ( m_SpriteState.IsDisp == false	) return;
	if ( m_IsGrab						) return;

	// 位置の取得.
	D3DXPOSITION3* pPos = &m_SpriteState.Transform.Position;

	// 他ウィンドウの情報を取得.
	const WindowManager::WndList& WindList = WindowManager::GetWindowList();

	// ウィンドウに当たっているか調べる.
	const int			 InWndZOrder = WindowManager::GetWindowZOrder( m_InWndHandle );
	const D3DXPOSITION3& CPos		 = *pPos + m_AddCenterPosition;
	for ( auto& [w, hWnd] : WindList ) {
		// ボックス通しが当たっているか調べる.
		if ( ( CPos.x - m_CollSize / 2.0f < w.right	 ) && ( CPos.x + m_CollSize / 2.0f	> w.left ) &&
			 ( CPos.y - m_CollSize / 2.0f < w.bottom ) && ( CPos.y + m_CollSize / 2.0f	> w.top  ) )
		{
			// 中に入っているウィンドウとの当たり判定を無くす.
			if ( m_InWndHandle == hWnd ) continue;

			// 中に入っているウィンドウより下のウィンドウか.
			if ( m_InWndHandle != NULL ) {
				if ( InWndZOrder < WindowManager::GetWindowZOrder( hWnd ) ) continue;
			}

			// 押されて中にめり込んだか.
			const D3DXPOSITION3& OldPos = m_SpriteState.Transform.OldPosition + m_AddCenterPosition;
			if ( ( OldPos.x < w.right  ) && ( OldPos.x > w.left ) &&
				 ( OldPos.y < w.bottom ) && ( OldPos.y > w.top ) )
			{
				// めり込んだウィンドウの中に入れる.
				m_InWndHandle = hWnd;
				return;
			}

			// 前回の座標で位置関係を取得.
			EDirection Dire	= EDire::None;
			if (		w.left	<= OldPos.x && OldPos.x <= w.right	&&	w.bottom	<= OldPos.y ) Dire = EDirection::Up;
			else if (	w.left	<= OldPos.x	&& OldPos.x <= w.right	&&	w.top		>= OldPos.y ) Dire = EDirection::Down;
			else if (	w.top	<= OldPos.y	&& OldPos.y <= w.bottom	&&	w.right		<= OldPos.x ) Dire = EDirection::Left;
			else if (	w.top	<= OldPos.y && OldPos.y <= w.bottom	&&	w.left		>= OldPos.x ) Dire = EDirection::Right;
			else if (	w.right	<= OldPos.x && w.bottom	<= OldPos.y ) {
				const float DistanceR = OldPos.x - w.right;
				const float DistanceD = OldPos.y - w.bottom;
				if ( DistanceR > DistanceD ) Dire = EDirection::Left;
				else						 Dire = EDirection::Up;
			}
			else if (	w.right	<= OldPos.x	&& w.top	>= OldPos.y ) {
				const float DistanceR = OldPos.x - w.right;
				const float DistanceU = w.top	 - OldPos.y;
				if ( DistanceR > DistanceU ) Dire = EDirection::Left;
				else						 Dire = EDirection::Down;
			}
			else if (	w.left	>= OldPos.x && w.bottom	<= OldPos.y ) {
				const float DistanceL = w.left	 - OldPos.x;
				const float DistanceD = OldPos.y - w.bottom;
				if ( DistanceL > DistanceD ) Dire = EDirection::Right;
				else						 Dire = EDirection::Up;
			}
			else if (	w.left	>= OldPos.x && w.top	>= OldPos.y ) {
				const float DistanceL = w.left	- OldPos.x;
				const float DistanceU = w.top	- OldPos.y;
				if ( DistanceL > DistanceU ) Dire = EDirection::Right;
				else						 Dire = EDirection::Down;
			}

			// 位置関係を見て当たり判定の動作を変更.
			switch ( Dire ) {
			case EDirection::Up:	HitUp( w );		break;
			case EDirection::Down:	HitDown( w );	break;
			case EDirection::Left:	HitLeft( w );	break;
			case EDirection::Right:	HitRight( w );	break;
			default:								break;
			}
		}
	}
}

//---------------------------.
// 中に入っているウィンドウとの当たり判定.
//---------------------------.
void CWindowObject::InWindowCollision()
{
	if ( m_InWndHandle == NULL ) return;

	// ウィンドウの情報を取得.
	RECT WndRect;
	const bool IsOldSmall	= m_IsInWndSmall;
	const bool IsWndOpen	= GetWindowRect( m_InWndHandle, &WndRect );
	DwmGetWindowAttribute( m_InWndHandle, DWMWA_EXTENDED_FRAME_BOUNDS, &WndRect, sizeof( RECT ) );
	m_IsInWndSmall = IsIconic( m_InWndHandle );

	// ウィンドウがないため非表示にする.
	m_SpriteState.IsDisp = !m_IsInWndSmall;

	// ウィンドウが破棄された.
	if ( IsWndOpen == false && IsOldSmall ) {
		EscapeWindow();
		m_IsTrashCan			= true;
		m_SpriteState.IsDisp	= false;
		return;
	}
	if ( IsWndOpen == false && m_IsInWndSmall == false ) {
		EscapeWindow();
		return;
	}
	
	// ウィンドウの情報を調整.
	if ( m_SpriteState.IsDisp == false ) return;
	WndRect.left	+= m_AddWndRect.left;
	WndRect.top		+= m_AddWndRect.top;
	WndRect.right	+= m_AddWndRect.right;
	WndRect.bottom	+= m_AddWndRect.bottom;

	// 描画エリアの設定.
	const bool IsWndBig = IsZoomed( m_InWndHandle );
	if ( IsWndBig == false ) {
		m_SpriteState.RenderArea.x	= static_cast<float>( WndRect.left					);
		m_SpriteState.RenderArea.y	= static_cast<float>( WndRect.top					);
		m_SpriteState.RenderArea.z	= static_cast<float>( WndRect.right - WndRect.left	);
		m_SpriteState.RenderArea.w	= static_cast<float>( WndRect.bottom - WndRect.top	);
	}
	else m_SpriteState.RenderArea	= { 0.0f, 0.0f, -1.0f, -1.0f };

	// ウィンドウから出ていないか確認.
	const D3DXPOSITION3& CPos = m_SpriteState.Transform.Position + m_AddCenterPosition;
	if (		CPos.y - m_CollSize / 2.0f	< WndRect.top		) HitUp(	WndRect, false );
	else if (	CPos.y + m_CollSize / 2.0f	> WndRect.bottom	) HitDown(	WndRect, false );
	if (		CPos.x - m_CollSize / 2.0f	< WndRect.left		) HitLeft(	WndRect, false );
	else if (	CPos.x + m_CollSize / 2.0f	> WndRect.right		) HitRight( WndRect, false );
}

//---------------------------.
// スクリーンの当たり判定.
//---------------------------.
void CWindowObject::ScreenCollision()
{
	if ( m_SpriteState.IsDisp	== false ) return;
	if ( m_InWndHandle			!= NULL  ) return;

	// スクリーンから出ていないか確認.
	const D3DXPOSITION3& CPos = m_SpriteState.Transform.Position + m_AddCenterPosition;
	if (		CPos.y - m_CollSize / 2.0f	< m_WndRect.top		) HitUp(	m_WndRect, false );
	else if (	CPos.y + m_CollSize / 2.0f	> m_WndRect.bottom	) HitDown(	m_WndRect, false );
	if (		CPos.x - m_CollSize / 2.0f	< m_WndRect.left	) HitLeft(	m_WndRect, false );
	else if (	CPos.x + m_CollSize / 2.0f	> m_WndRect.right	) HitRight( m_WndRect, false );
}

//---------------------------.
// 上側に当たった.
//---------------------------.
void CWindowObject::HitUp( const RECT& HitWnd, const bool IsFromOutside )
{
	// 当たった場所に揃える.
	const float HitPos = static_cast<float>( IsFromOutside ? HitWnd.bottom : HitWnd.top );
	m_SpriteState.Transform.Position.y = HitPos - m_AddCenterPosition.y + m_CollSize / 2.0f;

	// 上に当たった時の更新.
	HitUpUpdate();
}

//---------------------------.
// 下側に当たった.
//---------------------------.
void CWindowObject::HitDown( const RECT& HitWnd, const bool IsFromOutside )
{
	// 当たった場所に揃える.
	const float HitPos = static_cast<float>( IsFromOutside ? HitWnd.top : HitWnd.bottom );
	m_SpriteState.Transform.Position.y = HitPos - m_AddCenterPosition.y - m_CollSize / 2.0f;;
	
	// 下に当たった時の更新.
	HitDownUpdate();

	// 着地させる.
	Landing( HitWnd );
}

//---------------------------.
// 左側に当たった.
//---------------------------.
void CWindowObject::HitLeft( const RECT& HitWnd, const bool IsFromOutside )
{
	// 当たった場所に揃える.
	const float HitPos = static_cast<float>( IsFromOutside ? HitWnd.right : HitWnd.left );
	m_SpriteState.Transform.Position.x = HitPos - m_AddCenterPosition.x + m_CollSize / 2.0f;

	// 左に当たった時の更新.
	HitLeftUpdate();
}

//---------------------------.
// 右側に当たった.
//---------------------------.
void CWindowObject::HitRight( const RECT& HitWnd, const bool IsFromOutside )
{
	// 当たった場所に揃える.
	const float HitPos = static_cast<float>( IsFromOutside ? HitWnd.left : HitWnd.right );
	m_SpriteState.Transform.Position.x = HitPos - m_AddCenterPosition.x - m_CollSize / 2.0f;

	// 右に当たった時の更新.
	HitRightUpdate();
}

//---------------------------.
// 着地.
//---------------------------.
void CWindowObject::Landing( const RECT& HitWnd )
{
	// 着地させる.
	m_IsLanding	 = true;
	m_LandingWnd = HitWnd;

	// 落下してたか.
	if ( m_IsFall == false ) return;
	m_IsFall = false;

	// 着地時の初期化.
	LandingInit();
}
