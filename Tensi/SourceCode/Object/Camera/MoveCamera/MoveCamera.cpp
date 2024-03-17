#include "MoveCamera.h"
#include "..\..\..\Utility\Math\Math.h"

CMoveCamera::CMoveCamera()
	: m_CameraPosition	( 0.0f, 1.0f, 0.0f )
	, m_MovePoint		()
	, m_EndTime			( INIT_FLOAT )
	, m_NowPoint		( INIT_FLOAT )
	, m_IsMove			( false )
{
}

CMoveCamera::~CMoveCamera()
{
}

//---------------------------.
// 更新.
//---------------------------.
void CMoveCamera::Update( const float& DeltaTime )
{
	// 注視点の更新.
	if ( m_pTarget == nullptr ) m_LookPosition = m_CameraPosition;
	else						m_LookPosition = m_pTarget->GetPosition();

	// 視点の更新.
	m_Transform.Position.x = m_CameraPosition.x + ( sinf( m_Transform.Rotation.y ) );
	m_Transform.Position.y = m_CameraPosition.y + ( sinf( m_Transform.Rotation.x ) );
	m_Transform.Position.z = m_CameraPosition.z + ( cosf( m_Transform.Rotation.y ) );

	// 移動中か？.
	if ( m_IsMove == false ) return;

	// ベジェ曲線を使用してカメラを移動させる.
	m_CameraPosition = Math::Evaluate( m_MovePoint, m_NowPoint );
	m_NowPoint += ( 1.0f / m_EndTime ) * DeltaTime;

	// 移動終了.
	if ( m_NowPoint >= 1.0f ) {
		m_CameraPosition	= m_MovePoint.back();
		m_IsMove			= false;
	}
}

//---------------------------.
// 移動の開始.
//	移動中の場合処理は行わない.
//---------------------------.
void CMoveCamera::MoveStart( const std::vector<D3DXPOSITION3>& Points, const float EndTime )
{
	if ( m_IsMove ) return;

	// 移動を開始させる.
	m_MovePoint = Points;
	m_EndTime	= EndTime;
	m_NowPoint	= 0.0f;
	m_IsMove	= true;
}
