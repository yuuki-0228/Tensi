#include "WaterManager.h"
#include "Water/Water.h"
#include "..\..\..\..\..\..\Utility\Random\Random.h"

CWaterManager::CWaterManager()
	: m_pWaterList	()
{
}

CWaterManager::~CWaterManager()
{
}

//---------------------------.
// 初期化.
//---------------------------.
bool CWaterManager::Init()
{
	return true;
}

//---------------------------.
// 更新.
//---------------------------.
void CWaterManager::Update( const float& DeltaTime )
{
	WaterListDelete();	// 水リストの削除.

	for ( auto& w : m_pWaterList ) w->Update( DeltaTime );
}

//---------------------------.
// 描画.
//---------------------------.
void CWaterManager::Render()
{
	for ( auto& w : m_pWaterList ) w->Render();
}

//---------------------------.
// 水を表示する
//---------------------------.
void CWaterManager::Spill( const SWaterData& data, const HWND hWnd )
{
	// ベクトルをランダムで回転させる
	auto moveVec = data.MoveVec;
	auto rot = D3DXROTATION3( 0.0f, 0.0f, Random::GetRand( data.VecRotMin, data.VecRotMax ) );
	Math::VectorRotation( &moveVec, rot );

	// 速度をランダムで決める
	moveVec *= Random::GetRand( data.PowerMin, data.PowerMax );

	// 使用していないのがあれば使用する.
	for ( auto& s : m_pWaterList ) {
		if ( s->GetIsDisp() ) continue;

		s->Init();
		s->SetWater( data.Pos, moveVec, hWnd );
		return;
	}

	// 新しく作る
	m_pWaterList.emplace_back( std::make_unique<CWater>() );
	m_pWaterList.back()->Init();
	m_pWaterList.back()->SetWater( data.Pos, moveVec, hWnd );
}

//---------------------------.
// 水リストの削除.
//---------------------------.
void CWaterManager::WaterListDelete()
{
	for ( int i = static_cast<int>( m_pWaterList.size() ) - 1; i >= 0; i-- ) {
		if ( m_pWaterList[i]->GetIsDisp() ) break;
		m_pWaterList.pop_back();
	}
}
