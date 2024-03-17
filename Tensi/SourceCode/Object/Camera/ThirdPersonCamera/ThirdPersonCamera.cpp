#include "ThirdPersonCamera.h"

CThirdPersonCamera::CThirdPersonCamera()
{
}

CThirdPersonCamera::~CThirdPersonCamera()
{
}

//---------------------------.
// �X�V.
//	�^�[�Q�b�g���ݒ肳���Ă��Ȃ��ꍇ�����͍s��Ȃ�.
//---------------------------.
void CThirdPersonCamera::Update( const float& DeltaTime )
{
	if ( m_pTarget == nullptr ) return;

	const D3DXPOSITION3& TargetPos	= m_pTarget->GetPosition();
	const D3DXROTATION3& TargetRot	= m_pTarget->GetRotation();
	m_Transform.Rotation.y			= TargetRot.y;

	// Y����]�s���p��.
	D3DXMATRIX mRotationY;
	// Y����]�s����쐬.
	D3DXMatrixRotationY( &mRotationY, TargetRot.y );

	// ���x�N�g����p��(Z��).
	D3DXVECTOR3 vecAxisZ( 0.0f, 0.0f, 1.0f );
	// Z���x�N�g�����̂��̂���]��Ԃɂ��ω�������.
	D3DXVec3TransformCoord(
		&vecAxisZ,	//(out).
		&vecAxisZ, &mRotationY );

	// �J�����̈ʒu,�@�����ʒu��Ώۂɂ��낦��.
	m_Transform.Position	= TargetPos;
	m_LookPosition			= TargetPos;

	// �J�����̈ʒu, �����ʒu��Z���x�N�g����p���Ē���.
	m_Transform.Position	-= vecAxisZ * 4.0f;	// �Ώۂ̔w����.
	m_LookPosition			+= vecAxisZ * 3.0f;	// �Ώۂ�����Ō�������.

	// �J�����̈ʒu, �����ʒu�̍��������ꂼ�������.
	m_Transform.Position.y	+= 2.0f;
	m_LookPosition.y		+= 0.5f;
}
