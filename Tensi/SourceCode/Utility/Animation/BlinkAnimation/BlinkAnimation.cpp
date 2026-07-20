#include "BlinkAnimation.h"
#ifdef ENABLE_ANIMATION
#include <cmath>

namespace {
	// 円周率.
	const float PI = 3.14159265f;
}

CBlinkAnimation::CBlinkAnimation()
	: m_Alpha		( 1.0f )
	, m_MinAlpha	( 0.2f )
	, m_MaxAlpha	( 1.0f )
	, m_Period		( 1.0f )
	, m_Elapsed		( 0.0f )
	, m_IsBlinking	( false )
{
}

//---------------------------.
// 点滅を開始する.
//---------------------------.
void CBlinkAnimation::Start( const float Period, const float MinAlpha, const float MaxAlpha )
{
	m_Period		= ( Period > 0.0001f ) ? Period : 0.0001f;
	m_MinAlpha		= MinAlpha;
	m_MaxAlpha		= MaxAlpha;
	m_Elapsed		= 0.0f;
	m_Alpha			= MaxAlpha;
	m_IsBlinking	= true;
}

//---------------------------.
// 更新.
//---------------------------.
void CBlinkAnimation::Update( const float DeltaTime )
{
	if ( !m_IsBlinking ) return;

	m_Elapsed += DeltaTime;

	// コサイン波で最大値から始まるなめらかな明滅を作る.
	const float Wave = std::cos( m_Elapsed / m_Period * 2.0f * PI ) * 0.5f + 0.5f;
	m_Alpha = m_MinAlpha + ( m_MaxAlpha - m_MinAlpha ) * Wave;
}

//---------------------------.
// 点滅を止めてアルファ値を最大値へ戻す.
//---------------------------.
void CBlinkAnimation::Stop()
{
	m_IsBlinking	= false;
	m_Alpha			= m_MaxAlpha;
	m_Elapsed		= 0.0f;
}

#endif
