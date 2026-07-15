#include "ValueAnimation.h"
#ifdef ENABLE_ANIMATION
#include <cmath>

CValueAnimation::CValueAnimation()
	: m_Start		( 0.0f )
	, m_End			( 0.0f )
	, m_Value		( 0.0f )
	, m_Duration	( 0.0f )
	, m_Elapsed		( 0.0f )
	, m_Func		( Easing::OutQuad )
	, m_Loop		( ELoop::Once )
	, m_IsPlaying	( false )
	, m_IsFinished	( false )
{
}

//---------------------------.
// 再生を開始する.
//---------------------------.
void CValueAnimation::Play( const float Start, const float End, const float Duration,
							const EasingFunc Func, const ELoop Loop )
{
	m_Start			= Start;
	m_End			= End;
	m_Value			= Start;
	m_Duration		= Duration;
	m_Elapsed		= 0.0f;
	m_Func			= ( Func != nullptr ) ? Func : Easing::Linear;
	m_Loop			= Loop;
	m_IsPlaying		= true;
	m_IsFinished	= false;

	// 時間指定が無い場合は即座に終了値へ到達させる.
	if ( m_Duration <= 0.0f ) {
		m_Value			= End;
		m_IsPlaying		= false;
		m_IsFinished	= true;
	}
}

//---------------------------.
// 更新.
//---------------------------.
void CValueAnimation::Update( const float DeltaTime )
{
	if ( !m_IsPlaying ) return;

	m_Elapsed += DeltaTime;

	switch ( m_Loop ) {
	case ELoop::Once:
		// 終端に達したら固定して完了とする.
		if ( m_Elapsed >= m_Duration ) {
			m_Elapsed		= m_Duration;
			m_IsPlaying		= false;
			m_IsFinished	= true;
		}
		m_Value = m_Func( m_Elapsed, m_Duration, m_End, m_Start );
		break;

	case ELoop::Loop: {
		// 経過時間を1周期で折り返して先頭へ戻す.
		const float Looped = std::fmod( m_Elapsed, m_Duration );
		m_Value = m_Func( Looped, m_Duration, m_End, m_Start );
		break;
	}

	case ELoop::PingPong: {
		// 2周期を境に往路と復路を切り替えて往復させる.
		const float Phase = std::fmod( m_Elapsed, m_Duration * 2.0f );
		if ( Phase <= m_Duration )	m_Value = m_Func( Phase, m_Duration, m_End, m_Start );
		else						m_Value = m_Func( Phase - m_Duration, m_Duration, m_Start, m_End );
		break;
	}
	}
}

#endif
