#include "NormalFade.h"
#include "..\..\..\..\Resource\SpriteResource\SpriteResource.h"

CNormalFade::CNormalFade()
{
}

CNormalFade::~CNormalFade()
{
}

//---------------------------.
// ������.
//---------------------------.
bool CNormalFade::Init()
{
	// �摜�̐ݒ�.
	m_pFadeSprite		= SpriteResource::GetSprite( "Fade", &m_FadeState );
	m_pFadeSprite->SetRuleTexture( nullptr );

	// �����ݒ�.
	m_FadeState.Color.w = Color::ALPHA_MIN;
	return true;
}

//---------------------------.
// �`��.
//	UFO�̃T�C�Y���ő�T�C�Y�̏ꍇ�����͍s��Ȃ�.
//---------------------------.
void CNormalFade::Render()
{
	if ( m_FadeState.Color.w == Color::ALPHA_MIN ) return;

	m_pFadeSprite->RenderUI( &m_FadeState );
}

//---------------------------.
// �t�F�[�h�̐ݒ�.
//---------------------------.
void CNormalFade::FadeSetting()
{
	m_pFadeSprite->SetRuleTexture( nullptr );
}

//---------------------------.
// �t�F�[�h�C��.
//	�t�F�[�h�A�E�g���͏����͍s��Ȃ�.
//---------------------------.
bool CNormalFade::FadeIn( const float Speed )
{
	if ( m_NowFade == EFadeState::FadeOut ) return false;
	float* Alpha = &m_FadeState.Color.w;

	// �t�F�[�h�C�����ł͂Ȃ��ꍇ.
	if ( m_NowFade == EFadeState::None ){
		// �t�F�[�h�C���̏���.
		m_NowFade	= EFadeState::FadeIn;
		*Alpha		= Color::ALPHA_MIN;
	}

	// �s�����ɂ��Ă���.
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
bool CNormalFade::FadeOut( const float Speed )
{
	if ( m_NowFade == EFadeState::FadeIn ) return false;
	float* Alpha = &m_FadeState.Color.w;

	// �t�F�[�h�A�E�g���ł͂Ȃ��ꍇ.
	if ( m_NowFade == EFadeState::None ){
		// �t�F�[�h�A�E�g�̏���.
		m_NowFade	= EFadeState::FadeOut;
		*Alpha		= Color::ALPHA_MAX;
	}

	// �����ɂ��Ă���.
	*Alpha -= Speed;
	if ( *Alpha > Color::ALPHA_MIN ) return false;

	// �t�F�[�h�I��.
	*Alpha		= Color::ALPHA_MIN;
	m_NowFade	= EFadeState::None;
	return true;
}
