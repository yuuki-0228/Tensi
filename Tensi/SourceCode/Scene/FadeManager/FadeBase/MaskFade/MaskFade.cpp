#include "MaskFade.h"
#include "..\..\..\..\Resource\SpriteResource\SpriteResource.h"

CMaskFade::CMaskFade()
{
}

CMaskFade::~CMaskFade()
{
}

//---------------------------.
// ������.
//---------------------------.
bool CMaskFade::Init()
{
	// �摜�̐ݒ�.
	m_pFadeSprite	= SpriteResource::GetSprite( "Fade", &m_FadeState );
	m_pMaskTexture	= SpriteResource::GetSprite( "FadeMask" )->GetTexture();
	m_pFadeSprite->SetRuleTexture( m_pMaskTexture );

	// �����ݒ�.
	m_FadeState.Color.w = Color::ALPHA_MIN;
	return true;
}

//---------------------------.
// �`��.
//	UFO�̃T�C�Y���ő�T�C�Y�̏ꍇ�����͍s��Ȃ�.
//---------------------------.
void CMaskFade::Render()
{
	if ( m_FadeState.Color.w == Color::ALPHA_MIN ) return;

	m_pFadeSprite->RenderUI( &m_FadeState );
}

//---------------------------.
// �t�F�[�h�̐ݒ�.
//---------------------------.
void CMaskFade::FadeSetting()
{
	m_pFadeSprite->SetRuleTexture( m_pMaskTexture );
}

//---------------------------.
// �t�F�[�h�C��.
//	�t�F�[�h�A�E�g���͏����͍s��Ȃ�.
//---------------------------.
bool CMaskFade::FadeIn( const float Speed )
{
	if ( m_NowFade == EFadeState::FadeOut ) return false;
	float* Alpha = &m_FadeState.Color.w;

	// �t�F�[�h�C�����ł͂Ȃ��ꍇ.
	if ( m_NowFade == EFadeState::None ){
		// �t�F�[�h�C���̏���.
		m_NowFade	= EFadeState::FadeIn;
		*Alpha		= Color::ALPHA_MIN;
	}

	// �k�����Ă���.
	*Alpha += Speed;
	if ( *Alpha < Color::ALPHA_MAX ) return false;

	// �t�F�[�h�I��.
	*Alpha		= Color::ALPHA_MAX;
	m_NowFade	= EFadeState::None;
	return true;
}

//---------------------------.
// �t�F�[�h�A�E�g.
//	�t�F�[�h�C�����͏����͍s��Ȃ�.
//---------------------------.
bool CMaskFade::FadeOut( const float Speed )
{
	if ( m_NowFade == EFadeState::FadeIn ) return false;
	float* Alpha = &m_FadeState.Color.w;

	// �t�F�[�h�A�E�g���ł͂Ȃ��ꍇ.
	if ( m_NowFade == EFadeState::None ){
		// �t�F�[�h�A�E�g�̏���.
		m_NowFade	= EFadeState::FadeOut;
		*Alpha		= Color::ALPHA_MAX;
	}

	// �g�債�Ă���.
	*Alpha -= Speed;
	if ( *Alpha > Color::ALPHA_MIN ) return false;

	// �t�F�[�h�I��.
	*Alpha		= Color::ALPHA_MIN;
	m_NowFade	= EFadeState::None;
	return true;
}
