#include "WateringCan.h"
#include "WaterManager/WaterManager.h"
#include "..\..\..\..\..\Utility\Input\Input.h"
#include "..\..\..\..\..\Utility\SaveDataManager\SaveDataManager.h"
#include "..\..\..\..\..\Utility\Random\Random.h"
#include "..\..\..\..\..\Utility\Const\Const.h"

namespace {
	const float		OBJ_SIZE		= 55.0f;	// ジョウロのサイズ.
	const __int64	MAX_MOUSE_SPEED = 2;		// 一番重い時のマウス速度
	const float		WATER_MIN		= 0.0f;		// 水の量の最小値
	const float		WATER_MAX		= 1.0f;		// 水の量の最大値
	const float		WATERING_ROT	= -45.0f;	// 水やりの時の角度;
}

CWateringCan::CWateringCan()
	: m_pWaterGaugeSprite		( nullptr )
	, m_WaterGaugeBackState		()
	, m_WaterGaugeState			()
	, m_pWaterEffect			( nullptr )
	, m_LeftWaterTransform		()
	, m_RightWaterTransform		()
	, m_CenterWaterTransform	()
	, m_WateringTransform		()
	, m_LHitWaterData			()
	, m_RHitWaterData			()
	, m_LFullWaterData			()
	, m_RFullWaterData			()
	, m_WateringWaterData		()
	, m_WaterValue				( INIT_FLOAT )
	, m_GaugeDispTime			( INIT_FLOAT )
	, m_GaugeAlpha				( Color::ALPHA_MIN )
	, m_IsWatering				( false )
	, m_IsGaugeDisp				( false )
{
}

CWateringCan::~CWateringCan()
{
}

//---------------------------.
// 初期化.
//---------------------------.
bool CWateringCan::Init()
{
	// 画像の取得.
	m_pSprite = SpriteResource::GetSprite( "WateringCan", &m_SpriteState );

	// 水ゲージの画像を取得.
	m_pWaterGaugeSprite = SpriteResource::GetSprite( "WaterGauge", &m_WaterGaugeBackState );
	m_WaterGaugeState	= m_WaterGaugeBackState;
	m_WaterGaugeBackState.Transform.Scale.x = 10.0f;
	m_WaterGaugeState.Transform.Scale.x		= 10.0f;
	m_WaterGaugeBackState.Color = Color4::Gray;
	m_WaterGaugeState.Color		= Color4::Aqua;

	// 水エフェクトの設定.
	m_pWaterEffect = std::make_unique<CWaterManager>();
	m_pWaterEffect->Init();

	// オブジェクトの初期化.
	m_CollSize			= OBJ_SIZE;
	m_SpeedRate			= 0.996f;
	m_IsDisp			= true;
	m_ObjectTag			= EObjectTag::WateringCan;

	// 水のでる場所の設定.
	InitTransform();
	// 水データの初期化.
	InitWaterData();

	SaveDataManager::SetSaveData()->WateringCanTransform	= &m_SpriteState.Transform;
	SaveDataManager::SetSaveData()->WateringCanWaterValue = &m_WaterValue;

	WindowObjectInit();
	InitCollision();
	return true;
}

//---------------------------.
// 更新.
//---------------------------.
void CWateringCan::Update( const float& DeltaTime )
{
	m_DeltaTime = DeltaTime;
	if ( m_IsDisp == false ) return;

	// ウィンドウオブジェクトの更新.
	WindowObjectUpdate();

	FallSpeedUpdate();		// 落下速度の更新.
	GaugeUpdate();			// ゲージの更新
	CoolTimeUpdate();		// クールタイムの更新
	WateringUpdate();		// 水やりの更新

	m_pWaterEffect->Update( DeltaTime );

	TransformUpdate( m_SpriteState.Transform );
	UpdateCollision();
	if ( m_SpriteState.IsDisp == false ) return;
	ActorCollisionManager::PushObject( this, ECollObjType::BeAHit );
}

//---------------------------.
// 当たり判定終了後呼び出される更新.
//---------------------------.
void CWateringCan::LateUpdate( const float& DeltaTime )
{
	WindowObjectLateUpdate();

	ActorCollisionManager::SetSkipTagToTag( EObjectTag::WateringCan, EObjectTag::WaterFall, false );
}

//---------------------------.
// 描画.
//---------------------------.
void CWateringCan::Render()
{
	if ( m_IsDisp == false ) return;
	m_pSprite->RenderUI( &m_SpriteState );
	m_pWaterGaugeSprite->RenderUI( &m_WaterGaugeBackState );
	m_pWaterGaugeSprite->RenderUI( &m_WaterGaugeState );
	m_pWaterEffect->Render();
}

//---------------------------.
// 当たり判定関数.
//---------------------------.
void CWateringCan::Collision( CActor* pActor )
{
	if ( m_IsWatering			) return;
	if ( m_InWndHandle != NULL	) return;
	if ( pActor->GetObjectTag() != EObjectTag::WaterFall ) return;

	// 当たっているか.
	if ( Coll2D::IsBox2DToSphere2D(
		pActor->GetCollision<CBox2D>(),
		m_pCollisions->GetCollision<CSphere2D>() ) == false ) return;

	// 満タンか
	if ( m_WaterValue >= WATER_MAX ) {
		FullWaterPlay();
	}
	// 水を入れる.
	else {
		ChangeWaterValue( m_WaterValue + 0.1f * m_DeltaTime );
	}

	ActorCollisionManager::SetSkipTagToTag( EObjectTag::WateringCan, EObjectTag::WaterFall, true );
}

//---------------------------.
// 当たり判定の初期化.
//---------------------------.
void CWateringCan::InitCollision()
{
	m_pCollisions = std::make_unique<CCollisions>();
	m_pCollisions->InitCollision( ECollType::Sphere2D );
	m_pCollisions->GetCollision<CSphere2D>()->SetSprite( m_pSprite, &m_SpriteState );
	m_pCollisions->GetCollision<CSphere2D>()->SetRadius( 16.0f );
}

//---------------------------.
// 当たり判定の更新.
//---------------------------.
void CWateringCan::UpdateCollision()
{
}

//---------------------------.
// 掴みの更新.
//---------------------------.
void CWateringCan::GrabUpdate()
{
	// 水の量に対応したマウスの速度に変更する
	const auto s = Input::SetMouseSpeed();
	Input::SetMouseSpeed( static_cast<__int64>( s - ( s - MAX_MOUSE_SPEED ) * m_WaterValue ) );
}

//---------------------------.
// 離した時の初期化.
//---------------------------.
void CWateringCan::SeparateInit()
{
	Input::ResetMouseSpeed();
}

//---------------------------.
// 上に当たった時の更新.
//---------------------------.
void CWateringCan::HitUpUpdate()
{
	m_MoveVector.y = 0.0f;
}

//---------------------------.
// 下に当たった時の更新.
//---------------------------.
void CWateringCan::HitDownUpdate()
{
	if ( m_MoveVector.y > Const::WateringCan.HIT_VEC_Y ) {
		// 落下位置による補正の取得
		auto rate = m_MoveVector.y / Const::WateringCan.HIT_MAX_VEC_Y;
		if ( rate >= 1.0f ) rate = 1.0f;

		// 水を減らす
		auto dw = DecreaseWater( rate );
		if ( dw > 0.0f ) HitWaterPlay( rate, dw );
	}
	m_MoveVector.y = 0.0f;

	// 横方向の力を減らす.
	m_MoveVector.x *= 0.7f;
}

//---------------------------.
// 左に当たった時の更新.
//---------------------------.
void CWateringCan::HitLeftUpdate()
{
	m_MoveVector.x = 0.0f;
}

//---------------------------.
// 右に当たった時の更新.
//---------------------------.
void CWateringCan::HitRightUpdate()
{
	m_MoveVector.x = 0.0f;
}

//---------------------------.
// トランスフォームの初期化
//---------------------------.
void CWateringCan::InitTransform()
{
	// 親の設定.
	m_LeftWaterTransform.AttachParent(	 &m_SpriteState.Transform );
	m_RightWaterTransform.AttachParent(	 &m_SpriteState.Transform );
	m_CenterWaterTransform.AttachParent( &m_SpriteState.Transform );
	m_WateringTransform.AttachParent(	 &m_SpriteState.Transform );

	// 位置の調整
	m_LeftWaterTransform.Position.x		= -10.0f;
	m_LeftWaterTransform.Position.y		= -4.0f;
	m_RightWaterTransform.Position.x	=  18.0f;
	m_RightWaterTransform.Position.y	= -4.0f;
	m_CenterWaterTransform.Position.x	=  4.0f;
	m_CenterWaterTransform.Position.y	=  30.0f;
	m_WateringTransform.Position.x		= -28.0f;
	m_WateringTransform.Position.y		= -4.0f;
}

//---------------------------.
// 水のデータの初期化.
//---------------------------.
void CWateringCan::InitWaterData()
{
	m_LHitWaterData.VecRotMin		= Math::ToRadian( Const::WateringCan.WATER_VEC_ROT_MIN );
	m_LHitWaterData.VecRotMax		= Math::ToRadian( Const::WateringCan.WATER_VEC_ROT_MAX );
	m_RHitWaterData.VecRotMin		= Math::ToRadian( Const::WateringCan.WATER_VEC_ROT_MIN );
	m_RHitWaterData.VecRotMax		= Math::ToRadian( Const::WateringCan.WATER_VEC_ROT_MAX );
	m_LFullWaterData.VecRotMin		= Math::ToRadian( Const::WateringCan.WATER_VEC_ROT_MIN );
	m_LFullWaterData.VecRotMax		= Math::ToRadian( Const::WateringCan.WATER_VEC_ROT_MAX );
	m_RFullWaterData.VecRotMin		= Math::ToRadian( Const::WateringCan.WATER_VEC_ROT_MIN );
	m_RFullWaterData.VecRotMax		= Math::ToRadian( Const::WateringCan.WATER_VEC_ROT_MAX );
	m_WateringWaterData.VecRotMin	= Math::ToRadian( Const::WateringCan.WATER_VEC_ROT_MIN );
	m_WateringWaterData.VecRotMax	= Math::ToRadian( Const::WateringCan.WATER_VEC_ROT_MAX );

	m_LFullWaterData.PowerMin		= Const::WateringCan.FULL_ANIM_POWER_MIN;
	m_LFullWaterData.PowerMax		= Const::WateringCan.FULL_ANIM_POWER_MAX;
	m_RFullWaterData.PowerMin		= Const::WateringCan.FULL_ANIM_POWER_MIN;
	m_RFullWaterData.PowerMax		= Const::WateringCan.FULL_ANIM_POWER_MAX;
	m_WateringWaterData.PowerMin	= Const::WateringCan.WATERING_POWER_MIN;
	m_WateringWaterData.PowerMax	= Const::WateringCan.WATERING_POWER_MAX;
}

//---------------------------.
// 落下速度の更新.
//---------------------------.
void CWateringCan::FallSpeedUpdate()
{
	if ( m_MoveVector.y < Const::WateringCan.FALL_MAX_VECTOR ) {
		m_MoveVector.y += m_WaterValue;
	}
}

//---------------------------.
// ゲージの更新
//---------------------------.
void CWateringCan::GaugeUpdate()
{
	// 水ゲージを強制で表示する場合の表示時間の更新
	if ( m_IsGaugeDisp ) {
		m_GaugeDispTime += m_DeltaTime;
		if ( m_GaugeDispTime > Const::WateringCan.GAUGE_DISP_TIME_MAX ) {
			m_GaugeDispTime = INIT_FLOAT;
			m_IsGaugeDisp	= false;
		}
	}

	// アルファ値の設定.
	if ( ( m_IsGrab || m_IsGaugeDisp ) && m_GaugeAlpha < Color::ALPHA_MAX ) {
		m_GaugeAlpha += Const::WateringCan.GAUGE_DISP_ALPHA_SPEED * m_DeltaTime;
		if ( m_GaugeAlpha >= Color::ALPHA_MAX ) {
			m_GaugeAlpha = Color::ALPHA_MAX;
		}
	}
	else if ( ( !m_IsGrab && !m_IsGaugeDisp ) && m_GaugeAlpha > Color::ALPHA_MIN ) {
		m_GaugeAlpha -= Const::WateringCan.GAUGE_HIDDEN_ALPHA_SPEED * m_DeltaTime;
		if ( m_GaugeAlpha <= Color::ALPHA_MIN ) {
			m_GaugeAlpha = Color::ALPHA_MIN;
		}
	}
	m_WaterGaugeBackState.Color.w	= m_GaugeAlpha;
	m_WaterGaugeState.Color.w		= m_GaugeAlpha;

	// 位置の更新
	if ( m_GaugeAlpha > Color::ALPHA_MIN ) {
		m_WaterGaugeBackState.Transform.Position = m_SpriteState.Transform.Position;
		m_WaterGaugeState.Transform.Position = m_SpriteState.Transform.Position;

		m_WaterGaugeBackState.Transform.Position.y += 35.0f;
		m_WaterGaugeState.Transform.Position.y += 35.0f;
		m_WaterGaugeBackState.Transform.Position.x -= 15.0f;
		m_WaterGaugeState.Transform.Position.x -= 15.0f;
		m_WaterGaugeState.Transform.Scale.x = 10.0f * m_WaterValue;
	}
}

//---------------------------.
// クールタイムの更新.
//---------------------------.
void CWateringCan::CoolTimeUpdate()
{
	// 満タンの時のアニメーションの待機時間の更新
	if ( m_FullAnimCoolTime > 0.0f ) {
		m_FullAnimCoolTime -= m_DeltaTime;
		if ( m_FullAnimCoolTime <= 0.0f ) {
			m_FullAnimCoolTime = 0.0f;
		}
	}
}

//---------------------------.
// 水やりの更新.
//---------------------------.
void CWateringCan::WateringUpdate()
{
	if ( m_IsGrab && Input::GetIsRightClickPress() ) {
		m_IsWatering = true;
	}
	else {
		m_IsWatering = false;
	}

	const auto	rwr  = Math::ToRadian( WATERING_ROT );
	auto*		pRot = &m_SpriteState.Transform.Rotation;
	if ( m_IsWatering ) {
		if ( pRot->z > rwr ) {
			pRot->z -= Math::ToRadian( 10.0f );
			if ( pRot->z <= rwr ) pRot->z = rwr;
		}
		else {
			ChangeWaterValue( m_WaterValue - 0.1f * m_DeltaTime );
			if ( m_WaterValue > WATER_MIN ) WateringWaterPlay();
		}
	}
	else {
		if ( pRot->z < 0.0f ) {
			pRot->z += Math::ToRadian( 10.0f );
			if ( pRot->z >= 0.0f ) pRot->z = 0.0f;
		}
	}
}

//---------------------------.
// 水を減らす
//---------------------------.
float CWateringCan::DecreaseWater( const float rate )
{
	auto dw = ( ( WATER_MAX / 2.0f ) * rate ) * m_WaterValue;

	ChangeWaterValue( m_WaterValue - dw );
	return dw;
}

//---------------------------.
// 水の量を変動させる
//---------------------------.
void CWateringCan::ChangeWaterValue( const float value )
{
	auto old = m_WaterValue;
	m_WaterValue = value;
	m_WaterValue = std::clamp( m_WaterValue, WATER_MIN, WATER_MAX );

	// 水の量が変動したため水ゲージを一定時間表示する
	if ( old != m_WaterValue ) {
		m_IsGaugeDisp	= true;
		m_GaugeDispTime = INIT_FLOAT;
	}
}

//---------------------------.
// 満タンの時の水のエフェクトの再生.
//---------------------------.
void CWateringCan::FullWaterPlay()
{
	if ( m_FullAnimCoolTime > 0.0f ) return;

	// 位置の設定
	auto cwp = m_CenterWaterTransform.GetWorldPosition();
	auto lwp = m_LeftWaterTransform.GetWorldPosition();
	auto rwp = m_RightWaterTransform.GetWorldPosition();
	m_LFullWaterData.Pos = lwp;
	m_RFullWaterData.Pos = rwp;

	// ベクトルの設定
	auto lv = lwp - cwp;
	auto rv = rwp - cwp;
	D3DXVec3Normalize( &m_LFullWaterData.MoveVec, &lv );
	D3DXVec3Normalize( &m_RFullWaterData.MoveVec, &rv );

	// 水の表示
	m_pWaterEffect->Spill( m_LFullWaterData, m_InWndHandle );
	m_pWaterEffect->Spill( m_RFullWaterData, m_InWndHandle );

	m_FullAnimCoolTime = Const::WateringCan.FULL_ANIM_COOL_TIME;
}

//---------------------------.
// 地面にぶつかった時の水のエフェクトの再生
//---------------------------.
void CWateringCan::HitWaterPlay( const float rate, const float decWater )
{
	// 位置の設定
	auto cwp = m_CenterWaterTransform.GetWorldPosition();
	auto lwp = m_LeftWaterTransform.GetWorldPosition();
	auto rwp = m_RightWaterTransform.GetWorldPosition();
	m_LHitWaterData.Pos = lwp;
	m_RHitWaterData.Pos = rwp;

	// ベクトルの設定
	auto lv = lwp - cwp;
	auto rv = rwp - cwp;
	D3DXVec3Normalize( &m_LHitWaterData.MoveVec, &lv );
	D3DXVec3Normalize( &m_RHitWaterData.MoveVec, &rv );

	// 水の飛び出る強さの変更
	m_LHitWaterData.PowerMax = Const::WateringCan.HIT_WATER_POWER_MAX * rate;
	m_RHitWaterData.PowerMax = Const::WateringCan.HIT_WATER_POWER_MAX * rate;
	m_LHitWaterData.PowerMin = m_LHitWaterData.PowerMax / 2.0f;
	m_RHitWaterData.PowerMin = m_RHitWaterData.PowerMax / 2.0f;

	const int max = static_cast<int>( Const::WateringCan.HIT_WATER_NUM_MAX * decWater );
	for ( int i = 0; i < max; i++ ) {
		m_pWaterEffect->Spill( m_LHitWaterData, m_InWndHandle );
		m_pWaterEffect->Spill( m_RHitWaterData, m_InWndHandle );
	}
}

//---------------------------.
// 水やりの水のエフェクトの再生
//---------------------------.
void CWateringCan::WateringWaterPlay()
{
	if ( m_WaterValue <= WATER_MIN ) return;

	// 位置の設定
	auto cwp = m_CenterWaterTransform.GetWorldPosition();
	auto wwp = m_WateringTransform.GetWorldPosition();
	m_WateringWaterData.Pos = wwp;

	// ベクトルの設定
	auto wv = wwp - cwp;
	D3DXVec3Normalize( &m_WateringWaterData.MoveVec, &wv );

	// 水の表示
	const auto hWnd = WindowManager::GetPointOverTheWindow( wwp );
	m_pWaterEffect->Spill( m_WateringWaterData, hWnd );
}
