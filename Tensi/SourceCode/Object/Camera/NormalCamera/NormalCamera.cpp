#include "NormalCamera.h"

CNormalCamera::CNormalCamera()
{
}

CNormalCamera::~CNormalCamera()
{
}

//---------------------------.
// 更新.
//	ターゲットが設定されていない場合処理は行わない.
//---------------------------.
void CNormalCamera::Update( const float& DeltaTime )
{
	if ( m_pTarget == nullptr ) return;

	// 注視点の更新.
	m_LookPosition = m_pTarget->GetPosition();
}
