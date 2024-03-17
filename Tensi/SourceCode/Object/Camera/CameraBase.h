#pragma once
#include "..\Object.h"
#include "..\..\Global.h"

// �J�����̏�ԍ\����.
struct stCameraState {
	CObject*		pTarget;		// �^�[�Q�b�g.
	D3DXPOSITION3	Position;		// ���_.
	D3DXPOSITION3	LookPosition;	// �����_.
	D3DXROTATION3	Rotation;		// ��]�l.
	D3DXVECTOR3		ViewVector;		// �r���[�x�N�g��.
	float			Zoom;			// �Y�[��.

	stCameraState()
		: stCameraState(
			nullptr,
			{ 0.0f, 0.0f, 0.0f },
			{ 0.0f, 0.0f, 0.0f },
			{ 0.0f, 0.0f, 0.0f },
			{ 0.0f, 1.0f, 0.0f },
			0.0f )
	{}
	stCameraState( CObject* pObj, D3DXPOSITION3 Pos, D3DXPOSITION3 Look, D3DXROTATION3 Rot, D3DXVECTOR3 Vec, float zoom )
		: pTarget		( pObj )
		, Position		( Pos )
		, LookPosition	( Look )
		, Rotation		( Rot )
		, ViewVector	( Vec )
		, Zoom			( zoom )
	{}

} typedef SCameraState;

/************************************************
*	�J�����x�[�X�N���X.
*		���c�F�P.
**/
class CCameraBase
	: public CObject
{
public:
	CCameraBase();
	virtual ~CCameraBase();

	// �X�V.
	virtual void Update( const float& DeltaTime ) = 0;

	// �r���[�E�v���W�F�N�V�����̍X�V.
	virtual void ViewProjUpdate() final;

	// �������W�̎擾.
	inline D3DXVECTOR3 GetLookPosition() const { return m_LookPosition; }
	// �����x�N�g���̎擾.
	inline D3DXVECTOR3 GetLookVector() const { return m_Transform.Position - m_LookPosition; }

	// �r���[�s��擾.
	inline D3DXMATRIX GetViewMatrix() const { return m_ViewMatrix; }
	// �v���W�F�N�V�����s��擾.
	inline D3DXMATRIX GetProjMatrix() const { return m_ProjMatrix; }
	// �r���[�E�v���W�F�N�V�����̍����s��擾.
	inline D3DXMATRIX GetViewProjMatrix() const { return m_ViewMatrix * m_ProjMatrix; }
	// �J�����̏�Ԃ��擾.
	inline SCameraState GetCameraState() const {
		return SCameraState( m_pTarget,
			m_Transform.Position,
			m_LookPosition,
			m_Transform.Rotation,
			m_ViewVector,
			m_Zoom ); 
	}

	// �^�[�Q�b�g�̎擾.
	inline CObject* GetTarget() { return m_pTarget; }

	// �ړ����~�����邩.
	inline void SetIsMoveStop( const bool Flag ) { m_IsMoveStop = Flag; }

	// �������W�̐ݒ�.
	inline void SetLookPosition( const D3DXPOSITION3& p ) { m_LookPosition = p; }
	// �^�[�Q�b�g�̐ݒ�.
	inline void SetTarget( CObject* pTarget ) { m_pTarget = pTarget; }
	inline void ResetTarget() { m_pTarget = nullptr; }
	// �J�����̏�Ԃ�ݒ�.
	inline void SetCameraState( const SCameraState& State ) {
		m_pTarget				= State.pTarget;
		m_Transform.Position	= State.Position;
		m_LookPosition			= State.LookPosition;
		m_Transform.Rotation	= State.Rotation;
		m_ViewVector			= State.ViewVector;
		m_Zoom					= State.Zoom;
	}
	// �h��̐ݒ�.
	inline void SetShaking( const D3DXPOSITION3& Shaking ) { m_ShakingAddPosition = Shaking; }

protected:
	CObject*		m_pTarget;				// �^�[�Q�b�g.
	D3DXPOSITION3	m_LookPosition;			// �����_.
	D3DXVECTOR3		m_ViewVector;			// �r���[�x�N�g��.
	float			m_Zoom;					// �Y�[��.
	bool			m_IsMoveStop;			// �ړ��̒�~.

private:
	D3DXMATRIX		m_ViewMatrix;			// �r���[�s��.
	D3DXMATRIX		m_ProjMatrix;			// �v���W�F�N�V�����s��.
	D3DXPOSITION3	m_ShakingAddPosition;	// �h��ɂ��ǉ��̍��W.
	float			m_ViewAngle;			// ����p.
	float			m_Aspect;				// �A�X�y�N�g��(��������).
	float			m_MaxZView;				// �����r���[���ʂ�z�l.
	float			m_MinZView;				// �߂��r���[���ʂ�z�l.
};
