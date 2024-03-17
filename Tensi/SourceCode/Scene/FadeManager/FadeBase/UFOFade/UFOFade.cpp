#include "UFOFade.h"
#include "..\..\..\..\Resource\SpriteResource\SpriteResource.h"

namespace{
	constexpr float		END_FADEOUT_SCALE		= 2.0f;		// フェードアウトの最終サイズ.
	const D3DXSCALE3	END_FADEOUT_SCALE_VEC3	= { END_FADEOUT_SCALE,	END_FADEOUT_SCALE,	END_FADEOUT_SCALE	};	// フェードアウトの最終サイズ.
}

CUFOFade::CUFOFade()
	: m_pSpriteUFO	( nullptr )
	, m_UFOState	()
{
}

CUFOFade::~CUFOFade()
{
}

//---------------------------.
// 初期化.
//---------------------------.
bool CUFOFade::Init()
{
	// 画像の設定.
	m_pFadeSprite	= SpriteResource::GetSprite( "Fade",	&m_FadeState );
	m_pSpriteUFO	= SpriteResource::GetSprite( "UFO",	&m_UFOState );

	m_UFOState.Transform.Scale			= END_FADEOUT_SCALE_VEC3;
	m_UFOState.Transform.Position.x		= static_cast<float>( WND_W / 2 );
	m_UFOState.Transform.Position.y		= static_cast<float>( WND_H / 2 );
	return true;
}

//---------------------------.
// 描画.
//	UFOのサイズが最大サイズの場合処理は行わない.
//---------------------------.
void CUFOFade::Render()
{
	if ( m_UFOState.Transform.Scale == END_FADEOUT_SCALE_VEC3 ) return;

	m_pSpriteUFO->SetAlphaBlock( false );
	m_pSpriteUFO->RenderUI( &m_UFOState );
	m_pSpriteUFO->SetAlphaBlock( true );
	m_pFadeSprite->RenderUI( &m_FadeState );
}

//---------------------------.
// フェードの設定.
//---------------------------.
void CUFOFade::FadeSetting()
{
	m_UFOState.Transform.Scale			= END_FADEOUT_SCALE_VEC3;
	m_UFOState.Transform.Position.x		= static_cast<float>( WND_W / 2 );
	m_UFOState.Transform.Position.y		= static_cast<float>( WND_H / 2 );
}

//---------------------------.
// フェードイン.
//	フェードアウト中は処理は行わない.
//---------------------------.
bool CUFOFade::FadeIn( const float Speed )
{
	if ( m_NowFade == EFadeState::FadeOut ) return false;
	D3DXSCALE3* Scale = &m_UFOState.Transform.Scale;

	// フェードイン中ではない場合.
	if ( m_NowFade == EFadeState::None ){
		// フェードインの準備.
		m_NowFade	= EFadeState::FadeIn;
		*Scale		= END_FADEOUT_SCALE_VEC3;
	}

	// 縮小していく.
	*Scale -= D3DXSCALE3( Speed, Speed, Speed );
	if ( Scale->x > 0.0f ) return false;

	// フェード終了.
	*Scale		= { 0.0f, 0.0f, 0.0f };
	m_NowFade	= EFadeState::None;
	return true;
}

//---------------------------.
// フェードアウト.
//	フェードイン中は処理は行わない.
//---------------------------.
bool CUFOFade::FadeOut( const float Speed )
{
	if ( m_NowFade == EFadeState::FadeIn ) return false;
	D3DXSCALE3* Scale = &m_UFOState.Transform.Scale;

	// フェードアウト中ではない場合.
	if ( m_NowFade == EFadeState::None ){
		// フェードアウトの準備.
		m_NowFade	= EFadeState::FadeOut;
		*Scale		= { 0.0f, 0.0f, 0.0f };
	}

	// 拡大していく.
	*Scale += D3DXSCALE3( Speed, Speed, Speed );
	if ( Scale->x < END_FADEOUT_SCALE ) return false;

	// フェード終了.
	*Scale		= END_FADEOUT_SCALE_VEC3;
	m_NowFade	= EFadeState::None;
	return true;
}
