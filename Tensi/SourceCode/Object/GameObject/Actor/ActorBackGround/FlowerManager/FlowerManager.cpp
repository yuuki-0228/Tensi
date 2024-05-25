#include "FlowerManager.h"
#include "Flower/Flower.h"
#include "..\..\..\..\..\Resource\SpriteResource\SpriteResource.h"
#include "..\..\..\..\..\Utility\WindowManager\WindowManager.h"
#include "..\..\..\..\..\Utility\Random\Random.h"
#include "..\..\..\..\..\Utility\SaveDataManager\SaveDataManager.h"

namespace {
	const std::vector<D3DXCOLOR3> COLOR_LIST = {
		Color::Lightcoral,
		Color::Salmon,
		Color::Lightpink,
		Color::Magenta,
		Color::Lightsalmon,
		Color::Orange,
		Color::Yellow,
		Color::Peachpuff,
		Color::Cyan,
		Color::Lavender,
		Color::Thistle,
		Color::Orchid,
		Color::Fuchsia,
		Color::Blueviolet,
	};
}

CFlowerManager::CFlowerManager()
	: m_pFlowerList		()
	, m_FlowerSize		( INIT_FLOAT )
	, m_FlowerAnimCnt	( INIT_FLOAT )
{
}

CFlowerManager::~CFlowerManager()
{
}

//---------------------------.
// 初期化.
//---------------------------.
bool CFlowerManager::Init()
{
	const auto* pFlower = SpriteResource::GetSprite( "Flower" );
	m_FlowerSize = pFlower->GetSpriteState().Disp.w;

	SaveDataManager::SetFlowerSaveData( [this]() { return Save(); }, [this]( const auto& Data ) { Load( Data ); } );
	return true;
}

//---------------------------.
// 更新.
//---------------------------.
void CFlowerManager::Update( const float& DeltaTime )
{
	m_DeltaTime = DeltaTime;

	FlowerAnimCntUpdate();	// 花のアニメーション用カウントの更新.
	FlowerListDelete();		// 花のリストの削除.

	for ( auto& f : m_pFlowerList ) {
		f->SetFlowerAnim( m_FlowerAnimCnt );
		f->Update( DeltaTime );
	}
}

//---------------------------.
// 描画.
//---------------------------.
void CFlowerManager::SubRender()
{
	for ( auto& f : m_pFlowerList ) f->SubRender();
}

//---------------------------.
// 苗木を埋める.
//---------------------------.
void CFlowerManager::Fill( const int Num )
{
	const RECT& Size  = WindowManager::GetMyWndSize();
	const float Wnd_W = static_cast< float >( Size.right - Size.left );

	for ( int i = 0; i < Num; ++i ) {
		// 埋める場所をランダムで決める.
		D3DXPOSITION3 Pos = INIT_FLOAT3;
		Pos.x = Random::GetRand( m_FlowerSize / 2.0f, Wnd_W - m_FlowerSize / 2.0f );

		const int		  Type = Random::GetRand( 0, static_cast<int>( CFlower::EFlowerType::Max ) );
		const D3DXCOLOR3& Color = Random::GetRand( COLOR_LIST );

		// 使用していないのがあれば使用する.
		for ( auto& s : m_pFlowerList ) {
			if ( s->GetIsDisp() ) continue;

			s->Init();
			s->Fill( Type, Color, Pos );
			return;
		}

		// 新しく作る
		m_pFlowerList.emplace_back( std::make_unique<CFlower>() );
		m_pFlowerList.back()->Init();
		m_pFlowerList.back()->Fill( Type, Color, Pos );
	}
}

//---------------------------.
// 表示している本数の取得.
//---------------------------.
int CFlowerManager::GetNum()
{
	return static_cast<int>( std::count_if( 
		m_pFlowerList.begin(),
		m_pFlowerList.end(),
		[]( const auto& f ) { return f->GetIsDisp(); }
	) );
}

//---------------------------.
// 保存.
//---------------------------.
std::vector<SFlowerData> CFlowerManager::Save()
{
	const int size = static_cast<int>( m_pFlowerList.size() );

	// 花のセーブデータの取得.
	std::vector<SFlowerData> Data;
	Data.resize( size );
	for ( int i = 0; i < size; ++i ) {
		Data[i] = m_pFlowerList[i]->GetFlowerData();
	}

	return Data;
}

//---------------------------.
// 読み込み.
//---------------------------.
void CFlowerManager::Load( std::vector<SFlowerData> Data )
{
	const int size = static_cast<int>( Data.size() );

	m_pFlowerList.resize( size );
	for ( int i = 0; i < size; ++i ) {
		m_pFlowerList[i] = std::make_unique<CFlower>();
		m_pFlowerList[i]->Init();
		m_pFlowerList[i]->SetFlowerData( Data[i] );
	}

	// 配列内の使用していない要素を削除する
	m_pFlowerList.erase(
		std::remove_if(
			std::begin( m_pFlowerList ),
			std::end( m_pFlowerList ),
			[]( const auto& f ) { return f->GetIsDisp() == false; } ),
		std::cend( m_pFlowerList )
	);
}

//---------------------------.
// 花リストの削除.
//---------------------------.
void CFlowerManager::FlowerListDelete()
{
	for ( int i = static_cast<int>( m_pFlowerList.size() ) - 1; i >= 0; i-- ) {
		if ( m_pFlowerList[i]->GetIsDisp() ) break;
		m_pFlowerList.pop_back();
	}
}

//---------------------------.
// 花のアニメーション用カウントの更新.
//---------------------------.
void CFlowerManager::FlowerAnimCntUpdate()
{
	m_FlowerAnimCnt += Math::DEGREE_MAX * m_DeltaTime;
	if ( m_FlowerAnimCnt >= Math::DEGREE_MAX ) {
		m_FlowerAnimCnt -= Math::DEGREE_MAX;
	}
}
