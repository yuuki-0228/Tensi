#include "ThirdPersonCamera.h"

CThirdPersonCamera::CThirdPersonCamera()
{
}

CThirdPersonCamera::~CThirdPersonCamera()
{
}

//---------------------------.
// 更新.
//	ターゲットが設定させていない場合処理は行わない.
//---------------------------.
void CThirdPersonCamera::Update( const float& DeltaTime )
{
	if ( m_pTarget == nullptr ) return;

	const D3DXPOSITION3& TargetPos	= m_pTarget->GetPosition();
	const D3DXROTATION3& TargetRot	= m_pTarget->GetRotation();
	m_Transform.Rotation.y			= TargetRot.y;

	// Y軸回転行列を用意.
	D3DXMATRIX mRotationY;
	// Y軸回転行列を作成.
	D3DXMatrixRotationY( &mRotationY, TargetRot.y );

	// 軸ベクトルを用意(Z軸).
	D3DXVECTOR3 vecAxisZ( 0.0f, 0.0f, 1.0f );
	// Z軸ベクトルそのものを回転状態により変化させる.
	D3DXVec3TransformCoord(
		&vecAxisZ,	//(out).
		&vecAxisZ, &mRotationY );

	// カメラの位置,　注視位置を対象にそろえる.
	m_Transform.Position	= TargetPos;
	m_LookPosition			= TargetPos;

	// カメラの位置, 注視位置をZ軸ベクトルを用いて調整.
	m_Transform.Position	-= vecAxisZ * 4.0f;	// 対象の背中側.
	m_LookPosition			+= vecAxisZ * 3.0f;	// 対象を挟んで向こう側.

	// カメラの位置, 注視位置の高さをそれぞれ微調整.
	m_Transform.Position.y	+= 2.0f;
	m_LookPosition.y		+= 0.5f;
}
