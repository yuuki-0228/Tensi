#include "Actor.h"
#include "..\..\..\Common\DirectX\DirectX11.h"
#include "..\..\Camera\CameraManager\CameraManager.h"
#include <cmath>

CActor::CActor()
	: m_pCollisions		()
	, m_IsDisp			( true )
	, m_Distance		( 1.0f )
	, m_Wnd_w			( INIT_FLOAT )
	, m_Wnd_h			( INIT_FLOAT )
	, m_WndRect			()
{
	m_Wnd_w = DirectX11::GetWndWidth();
	m_Wnd_h = DirectX11::GetWndHeight();

	m_WndRect.left		= static_cast<LONG>( 0		 );
	m_WndRect.right		= static_cast<LONG>( m_Wnd_w );
	m_WndRect.top		= static_cast<LONG>( 0		 );
	m_WndRect.bottom	= static_cast<LONG>( m_Wnd_h );
}

CActor::~CActor()
{
}

//---------------------------.
// スクリーン座標を取得.
//---------------------------.
D3DXVECTOR3 CActor::GetScreenPos()
{
	// ビューポート行列.
	D3DXMATRIX mViewport;
	D3DXMatrixIdentity( &mViewport );
	mViewport._11 = FWND_W *  0.5f;
	mViewport._22 = FWND_H * -0.5f;
	mViewport._41 = FWND_W *  0.5f;
	mViewport._42 = FWND_H *  0.5f;

	// スクリーン座標.
	D3DXMATRIX	m	= CameraManager::GetViewProjMatrix() * mViewport;
	D3DXVECTOR3 Out = { 0.0f, 0.0f, 0.0f };
	D3DXVec3TransformCoord( &Out, &m_Transform.Position, &m );
	if ( !std::isfinite(  Out.x ) ) Out.x = 0.0f;
	if ( !std::isfinite(  Out.y ) ) Out.y = 0.0f;
	if ( !std::isfinite(  Out.z ) ) Out.z = 0.0f;

	return Out;
}

//---------------------------.
// 画面内外にいるか.
//---------------------------.
bool CActor::IsScreenOut( const float& Size )
{
	// スクリーン座標を取得.
	D3DXVECTOR3	ScreenPos	= Math::Round_n( GetScreenPos(), 5 );

	// 画面内にいるか.
	if ( -Size * 2 < ScreenPos.x && ScreenPos.x < FWND_W + Size * 2 &&
		 -Size * 2 < ScreenPos.y && ScreenPos.y < FWND_H + Size * 2 &&
		 ScreenPos.z < 1.15f ) return false;

	return true;	// 画面外.
}

//---------------------------.
// 距離によるフェードの更新.
//---------------------------.
void CActor::DistanceFadeUpdate( const float& StartDistance, const float& AlphaDistance )
{
	if ( m_IsDisp == false ) return;

	const D3DXVECTOR3&	DistanceVector	= m_Transform.Position - CameraManager::GetPosition();
	const float			Distance		= D3DXVec3Length( &DistanceVector );

	// 現在の距離がアルファフェードを入れる距離以上か.
	m_Distance = Color::ALPHA_MAX;
	if ( Distance <= StartDistance ) return;
	const float OverDistance = Distance - StartDistance;
	
	// 距離を変更させる.
	m_Distance = Color::ALPHA_MAX - OverDistance / AlphaDistance / 10.0f;

	// 非表示にする.
	if ( m_Distance <= Color::ALPHA_MIN ) {
		m_Distance	= Color::ALPHA_MIN;
		m_IsDisp	= false;
	}
}
