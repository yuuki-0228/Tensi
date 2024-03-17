#include "UFOFade.h"
#include "..\..\..\..\Resource\SpriteResource\SpriteResource.h"

namespace{
	constexpr float		END_FADEOUT_SCALE		= 2.0f;		// �t�F�[�h�A�E�g�̍ŏI�T�C�Y.
	const D3DXSCALE3	END_FADEOUT_SCALE_VEC3	= { END_FADEOUT_SCALE,	END_FADEOUT_SCALE,	END_FADEOUT_SCALE	};	// �t�F�[�h�A�E�g�̍ŏI�T�C�Y.
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
// ������.
//---------------------------.
bool CUFOFade::Init()
{
	// �摜�̐ݒ�.
	m_pFadeSprite	= SpriteResource::GetSprite( "Fade",	&m_FadeState );
	m_pSpriteUFO	= SpriteResource::GetSprite( "UFO",	&m_UFOState );

	m_UFOState.Transform.Scale			= END_FADEOUT_SCALE_VEC3;
	m_UFOState.Transform.Position.x		= static_cast<float>( WND_W / 2 );
	m_UFOState.Transform.Position.y		= static_cast<float>( WND_H / 2 );
	return true;
}

//---------------------------.
// �`��.
//	UFO�̃T�C�Y���ő�T�C�Y�̏ꍇ�����͍s��Ȃ�.
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
// �t�F�[�h�̐ݒ�.
//---------------------------.
void CUFOFade::FadeSetting()
{
	m_UFOState.Transform.Scale			= END_FADEOUT_SCALE_VEC3;
	m_UFOState.Transform.Position.x		= static_cast<float>( WND_W / 2 );
	m_UFOState.Transform.Position.y		= static_cast<float>( WND_H / 2 );
}

//---------------------------.
// �t�F�[�h�C��.
//	�t�F�[�h�A�E�g���͏����͍s��Ȃ�.
//---------------------------.
bool CUFOFade::FadeIn( const float Speed )
{
	if ( m_NowFade == EFadeState::FadeOut ) return false;
	D3DXSCALE3* Scale = &m_UFOState.Transform.Scale;

	// �t�F�[�h�C�����ł͂Ȃ��ꍇ.
	if ( m_NowFade == EFadeState::None ){
		// �t�F�[�h�C���̏���.
		m_NowFade	= EFadeState::FadeIn;
		*Scale		= END_FADEOUT_SCALE_VEC3;
	}

	// �k�����Ă���.
	*Scale -= D3DXSCALE3( Speed, Speed, Speed );
	if ( Scale->x > 0.0f ) return false;

	// �t�F�[�h�I��.
	*Scale		= { 0.0f, 0.0f, 0.0f };
	m_NowFade	= EFadeState::None;
	return true;
}

//---------------------------.
// �t�F�[�h�A�E�g.
//	�t�F�[�h�C�����͏����͍s��Ȃ�.
//---------------------------.
bool CUFOFade::FadeOut( const float Speed )
{
	if ( m_NowFade == EFadeState::FadeIn ) return false;
	D3DXSCALE3* Scale = &m_UFOState.Transform.Scale;

	// �t�F�[�h�A�E�g���ł͂Ȃ��ꍇ.
	if ( m_NowFade == EFadeState::None ){
		// �t�F�[�h�A�E�g�̏���.
		m_NowFade	= EFadeState::FadeOut;
		*Scale		= { 0.0f, 0.0f, 0.0f };
	}

	// �g�債�Ă���.
	*Scale += D3DXSCALE3( Speed, Speed, Speed );
	if ( Scale->x < END_FADEOUT_SCALE ) return false;

	// �t�F�[�h�I��.
	*Scale		= END_FADEOUT_SCALE_VEC3;
	m_NowFade	= EFadeState::None;
	return true;
}
