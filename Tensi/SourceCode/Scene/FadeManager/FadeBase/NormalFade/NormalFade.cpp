#include "NormalFade.h"
#include "..\..\..\..\Resource\SpriteResource\SpriteResource.h"

CNormalFade::CNormalFade()
{
}

CNormalFade::~CNormalFade()
{
}

//---------------------------.
// 初期化.
//---------------------------.
bool CNormalFade::Init()
{
	// 画像の設定.
	m_pFadeSprite		= SpriteResource::GetSprite( "Fade", &m_FadeState );
	m_pFadeSprite->SetRuleTexture( nullptr );

	// 初期設定.
	m_FadeState.Color.w = Color::ALPHA_MIN;
	return true;
}

//---------------------------.
// 描画.
//	UFOのサイズが最大サイズの場合処理は行わない.
//---------------------------.
void CNormalFade::Render()
{
	if ( m_FadeState.Color.w == Color::ALPHA_MIN ) return;

	m_pFadeSprite->RenderUI( &m_FadeState );
}

//---------------------------.
// フェードの設定.
//---------------------------.
void CNormalFade::FadeSetting()
{
	m_pFadeSprite->SetRuleTexture( nullptr );
}

//---------------------------.
// フェードイン.
//	フェードアウト中は処理は行わない.
//---------------------------.
bool CNormalFade::FadeIn( const float Speed )
{
	if ( m_NowFade == EFadeState::FadeOut ) return false;
	float* Alpha = &m_FadeState.Color.w;

	// フェードイン中ではない場合.
	if ( m_NowFade == EFadeState::None ){
		// フェードインの準備.
		m_NowFade	= EFadeState::FadeIn;
		*Alpha		= Color::ALPHA_MIN;
	}

	// 不透明にしていく.
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
bool CNormalFade::FadeOut( const float Speed )
{
	if ( m_NowFade == EFadeState::FadeIn ) return false;
	float* Alpha = &m_FadeState.Color.w;

	// フェードアウト中ではない場合.
	if ( m_NowFade == EFadeState::None ){
		// フェードアウトの準備.
		m_NowFade	= EFadeState::FadeOut;
		*Alpha		= Color::ALPHA_MAX;
	}

	// 透明にしていく.
	*Alpha -= Speed;
	if ( *Alpha > Color::ALPHA_MIN ) return false;

	// フェード終了.
	*Alpha		= Color::ALPHA_MIN;
	m_NowFade	= EFadeState::None;
	return true;
}
