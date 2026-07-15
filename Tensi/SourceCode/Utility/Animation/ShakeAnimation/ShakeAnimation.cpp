#include "ShakeAnimation.h"
#ifdef ENABLE_ANIMATION
#include <cstdlib>

namespace {
	// -1.0~1.0 ‚Ì—”‚ğ•Ô‚·.
	float RandRange()
	{
		return ( static_cast<float>( rand() ) / static_cast<float>( RAND_MAX ) ) * 2.0f - 1.0f;
	}
}

CShakeAnimation::CShakeAnimation()
	: m_Offset		( 0.0f, 0.0f )
	, m_Strength	( 0.0f )
	, m_Duration	( 0.0f )
	, m_Elapsed		( 0.0f )
{
}

//---------------------------.
// U“®‚ğŠJn‚·‚é.
//---------------------------.
void CShakeAnimation::Start( const float Strength, const float Duration )
{
	m_Strength	= Strength;
	m_Duration	= Duration;
	m_Elapsed	= 0.0f;
}

//---------------------------.
// XV.
//---------------------------.
void CShakeAnimation::Update( const float DeltaTime )
{
	// Šù‚É—h‚ê‚ªû‚Ü‚Á‚Ä‚¢‚ê‚Î‰½‚à‚µ‚È‚¢.
	if ( m_Elapsed >= m_Duration ) {
		m_Offset = D3DXVECTOR2( 0.0f, 0.0f );
		return;
	}

	m_Elapsed += DeltaTime;

	// c‚èŠÔ‚ÌŠ„‡‚Å—h‚ê•‚ğŒ¸Š‚³‚¹‚é( 2æ‚Å©‘R‚Èû‘©‚É‚·‚é ).
	float Ratio = 1.0f - m_Elapsed / m_Duration;
	if ( Ratio < 0.0f ) Ratio = 0.0f;
	const float Current = m_Strength * Ratio * Ratio;

	// ƒ‰ƒ“ƒ_ƒ€‚ÈŒü‚«‚Ö—h‚ç‚·.
	m_Offset.x = RandRange() * Current;
	m_Offset.y = RandRange() * Current;
}

//---------------------------.
// —h‚ê‚ğ~‚ß‚é.
//---------------------------.
void CShakeAnimation::Stop()
{
	m_Elapsed	= m_Duration;
	m_Offset	= D3DXVECTOR2( 0.0f, 0.0f );
}

#endif
