#include "WateringEffectManager.h"
#include "WateringEffect/WateringEffect.h"

CWateringEffectManager::CWateringEffectManager()
	: m_pWateringList	()
{
}

CWateringEffectManager::~CWateringEffectManager()
{
}

//---------------------------.
// 初期化.
//---------------------------.
bool CWateringEffectManager::Init()
{
	return false;
}

//---------------------------.
// 更新.
//---------------------------.
void CWateringEffectManager::Update( const float& DeltaTime )
{
	for ( auto& w : m_pWateringList ) w->Update( DeltaTime );
}

//---------------------------.
// 描画.
//---------------------------.
void CWateringEffectManager::Render()
{
	for ( auto& w : m_pWateringList ) w->Render();
}

//---------------------------.
// 再生.
//---------------------------.
void CWateringEffectManager::Play( const D3DXPOSITION3& Pos )
{
	// 使用していないのがあれば使用する.
	for ( auto& w : m_pWateringList ) {
		if ( w->GetIsAnimPlay() ) continue;

		w->Init();
		w->Play( Pos );
		return;
	}

	// 新しく作る
	m_pWateringList.emplace_back( std::make_unique<CWateringEffect>() );
	m_pWateringList.back()->Init();
	m_pWateringList.back()->Play( Pos );
}

//---------------------------.
// 水リストの削除.
//---------------------------.
void CWateringEffectManager::WateringListDelete()
{
	for ( int i = static_cast<int>( m_pWateringList.size() ) - 1; i >= 0; i-- ) {
		if ( m_pWateringList[i]->GetIsAnimPlay() ) break;
		m_pWateringList.pop_back();
	}
}
