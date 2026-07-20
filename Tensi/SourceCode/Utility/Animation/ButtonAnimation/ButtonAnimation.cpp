#include "ButtonAnimation.h"
#ifdef ENABLE_ANIMATION

namespace {
	// ボタン向けのバネの硬さ( 素早く反応させる ).
	const float BUTTON_STIFFNESS	= 520.0f;
	// ボタン向けの減衰率( 少しだけ弾ませる ).
	const float BUTTON_DAMPING		= 0.5f;
}

CButtonAnimation::CButtonAnimation()
	: m_Spring		()
	, m_PressScale	( 0.9f )
	, m_HoverScale	( 1.05f )
	, m_IsPressed	( false )
	, m_IsHover		( false )
{
	m_Spring.Init( BUTTON_STIFFNESS, BUTTON_DAMPING );
}

//---------------------------.
// 初期化.
//---------------------------.
void CButtonAnimation::Init( const float PressScale, const float HoverScale )
{
	m_PressScale	= PressScale;
	m_HoverScale	= HoverScale;
	Reset();
}

//---------------------------.
// 更新.
//---------------------------.
void CButtonAnimation::Update( const float DeltaTime )
{
	m_Spring.Update( DeltaTime );
}

//---------------------------.
// 押した瞬間に呼ぶ.
//---------------------------.
void CButtonAnimation::Press()
{
	m_IsPressed = true;
	UpdateTarget();
}

//---------------------------.
// 離した瞬間に呼ぶ.
//---------------------------.
void CButtonAnimation::Release()
{
	if ( !m_IsPressed ) return;

	m_IsPressed = false;
	UpdateTarget();
}

//---------------------------.
// ホバー状態を設定する.
//---------------------------.
void CButtonAnimation::SetHover( const bool IsHover )
{
	if ( m_IsHover == IsHover ) return;

	m_IsHover = IsHover;
	UpdateTarget();
}

//---------------------------.
// 即座に等倍へ戻す.
//---------------------------.
void CButtonAnimation::Reset()
{
	m_IsPressed	= false;
	m_IsHover	= false;
	m_Spring.Init( BUTTON_STIFFNESS, BUTTON_DAMPING );
}

//---------------------------.
// 現在の状態に応じた追従先を設定する.
//---------------------------.
void CButtonAnimation::UpdateTarget()
{
	if ( m_IsPressed )		m_Spring.SetTarget( m_PressScale, m_PressScale );
	else if ( m_IsHover )	m_Spring.SetTarget( m_HoverScale, m_HoverScale );
	else					m_Spring.SetTarget( 1.0f, 1.0f );
}

#endif
