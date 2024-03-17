#pragma once
#include "..\CameraBase.h"

/************************************************
*	�ړ�����J�����N���X.
*		���c�F�P.
**/
class CMoveCamera final
	: public CCameraBase
{
public:
	CMoveCamera();
	~CMoveCamera();

	// �X�V.
	virtual void Update( const float& DeltaTime ) override;

	// �ړ��̊J�n.
	void MoveStart( const std::vector<D3DXPOSITION3>& Points, const float EndTime );

private:
	D3DXVECTOR3					m_CameraPosition;	// �J�����̍��W.
	std::vector<D3DXPOSITION3>	m_MovePoint;		// �ړ��n�_.
	float						m_EndTime;			// ���b�ňړ����I�������邩.
	float						m_NowPoint;			// ���݂̒n�_(0.0f ~ 1.0f).
	bool						m_IsMove;			// �ړ�����.
};
