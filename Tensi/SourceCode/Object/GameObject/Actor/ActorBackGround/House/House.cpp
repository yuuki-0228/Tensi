#include "House.h"
#include "..\..\..\..\..\Utility\WindowManager\WindowManager.h"
#include "..\..\..\..\..\Resource\SpriteResource\SpriteResource.h"
#include "..\..\..\..\..\Utility\Random\Random.h"
#include "..\..\..\..\..\Utility\Input\Input.h"

CHouse::CHouse()
	: m_pTent		( nullptr )
	, m_TentState	()
{
}

CHouse::~CHouse()
{
}

//---------------------------.
// 初期化.
//---------------------------.
bool CHouse::Init()
{
	// 画像の取得.
	m_pTent = SpriteResource::GetSprite( "Tent", &m_TentState );
	return true;
}

//---------------------------.
// 更新.
//---------------------------.
void CHouse::Update( const float& DeltaTime )
{
	// マウスの位置の取得.
	const D3DXPOSITION2& MousePos = Input::GetMousePosition();

	// 左上の座標を取得.
	const D3DXPOSITION3& Pos = m_pTent->GetSpriteUpperLeftPos( &m_TentState );

	// サイズの取得.
	const SSize& Size = m_pTent->GetSpriteState().Disp;

	// マウスが画像の上にあるか.
	if ( Pos.x <= MousePos.x && MousePos.x <= Pos.x + Size.w &&
		 Pos.y <= MousePos.y && MousePos.y <= Pos.y + Size.h )
	{
		// ダブルクリックされたか.
		if ( Input::GetIsLeftDoubleClick() == false ) return;

		// エクスプローラーを開く.
		std::filesystem::path result = std::filesystem::absolute( "Data\\スライムのお家" );
		std::string command = "explorer.exe " + result.string();
		system( command.c_str() );

		// ダブルクリックを終了.
		Input::SetIsLeftDoubleClick( false );
	}
}

//---------------------------.
// 描画.
//---------------------------.
void CHouse::SubRender()
{
	m_pTent->RenderUI( &m_TentState );
}

//---------------------------.
// 設置する.
//---------------------------.
void CHouse::Setting( const D3DXPOSITION3& Pos )
{
	if ( m_IsDisp ) return;

	m_IsDisp = true;

	// タスクバーのサイズを取得.
	const RECT& Rect = WindowManager::GetTaskBarRect();

	// テントの位置の設定.
	m_TentState.Transform.Position		= Pos;
	m_TentState.Transform.Position.y	= static_cast<float>( Rect.top );
}
