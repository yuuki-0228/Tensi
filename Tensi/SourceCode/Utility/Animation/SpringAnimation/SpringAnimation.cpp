#include "SpringAnimation.h"
#ifdef ENABLE_ANIMATION
#include <cmath>

namespace {
	// 1回のバネ計算で許容する最大の時間( 数値の発散を防ぐために分割する ).
	const float MAX_STEP_TIME		= 1.0f / 120.0f;
	// 拡縮が目標値に十分近いとみなす閾値.
	const float SETTLE_THRESHOLD	= 0.0005f;
}

CSpringAnimation::CSpringAnimation()
	: m_Scale		( 1.0f, 1.0f )
	, m_Velocity	( 0.0f, 0.0f )
	, m_Target		( 1.0f, 1.0f )
	, m_Stiffness	( 240.0f )
	, m_Damping		( 0.32f )
{
}

//---------------------------.
// 初期化.
//---------------------------.
void CSpringAnimation::Init( const float Stiffness, const float Damping )
{
	m_Stiffness = Stiffness;
	m_Damping	= Damping;
	Reset();
}

//---------------------------.
// 更新.
//---------------------------.
void CSpringAnimation::Update( const float DeltaTime )
{
	if ( DeltaTime <= 0.0f ) return;

	// 減衰係数( 臨界減衰を基準に減衰率を掛ける ).
	const float DampingCoefficient = 2.0f * m_Damping * std::sqrt( m_Stiffness );

	// 大きなフレーム落ちでも発散しないように時間を分割して計算する.
	float RemainingTime = DeltaTime;
	while ( RemainingTime > 0.0f ) {
		const float Step = min( RemainingTime, MAX_STEP_TIME );
		RemainingTime -= Step;

		// バネの加速度から速度・拡縮値を更新する( X軸 ).
		const float AccelX = -m_Stiffness * ( m_Scale.x - m_Target.x ) - DampingCoefficient * m_Velocity.x;
		m_Velocity.x	+= AccelX * Step;
		m_Scale.x		+= m_Velocity.x * Step;

		// バネの加速度から速度・拡縮値を更新する( Y軸 ).
		const float AccelY = -m_Stiffness * ( m_Scale.y - m_Target.y ) - DampingCoefficient * m_Velocity.y;
		m_Velocity.y	+= AccelY * Step;
		m_Scale.y		+= m_Velocity.y * Step;
	}

	// 十分に落ち着いたら目標値へ固定する.
	if ( std::abs( m_Scale.x - m_Target.x ) < SETTLE_THRESHOLD && std::abs( m_Velocity.x ) < SETTLE_THRESHOLD ) {
		m_Scale.x		= m_Target.x;
		m_Velocity.x	= 0.0f;
	}
	if ( std::abs( m_Scale.y - m_Target.y ) < SETTLE_THRESHOLD && std::abs( m_Velocity.y ) < SETTLE_THRESHOLD ) {
		m_Scale.y		= m_Target.y;
		m_Velocity.y	= 0.0f;
	}
}

//---------------------------.
// 瞬間的に拡縮させ, 弾性で等倍へ戻す.
//---------------------------.
void CSpringAnimation::Punch( const float ScaleX, const float ScaleY )
{
	// 追従先は等倍に戻し, 現在値だけを変化させて弾ませる.
	m_Target	= D3DXVECTOR2( 1.0f, 1.0f );
	m_Scale		= D3DXVECTOR2( ScaleX, ScaleY );
	m_Velocity	= D3DXVECTOR2( 0.0f, 0.0f );
}

//---------------------------.
// 追従先の拡縮値を設定する.
//---------------------------.
void CSpringAnimation::SetTarget( const float ScaleX, const float ScaleY )
{
	m_Target = D3DXVECTOR2( ScaleX, ScaleY );
}

//---------------------------.
// 即座に等倍へ戻す.
//---------------------------.
void CSpringAnimation::Reset()
{
	m_Scale		= D3DXVECTOR2( 1.0f, 1.0f );
	m_Velocity	= D3DXVECTOR2( 0.0f, 0.0f );
	m_Target	= D3DXVECTOR2( 1.0f, 1.0f );
}

#endif
