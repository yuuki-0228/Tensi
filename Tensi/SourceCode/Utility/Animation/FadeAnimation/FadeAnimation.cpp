#include "FadeAnimation.h"
#ifdef ENABLE_ANIMATION

CFadeAnimation::CFadeAnimation()
	: m_Start		( 1.0f )
	, m_End			( 1.0f )
	, m_Alpha		( 1.0f )
	, m_Duration	( 0.0f )
	, m_Elapsed		( 0.0f )
	, m_Func		( Easing::OutQuad )
	, m_IsPlaying	( false )
	, m_IsFinished	( false )
{
}

//---------------------------.
// フェードインを開始する.
//---------------------------.
void CFadeAnimation::FadeIn( const float Duration, const EasingFunc Func )
{
	Play( 1.0f, Duration, Func );
}

//---------------------------.
// フェードアウトを開始する.
//---------------------------.
void CFadeAnimation::FadeOut( const float Duration, const EasingFunc Func )
{
	Play( 0.0f, Duration, Func );
}

//---------------------------.
// アルファ値を即座に設定する.
//---------------------------.
void CFadeAnimation::SetAlpha( const float Alpha )
{
	m_Start			= Alpha;
	m_End			= Alpha;
	m_Alpha			= Alpha;
	m_IsPlaying		= false;
	m_IsFinished	= false;
}

//---------------------------.
// 更新.
//---------------------------.
void CFadeAnimation::Update( const float DeltaTime )
{
	if ( !m_IsPlaying ) return;

	m_Elapsed = min( m_Elapsed + DeltaTime, m_Duration );
	m_Alpha   = m_Func( m_Elapsed, m_Duration, m_End, m_Start );

	// 終端に達したら完了とする.
	if ( m_Elapsed >= m_Duration ) {
		m_IsPlaying		= false;
		m_IsFinished	= true;
	}
}

//---------------------------.
// 指定のアルファ値へのフェードを開始する.
//---------------------------.
void CFadeAnimation::Play( const float End, const float Duration, const EasingFunc Func )
{
	m_Start			= m_Alpha;
	m_End			= End;
	m_Duration		= Duration;
	m_Elapsed		= 0.0f;
	m_Func			= ( Func != nullptr ) ? Func : Easing::Linear;
	m_IsPlaying		= true;
	m_IsFinished	= false;

	// 時間指定が無い場合は即座に終了値へ到達させる.
	if ( m_Duration <= 0.0f ) {
		m_Alpha			= End;
		m_IsPlaying		= false;
		m_IsFinished	= true;
	}
}

#endif
