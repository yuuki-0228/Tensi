#include "NormalCamera.h"

CNormalCamera::CNormalCamera()
{
}

CNormalCamera::~CNormalCamera()
{
}

//---------------------------.
// �X�V.
//	�^�[�Q�b�g���ݒ肳��Ă��Ȃ��ꍇ�����͍s��Ȃ�.
//---------------------------.
void CNormalCamera::Update( const float& DeltaTime )
{
	if ( m_pTarget == nullptr ) return;

	// �����_�̍X�V.
	m_LookPosition = m_pTarget->GetPosition();
}
