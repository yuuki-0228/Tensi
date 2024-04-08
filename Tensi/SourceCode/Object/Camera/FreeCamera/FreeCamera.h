#pragma once
#include "..\CameraBase.h"

/************************************************
*	���R�ɓ�������J�����N���X.
**/
class CFreeCamera final
	: public CCameraBase
{
public:
	CFreeCamera();
	~CFreeCamera();

	// �X�V.
	virtual void Update( const float& DeltaTime ) override;

private:
	D3DXPOSITION3	m_CameraPosition;	// �J�����̍��W.
	D3DXVECTOR3		m_MoveVec;			// �ړ��x�N�g��.
	D3DXVECTOR3		m_SideMoveVec;		// ���ړ��x�N�g��.
};
