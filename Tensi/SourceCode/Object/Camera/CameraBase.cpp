#include "CameraBase.h"
#include "..\..\Utility\Math\Math.h"

CCameraBase::CCameraBase()
	: m_pTarget				( nullptr )
	, m_LookPosition		( INIT_FLOAT3 )
	, m_ViewVector			( Math::Y_VECTOR )
	, m_Zoom				( INIT_FLOAT )
	, m_IsMoveStop			( false )
	, m_ViewMatrix			()
	, m_ProjMatrix			()
	, m_ShakingAddPosition	( INIT_FLOAT3 )
	, m_ViewAngle			( Math::PI / 4.0f )
	, m_Aspect				( FWND_W / FWND_H )
	, m_MaxZView			( 2500.0f )
	, m_MinZView			( 0.1f )
{
}

CCameraBase::~CCameraBase()
{
}

//---------------------------.
// ビュー・プロジェクションの更新.
//---------------------------.
void CCameraBase::ViewProjUpdate()
{
	// ビュー(カメラ)変換.
	const D3DXPOSITION3& Pos	= m_Transform.Position	+ m_ShakingAddPosition;
	const D3DXPOSITION3& Look	= m_LookPosition		+ m_ShakingAddPosition;
	D3DXMatrixLookAtLH(
		&m_ViewMatrix,							// (out)ビュー計算結果.
		&Pos,									// カメラ座標.
		&Look,									// カメラ注視座標.
		&m_ViewVector );						// カメラベクトル.

	// プロジェクション(射影)変換.
	D3DXMatrixPerspectiveFovLH(
		&m_ProjMatrix,							// (out)プロジェクション計算結果.
		m_ViewAngle - Math::ToRadian( m_Zoom ),	// y方向の視野(rad)数値を大きくしたら視野が狭くなる.
		m_Aspect,								// アスペクト比(幅÷高さ).
		m_MinZView,								// 近いビュー平面のz値.
		m_MaxZView );							// 遠いビュー平面のz値.
}
