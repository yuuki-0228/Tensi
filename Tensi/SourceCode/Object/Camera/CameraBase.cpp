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
// �r���[�E�v���W�F�N�V�����̍X�V.
//---------------------------.
void CCameraBase::ViewProjUpdate()
{
	// �r���[(�J����)�ϊ�.
	const D3DXPOSITION3& Pos	= m_Transform.Position	+ m_ShakingAddPosition;
	const D3DXPOSITION3& Look	= m_LookPosition		+ m_ShakingAddPosition;
	D3DXMatrixLookAtLH(
		&m_ViewMatrix,							// (out)�r���[�v�Z����.
		&Pos,									// �J�������W.
		&Look,									// �J�����������W.
		&m_ViewVector );						// �J�����x�N�g��.

	// �v���W�F�N�V����(�ˉe)�ϊ�.
	D3DXMatrixPerspectiveFovLH(
		&m_ProjMatrix,							// (out)�v���W�F�N�V�����v�Z����.
		m_ViewAngle - Math::ToRadian( m_Zoom ),	// y�����̎���(rad)���l��傫�������王�삪�����Ȃ�.
		m_Aspect,								// �A�X�y�N�g��(��������).
		m_MinZView,								// �߂��r���[���ʂ�z�l.
		m_MaxZView );							// �����r���[���ʂ�z�l.
}
