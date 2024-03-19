#include "Player.h"
#include "..\..\SleepEffectManager\SleepEffectManager.h"
#include "..\..\NoteEffectManager\NoteEffectManager.h"
#include "..\..\..\..\..\Common\DirectX\DirectX11.h"
#include "..\..\..\..\..\Common\SoundManeger\SoundManeger.h"
#include "..\..\..\..\..\Scene\SceneManager\SceneManager.h"
#include "..\..\..\..\..\Utility\Math\Math.h"
#include "..\..\..\..\..\Utility\Random\Random.h"
#include "..\..\..\..\..\Utility\Input\Input.h"
#include "..\..\..\..\..\Utility\SaveDataManager\SaveDataManager.h"

namespace {
	const D3DXVECTOR3	LEFT_MOVE_VECTOR	= { -10.0f, -20.0f, 0.0f };	// 左に移動するベクトル.
	const D3DXVECTOR3	RIGHT_MOVE_VECTOR	= { 10.0f,  -20.0f, 0.0f };	// 右に移動するベクトル.
	const D3DXVECTOR3	JUMP_VECTOR			= { 0.0f,   -20.0f, 0.0f };	// ジャンプベクトル.
	const float			PLAYER_SIZE			= 80.0f;					// 画像のサイズ.
	const float			CEILING_POS			= -PLAYER_SIZE * 5.0f;		// 天井の位置.

	// 見た目No.
	enum enImageNo : unsigned char {
		Image_Normal,		// 通常時.
		Image_Right,		// 右向き.
		Image_Left			// 左向き.
	} typedef EImageNo;

	// 行動No.
	enum enActionNo : unsigned char {
		ActionWait,						// 待機.
		ActionMove,						// ランダムな場所に移動.
		ActionJump,						// ジャンプ.
		ActionPlayWithBall,				// ボールで遊ぶ.

		Random_Action_Max,
		ActionSleep = Random_Action_Max,// 寝る.
		ActionMusic,					// 音が鳴ってるときの行動.

		Action_Max
	} typedef EActionNo;
}

CPlayer::CPlayer()
	: m_pCollapsed		( nullptr )
	, m_CollapsedState	()
	, m_pSleepEffect	( nullptr )
	, m_pNoteEffect		( nullptr )
	, m_ActionTime		( INIT_FLOAT )
	, m_MoveTimeCnt		( INIT_FLOAT )
	, m_CoolTime		( INIT_FLOAT )
	, m_Action			( INIT_INT )
	, m_IsLandingWait	( false )
	, m_IsOldMoveLeft	( false )
{
}

CPlayer::~CPlayer()
{
}

//---------------------------.
// 初期化.
//---------------------------.
bool CPlayer::Init()
{
	// 画像の取得.
	m_pSprite		= SpriteResource::GetSprite( "Slime", &m_SpriteState );
	m_pCollapsed	= SpriteResource::GetSprite( "CollapsedSlime", &m_CollapsedState );

	// 眠りエフェクトの初期化.
	m_pSleepEffect	= std::make_unique<CSleepEffectManager>();
	m_pSleepEffect->Init();

	// 音符エフェクトの初期化.
	m_pNoteEffect	= std::make_unique<CNoteEffectManager>();
	m_pNoteEffect->Init();

	// タスクバーの情報を取得.
	const RECT& TaskBar = WindowManager::GetTaskBarRect();

	// 画像の設定.
	m_SpriteState.Transform.Position.x		= 256.0f;
	m_SpriteState.Transform.Position.y		= static_cast<float>( TaskBar.top );
	m_SpriteState.Color.w					= 0.85f;
	m_CollapsedState.Color.w				= 0.85f;
	m_SpriteState.AnimState.FrameTime		= 50;
	m_SpriteState.AnimState.IsAnimationY	= true;
	m_SpriteState.AnimState.PatternNo.x 	= Image_Normal;

	// オブジェクトの初期化.
	m_CollSize			= PLAYER_SIZE;
	m_SpeedRate			= 0.95f;
	m_CeilingPosY		= CEILING_POS;
	m_ObjectTag			= EObjectTag::Player;

	SaveDataManager::SetSaveData()->PlayerTransform = &m_SpriteState.Transform;

	WindowObjectInit();
	InitCollision();
	return true;
}

//---------------------------.
// 更新.
//---------------------------.
void CPlayer::Update( const float& DeltaTime )
{
	m_DeltaTime = DeltaTime;
	if ( m_IsDisp == false	) return;

	ActionUpdate();		// 行動の更新.
	EffectUpdate();		// エフェクトの更新.

	// ウィンドウオブジェクトの更新.
	WindowObjectUpdate();

	int a_b = 0;
	a_b = 10;

	// 描画エリアをそろえる.
	m_CollapsedState.RenderArea		= m_SpriteState.RenderArea;
	m_CollapsedState.RenderArea.w	= -1.0f;

	TransformUpdate( m_SpriteState.Transform );
	UpdateCollision();
	if ( m_SpriteState.IsDisp == false ) return;
	ActorCollisionManager::PushObject( this, ECollObjType::Both );
}

//---------------------------.
// 当たり判定終了後呼び出される更新.
//---------------------------.
void CPlayer::LateUpdate( const float& DeltaTime )
{
	WindowObjectLateUpdate();
}

//---------------------------.
// 描画.
//---------------------------.
void CPlayer::Render()
{
	if ( m_IsDisp == false ) return;
	m_pSleepEffect->Render();
	m_pNoteEffect->Render();
	if ( m_IsLandingWait )	m_pCollapsed->RenderUI( &m_CollapsedState );
	else					m_pSprite->RenderUI( &m_SpriteState );
}

//---------------------------.
// 当たり判定関数.
//---------------------------.
void CPlayer::Collision( CActor* pActor )
{
}

//---------------------------.
// スライムをつぶす.
//---------------------------.
void CPlayer::Collapsed()
{
	// 着地硬直の設定.
	m_IsGrab		= false;
	m_IsLandingWait = true;
	m_CoolTime		= 4.0f;

	// つぶれた画像の位置の設定.
	m_CollapsedState.Transform.Position = m_SpriteState.Transform.Position;

	// 行動の終了.
	ActionEnd();
}

//---------------------------.
// 当たり判定の初期化.
//---------------------------.
void CPlayer::InitCollision()
{
	m_pCollisions = std::make_unique<CCollisions>();
	m_pCollisions->InitCollision( ECollType::Sphere2D );
	m_pCollisions->GetCollision<CSphere2D>()->SetSprite( m_pSprite, &m_SpriteState );
	m_pCollisions->GetCollision<CSphere2D>()->SetRadius( 40.0f );
}

//---------------------------.
// 当たり判定の更新.
//---------------------------.
void CPlayer::UpdateCollision()
{
}

//---------------------------.
// 掴めるかの確認.
//---------------------------.
bool CPlayer::GrabCheck()
{
	if ( m_Action == ActionSleep	) return false;
	if ( m_IsLandingWait			) return false;
	return true;
}

//---------------------------.
// 掴んだ時の初期化.
//---------------------------.
void CPlayer::GrabInit()
{
	// 行動を待機に変更する.
	ActionEnd();
	m_Action		= ActionWait;
	m_ActionTime	= 2.0f;
}

//---------------------------.
// 移動時の更新.
//---------------------------.
void CPlayer::MoveUpdate()
{
	// 見た目の変更.
	if ( m_IsGrab ) {
		m_SpriteState.AnimState.PatternNo.x = Image_Normal;
	}
	else {
		if (		m_MoveVector.x > 0.0f ) m_SpriteState.AnimState.PatternNo.x = Image_Right;
		else if (	m_MoveVector.x < 0.0f ) m_SpriteState.AnimState.PatternNo.x = Image_Left;
		else								m_SpriteState.AnimState.PatternNo.x = Image_Normal;
	}
}

//---------------------------.
// 上に当たった時の更新.
//---------------------------.
void CPlayer::HitUpUpdate()
{
	// 移動の力を無くす.
	if ( m_MoveVector.y < 0.0f ) m_MoveVector.y = 0.0f;
}

//---------------------------.
// 下に当たった時の更新.
//---------------------------.
void CPlayer::HitDownUpdate()
{
	// 移動の力を無くす.
	if ( m_MoveVector.y > 0.0f ) m_MoveVector.y = 0.0f;

	// 横方向の力を減らす.
	m_MoveVector.x *= 0.8f;
}

//---------------------------.
// 左に当たった時の更新.
//---------------------------.
void CPlayer::HitLeftUpdate()
{
	// 移動の力を無くす.
	if ( m_MoveVector.x > 0.0f ) m_MoveVector.x = 0.0f;

	// 上方向の力を減らす.
	if ( m_MoveVector.y < 0.0f ) m_MoveVector.y *= 0.8f;
}

//---------------------------.
// 右に当たった時の更新.
//---------------------------.
void CPlayer::HitRightUpdate()
{
	// 移動の力を無くす.
	if ( m_MoveVector.x > 0.0f ) m_MoveVector.x = 0.0f;

	// 上方向の力を減らす.
	if ( m_MoveVector.y < 0.0f ) m_MoveVector.y *= 0.8f;
}

//---------------------------.
// 着地時の初期化.
//---------------------------.
void CPlayer::LandingInit()
{
	// 着地後に硬直させるか.
	if ( m_IsLandingWait ) return;
	const float Fall	= m_SpriteState.Transform.Position.y - m_StartPosition.y;
	m_StartPosition		= INIT_FLOAT3;
	if ( Fall <= 400.0f ) return;

	// 着地硬直の設定.
	float Power = Fall / ( m_Wnd_h / 2.0f );
	m_IsGrab		= false;
	m_IsLandingWait = true;
	m_CoolTime		= 4.0f * Power;

	// つぶれた画像の位置の設定.
	m_CollapsedState.Transform.Position = m_SpriteState.Transform.Position;

	// 行動の終了.
	ActionEnd();
}

//---------------------------.
// 着地の更新.
//---------------------------.
void CPlayer::LandingUpdate()
{
	if ( m_IsLandingWait == false ) return;

	// つぶれた画像の位置の設定.
	m_CollapsedState.Transform.Position = m_SpriteState.Transform.Position;

	// クリックされたか.
	if ( m_pCollapsed->GetIsOverlappingTheMouse( &m_CollapsedState ) &&
		 KeyInput::IsKeyDown( VK_LBUTTON ) )
	{
		// 待ち時間を少し減らす.
		m_CoolTime -= 0.5f;
	}

	// 待ち時間を減らす.
	m_CoolTime -= m_DeltaTime;

	// 着地の硬直を解除する.
	if ( m_CoolTime < 0.0f ) {
		m_IsLandingWait = false;
		m_CoolTime		= INIT_FLOAT;

		// ジャンプアクションに変更.
		m_Action		= ActionJump;
		m_ActionTime	= 3.0f;
	}
}

//---------------------------.
// エフェクトの更新.
//---------------------------.
void CPlayer::EffectUpdate()
{
	// 座標の更新.
	m_pSleepEffect->SetPosition( m_SpriteState.Transform.Position );
	m_pNoteEffect->SetPosition( m_SpriteState.Transform.Position );

	// エフェクトの更新.
	m_pSleepEffect->Update( m_DeltaTime );
	m_pNoteEffect->Update( m_DeltaTime );
}

//---------------------------.
// 行動の終了時の初期化.
//---------------------------.
void CPlayer::ActionEnd()
{
	switch ( m_Action ) {
	case ActionWait:
		// 初期化.
		m_EndPosition = INIT_FLOAT3;
		m_MoveTimeCnt = INIT_FLOAT;
		break;
	case ActionMove:
		break;
	case ActionJump:
		break;
	case ActionSleep:
		// 初期化.
		m_SpriteState.AnimState.FrameTime = 50;

		// エフェクトの停止.
		m_pSleepEffect->Stop();
		break;
	case ActionPlayWithBall:
		// 初期化.
		m_EndPosition = INIT_FLOAT3;
		m_MoveTimeCnt = INIT_FLOAT;
		break;
	case ActionMusic:
		// 初期化.
		m_EndPosition = INIT_FLOAT3;
		m_MoveTimeCnt = INIT_FLOAT;

		// エフェクトの停止.
		m_pNoteEffect->Stop();
		break;
	default:
		break;
	}
}

//---------------------------.
// 行動の開始の初期化.
//---------------------------.
void CPlayer::ActionStart()
{
	switch ( m_Action ) {
	case ActionWait:
		break;
	case ActionMove:
		break;
	case ActionJump:
		break;
	case ActionSleep:
		// エフェクトの開始.
		m_pSleepEffect->Play();

		// 行動の行う時間を長くする.
		m_ActionTime *= Random::GetRand( 10.0f, 30.0f );
		m_SpriteState.AnimState.FrameTime = 100;
		break;
	case ActionPlayWithBall:
		// 行動の行う時間を長くする.
		m_ActionTime *= Random::GetRand( 3.0f, 20.0f );
		break;
	case ActionMusic:
		// エフェクトの開始.
		m_pNoteEffect->Play( 128.0f );
		m_EndPosition = m_SpriteState.Transform.Position;

		// 行動の行う時間を長くする.
		m_ActionTime *= Random::GetRand( 10.0f, 60.0f );
		break;
	default:
		break;
	}
}

//---------------------------.
// 行動の更新.
//---------------------------.
void CPlayer::ActionUpdate()
{
	if ( m_IsInWndSmall ) return;

	// 着地の硬直の更新.
	LandingUpdate();
	if ( m_IsLandingWait ) return;

	// 行動の更新.
	switch ( m_Action ) {
	case ActionWait:			ActionWaitUpdate();			break;
	case ActionMove:			ActionMoveUpdate();			break;
	case ActionJump:			ActionJumpUpdate();			break;
	case ActionSleep:			ActionSleepUpdate();		break;
	case ActionPlayWithBall:	ActionPlayWithBallUpdate();	break;
	case ActionMusic:			ActionMusicUpdate();		break;
	default:												break;
	}

	// 行動の時間を減らす.
	if ( m_IsGrab ) return;
	m_ActionTime -= m_DeltaTime;
	if ( m_ActionTime <= 0.0f ) {
		// 行動の変更.
		ActionEnd();
		m_Action		= Random::GetRand( 0, Random_Action_Max - 1 );
		m_ActionTime	= Random::GetRand( 3.0f, 10.0f );
		if ( Random::Probability( 1, 20 )			) m_Action = ActionSleep;
		if ( SoundManager::GetPeakValue() > 0.0f	) {
			if ( Random::Probability( 1, 5 )		) m_Action = ActionMusic;
		}
		ActionStart();
	}
}

//---------------------------.
// 待機.
//---------------------------.
void CPlayer::ActionWaitUpdate()
{
}

//---------------------------.
// 移動の行動の更新.
//---------------------------.
void CPlayer::ActionMoveUpdate()
{
	// 移動時間を加算.
	m_MoveTimeCnt += m_DeltaTime;

	// 浮いている場合は処理は行わない.
	if ( m_IsLanding == false ) return;

	// クールタイムの減少.
	m_CoolTime -= m_DeltaTime;
	if ( m_CoolTime > 0.0f ) return;

	// 目的地を設定.
	if ( m_EndPosition == D3DXPOSITION3( INIT_FLOAT3 ) || m_EndPosition.x < m_LandingWnd.left || m_EndPosition.x > m_LandingWnd.right ) {
		m_EndPosition.x = Random::GetRand( m_LandingWnd.left + PLAYER_SIZE * 2.0f, m_LandingWnd.right - PLAYER_SIZE * 2.0f );
		m_IsOldMoveLeft	= m_EndPosition.x < m_SpriteState.Transform.Position.x;
	}
	
	// 移動ベクトルの追加.
	const bool	IsLeftMove = m_EndPosition.x < m_SpriteState.Transform.Position.x;
	D3DXVECTOR3 MoveVecotr = IsLeftMove ? LEFT_MOVE_VECTOR : RIGHT_MOVE_VECTOR;
	m_VectorList.emplace_back( MoveVecotr );

	// 目的地に到着したか.
	if ( m_IsOldMoveLeft != IsLeftMove || m_MoveTimeCnt > 10.0f ) {
		m_EndPosition	= INIT_FLOAT3;
		m_MoveTimeCnt	= INIT_FLOAT;
	}
	m_IsOldMoveLeft = IsLeftMove;

	// クールタイムの設定.
	m_CoolTime = Random::GetRand( 0.1f, 1.0f );
}

//---------------------------.
// ボールで遊ぶ行動の更新.
//---------------------------.
void CPlayer::ActionPlayWithBallUpdate()
{
	// 浮いている場合は処理は行わない.
	if ( m_IsLanding == false ) return;

	// クールタイムの減少.
	m_CoolTime -= m_DeltaTime;
	if ( m_CoolTime > 0.0f ) return;

	// 目的地の更新.
	m_EndPosition = WindowObjectManager::GetTransform( EObjectTag::Ball, 0 )->Position;
	
	// 移動ベクトルの追加.
	const bool	IsLeftMove = m_EndPosition.x < m_SpriteState.Transform.Position.x;
	D3DXVECTOR3 MoveVecotr = IsLeftMove ? LEFT_MOVE_VECTOR : RIGHT_MOVE_VECTOR;
	m_VectorList.emplace_back( MoveVecotr );

	// クールタイムの設定.
	m_CoolTime = Random::GetRand( 0.05f, 0.25f );
}

//---------------------------.
// ジャンプの行動の更新.
//---------------------------.
void CPlayer::ActionJumpUpdate()
{
	// 浮いている場合は処理は行わない.
	if ( m_IsLanding == false ) return;

	// クールタイムの減少.
	m_CoolTime -= m_DeltaTime;
	if ( m_CoolTime > 0.0f ) return;

	// ベクトルの追加.
	m_VectorList.emplace_back( JUMP_VECTOR );

	// クールタイムの設定.
	m_CoolTime = Random::GetRand( 0.1f, 1.0f );
}

//---------------------------.
// 寝るの行動の更新.
//---------------------------.
void CPlayer::ActionSleepUpdate()
{
	// 眠りエフェクトのポーズの設定.
	m_pSleepEffect->Pause( !m_SpriteState.IsDisp );

	// クリックされたか.
	if ( m_pSprite->GetIsOverlappingTheMouse( &m_SpriteState ) &&
		 KeyInput::IsKeyDown( VK_LBUTTON ) )
	{
		// 行動の時間を減らす.
		m_ActionTime *= 0.5f;
	}
}

//---------------------------.
// 音が鳴ってるときの行動の更新.
//---------------------------.
void CPlayer::ActionMusicUpdate()
{
	// 浮いている場合は処理は行わない.
	if ( m_IsLanding == false ) return;

	// 眠りエフェクトのポーズの設定.
	m_pNoteEffect->Pause( !m_SpriteState.IsDisp );

	// 音が止まった場合行動を終了する.
	if ( SoundManager::GetPeakValue() == 0.0f ) {
		m_ActionTime = 0.0f;
	}

	// クールタイムの減少.
	m_CoolTime -= m_DeltaTime;
	if ( m_CoolTime > 0.0f ) return;
	
	// 移動ベクトルの追加.
	const bool	IsLeftMove = m_EndPosition.x < m_SpriteState.Transform.Position.x;
	D3DXVECTOR3 MoveVecotr = IsLeftMove ? LEFT_MOVE_VECTOR : RIGHT_MOVE_VECTOR;
	MoveVecotr.y /= 2.0f;
	m_VectorList.emplace_back( MoveVecotr );

	// クールタイムの設定.
	m_CoolTime = Random::GetRand( 0.05f, 0.25f );
}
