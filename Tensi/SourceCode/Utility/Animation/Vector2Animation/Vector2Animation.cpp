#include "Vector2Animation.h"
#ifdef ENABLE_ANIMATION

CVector2Animation::CVector2Animation()
	: m_X		()
	, m_Y		()
	, m_Value	( 0.0f, 0.0f )
{
}

//---------------------------.
// 再生を開始する.
//---------------------------.
void CVector2Animation::Play( const D3DXVECTOR2& Start, const D3DXVECTOR2& End, const float Duration,
							  const EasingFunc Func, const ELoop Loop )
{
	m_X.Play( Start.x, End.x, Duration, Func, Loop );
	m_Y.Play( Start.y, End.y, Duration, Func, Loop );
	m_Value = Start;
}

//---------------------------.
// 更新.
//---------------------------.
void CVector2Animation::Update( const float DeltaTime )
{
	m_X.Update( DeltaTime );
	m_Y.Update( DeltaTime );
	m_Value.x = m_X.GetValue();
	m_Value.y = m_Y.GetValue();
}

//---------------------------.
// 再生を止める.
//---------------------------.
void CVector2Animation::Stop()
{
	m_X.Stop();
	m_Y.Stop();
}

#endif
