#include "SleepEffectManager.h"
#include "SleepEffect\SleepEffect.h"
#include "..\..\..\..\..\Resource\SpriteResource\SpriteResource.h"

namespace {
	constexpr float COOL_TIME = 0.5f;	// クールタイム.
}

CSleepEffectManager::CSleepEffectManager()
	: m_pSurprisedEffect		( nullptr )
	, m_SurprisedEffectState	()
	, m_pSleepEffectList		()
	, m_CoolTime				( INIT_FLOAT )
	, m_EffectAnimAng			( INIT_FLOAT )
	, m_IsAnimation				( false )
{
}

CSleepEffectManager::~CSleepEffectManager()
{
}

//---------------------------.
// 初期化.
//---------------------------.
bool CSleepEffectManager::Init()
{
	// 画像の取得.
	m_pSurprisedEffect = SpriteResource::GetSprite( "SurprisedEffect", &m_SurprisedEffectState );
	return true;
}

//---------------------------.
// 更新.
//---------------------------.
void CSleepEffectManager::Update( const float& DeltaTime )
{
	m_DeltaTime = DeltaTime;

	SurprisedEffect();			// びっくりエフェクトの再生.
	SleepEffectPlay();			// 眠りエフェクトの再生.
	SleepEffectListDelete();	// 眠りエフェクトリストの削除.

	for ( auto& e : m_pSleepEffectList ) e->Update( DeltaTime );
}

//---------------------------.
// 描画.
//---------------------------.
void CSleepEffectManager::Render()
{
	for ( auto& e : m_pSleepEffectList ) e->Render();
	m_pSurprisedEffect->RenderUI( &m_SurprisedEffectState );
}

//---------------------------.
// アニメーションの開始.
//---------------------------.
void CSleepEffectManager::Play()
{
	// 眠りエフェクトアニメーションの開始.
	m_IsAnimation = true;
}

//---------------------------.
// 一時停止.
//---------------------------.
void CSleepEffectManager::Pause( const bool Flag )
{
	m_IsPause = Flag;
}

//---------------------------.
// アニメーションの停止.
//---------------------------.
void CSleepEffectManager::Stop()
{
	// 眠りエフェクトアニメーションの停止.
	m_IsAnimation = false;

	// びっくりエフェクトの初期化.
	if ( m_IsPause ) return;
	m_EffectAnimAng								= Math::ToRadian( 45.0f );
	m_SurprisedEffectState.Color.w				= Color::ALPHA_MAX;
	m_SurprisedEffectState.IsDisp				= true;
	m_SurprisedEffectState.Transform.Position	= m_Transform.Position;
	m_SurprisedEffectState.Transform.Position.y -= 128.0f;
}

//---------------------------.
// びっくりエフェクトの再生.
//---------------------------.
void CSleepEffectManager::SurprisedEffect()
{
	if ( m_SurprisedEffectState.IsDisp == false ) return;

	// アニメーション用の角度の加算.
	if ( m_EffectAnimAng < Math::RADIAN_MID ) {
		m_EffectAnimAng += Math::RADIAN_MAX * m_DeltaTime;
		if ( m_EffectAnimAng >= Math::RADIAN_MID ) {
			m_EffectAnimAng = Math::RADIAN_MID;
		}

		// 拡縮の設定.
		m_SurprisedEffectState.Transform.Scale.y = 1.0f - sinf( m_EffectAnimAng );
	}

	// アルファ値の設定.
	m_SurprisedEffectState.Color.w -= Color::ALPHA_MAX * m_DeltaTime;

	// アニメーションが終了したか.
	if ( m_SurprisedEffectState.Color.w < Color::ALPHA_MIN ) {
		m_SurprisedEffectState.IsDisp = false;
	}
}

//---------------------------.
// 眠りエフェクトの再生.
//---------------------------.
void CSleepEffectManager::SleepEffectPlay()
{
	if ( m_IsPause				) return;
	if ( m_IsAnimation == false ) return;

	// クールタイムを減らす.
	m_CoolTime -= m_DeltaTime;
	if ( m_CoolTime > 0.0f ) return;

	// クールタイムの設定.
	m_CoolTime = COOL_TIME;

	// 使用していないのがあれば使用する.
	for ( auto& s : m_pSleepEffectList ) {
		if ( s->GetIsAnimPlay() ) continue;

		// アニメーションを開始させる.
		s->Play( m_Transform.Position );
		return;
	}

	// 新しく作成.
	m_pSleepEffectList.emplace_back( std::make_unique<CSleepEffect>() );

	// アニメーションを開始させる.
	m_pSleepEffectList.back()->Init();
	m_pSleepEffectList.back()->Play( m_Transform.Position );
}

//---------------------------.
// 眠りエフェクトリストの削除.
//---------------------------.
void CSleepEffectManager::SleepEffectListDelete()
{
	for ( int i = static_cast<int>( m_pSleepEffectList.size() ) - 1; i >= 0; i-- ) {
		if ( m_pSleepEffectList[i]->GetIsAnimPlay() ) break;
		m_pSleepEffectList.pop_back();
	}
}
