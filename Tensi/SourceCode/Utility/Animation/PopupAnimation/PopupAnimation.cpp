#include "PopupAnimation.h"
#ifdef ENABLE_ANIMATION

CPopupAnimation::CPopupAnimation()
	: m_State		( EPopupState::Closed )
	, m_Elapsed		( 0.0f )
	, m_Duration	( 0.0f )
	, m_StartScale	( 0.8f )
	, m_Back		( 1.7f )
	, m_Scale		( 0.0f )
	, m_Alpha		( 0.0f )
{
}

//---------------------------.
// 初期化.
//---------------------------.
void CPopupAnimation::Init( const float StartScale, const float Back )
{
	m_StartScale	= StartScale;
	m_Back			= Back;
}

//---------------------------.
// 開く.
//---------------------------.
void CPopupAnimation::Open( const float Duration )
{
	m_State		= EPopupState::Opening;
	m_Elapsed	= 0.0f;
	m_Duration	= Duration;
	m_Scale		= m_StartScale;
	m_Alpha		= 0.0f;

	// 時間指定が無い場合は即座に開き切る.
	if ( m_Duration <= 0.0f ) {
		m_State	= EPopupState::Opened;
		m_Scale	= 1.0f;
		m_Alpha	= 1.0f;
	}
}

//---------------------------.
// 閉じる.
//---------------------------.
void CPopupAnimation::Close( const float Duration )
{
	m_State		= EPopupState::Closing;
	m_Elapsed	= 0.0f;
	m_Duration	= Duration;

	// 時間指定が無い場合は即座に閉じ切る.
	if ( m_Duration <= 0.0f ) {
		m_State	= EPopupState::Closed;
		m_Scale	= m_StartScale;
		m_Alpha	= 0.0f;
	}
}

//---------------------------.
// 更新.
//---------------------------.
void CPopupAnimation::Update( const float DeltaTime )
{
	if ( !IsPlaying() ) return;

	m_Elapsed = min( m_Elapsed + DeltaTime, m_Duration );

	switch ( m_State ) {
	case EPopupState::Opening:
		// 弾みながら拡大しつつフェードインする.
		m_Scale	= Easing::OutBack( m_Elapsed, m_Duration, m_Back, 1.0f, m_StartScale );
		m_Alpha	= Easing::OutQuad( m_Elapsed, m_Duration, 1.0f, 0.0f );
		if ( m_Elapsed >= m_Duration ) m_State = EPopupState::Opened;
		break;

	case EPopupState::Closing:
		// 縮小しながらフェードアウトする.
		m_Scale	= Easing::InQuad( m_Elapsed, m_Duration, m_StartScale, 1.0f );
		m_Alpha	= Easing::InQuad( m_Elapsed, m_Duration, 0.0f, 1.0f );
		if ( m_Elapsed >= m_Duration ) m_State = EPopupState::Closed;
		break;

	default:
		break;
	}
}

#endif
