#include "SlideAnimation.h"
#ifdef ENABLE_ANIMATION

CSlideAnimation::CSlideAnimation()
	: m_Start		( 0.0f, 0.0f )
	, m_End			( 0.0f, 0.0f )
	, m_Position	( 0.0f, 0.0f )
	, m_Duration	( 0.0f )
	, m_Elapsed		( 0.0f )
	, m_Func		( Easing::OutCubic )
	, m_IsPlaying	( false )
	, m_IsFinished	( false )
{
}

//---------------------------.
// スライドインを開始する.
//---------------------------.
void CSlideAnimation::SlideIn( const D3DXVECTOR2& Target, const D3DXVECTOR2& Offset,
							   const float Duration, const EasingFunc Func )
{
	Play( Target + Offset, Target, Duration, Func );
}

//---------------------------.
// スライドアウトを開始する.
//---------------------------.
void CSlideAnimation::SlideOut( const D3DXVECTOR2& Target, const D3DXVECTOR2& Offset,
								const float Duration, const EasingFunc Func )
{
	Play( Target, Target + Offset, Duration, Func );
}

//---------------------------.
// 更新.
//---------------------------.
void CSlideAnimation::Update( const float DeltaTime )
{
	if ( !m_IsPlaying ) return;

	m_Elapsed	 = min( m_Elapsed + DeltaTime, m_Duration );
	m_Position.x = m_Func( m_Elapsed, m_Duration, m_End.x, m_Start.x );
	m_Position.y = m_Func( m_Elapsed, m_Duration, m_End.y, m_Start.y );

	// 終端に達したら完了とする.
	if ( m_Elapsed >= m_Duration ) {
		m_IsPlaying		= false;
		m_IsFinished	= true;
	}
}

//---------------------------.
// 開始位置から終了位置への移動を開始する.
//---------------------------.
void CSlideAnimation::Play( const D3DXVECTOR2& Start, const D3DXVECTOR2& End,
							const float Duration, const EasingFunc Func )
{
	m_Start			= Start;
	m_End			= End;
	m_Position		= Start;
	m_Duration		= Duration;
	m_Elapsed		= 0.0f;
	m_Func			= ( Func != nullptr ) ? Func : Easing::Linear;
	m_IsPlaying		= true;
	m_IsFinished	= false;

	// 時間指定が無い場合は即座に終了位置へ到達させる.
	if ( m_Duration <= 0.0f ) {
		m_Position		= End;
		m_IsPlaying		= false;
		m_IsFinished	= true;
	}
}

#endif
