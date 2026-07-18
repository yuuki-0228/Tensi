#include "IconSmokeEffectManager.h"
#include "IconSmokeEffect\IconSmokeEffect.h"
#include "..\..\..\..\..\Utility\Const\Const.h"
#include "..\..\..\..\..\Utility\Random\Random.h"

CIconSmokeEffectManager::CIconSmokeEffectManager()
	: m_pSmokeList		()
	, m_Position		( INIT_FLOAT3 )
	, m_SpawnCoolTime	( INIT_FLOAT )
	, m_Size			( 1.0f )
	, m_Spread			( INIT_FLOAT )
	, m_IsPlay			( false )
{
}

CIconSmokeEffectManager::~CIconSmokeEffectManager()
{
}

//---------------------------.
// 初期化.
//---------------------------.
bool CIconSmokeEffectManager::Init()
{
	return true;
}

//---------------------------.
// 更新.
//---------------------------.
void CIconSmokeEffectManager::Update( const float& DeltaTime )
{
	// 発生中なら一定間隔で煙を出す.
	if ( m_IsPlay ) {
		m_SpawnCoolTime -= DeltaTime;
		if ( m_SpawnCoolTime <= 0.0f ) {
			SpawnSmoke();
			m_SpawnCoolTime = Const::Explore().SMOKE_SPAWN_INTERVAL;
		}
	}

	// 煙の更新.
	for ( auto& s : m_pSmokeList ) s->Update( DeltaTime );
}

//---------------------------.
// サブウィンドウ( アイコンの後ろ )に描画.
//---------------------------.
void CIconSmokeEffectManager::SubRender()
{
	for ( auto& s : m_pSmokeList ) s->SubRender();
}

//---------------------------.
// 煙の発生を開始する.
//---------------------------.
void CIconSmokeEffectManager::Play()
{
	m_IsPlay		= true;
	m_SpawnCoolTime = INIT_FLOAT;	// すぐに1つ目を出す.
}

//---------------------------.
// 煙の発生を停止する.
//---------------------------.
void CIconSmokeEffectManager::Stop()
{
	m_IsPlay = false;
}

//---------------------------.
// 煙を1つ発生させる.
//---------------------------.
void CIconSmokeEffectManager::SpawnSmoke()
{
	// 発生位置を上下左右にランダムでずらす.
	D3DXPOSITION3 Pos = m_Position;
	Pos.x += Random::GetRand( -m_Spread, m_Spread );
	Pos.y += Random::GetRand( -m_Spread, m_Spread );

	// 使用していないものがあれば再利用する.
	for ( auto& s : m_pSmokeList ) {
		if ( s->GetIsAnimPlay() ) continue;
		s->Play( Pos, m_Size );
		return;
	}

	// 新しく作る.
	m_pSmokeList.emplace_back( std::make_unique<CIconSmokeEffect>() );
	m_pSmokeList.back()->Init();
	m_pSmokeList.back()->Play( Pos, m_Size );
}
