#include "WateringAnimManager.h"
#include "WateringAnim/WateringAnim.h"

CWateringAnimManager::CWateringAnimManager()
	: m_pWateringList	()
{
}

CWateringAnimManager::~CWateringAnimManager()
{
}

//---------------------------.
// 初期化.
//---------------------------.
bool CWateringAnimManager::Init()
{
	return false;
}

//---------------------------.
// 更新.
//---------------------------.
void CWateringAnimManager::Update( const float& DeltaTime )
{
	for ( auto& w : m_pWateringList ) w->Update( DeltaTime );
}

//---------------------------.
// 描画.
//---------------------------.
void CWateringAnimManager::Render()
{
	for ( auto& w : m_pWateringList ) w->Render();
}

//---------------------------.
// 再生.
//---------------------------.
void CWateringAnimManager::Play( const D3DXPOSITION3& Pos )
{
	// 使用していないのがあれば使用する.
	for ( auto& w : m_pWateringList ) {
		if ( w->GetIsDisp() ) continue;

		w->Init();
		w->Play( Pos );
		return;
	}

	// 新しく作る
	m_pWateringList.emplace_back( std::make_unique<CWateringAnim>() );
	m_pWateringList.back()->Init();
	m_pWateringList.back()->Play( Pos );
}

//---------------------------.
// 水リストの削除.
//---------------------------.
void CWateringAnimManager::WateringListDelete()
{
	for ( int i = static_cast<int>( m_pWateringList.size() ) - 1; i >= 0; i-- ) {
		if ( m_pWateringList[i]->GetIsDisp() ) break;
		m_pWateringList.pop_back();
	}
}
