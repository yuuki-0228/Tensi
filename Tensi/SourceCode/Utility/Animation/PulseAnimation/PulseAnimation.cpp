#include "PulseAnimation.h"
#ifdef ENABLE_ANIMATION
#include <cmath>

namespace {
	// 円周率.
	const float PI = 3.14159265f;
}

CPulseAnimation::CPulseAnimation()
	: m_Scale		( 1.0f )
	, m_MinScale	( 1.0f )
	, m_MaxScale	( 1.08f )
	, m_Period		( 1.2f )
	, m_Elapsed		( 0.0f )
	, m_IsPulsing	( false )
{
}

//---------------------------.
// 鼓動を開始する.
//---------------------------.
void CPulseAnimation::Start( const float MinScale, const float MaxScale, const float Period )
{
	m_MinScale	= MinScale;
	m_MaxScale	= MaxScale;
	m_Period	= ( Period > 0.0001f ) ? Period : 0.0001f;
	m_Elapsed	= 0.0f;
	m_Scale		= MinScale;
	m_IsPulsing	= true;
}

//---------------------------.
// 更新.
//---------------------------.
void CPulseAnimation::Update( const float DeltaTime )
{
	if ( !m_IsPulsing ) return;

	m_Elapsed += DeltaTime;

	// コサイン波で最小値から始まるなめらかな鼓動を作る.
	const float Wave = 0.5f - std::cos( m_Elapsed / m_Period * 2.0f * PI ) * 0.5f;
	m_Scale = m_MinScale + ( m_MaxScale - m_MinScale ) * Wave;
}

//---------------------------.
// 鼓動を止めて拡縮値を最小値へ戻す.
//---------------------------.
void CPulseAnimation::Stop()
{
	m_IsPulsing	= false;
	m_Scale		= m_MinScale;
	m_Elapsed	= 0.0f;
}

#endif
