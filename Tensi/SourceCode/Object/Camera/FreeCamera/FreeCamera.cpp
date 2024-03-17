#include "FreeCamera.h"
#include "..\..\..\Utility\Math\Math.h"
#include "..\..\..\Utility\Input\Input.h"
#include "..\..\..\Utility\Const\Const.h"

CFreeCamera::CFreeCamera()
	: m_CameraPosition	( 0.0f, 1.0f, 0.0f )
	, m_MoveVec			( INIT_FLOAT3 )
	, m_SideMoveVec		( INIT_FLOAT3 )
{
}

CFreeCamera::~CFreeCamera()
{
}

//---------------------------.
// �X�V.
//---------------------------.
void CFreeCamera::Update( const float& DeltaTime )
{
	// �J�����̑���.
//	if ( KeyInput::IsKeyPress( VK_CONTROL ) ) {
		if ( Input::IsKeyPress( "FreeCameraMoveF" ) ) m_CameraPosition			-= m_MoveVec	 * Const::FreeCamera.MOVE_SPEED;
		if ( Input::IsKeyPress( "FreeCameraMoveB" ) ) m_CameraPosition			+= m_MoveVec	 * Const::FreeCamera.MOVE_SPEED;
		if ( Input::IsKeyPress( "FreeCameraMoveL" ) ) m_CameraPosition			+= m_SideMoveVec * Const::FreeCamera.MOVE_SPEED;
		if ( Input::IsKeyPress( "FreeCameraMoveR" ) ) m_CameraPosition			-= m_SideMoveVec * Const::FreeCamera.MOVE_SPEED;
		if ( Input::IsKeyPress( "FreeCameraMoveU" ) ) m_CameraPosition.y		+= Const::FreeCamera.MOVE_SPEED;
		if ( Input::IsKeyPress( "FreeCameraMoveD" ) ) m_CameraPosition.y		-= Const::FreeCamera.MOVE_SPEED;
		if ( Input::IsKeyPress( "FreeCameraRotXU" ) ) m_Transform.Rotation.x	-= Const::FreeCamera.ROTATION_SPEED;
		if ( Input::IsKeyPress( "FreeCameraRotXD" ) ) m_Transform.Rotation.x	+= Const::FreeCamera.ROTATION_SPEED;
		if ( Input::IsKeyPress( "FreeCameraRotYL" ) ) m_Transform.Rotation.y	-= Const::FreeCamera.ROTATION_SPEED;
		if ( Input::IsKeyPress( "FreeCameraRotYR" ) ) m_Transform.Rotation.y	+= Const::FreeCamera.ROTATION_SPEED;
//	}

	// �ړ��x�N�g���̍X�V.
	m_MoveVec.x		= sinf( m_Transform.Rotation.y );
	m_MoveVec.z		= cosf( m_Transform.Rotation.y );
	m_SideMoveVec.x = m_MoveVec.z;
	m_SideMoveVec.z = -m_MoveVec.x;

	// �����_�̍X�V.
	m_LookPosition = m_CameraPosition;

	// ���_�̍X�V.
	m_Transform.Position.x = m_LookPosition.x + ( sinf( m_Transform.Rotation.y ) );
	m_Transform.Position.y = m_LookPosition.y + ( sinf( m_Transform.Rotation.x ) );
	m_Transform.Position.z = m_LookPosition.z + ( cosf( m_Transform.Rotation.y ) );
}
