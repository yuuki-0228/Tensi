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
// �X�V.
//---------------------------.
void CMoveCamera::Update( const float& DeltaTime )
{
	// �����_�̍X�V.
	if ( m_pTarget == nullptr ) m_LookPosition = m_CameraPosition;
	else						m_LookPosition = m_pTarget->GetPosition();

	// ���_�̍X�V.
	m_Transform.Position.x = m_CameraPosition.x + ( sinf( m_Transform.Rotation.y ) );
	m_Transform.Position.y = m_CameraPosition.y + ( sinf( m_Transform.Rotation.x ) );
	m_Transform.Position.z = m_CameraPosition.z + ( cosf( m_Transform.Rotation.y ) );

	// �ړ������H.
	if ( m_IsMove == false ) return;

	// �x�W�F�Ȑ����g�p���ăJ�������ړ�������.
	m_CameraPosition = Math::Evaluate( m_MovePoint, m_NowPoint );
	m_NowPoint += ( 1.0f / m_EndTime ) * DeltaTime;

	// �ړ��I��.
	if ( m_NowPoint >= 1.0f ) {
		m_CameraPosition	= m_MovePoint.back();
		m_IsMove			= false;
	}
}

//---------------------------.
// �ړ��̊J�n.
//	�ړ����̏ꍇ�����͍s��Ȃ�.
//---------------------------.
void CMoveCamera::MoveStart( const std::vector<D3DXPOSITION3>& Points, const float EndTime )
{
	if ( m_IsMove ) return;

	// �ړ����J�n������.
	m_MovePoint = Points;
	m_EndTime	= EndTime;
	m_NowPoint	= 0.0f;
	m_IsMove	= true;
}
