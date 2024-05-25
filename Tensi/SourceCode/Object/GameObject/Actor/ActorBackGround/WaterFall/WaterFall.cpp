#include "WaterFall.h"
#include "WaterFallTile/WaterFallTile.h"
#include "..\..\..\..\..\Resource\SpriteResource\SpriteResource.h"
#include "..\..\..\..\..\Utility\WindowManager\WindowManager.h"
#include "..\..\..\..\..\Utility\Random\Random.h"
#include "..\..\..\..\..\Utility\SaveDataManager\SaveDataManager.h"
#include "..\..\..\..\..\Utility\Const\Const.h"

CWaterFall::CWaterFall()
	: m_WaterFallTileList	()
	, m_ImageSize			( INIT_FLOAT )
{
}

CWaterFall::~CWaterFall()
{
}

//---------------------------.
// 初期化.
//---------------------------.
bool CWaterFall::Init()
{
	// 画像の取得.
	const auto* pWaterFall = SpriteResource::GetSprite( "WaterFall" );
	m_ImageSize	= pWaterFall->GetSpriteState().Disp.w;

	SaveDataManager::SetSaveData()->WaterFallTransform = &m_Transform;
	SaveDataManager::PushLoadFunction( [this]() { CreateWaterFall(); } );
	return true;
}

//---------------------------.
// 更新.
//---------------------------.
void CWaterFall::Update( const float& DeltaTime )
{
	for ( auto& w : m_WaterFallTileList ) w->Update( DeltaTime );
}

//---------------------------.
// 描画.
//---------------------------.
void CWaterFall::SubRender()
{
	for ( auto& w : m_WaterFallTileList ) w->SubRender();
}

//---------------------------.
// 滝の設定.
//---------------------------.
void CWaterFall::Setting()
{
	// 表示する位置の設定.
	const RECT& Size		= WindowManager::GetMyWndSize();
	const float Wnd_W		= static_cast< float >( Size.right - Size.left );
	m_Transform.Position.x  = Random::GetRand( 0.0f , Wnd_W - m_ImageSize * Const::WaterFall.WATER_FALL_W );
	m_Transform.Position.x += m_ImageSize / 2.0f;

	// タスクバーのサイズを取得.
	const RECT& Rect = WindowManager::GetTaskBarRect();

	// 設置場所の設定
	m_Transform.Position.y = static_cast<float>( Rect.top ) + 3.0f;

	// 滝を作る.
	CreateWaterFall();
}

//---------------------------.
// 滝を作る.
//---------------------------.
void CWaterFall::CreateWaterFall()
{
	// 滝のタイルの設定.
	D3DXPOSITION3	SetPos  = m_Transform.Position;
	const float		startX	= SetPos.x;
	bool			isFirst = true;
	while ( SetPos.y > 0.0f )
	{
		for ( int i = 0; i < Const::WaterFall.WATER_FALL_W; ++i ) {
			m_WaterFallTileList.emplace_back( std::make_unique<CWaterFallTile>() );
			m_WaterFallTileList.back()->Init();
			m_WaterFallTileList.back()->Setting( SetPos, isFirst );

			// 配置するx座標ずらす
			SetPos.x += m_ImageSize;
		}

		// 一番最初の列を表示し終わったか.
		if ( isFirst ) isFirst = false;

		// 配置するx座標を元に戻す
		SetPos.x = startX;

		// 配置するy座標を上にあげる
		SetPos.y -= m_ImageSize;
	}
}
