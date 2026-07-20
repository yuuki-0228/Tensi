#include "FloatingAnimation.h"
#ifdef ENABLE_ANIMATION
#include <cmath>

namespace {
	// 円周率.
	const float PI = 3.14159265f;
}

CFloatingAnimation::CFloatingAnimation()
	: m_Offset		( 0.0f, 0.0f )
	, m_Amplitude	( 0.0f )
	, m_Period		( 1.0f )
	, m_Elapsed		( 0.0f )
	, m_IsFloating	( false )
{
}

//---------------------------.
// 浮遊を開始する.
//---------------------------.
void CFloatingAnimation::Start( const float Amplitude, const float Period, const float Phase )
{
	m_Amplitude		= Amplitude;
	m_Period		= max( Period, 0.0001f );
	m_Elapsed		= Phase;
	m_IsFloating	= true;
}

//---------------------------.
// 更新.
//---------------------------.
void CFloatingAnimation::Update( const float DeltaTime )
{
	if ( !m_IsFloating ) return;

	m_Elapsed += DeltaTime;

	// サイン波で上下のオフセットを算出する.
	m_Offset.y = std::sin( m_Elapsed / m_Period * 2.0f * PI ) * m_Amplitude;
}

//---------------------------.
// 浮遊を止めてオフセットを0へ戻す.
//---------------------------.
void CFloatingAnimation::Stop()
{
	m_IsFloating	= false;
	m_Offset		= D3DXVECTOR2( 0.0f, 0.0f );
	m_Elapsed		= 0.0f;
}

#endif
