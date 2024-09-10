#include "WeedManager.h"
#include "Weed/Weed.h"
#include "..\..\..\..\..\..\Resource\SpriteResource\SpriteResource.h"
#include "..\..\..\..\..\..\Utility\WindowManager\WindowManager.h"
#include "..\..\..\..\..\..\Utility\Random\Random.h"
#include "..\..\..\..\..\..\Utility\SaveDataManager\SaveDataManager.h"

CWeedManager::CWeedManager()
	: m_pWeedList	()
	, m_WeedSize	( INIT_FLOAT )
{
}

CWeedManager::~CWeedManager()
{
}

//---------------------------.
// 初期化.
//---------------------------.
bool CWeedManager::Init()
{
	const auto* pWeed = SpriteResource::GetSprite( "Weed" );
	m_WeedSize = pWeed->GetSpriteState().Disp.w;

	SaveDataManager::SetWeedSaveData( [this]() { return Save(); }, [this]( const auto& Data ) { Load( Data ); } );

	return true;
}

//---------------------------.
// 更新.
//---------------------------.
void CWeedManager::Update( const float& DeltaTime )
{
	WeedListDelete();	// 雑草リストの削除.

	for ( auto& w : m_pWeedList ) w->Update( DeltaTime );
}

//---------------------------.
// 描画.
//---------------------------.
void CWeedManager::SubRender()
{
	for ( auto& w : m_pWeedList ) w->SubRender();
}

//---------------------------.
// 雑草を埋める.
//---------------------------.
void CWeedManager::Fill( const int Num )
{
	const RECT& Size  = WindowManager::GetMyWndSize();
	const float Wnd_W = static_cast< float >( Size.right - Size.left );

	for ( int i = 0; i < Num; ++i ) {
		// 埋める場所をランダムで決める.
		D3DXPOSITION3 Pos = INIT_FLOAT3;
		Pos.x = Random::GetRand( 0.0f, Wnd_W );

		// 使用していないのがあれば使用する.
		for ( auto& s : m_pWeedList ) {
			if ( s->GetIsDisp() ) continue;

			s->Init();
			s->Fill( Pos );
			return;
		}

		// 新しく作る
		m_pWeedList.emplace_back( std::make_unique<CWeed>() );
		m_pWeedList.back()->Init();
		m_pWeedList.back()->Fill( Pos );
	}
}

//---------------------------.
// 表示している本数の取得
//---------------------------.
int CWeedManager::GetNum()
{
	return static_cast<int>( std::count_if(
		m_pWeedList.begin(),
		m_pWeedList.end(),
		[]( const auto& w ) { return w->GetIsDisp(); }
	) );
}

//---------------------------.
// 保存.
//---------------------------.
std::vector<SWeedData> CWeedManager::Save()
{
	const int size = static_cast<int>( m_pWeedList.size() );

	// 雑草のセーブデータの取得.
	std::vector<SWeedData> Data;
	Data.resize( size );
	for ( int i = 0; i < size; ++i ) {
		Data[i] = m_pWeedList[i]->GetWeedData();
	}

	return Data;
}

//---------------------------.
// 読み込み.
//---------------------------.
void CWeedManager::Load( std::vector<SWeedData> Data )
{
	const int size = static_cast<int>( Data.size() );

	m_pWeedList.resize( size );
	for ( int i = 0; i < size; ++i ) {
		m_pWeedList[i] = std::make_unique<CWeed>();
		m_pWeedList[i]->Init();
		m_pWeedList[i]->SetWeedData( Data[i] );
	}

	// 配列内の使用していない要素を削除する
	m_pWeedList.erase( 
		std::remove_if(
			std::begin( m_pWeedList ),
			std::end( m_pWeedList ),
			[]( const auto& w ) { return w->GetIsDisp() == false; } ),
		std::cend( m_pWeedList )
	);
}

//---------------------------.
// 雑草リストの削除.
//---------------------------.
void CWeedManager::WeedListDelete()
{
	for ( int i = static_cast<int>( m_pWeedList.size() ) - 1; i >= 0; i-- ) {
		if ( m_pWeedList[i]->GetIsDisp() ) break;
		m_pWeedList.pop_back();
	}
}
