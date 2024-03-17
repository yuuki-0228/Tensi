#include "MaskFade.h"
#include "..\..\..\..\Resource\SpriteResource\SpriteResource.h"

CMaskFade::CMaskFade()
{
}

CMaskFade::~CMaskFade()
{
}

//---------------------------.
// 初期化.
//---------------------------.
bool CMaskFade::Init()
{
	// 画像の設定.
	m_pFadeSprite	= SpriteResource::GetSprite( "Fade", &m_FadeState );
	m_pMaskTexture	= SpriteResource::GetSprite( "FadeMask" )->GetTexture();
	m_pFadeSprite->SetRuleTexture( m_pMaskTexture );

	// 初期設定.
	m_FadeState.Color.w = Color::ALPHA_MIN;
	return true;
}

//---------------------------.
// 描画.
//	UFOのサイズが最大サイズの場合処理は行わない.
//---------------------------.
void CMaskFade::Render()
{
	if ( m_FadeState.Color.w == Color::ALPHA_MIN ) return;

	m_pFadeSprite->RenderUI( &m_FadeState );
}

//---------------------------.
// フェードの設定.
//---------------------------.
void CMaskFade::FadeSetting()
{
	m_pFadeSprite->SetRuleTexture( m_pMaskTexture );
}

//---------------------------.
// フェードイン.
//	フェードアウト中は処理は行わない.
//---------------------------.
bool CMaskFade::FadeIn( const float Speed )
{
	if ( m_NowFade == EFadeState::FadeOut ) return false;
	float* Alpha = &m_FadeState.Color.w;

	// フェードイン中ではない場合.
	if ( m_NowFade == EFadeState::None ){
		// フェードインの準備.
		m_NowFade	= EFadeState::FadeIn;
		*Alpha		= Color::ALPHA_MIN;
	}

	// 縮小していく.
	*Alpha += Speed;
	if ( *Alpha < Color::ALPHA_MAX ) return false;

	// フェード終了.
	*Alpha		= Color::ALPHA_MAX;
	m_NowFade	= EFadeState::None;
	return true;
}

//---------------------------.
// フェードアウト.
//	フェードイン中は処理は行わない.
//---------------------------.
bool CMaskFade::FadeOut( const float Speed )
{
	if ( m_NowFade == EFadeState::FadeIn ) return false;
	float* Alpha = &m_FadeState.Color.w;

	// フェードアウト中ではない場合.
	if ( m_NowFade == EFadeState::None ){
		// フェードアウトの準備.
		m_NowFade	= EFadeState::FadeOut;
		*Alpha		= Color::ALPHA_MAX;
	}

	// 拡大していく.
	*Alpha -= Speed;
	if ( *Alpha > Color::ALPHA_MIN ) return false;

	// フェード終了.
	*Alpha		= Color::ALPHA_MIN;
	m_NowFade	= EFadeState::None;
	return true;
}
